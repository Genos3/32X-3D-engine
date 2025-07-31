#include "common.h"

#if 1 // !ENABLE_ASM
  RAM_CODE void draw_face_tr(g_poly_t *poly, int face_id) { //draw face transformed
    if (!poly->num_vertices) return;
    
    // view 3d clipping
    
    if (poly->frustum_clip_sides) {
      if (poly->frustum_clip_sides & NEAR_PLANE) {
        clip_poly_plane(poly, 0);
      }
      
      #if ENABLE_FAR_PLANE_CLIPPING
        if (poly->frustum_clip_sides & FAR_PLANE) {
          clip_poly_plane(poly, 1);
        }
      #endif
      
      if (poly->frustum_clip_sides & LEFT_PLANE) {
        clip_poly_plane(poly, 2);
      }
      
      if (poly->frustum_clip_sides & RIGHT_PLANE) {
        clip_poly_plane(poly, 3);
      }
      
      if (poly->frustum_clip_sides & TOP_PLANE) {
        clip_poly_plane(poly, 4);
      }
      
      if (poly->frustum_clip_sides & BOTTOM_PLANE) {
        clip_poly_plane(poly, 5);
      }
      
      if (!poly->num_vertices) return;
    }
    
    // perspective division
    
    project_poly_vertices(poly);
    
    if (cfg.draw_polys && poly->flags.is_visible) {
      if (!poly->flags.has_texture) { // no texture
        #if ENABLE_Z_BUFFER
          draw_poly_zb(poly);
        #else
          #if ENABLE_ASM
            draw_poly_asm(poly);
          #else
            draw_poly(poly);
          #endif
        #endif
      } else if (poly->face_type & SPRITE) { // sprite
        #if ENABLE_Z_BUFFER
          draw_sprite_zb(poly);
        #else
          draw_sprite(poly);
        #endif
      } else { // normal face
        #if TX_PERSP_MODE == 1
          #if ENABLE_Z_BUFFER
            draw_poly_tx_affine_zb(poly);
          #else
            if (cfg.tx_perspective_mapping_enabled) {
              draw_poly_tx_sub_ps(poly);
            } else {
              draw_poly_tx_affine(poly);
            }
          #endif
        #else
          #if ENABLE_Z_BUFFER
            draw_poly_tx_affine_zb(poly);
          #else
            #if ENABLE_ASM
              draw_poly_tx_affine_asm(poly);
            #else
              draw_poly_tx_affine(poly);
            #endif
          #endif
        #endif
      }
    }
    
    if (cfg.draw_lines) {
      u16 color;
      if (VIEW_TEST_DIST && cfg.face_enable_persp_tx) {
        color = PAL_BLUE;
      } else {
        color = PAL_MAGENTA;
      }
      #if !defined(PC) && PALETTE_MODE
        color = dup8(color);
      #endif
      draw_lines_poly(poly, color);
    }
    
    if (dbg_show_poly_num) {
      if (dbg_num_poly_dsp == face_id) {
        draw_lines_poly(poly, PAL_RED);
      }
    }
    
    r_scene_num_polys++;
  }
#endif

RAM_CODE void draw_lines_poly(g_poly_t *poly, u16 color) {
  #if ENABLE_Z_BUFFER
    for (int i = 0; i < poly->num_vertices - 1; i++) {
      poly->vertices[i].z = fp_mul(poly->vertices[i].z, fix(0.99));
    }
  #endif
  
  for (int i = 0; i < poly->num_vertices - 1; i++) {
    #if 0 // ENABLE_ASM
      draw_line_asm(poly->vertices[i].x, poly->vertices[i].y, poly->vertices[i + 1].x, poly->vertices[i + 1].y, color);
    #else
      #if ENABLE_Z_BUFFER
        draw_line_zb(poly->vertices[i].x, poly->vertices[i].y, poly->vertices[i].z, poly->vertices[i + 1].x, poly->vertices[i + 1].y, poly->vertices[i + 1].z, color);
      #else
        draw_line(poly->vertices[i].x, poly->vertices[i].y, poly->vertices[i + 1].x, poly->vertices[i + 1].y, color);
      #endif
    #endif
  }
  
  #if 0 // ENABLE_ASM
    draw_line_asm(poly->vertices[poly->num_vertices - 1].x, poly->vertices[poly->num_vertices - 1].y, poly->vertices[0].x, poly->vertices[0].y, color);
  #else
    #if ENABLE_Z_BUFFER
      draw_line_zb(poly->vertices[poly->num_vertices - 1].x, poly->vertices[poly->num_vertices - 1].y, poly->vertices[poly->num_vertices - 1].z, poly->vertices[0].x, poly->vertices[0].y, poly->vertices[0].z, color);
    #else
      draw_line(poly->vertices[poly->num_vertices - 1].x, poly->vertices[poly->num_vertices - 1].y, poly->vertices[0].x, poly->vertices[0].y , color);
    #endif
  #endif
}

void draw_axis(){
  if (calc_vec_length(cam.pos) > vp.z_far) return;
  
  line_t line;
  line.p0.x = camera_matrix[0];
  line.p0.y = camera_matrix[4];
  line.p0.z = camera_matrix[8];
  
  for (int i = 1; i <= 3; i++) {
    u16 color;
    if (i == 1) {
      color = PAL_RED;
    } else
    if (i == 2) {
      color = PAL_GREEN;
    } else {
      color = PAL_BLUE;
    }
    
    #if !defined(PC) && PALETTE_MODE
      color = dup8(color);
    #endif
    
    line.p1.x = line.p0.x + fp_mul(camera_matrix[i], AXIS_VECTOR_SIZE);
    line.p1.y = line.p0.y + fp_mul(camera_matrix[i + 4], AXIS_VECTOR_SIZE);
    line.p1.z = line.p0.z + fp_mul(camera_matrix[i + 8], AXIS_VECTOR_SIZE);
    
    draw_line_3d(line, color);
  }
}

void draw_line_3d(line_t line, u16 color) {
  if (clip_line(&line)) return;
  
  project_vertex(&line.p0);
  project_vertex(&line.p1);
  
  #if !ENABLE_Z_BUFFER
    #if 0 // ENABLE_ASM
      draw_line_asm(line.p0.x, line.p0.y, line.p1.x, line.p1.y, color);
    #else
      draw_line(line.p0.x, line.p0.y, line.p1.x, line.p1.y, color);
    #endif
  #else
    draw_line_zb(line.p0.x, line.p0.y, line.p0.z, line.p1.x, line.p1.y, line.p1.z, color);
  #endif
}

void draw_point_3d(vec3_t pt, u16 color) {
  // frustum culling
  
  fixed w = fp_mul(pt.z, vp.screen_side_x_dt);
  fixed h = fp_mul(pt.z, vp.screen_side_y_dt);
  
  if (ENABLE_FAR_PLANE_CLIPPING && pt.z > vp.z_far) return;
  if (pt.z < vp.z_near || pt.x < -w || pt.x < -w || pt.x > w || pt.y > h) return;
  
  project_vertex(&pt);
  
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      set_pixel(pt.x + j - 1, pt.y + i - 1, color);
    }
  }
}

#if ENABLE_Z_BUFFER
  void draw_z_buffer() {
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
      // u32 color = z_buffer[i] >> (FP + 2);
      u32 color;
      if (z_buffer[i]) {
        color = fp_div(1 << FP, z_buffer[i]) >> (FP + 2);
      } else {
        color = 31;
      }
      
      if (color > 31) {
        color = 31;
      }
      
      color = 31 - color;
      screen[i] = RGB15(color, color, color);
    }
  }
#endif

void draw_debug_buffers() {
  #if ENABLE_Z_BUFFER && DRAW_Z_BUFFER
    draw_z_buffer();
  #endif
  
  if (cfg.draw_lines) {
    #if ENABLE_GRID_FRUSTUM_CULLING && DRAW_FRUSTUM
      draw_frustum(PAL_RED);
    #endif
  }
}