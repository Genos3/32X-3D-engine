#include "common.h"

RAM_CODE void transform_model(fixed matrix[restrict], const model_t *model) {
  // transform the vertices
  
  for (int i = 0; i < model->num_vertices; i++) {
    tr_vertices[i].x = matrix[0] + dp3_fp8(model->vertices[i].x, matrix[1], model->vertices[i].y, matrix[2], model->vertices[i].z, matrix[3]);
    tr_vertices[i].y = matrix[4] + dp3_fp8(model->vertices[i].x, matrix[5], model->vertices[i].y, matrix[6], model->vertices[i].z, matrix[7]);
    tr_vertices[i].z = matrix[8] + dp3_fp8(model->vertices[i].x, matrix[9], model->vertices[i].y, matrix[10], model->vertices[i].z, matrix[11]);
  }
  
  #if DRAW_NORMALS
    if (model->flags.has_normals) {
      // transform the normals
      
      for (int i = 0; i < model->num_faces; i++) {
        tr_normals[i].x = dp3_fp8(model->normals[i].x, matrix[1], model->normals[i].y, matrix[2], model->normals[i].z, matrix[3]);
        tr_normals[i].y = dp3_fp8(model->normals[i].x, matrix[5], model->normals[i].y, matrix[6], model->normals[i].z, matrix[7]);
        tr_normals[i].z = dp3_fp8(model->normals[i].x, matrix[9], model->normals[i].y, matrix[10], model->normals[i].z, matrix[11]);
      }
    }
  #endif
}

RAM_CODE void transform_vx_model(fixed matrix[restrict], const vx_model_t *model) {
  // transform the vertices
  
  for (int i = 0; i < 8; i++) {
    tr_vertices[i].x = matrix[0] + dp3_fp8(model->vertices[i].x, matrix[1], model->vertices[i].y, matrix[2], model->vertices[i].z, matrix[3]);
    tr_vertices[i].y = matrix[4] + dp3_fp8(model->vertices[i].x, matrix[5], model->vertices[i].y, matrix[6], model->vertices[i].z, matrix[7]);
    tr_vertices[i].z = matrix[8] + dp3_fp8(model->vertices[i].x, matrix[9], model->vertices[i].y, matrix[10], model->vertices[i].z, matrix[11]);
  }
}

RAM_CODE void transform_model_vis(fixed matrix[restrict], int dl_vt_offset, int dl_mode, const model_t *model) {
  int num_vertices;
  if (!dl_mode) {
    num_vertices = model->num_vertices;
  } else {
    num_vertices = dl.num_vertices;
  }
  
  for (int i = 0; i < num_vertices; i++) {
    vec3_t p;
    const vec3_s16_t *vt;
    if (!dl_mode) {
      vt = &model->vertices[i];
    } else {
      int vt_id = dl.vt_list[i];
      vt = &model->vertices[vt_id];
    }
    
    // transform the vertices
    
    p.x = matrix[0] + dp3_fp8(vt->x, matrix[1], vt->y, matrix[2], vt->z, matrix[3]);
    p.y = matrix[4] + dp3_fp8(vt->x, matrix[5], vt->y, matrix[6], vt->z, matrix[7]);
    p.z = matrix[8] + dp3_fp8(vt->x, matrix[9], vt->y, matrix[10], vt->z, matrix[11]);
    
    tr_vertices[dl_vt_offset + i] = p;
    
    // compare the vertices against the frustum planes and store the results
    
    /* int plane_sides = 0;
    
    fixed w = fp_mul(p.z, vp.screen_side_x_dt);
    fixed h = fp_mul(p.z, vp.screen_side_y_dt);
    
    plane_sides |= (p.z < vp.z_near); // near
    plane_sides |= (p.z > vp.z_far) << 1; // far
    plane_sides |= (p.x < -w) << 2; // left
    plane_sides |= (p.x > w) << 3; // right
    plane_sides |= (p.y < -h) << 4; // top
    plane_sides |= (p.y > h) << 5; // bottom
    
    clipping_vt_list[dl_vt_offset + i] = plane_sides; */
  }
  
  #if DRAW_NORMALS
    if (model->flags.has_normals) {
      int num_faces;
      
      if (!dl_mode) {
        num_faces = model->num_faces;
      } else {
        num_faces = dl.num_faces;
      }
      
      for (int i = 0; i < num_faces; i++) {
        const vec3_s16_t *vt;
        
        if (!dl_mode) {
          vt = &model->normals[i];
        } else {
          vt = &model->normals[dl.pl_list[i]];
        }
        
        // transform the normals
        
        tr_normals[i].x = dp3_fp8(vt->x, matrix[1], vt->y, matrix[2], vt->z, matrix[3]);
        tr_normals[i].y = dp3_fp8(vt->x, matrix[5], vt->y, matrix[6], vt->z, matrix[7]);
        tr_normals[i].z = dp3_fp8(vt->x, matrix[9], vt->y, matrix[10], vt->z, matrix[11]);
      }
    }
  #endif
}

RAM_CODE void transform_vertices(fixed matrix[restrict], const vec3_s16_t *vt_in, vec3_t *vt_out, int num_elements) {
  for (int i = 0; i < num_elements; i++) {
    // transform the vertices
    vt_out[i].x = matrix[0] + dp3_fp8(vt_in[i].x, matrix[1], vt_in[i].y, matrix[2], vt_in[i].z, matrix[3]);
    vt_out[i].y = matrix[4] + dp3_fp8(vt_in[i].x, matrix[5], vt_in[i].y, matrix[6], vt_in[i].z, matrix[7]);
    vt_out[i].z = matrix[8] + dp3_fp8(vt_in[i].x, matrix[9], vt_in[i].y, matrix[10], vt_in[i].z, matrix[11]);
  }
}

void transform_vertices_pos(fixed matrix[restrict], const vec3_t *vt_in, vec3_t *vt_out, int num_elements) {
  for (int i = 0; i < num_elements; i++) {
    vt_out[i].x = vt_in[i].x + matrix[0];
    vt_out[i].y = vt_in[i].y + matrix[4];
    vt_out[i].z = vt_in[i].z + matrix[8];
  }
}

void transform_matrix(fixed matrix_0[restrict], fixed matrix_1[restrict]) {
  fixed dst[12];
  dst[0] = matrix_1[0] + dp3(matrix_0[0], matrix_1[1], matrix_0[4], matrix_1[2], matrix_0[8], matrix_1[3]);
  dst[4] = matrix_1[4] + dp3(matrix_0[0], matrix_1[5], matrix_0[4], matrix_1[6], matrix_0[8], matrix_1[7]);
  dst[8] = matrix_1[8] + dp3(matrix_0[0], matrix_1[9], matrix_0[4], matrix_1[10], matrix_0[8], matrix_1[11]);
  
  for (int i = 1; i <= 3; i++) {
    dst[i] = dp3(matrix_0[i], matrix_1[1], matrix_0[i + 4], matrix_1[2], matrix_0[i + 8], matrix_1[3]);
    dst[i + 4] = dp3(matrix_0[i], matrix_1[5], matrix_0[i + 4], matrix_1[6], matrix_0[i + 8], matrix_1[7]);
    dst[i + 8] = dp3(matrix_0[i], matrix_1[9], matrix_0[i + 4], matrix_1[10], matrix_0[i + 8], matrix_1[11]);
  }
  
  memcpy32(matrix_0, dst, 12);
  /* for (int i=0; i<12; i++){
    matrix_0[i]=dst[i];
  } */
}

// p0 and p1 has to be different

RAM_CODE void transform_vertex(vec3_t p0, vec3_t *p1, fixed matrix[restrict]) {
  p1->x = matrix[0] + dp3(p0.x, matrix[1], p0.y, matrix[2], p0.z, matrix[3]);
  p1->y = matrix[4] + dp3(p0.x, matrix[5], p0.y, matrix[6], p0.z, matrix[7]);
  p1->z = matrix[8] + dp3(p0.x, matrix[9], p0.y, matrix[10], p0.z, matrix[11]);
}

RAM_CODE void transform_vertex_pos(vec3_t p0, vec3_t *p1, fixed matrix[restrict]) {
  p1->x = p0.x + matrix[0];
  p1->y = p0.y + matrix[4];
  p1->z = p0.z + matrix[8];
}

RAM_CODE void transform_vertex_z(vec3_t p, fixed *z, fixed matrix[restrict]) {
  *z = matrix[8] + dp3(p.x, matrix[9], p.y, matrix[10], p.z, matrix[11]);
}

// axis x = 0, y = 1, z = 2

void transform_vertex_axis(fixed *ax, int axis, fixed matrix[restrict]) {
  *ax = fp_mul(*ax, matrix[1 + axis * 4 + axis]);
}

void transform_frustum(frustum_t *fr, tr_frustum_t *tr_fr, fixed matrix[restrict], u8 transform_normals) {
  for (int i = 0; i < 8; i++) {
    vec3_t *vt = &fr->vertices[i];
    tr_fr->vertices[i].x = matrix[0] + dp3(vt->x, matrix[1], vt->y, matrix[2], vt->z, matrix[3]);
    tr_fr->vertices[i].y = matrix[4] + dp3(vt->x, matrix[5], vt->y, matrix[6], vt->z, matrix[7]);
    tr_fr->vertices[i].z = matrix[8] + dp3(vt->x, matrix[9], vt->y, matrix[10], vt->z, matrix[11]);
  }
  
  if (transform_normals) {
    for (int i = 0; i < 6; i++) {
      vec3_t *vt = &fr->normals[i];
      tr_fr->normals[i].x = dp3(vt->x, matrix[1], vt->y, matrix[2], vt->z, matrix[3]);
      tr_fr->normals[i].y = dp3(vt->x, matrix[5], vt->y, matrix[6], vt->z, matrix[7]);
      tr_fr->normals[i].z = dp3(vt->x, matrix[9], vt->y, matrix[10], vt->z, matrix[11]);
    }
  }
}

void project_poly_vertices(g_poly_t *poly) {
  for (int i = 0; i < poly->num_vertices; i++) {
    vec5_t *vt = &poly->vertices[i];
    
    #if PERSPECTIVE_ENABLED // perspective division
      // unsigned fixed point division
      // set the shift between 8 and 12
      #if !DIV_LUT_ENABLE
        fixed_u rz = fp_div(vp.focal_length, vt->z);
      #else
        fixed_u rz = div_lut[(fixed_u)vt->z >> 10] << (FP - 10); // .16 result
        rz = vp.focal_length_i * rz; // .16
      #endif
      
      vt->x = fp_mul(vt->x, rz);
      vt->y = fp_mul(vt->y, rz);
    #endif
    
    // convert to screen coordinates
    
    #if 0 && DOUBLED_PIXELS
      vt->x >>= 1;
    #endif
    
    vt->x += SCREEN_WIDTH_FP >> 1;
    vt->y += SCREEN_HEIGHT_FP >> 1;
    
    // clamp the vertex to the screen borders
    
    vt->x = max_c(vt->x, 0);
    vt->y = max_c(vt->y, 0);
    vt->x = min_c(vt->x, SCREEN_WIDTH_FP);
    vt->y = min_c(vt->y, SCREEN_HEIGHT_FP);
  }
}

void project_vertex(vec3_t *vt) {
  #if PERSPECTIVE_ENABLED // perspective division
    // unsigned fixed point division
    // set the shift between 8 and 12
    #if !DIV_LUT_ENABLE
      fixed_u rz = fp_div(vp.focal_length, vt->z);
    #else
      fixed_u rz = div_lut[(fixed_u)vt->z >> 10] << (FP - 10); // .16 result
      rz = vp.focal_length_i * rz; // .16
    #endif
    vt->x = fp_mul(vt->x, rz);
    vt->y = fp_mul(vt->y, rz);
  #endif
  
  // convert to screen coordinates
  
  #if 0 && DOUBLED_PIXELS
    vt->x >>= 1;
  #endif
  
  vt->x += SCREEN_WIDTH_FP >> 1;
  vt->y += SCREEN_HEIGHT_FP >> 1;
  
  // clamp the vertex to the screen borders
  
  vt->x = max_c(vt->x, 0);
  vt->y = max_c(vt->y, 0);
  vt->x = min_c(vt->x, SCREEN_WIDTH_FP);
  vt->y = min_c(vt->y, SCREEN_HEIGHT_FP);
}