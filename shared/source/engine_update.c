#include "common.h"

void update_matrix_transforms();
void draw_scene();
void add_obj_to_dl(int id);
void reset_display_list();

#ifdef PC
  extern float delta_time_f;
#endif

void draw() {
  // clean the screen
  if (cfg.clean_screen) {
    // use the sub CPU
    #if USE_SECOND_CPU
      MARS_SYS_COMM4 = MARS_CMD_DRAW_BG;
    #else
      if (scn.draw_sky) {
        #if PALETTE_MODE
          draw_sky_pal();
        #else
          draw_sky();
        #endif
      } else {
        #ifdef PC
          fill_rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, PAL_BG);
        #else
          fill_rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, dup8(PAL_BG));
        #endif
      }
    #endif
  }
  
  update_matrix_transforms();
  
  #if ENABLE_Z_BUFFER
    // memset32(z_buffer, 0xffffffff, SCREEN_WIDTH * SCREEN_HEIGHT);
    memset32(z_buffer, 0, SCREEN_WIDTH * SCREEN_HEIGHT);
  #endif
  
  draw_scene();
  
  #if DRAW_DEBUG_BUFFERS
    draw_debug_buffers();
  #endif
  
  if (cfg.draw_lines) {
    draw_axis();
  }
  
  frame_counter++;
}

void update_matrix_transforms() {
  set_matrix(camera_matrix, identity_matrix);
  set_matrix(camera_sprite_matrix, identity_matrix);
  
  #if !PERSPECTIVE_ENABLED
    scale_matrix(fix(16), fix(16), fix(16), camera_matrix); // 16
    scale_matrix(fix(16), fix(16), fix(16), camera_sprite_matrix);
  #endif
  
  translate_matrix(-cam.pos.x, -cam.pos.y, -cam.pos.z, camera_matrix);
  rotate_matrix(-cam.rot.x, -cam.rot.y, -cam.rot.z, camera_matrix, 1);
  
  #if ENABLE_GRID_FRUSTUM_CULLING
    set_matrix(frustum_matrix, identity_matrix);
    
    #if DRAW_FRUSTUM
      set_matrix(view_frustum_matrix, identity_matrix);
      #if ADVANCE_FRUSTUM
        translate_matrix(0, 0, 1 << FP, frustum_matrix); // view frustum area
        translate_matrix(0, 0, 1 << FP, view_frustum_matrix);
      #endif
    #endif
    
    rotate_matrix(-cam.rot.x, -cam.rot.y, -cam.rot.z, frustum_matrix, 0);
    #if DRAW_FRUSTUM && FIXED_FRUSTUM
      rotate_matrix(-cam.rot.x, -cam.rot.y, -cam.rot.z, view_frustum_matrix, 0);
    #endif
    
    #if !FIXED_FRUSTUM
      translate_matrix(cam.pos.x, cam.pos.y, cam.pos.z, frustum_matrix);
    #endif
    
    #if DRAW_FRUSTUM
      #if FIXED_FRUSTUM
        transform_matrix(view_frustum_matrix, camera_matrix); // set frustum in origin
       #endif
      transform_frustum(&frustum, &tr_view_frustum, view_frustum_matrix, 0);
    #endif
    
    transform_frustum(&frustum, &tr_frustum, frustum_matrix, 1);
    calc_dist_frustum();
  #endif
  
  //set_matrix_sp(1, 0, 0, 0, camera_sprite_matrix, camera_matrix);
  /* #if ENABLE_GRID_FRUSTUM_CULLING && DRAW_GRID
    change_model(model_id);
  #endif */
}

void draw_scene() {
  r_scene_num_polys = 0;
  r_scene_num_elements_ot = 0;
  r_scene_num_objs = 0;
  
  reset_display_list();
  
  set_matrix(model_matrix, identity_matrix);
  
  if (!g_obj.static_rot) {
    rotate_matrix(g_obj.rot.x, g_obj.rot.y, g_obj.rot.z, model_matrix, 0);
  }
  
  if (!g_obj.static_pos) {
    translate_matrix(g_obj.pos.x, g_obj.pos.y, g_obj.pos.z, model_matrix);
  }
  
  transform_matrix(model_matrix, camera_matrix);
  
  if (g_obj.type == OBJ_MODEL) {
    if (g_model->num_sprite_vertices) {
      transform_vertices(camera_sprite_matrix, g_model->sprite_vertices, tr_sprite_vertices, g_model->num_sprite_vertices);
    }
    
    #if ENABLE_GRID_FRUSTUM_CULLING
      if (g_model->flags.has_grid) {
        // profile_start();
        #if 0 // ENABLE_ASM
          draw_grid_asm(g_obj.pos, g_model); // check grid with frustum culling
        #else
          draw_grid(g_obj.pos, g_model); // check grid with frustum culling
        #endif
        // profile_stop();
        
        if (!dl.num_faces) return;
        
        // init the ordering table
        memset32(g_ot_list.pnt, -1, g_ot_list.size >> 1);
        
        #if 0 // ENABLE_ASM
          transform_model_vis_asm(model_matrix, 0, 1, g_model);
          set_ot_list_vis_asm(g_obj.pos, 1, g_model);
        #else
          transform_model_vis(model_matrix, 0, 1, g_model);
          set_ot_list_vis(g_obj.pos, 1, g_model, &g_ot_list);
        #endif
        
        if (r_scene_num_elements_ot) {
          #if ENABLE_Z_BUFFER
            draw_ot_list_vis(g_obj.pos, 1, 1, g_model, &g_ot_list); // draw from front to back
          #else
            #if 0 // ENABLE_ASM
              draw_ot_list_vis_asm(g_obj.pos, 0, 1, g_model, &g_ot_list); // draw from back to front
            #else
              draw_ot_list_vis(g_obj.pos, 0, 1, g_model, &g_ot_list); // draw from back to front
            #endif
          #endif
        }
      } else
    #endif
    {
      // init the ordering table
      memset32(g_ot_list.pnt, -1, g_ot_list.size >> 1);
      
      #if 0 // ENABLE_ASM
        transform_model_vis_asm(model_matrix, 0, 0, g_model);
        set_ot_list_vis_asm(g_obj.pos, 0, g_model, &g_ot_list);
      #else
        transform_model_vis(model_matrix, 0, 0, g_model);
        set_ot_list_vis(g_obj.pos, 0, g_model, &g_ot_list);
      #endif
      
      if (r_scene_num_elements_ot) {
        #if ENABLE_Z_BUFFER
          draw_ot_list_vis(g_obj.pos, 1, 0, g_model, &g_ot_list); // draw from front to back
        #else
          #if 0 // ENABLE_ASM
            draw_ot_list_vis_asm(g_obj.pos, 0, 0, g_model, &g_ot_list); // draw from back to front
          #else
            draw_ot_list_vis(g_obj.pos, 0, 0, g_model, &g_ot_list); // draw from back to front
          #endif
        #endif
      }
    }
  } else if (g_obj.type == OBJ_VOXEL) {
    add_obj_to_dl(0);
    
    // init the ordering table
    memset32(g_ot_list.pnt, -1, g_ot_list.size >> 1);
    
    set_ot_list_obj(&g_ot_list);
    
    draw_ot_list_vis(g_obj.pos, 1, 0, g_model, &g_ot_list);
  }
}

void add_obj_to_dl(int id) {
  fixed radius = 0;
  
  if (g_obj.type == OBJ_MODEL) {
    // radius = g_model->model_radius; // TODO: add
  } else if (g_obj.type == OBJ_VOXEL) {
    radius = g_vx_model->model_radius;
  }
  
  if (check_frustum_culling(g_obj.pos, radius, 0)) {
    transform_vertex(g_obj.pos, &scn.obj_tr_pos[id], camera_matrix);
    
    dl.obj_list[dl.num_objects] = id;
    
    dl.num_objects++;
  }
}

void draw_dbg() {
  char str[20];
  int y_pos = 4;
  
  // void (*draw_str_ptr)(int, int, char*, u16) = &draw_str;
  
  sprintf_c(str, "%d fps", fps);
  draw_str(4, y_pos, str, PAL_YELLOW);
  #ifndef PC
    sprintf_c(str, "%d cy", frame_cycles);
    draw_str(4, y_pos += 8, str, PAL_YELLOW);
    // sprintf_c(str, "%d ms", (delta_time * 1000) >> FP);
    // draw_str(4, 20, str, PAL_YELLOW);
    sprintf_c(str, "%d ms", delta_time);
    draw_str(4, y_pos += 8, str, PAL_YELLOW);
  #else
    sprintf_c(str, "%d ms", (int)(delta_time_f));
    draw_str(4, y_pos += 8, str, PAL_YELLOW);
  #endif
  sprintf_c(str, "%d %d", cam.rot.x >> 8, cam.rot.y >> 8);
  draw_str(4, y_pos += 8, str, PAL_YELLOW);
  sprintf_c(str, "%d %d %d", cam.pos.x >> FP, cam.pos.y >> FP, cam.pos.z >> FP);
  draw_str(4, y_pos += 8, str, PAL_YELLOW);
  sprintf_c(str, "%d pl", r_scene_num_polys);
  draw_str(4, y_pos += 8, str, PAL_YELLOW);
  sprintf_c(str, "%d tiles", dl.num_tiles);
  draw_str(4, y_pos += 8, str, PAL_YELLOW);
  
  if (*dbg_screen_output) {
    draw_str(4, y_pos += 8, dbg_screen_output, PAL_YELLOW);
  }
  // sprintf_c(str, "%d", frame_frt);
  // draw_str(4, 68, str, PAL_YELLOW);
  
  #ifdef PC
    if (dbg_show_poly_num) {
      sprintf_c(str, "%d", dbg_num_poly_dsp);
      draw_str(4, y_pos += 8, str, PAL_YELLOW);
    } else if (dbg_show_grid_tile_num) {
      sprintf_c(str, "%d", dbg_num_grid_tile_dsp);
      draw_str(4, y_pos += 8, str, PAL_YELLOW);
    }
  #endif
}

void reset_display_list() {
  #if ENABLE_GRID_FRUSTUM_CULLING
    if (g_model->flags.has_grid) {
      memset8(dl.visible_vt_list, 0, g_model->num_vertices);
    }
  #endif
  
  dl.num_tiles = 0;
  dl.num_faces = 0;
  dl.num_vertices = 0;
  dl.num_objects = 0;
  dl.total_vertices = 0;
}