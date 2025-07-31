#include "common.h"

void clip_line_plane(line_t *line, int plane);
void calc_clip_line(const vec3_t *vt_0, const vec3_t *vt_1, fixed dist_1, fixed dist_2, vec3_t *i_vt);

#if 1 // !ENABLE_ASM
  RAM_CODE void clip_poly_plane(g_poly_t *poly, int plane) {
    if (!poly->num_vertices) return;
    
    poly_tx_t poly_cl;
    int num_vt = poly->num_vertices - 1;
    
    // calculate the distance to the plane
    
    fixed dist_1 = 0; // distance to border, positive is inside
    
    switch (plane) {
      case 0:  // near
        dist_1 = poly->vertices[num_vt].z - vp.z_near;
        break;
      case 1:  // far
        dist_1 = vp.z_far - poly->vertices[num_vt].z;
        break;
      case 2:  // left
        dist_1 = poly->vertices[num_vt].x + fp_mul(poly->vertices[num_vt].z, vp.screen_side_x_dt);
        break;
      case 3:  // right
        dist_1 = fp_mul(poly->vertices[num_vt].z, vp.screen_side_x_dt) - poly->vertices[num_vt].x;
        break;
      case 4:  // top
        dist_1 = poly->vertices[num_vt].y + fp_mul(poly->vertices[num_vt].z, vp.screen_side_y_dt);
        break;
      case 5:  // bottom
        dist_1 = fp_mul(poly->vertices[num_vt].z, vp.screen_side_y_dt) - poly->vertices[num_vt].y;
    }
    
    poly_cl.num_vertices = 0;
    
    for (int i = 0; i < poly->num_vertices; i++) {
      fixed dist_2 = 0; // distance to border, positive is inside
      
      switch (plane) {
        case 0:  // near
          dist_2 = poly->vertices[i].z - vp.z_near;
          break;
        case 1:  // far
          dist_2 = vp.z_far - poly->vertices[i].z;
          break;
        case 2:  // left
          dist_2 = poly->vertices[i].x + fp_mul(poly->vertices[i].z, vp.screen_side_x_dt);
          break;
        case 3:  // right
          dist_2 = fp_mul(poly->vertices[i].z, vp.screen_side_x_dt) - poly->vertices[i].x;
          break;
        case 4:  // top
          dist_2 = poly->vertices[i].y + fp_mul(poly->vertices[i].z, vp.screen_side_y_dt);
          break; // bottom
        case 5:
          dist_2 = fp_mul(poly->vertices[i].z, vp.screen_side_y_dt) - poly->vertices[i].y;
      }
      
      if (dist_2 >= 0) { // inside
        if (dist_1 < 0) { // outside
          clip_poly_line_plane(&poly->vertices[i], &poly->vertices[num_vt], dist_2, dist_1, &poly_cl.vertices[poly_cl.num_vertices], poly->flags.has_texture);
          poly_cl.num_vertices++;
        }
        
        poly_cl.vertices[poly_cl.num_vertices].x = poly->vertices[i].x;
        poly_cl.vertices[poly_cl.num_vertices].y = poly->vertices[i].y;
        poly_cl.vertices[poly_cl.num_vertices].z = poly->vertices[i].z;
        
        if (poly->flags.has_texture) {
          poly_cl.vertices[poly_cl.num_vertices].u = poly->vertices[i].u;
          poly_cl.vertices[poly_cl.num_vertices].v = poly->vertices[i].v;
        }
        
        poly_cl.num_vertices++;
      } else
      if (dist_1 >= 0) { // inside
        clip_poly_line_plane(&poly->vertices[num_vt], &poly->vertices[i], dist_1, dist_2, &poly_cl.vertices[poly_cl.num_vertices], poly->flags.has_texture);
        poly_cl.num_vertices++;
      }
      
      num_vt = i;
      dist_1 = dist_2;
    }
    
    poly->num_vertices = poly_cl.num_vertices;
    //r_poly_vt = poly_cl;
    
    if (!poly->flags.has_texture) {
      for (int i = 0; i < poly->num_vertices; i++) {
        poly->vertices[i].x = poly_cl.vertices[i].x;
        poly->vertices[i].y = poly_cl.vertices[i].y;
        poly->vertices[i].z = poly_cl.vertices[i].z;
      }
    } else {
      for (int i = 0; i < poly->num_vertices; i++) {
        poly->vertices[i].x = poly_cl.vertices[i].x;
        poly->vertices[i].y = poly_cl.vertices[i].y;
        poly->vertices[i].z = poly_cl.vertices[i].z;
        poly->vertices[i].u = poly_cl.vertices[i].u;
        poly->vertices[i].v = poly_cl.vertices[i].v;
      }
    }
  }

  RAM_CODE void clip_poly_line_plane(const vec5_t *vt_0, const vec5_t *vt_1, fixed dist_1, fixed dist_2, vec5_t *i_vt, int has_texture) {
    fixed_u length = dist_1 - dist_2;
    fixed s;
    
    if (length != 0) {
      // unsigned fixed point division
      #if !DIV_LUT_ENABLE
        s = fp_div(dist_1, length);
      #else
        s = div_lut[length >> 8] << 8; // .16 result
        s = fp_mul(dist_1, s);
      #endif
    } else {
      s = 1 << FP;
    }
    
    // obtain the intersecting point
    
    i_vt->x = vt_0->x + fp_mul((vt_1->x - vt_0->x), s);
    i_vt->y = vt_0->y + fp_mul((vt_1->y - vt_0->y), s);
    i_vt->z = vt_0->z + fp_mul((vt_1->z - vt_0->z), s);
    
    if (has_texture) {
      i_vt->u = vt_0->u + fp_mul32((vt_1->u - vt_0->u), s);
      i_vt->v = vt_0->v + fp_mul32((vt_1->v - vt_0->v), s);
    }
  }
#endif

int clip_line(line_t *line) {
  // set the bitflags
  
  fixed w = fp_mul(line->p0.z, vp.screen_side_x_dt);
  fixed h = fp_mul(line->p0.z, vp.screen_side_y_dt);
  
  int plane_sides_vt_0 = 0;
  
  plane_sides_vt_0 |= (line->p0.z < vp.z_near);
  // plane_sides_vt_0 |= (line->p0.z > vp.z_far) << 1;
  plane_sides_vt_0 |= (line->p0.x < -w) << 2;
  plane_sides_vt_0 |= (line->p0.x > w) << 3;
  plane_sides_vt_0 |= (line->p0.y < -h) << 4;
  plane_sides_vt_0 |= (line->p0.y > h) << 5;
  
  w = fp_mul(line->p1.z, vp.screen_side_x_dt);
  h = fp_mul(line->p1.z, vp.screen_side_y_dt);
  
  int plane_sides_vt_1 = 0;
  
  plane_sides_vt_1 |= (line->p1.z < vp.z_near);
  // plane_sides_vt_1 |= (line->p1.z > vp.z_far) << 1;
  plane_sides_vt_1 |= (line->p1.x < -w) << 2;
  plane_sides_vt_1 |= (line->p1.x > w) << 3;
  plane_sides_vt_1 |= (line->p1.y < -h) << 4;
  plane_sides_vt_1 |= (line->p1.y > h) << 5;
  
  // both points are outside the same plane
  
  if (plane_sides_vt_0 & plane_sides_vt_1) return 1;
  
  // both points are inside
  
  if (!plane_sides_vt_0 && !plane_sides_vt_1) return 0;
  
  // if the same bit has different values then the points are on different sides of the plane
  
  if ((plane_sides_vt_0 & NEAR_PLANE) != (plane_sides_vt_1 & NEAR_PLANE)) {
    clip_line_plane(line, 0);
  }
  
  #if ENABLE_FAR_PLANE_CLIPPING
    if ((plane_sides_vt_0 & FAR_PLANE) != (plane_sides_vt_1 & FAR_PLANE)) {
      clip_line_plane(line, 1);
    }
  #endif
  
  if ((plane_sides_vt_0 & LEFT_PLANE) != (plane_sides_vt_1 & LEFT_PLANE)) {
    clip_line_plane(line, 2);
  }
  
  if ((plane_sides_vt_0 & RIGHT_PLANE) != (plane_sides_vt_1 & RIGHT_PLANE)) {
    clip_line_plane(line, 3);
  }
  
  if ((plane_sides_vt_0 & TOP_PLANE) != (plane_sides_vt_1 & TOP_PLANE)) {
    clip_line_plane(line, 4);
  }
  
  if ((plane_sides_vt_0 & BOTTOM_PLANE) != (plane_sides_vt_1 & BOTTOM_PLANE)) {
    clip_line_plane(line, 5);
  }
  
  return 0;
}

void clip_line_plane(line_t *line, int plane) {
  // calculate the distance to the plane
  
  fixed dist_1 = 0; // distance to border, positive is inside
  fixed dist_2 = 0;
  switch (plane) {
    case 0:  // near
      dist_1 = line->p0.z - vp.z_near;
      dist_2 = line->p1.z - vp.z_near;
      break;
    case 1:  // far
      dist_1 = vp.z_far - line->p0.z;
      dist_2 = vp.z_far - line->p1.z;
      break;
    case 2:  // left
      dist_1 = line->p0.x + fp_mul(line->p0.z, vp.screen_side_x_dt);
      dist_2 = line->p1.x + fp_mul(line->p1.z, vp.screen_side_x_dt);
      break;
    case 3:  // right
      dist_1 = fp_mul(line->p0.z, vp.screen_side_x_dt) - line->p0.x;
      dist_2 = fp_mul(line->p1.z, vp.screen_side_x_dt) - line->p1.x;
      break;
    case 4:  // top
      dist_1 = line->p0.y + fp_mul(line->p0.z, vp.screen_side_y_dt);
      dist_2 = line->p1.y + fp_mul(line->p1.z, vp.screen_side_y_dt);
      break;
    case 5:  // bottom
      dist_1 = fp_mul(line->p0.z, vp.screen_side_y_dt) - line->p0.y;
      dist_2 = fp_mul(line->p1.z, vp.screen_side_y_dt) - line->p1.y;
  }
  
  if ((dist_1 < 0 && dist_2 < 0) || (dist_1 >= 0 && dist_2 >= 0)) return;
  
  vec3_t i_vt;
  
  if (dist_1 < 0 && dist_2 >= 0) {
    calc_clip_line(&line->p1, &line->p0, dist_2, dist_1, &i_vt);
  } else
  if (dist_1 >= 0 && dist_2 < 0) {
    calc_clip_line(&line->p0, &line->p1, dist_1, dist_2, &i_vt);
  }
  
  // assign it to the line
  
  if (dist_1 < 0) { // outside
    line->p0.x = i_vt.x;
    line->p0.y = i_vt.y;
    line->p0.z = i_vt.z;
  } else { // inside
    line->p1.x = i_vt.x;
    line->p1.y = i_vt.y;
    line->p1.z = i_vt.z;
  }
}

void calc_clip_line(const vec3_t *vt_0, const vec3_t *vt_1, fixed dist_1, fixed dist_2, vec3_t *i_vt) {
  fixed_u length = dist_1 - dist_2;
  fixed s;
  
  if (length != 0) {
    // unsigned fixed point division
    #if !DIV_LUT_ENABLE
      s = fp_div(dist_1, length);
    #else
      s = div_lut[length >> 8] << 8; // .16 result
      s = fp_mul(dist_1, s);
    #endif
  } else {
    s = 1 << FP;
  }
  
  // obtain the intersecting point
  
  i_vt->x = vt_0->x + fp_mul((vt_1->x - vt_0->x), s);
  i_vt->y = vt_0->y + fp_mul((vt_1->y - vt_0->y), s);
  i_vt->z = vt_0->z + fp_mul((vt_1->z - vt_0->z), s);
}