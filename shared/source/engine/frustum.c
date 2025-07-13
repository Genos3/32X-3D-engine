#include "common.h"

// near, far, left, right, top, bottom
static const u8 frustum_faces[] = {0,1,2,3, 5,4,7,6, 0,3,7,4, 2,1,5,6, 1,0,4,5, 3,2,6,7};

#if DRAW_FRUSTUM
  static const u8 frustum_lines[] = {
    0,1, 1,2, 2,3, 3,0,
    4,5, 5,6, 6,7, 7,4,
    0,4, 1,5, 2,6, 3,7
  };
#endif

#define FRUSTUM_LINES_SIZE sizeof(frustum_lines >> 1)

void set_frustum() {
  //vp.focal_length=mw/tan(45*PI_fp/180);
  fixed x = fp_mul(vp.screen_side_x_dt, vp.z_near);
  fixed y = fp_mul(vp.screen_side_y_dt, vp.z_near);
  //fov=(float)atan(mw/vp.focal_length)*180/PI_fp*2;
  
  frustum.vertices[0].x = -x;
  frustum.vertices[0].y = -y;
  frustum.vertices[0].z = vp.z_near;
  frustum.vertices[1].x = x;
  frustum.vertices[1].y = -y;
  frustum.vertices[1].z = vp.z_near;
  frustum.vertices[2].x = x;
  frustum.vertices[2].y = y;
  frustum.vertices[2].z = vp.z_near;
  frustum.vertices[3].x = -x;
  frustum.vertices[3].y = y;
  frustum.vertices[3].z = vp.z_near;
  
  x = fp_mul(vp.screen_side_x_dt, vp.z_far);
  y = fp_mul(vp.screen_side_y_dt, vp.z_far);
  
  frustum.vertices[4].x = -x;
  frustum.vertices[4].y = -y;
  frustum.vertices[4].z = vp.z_far;
  frustum.vertices[5].x = x;
  frustum.vertices[5].y = -y;
  frustum.vertices[5].z = vp.z_far;
  frustum.vertices[6].x = x;
  frustum.vertices[6].y = y;
  frustum.vertices[6].z = vp.z_far;
  frustum.vertices[7].x = -x;
  frustum.vertices[7].y = y;
  frustum.vertices[7].z = vp.z_far;
  
  frustum.normals[0].x = 0; // near plane
  frustum.normals[0].y = 0;
  frustum.normals[0].z = -(1 << FP);
  frustum.normals[1].x = 0; // far plane
  frustum.normals[1].y = 0;
  frustum.normals[1].z = (1 << FP);
  
  for (int i = 2; i < 6; i++) {
    vec3_t normal, n_normal;
    poly_t poly;
    
    poly.vertices[0] = frustum.vertices[frustum_faces[i * 4]];
    poly.vertices[1] = frustum.vertices[frustum_faces[i * 4 + 1]];
    poly.vertices[2] = frustum.vertices[frustum_faces[i * 4 + 2]];
    
    calc_normal(&poly.vertices[0], &poly.vertices[1], &poly.vertices[2], &normal); // cross product
    
    /* while (abs_c(normal.z) > DIV_LUT_SIZE_R || abs_c(normal.y) > DIV_LUT_SIZE_R || abs_c(normal.z) > DIV_LUT_SIZE_R) {
      normal.x >>= 1;
      normal.y >>= 1;
      normal.z >>= 1;
    }
    
    while ((normal.x && abs_c(normal.z) < 1024) || (normal.y && abs_c(normal.y) < 1024) || (normal.z && abs_c(normal.z) < 1024)) {
      normal.x <<= 1;
      normal.y <<= 1;
      normal.z <<= 1;
    } */
    
    normalize(&normal, &n_normal);
    frustum.normals[i] = n_normal;
  }
}

void calc_dist_frustum() {
  for (int i = 0; i < 6; i++) {
    tr_frustum.normals[i].d = -dot((vec3_t *)&tr_frustum.normals[i], &tr_frustum.vertices[frustum_faces[i * 4]]);
  }
}

#if DRAW_FRUSTUM
  void draw_frustum(u16 color) {
    for (int i = 0; i < FRUSTUM_LINES_SIZE; i += 2) {
      line_t line;
      line.p0 = tr_view_frustum.vertices[frustum_lines[i]];
      line.p1 = tr_view_frustum.vertices[frustum_lines[i + 1]];

      draw_line_3d(line, color);
    }
    
    #if DRAW_NORMALS
      for (int i = 0; i < 6; i++) {
        line_t line;
        line.p0 = tr_view_frustum.vertices[frustum_faces[i * 4]];
        line.p1.x = fp_mul(tr_view_frustum.normals[i].x, AXIS_VECTOR_SIZE);
        line.p1.y = fp_mul(tr_view_frustum.normals[i].y, AXIS_VECTOR_SIZE);
        line.p1.z = fp_mul(tr_view_frustum.normals[i].z, AXIS_VECTOR_SIZE);

        draw_line_3d(line, color);
      }
    #endif
  }
#endif