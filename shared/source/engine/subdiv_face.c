#include "common.h"

// subdivides the face along a series of axis-aligned planes on Z in camera space and parallel to the camera near plane

void clip_poly_subdiv_plane(g_poly_t *poly, fixed plane_dist, int face_id);

#if TX_PERSP_MODE == 2
  #define SUBDIV_LEVELS 4
  static fixed dist_subdiv_clip[] = {fix(0.5), fix(1.2), fix(2.2), fix(3.4)};
  
  RAM_CODE void subdiv_poly_clip(g_poly_t *poly, int face_id) {
    // obtain min and max z
    fixed min_z = poly->vertices[0].z;
    fixed max_z = poly->vertices[0].z;
    
    for (int i = 1; i < poly->num_vertices; i++) {
      if (poly->vertices[i].z < min_z) min_z = poly->vertices[i].z;
      if (poly->vertices[i].z > max_z) max_z = poly->vertices[i].z;
    }
    
    if (min_z < dist_subdiv_clip[SUBDIV_LEVELS - 1]) {
      cfg.face_enable_persp_tx = 1;
      
      for (int i = 0; i < SUBDIV_LEVELS; i++) {
        fixed plane_dist = dist_subdiv_clip[i];
        
        // clip if the plane is within the z range
        if (plane_dist > min_z && plane_dist < max_z) {
          clip_poly_subdiv_plane(poly, plane_dist, face_id);
        }
      }
    } else {
      cfg.face_enable_persp_tx = 0;
    }
    
    #if DRAW_TRIANGLES
      subdiv_poly_tris(poly, face_id);
    #else
      draw_face_tr(poly, face_id);
    #endif
  }
  
  RAM_CODE void clip_poly_subdiv_plane(g_poly_t *poly, fixed plane_dist, int face_id) {
    poly_tx_t poly_cl_0;
    poly_tx_t poly_cl_1;
    int num_vertices = poly->num_vertices - 1;
    
    // calculate the distance to the plane
    
    fixed dist_1 = poly->vertices[num_vertices].z - plane_dist; // distance to border, positive is inside
    
    poly_cl_0.num_vertices = 0;
    poly_cl_1.num_vertices = 0;
    
    for (int i = 0; i < poly->num_vertices; i++) {
      fixed dist_2 = 0;
      
      dist_2 = poly->vertices[i].z - plane_dist; // distance to border, positive is inside
      
      if (dist_2 >= 0) { // inside
        if (dist_1 < 0) { // outside
          clip_poly_line_plane(&poly->vertices[i], &poly->vertices[num_vertices], dist_2, dist_1, &poly_cl_0.vertices[poly_cl_0.num_vertices], poly->flags.has_texture);
          poly_cl_1.vertices[poly_cl_1.num_vertices] = poly_cl_0.vertices[poly_cl_0.num_vertices];
          poly_cl_0.num_vertices++;
          poly_cl_1.num_vertices++;
        }
        
        poly_cl_0.vertices[poly_cl_0.num_vertices] = poly->vertices[i];
        poly_cl_0.num_vertices++;
      } else {
        if (dist_1 >= 0) { // inside
          clip_poly_line_plane(&poly->vertices[num_vertices], &poly->vertices[i], dist_1, dist_2, &poly_cl_0.vertices[poly_cl_0.num_vertices], poly->flags.has_texture);
          poly_cl_1.vertices[poly_cl_1.num_vertices] = poly_cl_0.vertices[poly_cl_0.num_vertices];
          poly_cl_0.num_vertices++;
          poly_cl_1.num_vertices++;
        }
        
        poly_cl_1.vertices[poly_cl_1.num_vertices] = poly->vertices[i];
        poly_cl_1.num_vertices++;
      }
      
      num_vertices = i;
      dist_1 = dist_2;
    }
    
    poly->num_vertices = poly_cl_1.num_vertices;
    
    for (int i = 0; i < poly->num_vertices; i++) {
      poly->vertices[i] = poly_cl_1.vertices[i];
    }
    
    #if DRAW_TRIANGLES
      subdiv_poly_tris(poly, face_id);
    #else
      draw_face_tr(poly, face_id);
    #endif
    
    poly->num_vertices = poly_cl_0.num_vertices;
    
    for (int i = 0; i < poly->num_vertices; i++) {
      poly->vertices[i] = poly_cl_0.vertices[i];
    }
  }
#endif

// subdivides the face along an edge into quads based on a midpoint
// intended for recursion

#if !TX_PERSP_MODE && !SUBDIV_MODE
  RAM_CODE void subdiv_poly(g_poly_t *poly, int face_id) {
    vec5_t sub_poly_vt[8 + 7];
    
    // if (poly->num_vertices < 3) return;
    
    for (int i = 0; i < poly->num_vertices; i++) {
      sub_poly_vt[i] = poly->vertices[i];
    }
    
    // obtain the mid vertex for every edge and add it to the polygon vertex list
    
    int sp_num_vertices = poly->num_vertices;
    int n = 1;
    
    for (int i = 0; i < poly->num_vertices; i++) {
      sub_poly_vt[sp_num_vertices].x = (sub_poly_vt[n].x + sub_poly_vt[i].x) >> 1;
      sub_poly_vt[sp_num_vertices].y = (sub_poly_vt[n].y + sub_poly_vt[i].y) >> 1;
      sub_poly_vt[sp_num_vertices].z = (sub_poly_vt[n].z + sub_poly_vt[i].z) >> 1;
      sub_poly_vt[sp_num_vertices].u = (sub_poly_vt[n].u + sub_poly_vt[i].u) >> 1;
      sub_poly_vt[sp_num_vertices].v = (sub_poly_vt[n].v + sub_poly_vt[i].v) >> 1;
      
      sp_num_vertices++;
      n++;
      if (n == poly->num_vertices) {
        n = 0;
      }
    }
    
    // obtain the center vertex for the polygon and add it to the polygon vertex list
    
    vec5_t mid_vt;
    mid_vt = sub_poly_vt[0];
    
    for (int i = 1; i < poly->num_vertices; i++) {
      mid_vt.x += sub_poly_vt[i].x;
      mid_vt.y += sub_poly_vt[i].y;
      mid_vt.z += sub_poly_vt[i].z;
      mid_vt.u += sub_poly_vt[i].u;
      mid_vt.v += sub_poly_vt[i].v;
    }
    
    // unsigned integer division
    fixed rc_num_vt = div_lut[poly->num_vertices]; // .16 result
    
    sub_poly_vt[sp_num_vertices].x = fp_mul(mid_vt.x, rc_num_vt);
    sub_poly_vt[sp_num_vertices].y = fp_mul(mid_vt.y, rc_num_vt);
    sub_poly_vt[sp_num_vertices].z = fp_mul(mid_vt.z, rc_num_vt);
    sub_poly_vt[sp_num_vertices].u = fp_mul(mid_vt.u, rc_num_vt);
    sub_poly_vt[sp_num_vertices].v = fp_mul(mid_vt.v, rc_num_vt);
    
    sp_num_vertices++;
    
    int total_num_vertices = sp_num_vertices;
    sp_num_vertices >>= 1;
    
    poly->num_vertices = 4;
    
    // subdivide the polygon
    
    poly->vertices[0] = sub_poly_vt[0]; // first vertex
    poly->vertices[1] = sub_poly_vt[sp_num_vertices]; // first edge middle vertex
    poly->vertices[2] = sub_poly_vt[total_num_vertices - 1]; // center vertex
    poly->vertices[3] = sub_poly_vt[total_num_vertices - 2]; // last edge middle vertices
    
    // draw_face_tr(poly, face_id);
    if (test_poly_ratio(poly)) {
      subdiv_poly(poly, face_id);
    } else {
      #if DRAW_TRIANGLES
        subdiv_poly_tris(poly, face_id);
      #else
        draw_face_tr(poly, face_id);
      #endif
    }
    
    for (int i = 1; i < sp_num_vertices; i++) {
      poly->num_vertices = 4;
      
      poly->vertices[0] = sub_poly_vt[i + sp_num_vertices - 1]; // first edge middle vertex
      poly->vertices[1] = sub_poly_vt[i]; // original vertex
      poly->vertices[2] = sub_poly_vt[i + sp_num_vertices]; // second edge middle vertex
      poly->vertices[3] = sub_poly_vt[total_num_vertices - 1]; // center vertex
      
      // draw_face_tr(poly, face_id);
      if (test_poly_ratio(poly)) {
        subdiv_poly(poly, face_id);
      } else {
        #if DRAW_TRIANGLES
          subdiv_poly_tris(poly, face_id);
        #else
          draw_face_tr(poly, face_id);
        #endif
      }
    }
  }
#endif

#if !TX_PERSP_MODE && SUBDIV_MODE
  // subdivide the face along an edge using a lut
  // intended for recursion
  
  RAM_CODE void subdiv_poly_edge_lut(g_poly_t *poly, int face_id) {
    vec5_t sub_poly_vt[8];
    u8 subdivided_edges = 0;
    
    // obtain the mid vertex for every edge and add it to the polygon vertex list
    
    int pl_mid_num_vt = 0;
    int n = poly->num_vertices - 1;
    
    for (int i = 0; i < poly->num_vertices; i++) {
      fixed max_z, min_z;
      if (poly->vertices[i].z > poly->vertices[n].z) {
        max_z = poly->vertices[i].z;
        min_z = poly->vertices[n].z;
      } else {
        max_z = poly->vertices[n].z;
        min_z = poly->vertices[i].z;
      }
      
      if (max_z > fp_mul(min_z, fix(1.5)) && max_z < TX_PERSP_DIST && max_z - min_z > fix(0.6)) {
      // if (max_z > fp_mul(min_z, fix(2)) && max_z < TX_PERSP_DIST && max_z - min_z > fix(0.6)) {
        int poly_total_vt = poly->num_vertices + pl_mid_num_vt;
        sub_poly_vt[poly_total_vt].x = (poly->vertices[n].x + poly->vertices[i].x) >> 1;
        sub_poly_vt[poly_total_vt].y = (poly->vertices[n].y + poly->vertices[i].y) >> 1;
        sub_poly_vt[poly_total_vt].z = (poly->vertices[n].z + poly->vertices[i].z) >> 1;
        sub_poly_vt[poly_total_vt].u = (poly->vertices[n].u + poly->vertices[i].u) >> 1;
        sub_poly_vt[poly_total_vt].v = (poly->vertices[n].v + poly->vertices[i].v) >> 1;
        
        subdivided_edges |= 1 << i;
        pl_mid_num_vt++;
      }
      
      n = i;
    }
    
    if (!pl_mid_num_vt) {
      #if DRAW_TRIANGLES
        subdiv_poly_tris(poly, face_id);
      #else
        draw_face_tr(poly, face_id);
      #endif
      return;
    } else {
      // copy the vertices
      
      for (int i = 0; i < poly->num_vertices; i++) {
        sub_poly_vt[i] = poly->vertices[i];
      }
      
      if (poly->num_vertices == 3) { // triangle
        for (int i = 0; i < tri_sub_num_pl[subdivided_edges]; i++) {
          u8 group_pnt = tri_sub_group_idx[subdivided_edges] + i;
          poly->num_vertices = tri_sub_num_vt[group_pnt];
          
          for (int j = 0; j < poly->num_vertices; j++) {
            u8 sub_pl_vt_pnt = tri_sub_faces_lut[tri_sub_face_index[group_pnt] + j];
            poly->vertices[j] = sub_poly_vt[sub_pl_vt_pnt];
          }
          
          subdiv_poly_edge_lut(poly, face_id);
        }
      } else { // quad
        for (int i = 0; i < quad_sub_num_pl[subdivided_edges]; i++) {
          u8 group_pnt = quad_sub_group_idx[subdivided_edges] + i;
          poly->num_vertices = quad_sub_num_vt[group_pnt];
          
          for (int j = 0; j < poly->num_vertices; j++) {
            u8 sub_pl_vt_pnt = quad_sub_faces_lut[quad_sub_face_index[group_pnt] + j];
            poly->vertices[j] = sub_poly_vt[sub_pl_vt_pnt];
          }
          
          subdiv_poly_edge_lut(poly, face_id);
        }
      }
    }
  }
  
  // subdivide the face along an edge
  // intended for recursion
  
  void subdiv_poly_edge(g_poly_t *poly, int face_id) {
    int n = 1;
    
    for (int i = 0; i < poly->num_vertices; i++) {
      fixed max_z, min_z;
      if (poly->vertices[i].z > poly->vertices[n].z) {
        max_z = poly->vertices[i].z;
        min_z = poly->vertices[n].z;
      } else {
        max_z = poly->vertices[n].z;
        min_z = poly->vertices[i].z;
      }
      
      // if (max_z > min_z * 2 && max_z < TX_PERSP_DIST && max_z - min_z > fix(0.6)) {
      if (max_z > fp_mul(min_z, fix(1.5)) && max_z < TX_PERSP_DIST && max_z - min_z > fix(0.6)) {
        vec5_t sub_poly_vt[8];
        vec5_t mid_vt;
        // obtain the mid vertex
        mid_vt.x = (poly->vertices[n].x + poly->vertices[i].x) >> 1;
        mid_vt.y = (poly->vertices[n].y + poly->vertices[i].y) >> 1;
        mid_vt.z = (poly->vertices[n].z + poly->vertices[i].z) >> 1;
        mid_vt.u = (poly->vertices[n].u + poly->vertices[i].u) >> 1;
        mid_vt.v = (poly->vertices[n].v + poly->vertices[i].v) >> 1;
        
        for (int j = 0; j < poly->num_vertices; j++) {
          sub_poly_vt[j] = poly->vertices[j];
        }
        
        // subdivide the polygon
        
        int sub_poly_num_vt = poly->num_vertices;
        
        poly->vertices[n] = mid_vt;
        
        subdiv_poly_edge(poly, face_id);
        
        poly->vertices[0] = mid_vt;
        poly->vertices[1] = sub_poly_vt[n];
        
        n++;
        if (n == sub_poly_num_vt) n = 0;
        
        poly->vertices[2] = sub_poly_vt[n];
        
        poly->num_vertices = 3;
        
        subdiv_poly_edge(poly, face_id);
        
        poly->num_vertices = sub_poly_num_vt;
        
        return;
      }
      
      n++;
      if (n == poly->num_vertices) n = 0;
    }
    
    draw_face_tr(poly, face_id);
  }
#endif

// subdivides the faces into quads and triangles

void subdiv_poly_into_quads(g_poly_t *poly, int face_id) {
  if (poly->num_vertices > 4) {
    vec5_t sub_poly_vt[8];
    
    for (int i = 0; i < poly->num_vertices; i++) {
      sub_poly_vt[i] = poly->vertices[i];
    }
    
    int sub_poly_num_vt = poly->num_vertices;
    
    for (int i = 0; i < sub_poly_num_vt - 3; i += 2) {
      poly->num_vertices = 4;
      
      poly->vertices[0] = sub_poly_vt[0];
      poly->vertices[1] = sub_poly_vt[i + 2];
      poly->vertices[2] = sub_poly_vt[i + 3];
      poly->vertices[3] = sub_poly_vt[i + 4];
      
      draw_face_tr(poly, face_id);
    }
    
    if (sub_poly_num_vt & 1) {
      poly->num_vertices = 3;
      
      poly->vertices[0] = sub_poly_vt[0];
      poly->vertices[1] = sub_poly_vt[sub_poly_num_vt - 2];
      poly->vertices[2] = sub_poly_vt[sub_poly_num_vt - 1];
      
      draw_face_tr(poly, face_id);
    }
  }
}

// subdivides the face into triangles and draws them

#if DRAW_TRIANGLES
  RAM_CODE void subdiv_poly_tris(g_poly_t *poly, int face_id) {
    if (poly->num_vertices == 3) {
      draw_face_tr(poly, face_id);
      return;
    }
    
    vec5_t sub_poly_vt[8];
    
    for (int i = 0; i < poly->num_vertices; i++) {
      sub_poly_vt[i] = poly->vertices[i];
    }
    
    int sub_poly_num_vt = poly->num_vertices;
    
    poly->num_vertices = 3;
    
    poly->vertices[0] = sub_poly_vt[0];
    poly->vertices[1] = sub_poly_vt[1];
    poly->vertices[2] = sub_poly_vt[2];
    
    draw_face_tr(poly, face_id);
    
    for (int i = 0; i < sub_poly_num_vt - 3; i++) {
      poly->num_vertices = 3;
      poly->vertices[0] = sub_poly_vt[0];
      poly->vertices[1] = sub_poly_vt[i + 2];
      poly->vertices[2] = sub_poly_vt[i + 3];
      
      draw_face_tr(poly, face_id);
    }
    
    poly->num_vertices = sub_poly_num_vt;
  }
#endif