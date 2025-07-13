#include "common.h"

void calc_fov() {
  fixed_u hfov_deg = 70; // 70
  fixed inv_aspect_ratio = SCREEN_HEIGHT_FP / SCREEN_WIDTH;
  vp.hfov = (hfov_deg << 8) * 256 / 360; // .8
  vp.half_hfov = vp.hfov >> 1;
  fixed tan_half_hfov = tan_c(vp.half_hfov);
  vp.half_vfov = atan_lut[fp_mul(tan_half_hfov, inv_aspect_ratio) >> 8]; // .8
  vp.vfov = vp.half_vfov << 1;
  
  // tan(x) = op / adj
  // tan(x) = sin(x) / cos(x)
  // sin(x) / cos(x) = op / adj
  // adj = op * cos(x) / sin(x)
  
  vp.focal_length = fp_div(SCREEN_HALF_WIDTH << FP, tan_half_hfov);
  vp.focal_length_i = vp.focal_length >> FP;
  vp.screen_side_x_dt = fp_div(SCREEN_HALF_WIDTH, vp.focal_length_i) + fix(0.01);
  vp.screen_side_y_dt = fp_div(SCREEN_HALF_HEIGHT, vp.focal_length_i) + fix(0.01);
}

#if PALETTE_MODE
  void init_palette() {
    hw_palette[PAL_BG] = CLR_BG;
    hw_palette[PAL_WHITE] = CLR_WHITE;
    hw_palette[PAL_BLACK] = CLR_BLACK;
    hw_palette[PAL_RED] = CLR_RED;
    hw_palette[PAL_GREEN] = CLR_GREEN;
    hw_palette[PAL_BLUE] = CLR_BLUE;
    hw_palette[PAL_YELLOW] = CLR_YELLOW;
    hw_palette[PAL_MAGENTA] = CLR_MAGENTA;
  }
#endif

void init_structs() {
  dl.pl_list = dl_pl_list;
  dl.vt_list = dl_vt_list;
  dl.obj_list = dl_obj_list;
  dl.tr_vt_index = dl_tr_vt_index;
  dl.visible_vt_list = dl_visible_vt_list;
  
  g_ot_list.pnt = ordering_table;
  g_ot_list.pl_list = ot_pl_list;
  g_ot_list.size = SCN_OT_SIZE;
}

void init_obj(object_t *obj) {
  obj->pos.x = 0;
  obj->pos.y = 0;
  obj->pos.z = 0;
  obj->rot.x = 0;
  obj->rot.y = 0;
  obj->rot.z = 0;
  obj->static_pos = 1;
  obj->static_rot = 1;
}