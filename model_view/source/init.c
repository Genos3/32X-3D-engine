#include "common.h"

void init_camera();
void init_model();
void init_scene();

void init_3d() {
  // configurable values
  
  cfg.tx_perspective_mapping_enabled = 0; // texture perspective view
  vp.z_near = fix(0.1); // clipping z near
  vp.z_far = fix(20);
  cfg.draw_lines = 0;
  cfg.draw_polys = 1;
  cfg.draw_textures = 0;
  cfg.static_light = 1; // precalculate lighting
  cfg.directional_lighting = 1;
  cfg.debug = 0;
  cfg.clean_screen = 1;
  cfg.animation_play = 1;
  // scn_floor_enabled = 0; // floor
  // scn.bg_color = RGB15(18, 26, 28); // RGB15(2, 2, 2);
  cfg.occlusion_cl_enabled = 0;
  #ifdef PC
    cfg.tx_perspective_mapping_enabled = 1;
    cfg.debug = 0;
  #endif
  scn.draw_sky = 1;
  scn.lightdir.x = 0;
  scn.lightdir.y = 1 << FP;
  scn.lightdir.z = 1 << FP;
  
  // fixed values
  
  // clipping_border_fp = 0; //-128;
  // model_id = 0;
  frame_counter = 1;
  normalize(&scn.lightdir, &scn.lightdir_n);
  calc_fov();
  
  init_camera();
  set_cam_pos();
  init_scene();
  init_model_list();
  change_model(0);
  init_model();
  change_texture(0);
  #if PALETTE_MODE
    init_palette();
    load_model_palette();
  #endif
  init_structs();
  //set_normals();
  set_frustum();
  //z_buffer = malloc(SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(u32));
  
  if (g_obj.type == OBJ_MODEL) {
    if (cfg.static_light) {
      calc_model_light(g_model);
    }
    
    if (g_model->flags.has_grid) {
      grid_tile_radius = DIAG_UNIT_DIST_RC << (g_model->grid.tile_size_bits - 1);
    }
  }
  
  zfar_ratio = fp_div(SCN_OT_SIZE << FP, vp.z_far - vp.z_near);
}

void init_camera() {
  cam.pos.x = 0;
  cam.pos.z = 0;
  cam.pos.y = 0;
  cam.rot.x = 0;
  cam.rot.y = 0;
  cam.rot.z = 128 << 8;
  cam.acc = fix(4.0); // 4.0
  cam.rot_speed = 60 * (1 << 8);
  
  cam.speed.x = 0;
  cam.speed.y = 0;
  cam.speed.z = 0;
}

void init_model() {
  // g_vx_model = model_list[0];
  
  init_obj(&g_obj);
  
  g_obj.mdl_pnt = g_model;
  g_obj.type = OBJ_MODEL;
  // g_obj.mdl_pnt = g_vx_model;
  // g_obj.type = OBJ_VOXEL;
}

void init_scene() {
  scn.curr_model = 0;
  
  if (scn.draw_sky) {
    // sky.start_color = RGB15(9, 13, 14);
    // sky.end_color = RGB15(18, 26, 28);
    sky.start_color = RGB15(7, 14, 23);
    sky.end_color = RGB15(24, 27, 29);
    //sky.end_color = RGB15(22, 25, 28);
    sky.start_angle = 220 << 8;
    sky.end_angle = 0;
    if (vp.vfov) {
      sky.fp_deg2px = SCREEN_HEIGHT_FP / vp.vfov; // .8
    }
    
    init_sky();
    set_sky_gradient();
  }
  
  scn.obj_list[0].mdl_pnt = g_obj.mdl_pnt;
  scn.obj_list[0].type = g_obj.type;
  
  tr_vertices = scene_tr_vertices;
}