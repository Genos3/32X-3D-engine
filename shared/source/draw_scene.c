#include "common.h"

#if !PALETTE_MODE
  static u16 sky_palette[SKY_NUM_COLOR_GRADIENT];
#endif

// calculates the gradient for the sky and updates the palette

void set_sky_gradient() {
  fixed_u r0 = sky.start_color_ch.r << FP;
  fixed_u g0 = sky.start_color_ch.g << FP;
  fixed_u b0 = sky.start_color_ch.b << FP;
  
  fixed_u r1 = sky.end_color_ch.r << FP;
  fixed_u g1 = sky.end_color_ch.g << FP;
  fixed_u b1 = sky.end_color_ch.b << FP;
  
  // calculate the deltas for each color
  
  fixed dt_r = (r1 - r0) >> SKY_NUM_COLOR_GRADIENT_BITS;
  fixed dt_g = (g1 - g0) >> SKY_NUM_COLOR_GRADIENT_BITS;
  fixed dt_b = (b1 - b0) >> SKY_NUM_COLOR_GRADIENT_BITS;
  
  for (int i = 0; i < SKY_NUM_COLOR_GRADIENT; i++) {
    u16 color = ((b0 >> FP) << 10) | ((g0 >> FP) << 5) | (r0 >> FP);
    
    #if PALETTE_MODE
      hw_palette[PAL_SKY_GRD_OFFSET + i] = color;
    #else
      sky_palette[i] = color;
    #endif
    
    r0 += dt_r;
    g0 += dt_g;
    b0 += dt_b;
  }
}

// draws a sky gradient

void draw_sky_pal() {
  // obtain the start and end points
  
  u16 cam_start_angle = cam.rot.x - vp.half_vfov - (128 << 8);
  // u16 cam_end_angle = cam.rot.x + vp.half_vfov - (128 << 8);
  
  fixed grd_y0 = (u16)(sky.start_angle - (128 << 8)) - cam_start_angle;
  fixed grd_y1 = (u16)(sky.end_angle - (128 << 8)) - cam_start_angle;
  
  // turn the angles to screen coords
  
  grd_y0 = grd_y0 * sky.fp_deg2px; // .16
  grd_y1 = grd_y1 * sky.fp_deg2px; // .16
  
  int grd_y0i = grd_y0 >> FP;
  int grd_y1i = grd_y1 >> FP;
  
  // calculate the gradient delta for the height
  
  fixed dgr_dy;
  if (grd_y1i > 0 && grd_y0i < SCREEN_HEIGHT) {
    // signed integer division
    #if !DIV_LUT_ENABLE
      fixed rdy = fp_div(1, grd_y1i - grd_y0i);
    #else
      fixed rdy = div_luts_s(grd_y1i - grd_y0i); // .16 result
    #endif
    dgr_dy = SKY_NUM_COLOR_GRADIENT * rdy;
  } else {
    dgr_dy = 0;
  }
  
  fixed scr_grd_offset = 0; // screen gradient offset
  
  // clipping
  
  if (grd_y0i < 0 && grd_y1i > 0) {
    scr_grd_offset += -grd_y0i * dgr_dy;
    grd_y0i = 0;
  } else if (grd_y1i <= 0) {
    scr_grd_offset = (SKY_NUM_COLOR_GRADIENT - 1) << FP;
    grd_y1i = 0;
  }
  
  if (grd_y0i < SCREEN_HEIGHT && grd_y1i > SCREEN_HEIGHT) {
    grd_y1i = SCREEN_HEIGHT - 1;
  } else if (grd_y0i >= SCREEN_HEIGHT) {
    grd_y0i = SCREEN_HEIGHT - 1;
  }
  
  // draw the gradient
  
  int scr_grd_offset_i = scr_grd_offset >> FP;
  
  u32 pal_color_idx = PAL_SKY_GRD_OFFSET + scr_grd_offset_i;
  
  #ifndef PC
    u16 color_data = dup8(pal_color_idx);
  #else
    u16 color_data = pal_color_idx;
  #endif
  
  u16 *vid_y_offset = screen;
  
  for (int i = 0; i < SCREEN_HEIGHT; i++) {
    
    // memset32(vid_y_offset, color_data, SCREEN_HALF_WIDTH);
    
    u16 *vid_pnt = vid_y_offset;
    
    #pragma GCC unroll 4
    for (int j = 0; j < SCREEN_WIDTH; j++) {
      #ifdef PC
        *vid_pnt++ = pc_palette[color_data];
      #else
        *vid_pnt++ = color_data;
      #endif
    }
    
    if (i >= grd_y0i && i < grd_y1i) {
      scr_grd_offset += dgr_dy;
      
      if (scr_grd_offset >> FP != scr_grd_offset_i) {
        scr_grd_offset_i = scr_grd_offset >> FP;
        pal_color_idx = PAL_SKY_GRD_OFFSET + scr_grd_offset_i;
        #ifndef PC
          color_data = dup8(pal_color_idx);
        #else
          color_data = pal_color_idx;
        #endif
      }
    }
    
    vid_y_offset += SCREEN_WIDTH;
  }
}

// draws a gradient for the sky

void draw_sky() {
  // obtain the start and end points
  
  u16 cam_start_angle = cam.rot.x - vp.half_vfov - (128 << 8);
  // u16 cam_end_angle = cam.rot.x + vp.half_vfov - (128 << 8);
  
  int grd_y0 = (u16)(sky.start_angle - (128 << 8)) - cam_start_angle;
  int grd_y1 = (u16)(sky.end_angle - (128 << 8)) - cam_start_angle;
  
  // turn the angles to screen coords
  
  grd_y0 = grd_y0 * sky.fp_deg2px; // .16
  grd_y1 = grd_y1 * sky.fp_deg2px; // .16
  
  int grd_y0i = grd_y0 >> FP;
  int grd_y1i = grd_y1 >> FP;
  
  fixed_u r0 = sky.start_color_ch.r;
  fixed_u g0 = sky.start_color_ch.g;
  fixed_u b0 = sky.start_color_ch.b;
  
  fixed_u r1 = sky.end_color_ch.r;
  fixed_u g1 = sky.end_color_ch.g;
  fixed_u b1 = sky.end_color_ch.b;
  
  // determine the number of shading levels by obtaining the channel with the largest difference
  
  int num_levels = max_c(max_c(r1 - r0, g1 - g0), b1 - b0);
  
  // calculate the deltas for each color and for the gradient
  
  fixed dr_grd, dg_grd, db_grd, grd_dy;
  if (grd_y1i > 0 && grd_y0i < SCREEN_HEIGHT) {
    // fixed rdy = rc_fp_div_luts_a(1 >> 20, grd_y1i - grd_y0i);
    // unsigned integer division
    #if !DIV_LUT_ENABLE
      fixed rc_v = fp_div(1, num_levels);
    #else
      fixed rc_v = div_lut[num_levels]; // .16 result
    #endif
    dr_grd = (r1 - r0) * rc_v;
    dg_grd = (g1 - g0) * rc_v;
    db_grd = (b1 - b0) * rc_v;
    // grd_dy = rc_fp_div_luts_a(num_levels, grd_y1i - grd_y0i);
    // signed integer division
    #if !DIV_LUT_ENABLE
      grd_dy = fp_div(1, grd_y1i - grd_y0i);
    #else
      grd_dy = div_luts_s(grd_y1i - grd_y0i); // .16 result
    #endif
    grd_dy = num_levels * grd_dy;
  } else {
    dr_grd = 0;
    dg_grd = 0;
    db_grd = 0;
    grd_dy = 0;
  }
  
  r0 <<= FP;
  g0 <<= FP;
  b0 <<= FP;
  
  fixed grd_offset = 0;
  
  // clipping
  
  if (grd_y0i < 0 && grd_y1i > 0) {
    //fixed s = rc_fp_div_luts_a(grd_y1 - 0, grd_y1 - grd_y0);
    //r0 = r0 + fp_mul(s, r1 - r0);
    grd_offset = -grd_y0i * grd_dy;
    r0 = r0 + fp_mul(dr_grd, grd_offset);
    g0 = g0 + fp_mul(dg_grd, grd_offset);
    b0 = b0 + fp_mul(db_grd, grd_offset);
    grd_offset &= (1 << FP) - 1;
    grd_y0i = 0;
  } else
  if (grd_y1i <= 0) {
    grd_y1i = 0;
    r0 = r1 << FP;
    g0 = g1 << FP;
    b0 = b1 << FP;
  }
  
  if (grd_y0i < SCREEN_HEIGHT && grd_y1i > SCREEN_HEIGHT) {
    grd_y1i = SCREEN_HEIGHT - 1;
  }
  
  // draw the gradient
  
  fixed_u sr = r0;
  fixed_u sg = g0;
  fixed_u sb = b0;
  
  u32 color = dup16(((sb >> FP) << 10) | ((sg >> FP) << 5) | (sr >> FP));
  u16 *vid_y_offset = screen;
  
  for (int i = 0; i < SCREEN_HEIGHT; i++) {
    memset32(vid_y_offset, color, SCREEN_HALF_WIDTH);
    
    /* u16 *vid_pnt = vid_y_offset;
    
    for (int j = 0; j < SCREEN_WIDTH; j++) {
      *vid_pnt++ = color;
    } */
    
    if (i >= grd_y0i && i < grd_y1i) {
      if (grd_offset >= 1 << FP) {

        sr += dr_grd;
        sg += dg_grd;
        sb += db_grd;
        
        color = dup16(((sb >> FP) << 10) | ((sg >> FP) << 5) | (sr >> FP));
        grd_offset -= 1 << FP;
      }
      
      grd_offset += grd_dy;
    }
    
    vid_y_offset += SCREEN_WIDTH;
  }
}

void draw_sky_def() {
  // obtain the start and end points
  
  u16 cam_start_angle = cam.rot.x - vp.half_vfov - (128 << 8);
  // u16 cam_end_angle = cam.rot.x + vp.half_vfov - (128 << 8);
  
  int grd_y0 = (u16)(sky.start_angle - (128 << 8)) - cam_start_angle;
  int grd_y1 = (u16)(sky.end_angle - (128 << 8)) - cam_start_angle;
  
  // turn the angles to screen coords
  
  grd_y0 = fp_mul(grd_y0, sky.fp_deg2px);
  grd_y1 = fp_mul(grd_y1, sky.fp_deg2px);
  
  int grd_y0i = grd_y0 >> FP;
  int grd_y1i = grd_y1 >> FP;
  
  fixed r0 = sky.start_color_ch.r << FP;
  fixed g0 = sky.start_color_ch.g << FP;
  fixed b0 = sky.start_color_ch.b << FP;
  
  fixed r1 = sky.end_color_ch.r << FP;
  fixed g1 = sky.end_color_ch.g << FP;
  fixed b1 = sky.end_color_ch.b << FP;
  
  // calculate the deltas for each color
  
  fixed drdy, dgdy, dbdy;
  if (grd_y1i > 0 && grd_y0i < SCREEN_HEIGHT) {
    // fixed rdy = rc_fp_div_luts_a(1 >> 20, grd_y1i - grd_y0i);
    // signed integer division
    fixed rdy = div_luts_s(grd_y1i - grd_y0i); // 24 bit result
    drdy = fp_mul(r1 - r0, rdy) >> 12;
    dgdy = fp_mul(g1 - g0, rdy) >> 12;
    dbdy = fp_mul(b1 - b0, rdy) >> 12;
  } else {
    drdy = 0;
    dgdy = 0;
    dbdy = 0;
  }
  
  // clipping
  
  if (grd_y0i < 0 && grd_y1i > 0) {
    //fixed s = rc_fp_div_luts_a(grd_y1 - 0, grd_y1 - grd_y0);
    //r0 = r0 + fp_mul(s, r1 - r0);
    r0 = r0 + (-grd_y0i * drdy);
    g0 = g0 + (-grd_y0i * dgdy);
    b0 = b0 + (-grd_y0i * dbdy);
    grd_y0i = 0;
  } else
  if (grd_y1i <= 0) {
    grd_y1i = 0;
    r0 = r1;
    g0 = g1;
    b0 = b1;
  }
  
  if (grd_y0i < SCREEN_HEIGHT && grd_y1i > SCREEN_HEIGHT) {
    grd_y1i = SCREEN_HEIGHT - 1;
  }
  
  // draw the gradient
  
  fixed sr = r0;
  fixed sg = g0;
  fixed sb = b0;
  
  u32 color = dup16(((sb >> FP) << 10) | ((sg >> FP) << 5) | (sr >> FP));
  u16 *vid_y_offset = screen;
  
  for (int i = 0; i < SCREEN_HEIGHT; i++) {
    memset32(vid_y_offset, color, SCREEN_HALF_WIDTH);
    
    /* u16 *vid_pnt = vid_y_offset;
    
    for (int j = 0; j < SCREEN_WIDTH; j++) {
      *vid_pnt++ = color;
    } */
    
    if (i >= grd_y0i && i < grd_y1i) {
      sr += drdy;
      sg += dgdy;
      sb += dbdy;
      
      color = dup16(((sb >> FP) << 10) | ((sg >> FP) << 5) | (sr >> FP));
    }
    
    vid_y_offset += SCREEN_WIDTH;
  }
}

void draw_sky_fw() {
  // obtain the start and end points
  
  u16 cam_start_angle = cam.rot.x - vp.half_vfov - (128 << 8);
  // u16 cam_end_angle = cam.rot.x + vp.half_vfov - (128 << 8);
  
  int grd_y0 = (u16)(sky.start_angle - (128 << 8)) - cam_start_angle;
  int grd_y1 = (u16)(sky.end_angle - (128 << 8)) - cam_start_angle;
  
  // turn the angles to screen coords
  
  grd_y0 = fp_mul(grd_y0, sky.fp_deg2px);
  grd_y1 = fp_mul(grd_y1, sky.fp_deg2px);
  
  int grd_y0i = grd_y0 >> FP;
  int grd_y1i = grd_y1 >> FP;
  
  fixed r0 = sky.start_color_ch.r;
  fixed g0 = sky.start_color_ch.g;
  fixed b0 = sky.start_color_ch.b;
  
  fixed r1 = sky.end_color_ch.r;
  fixed g1 = sky.end_color_ch.g;
  fixed b1 = sky.end_color_ch.b;
  
  // determine the number of shading levels by obtaining the channel with the larger difference
  
  int num_levels = max_c(max_c(r1 - r0, g1 - g0), b1 - b0);
  
  // calculate the deltas for each color and for the gradient
  
  fixed dr_grd, dg_grd, db_grd, grd_dy, dy_grd;
  if (grd_y1i > 0 && grd_y0i < SCREEN_HEIGHT) {
    // fixed rdy = rc_fp_div_luts_a(1 >> 20, grd_y1i - grd_y0i);
    // unsigned integer division
    fixed rc_v = div_luts(num_levels); // 24 bit result
    dr_grd = ((r1 - r0) * rc_v) >> 12;
    dg_grd = ((g1 - g0) * rc_v) >> 12;
    db_grd = ((b1 - b0) * rc_v) >> 12;
    // grd_dy = rc_fp_div_luts_a(num_levels, grd_y1i - grd_y0i);
    // signed integer division
    grd_dy = div_luts_s(grd_y1i - grd_y0i); // 24 bit result
    grd_dy = fp_mul(num_levels, grd_dy) >> 12;
    // dy_grd = rc_fp_div_luts_a(grd_y1i - grd_y0i, num_levels) >> FP;
    // unsigned integer division
    // dy_grd = div_luts(num_levels); // 24 bit result
    dy_grd = fp_mul(grd_y1i - grd_y0i, rc_v) >> 12;
  } else {
    dr_grd = 0;
    dg_grd = 0;
    db_grd = 0;
    grd_dy = 0;
    dy_grd = 0;
  }
  
  r0 <<= FP;
  g0 <<= FP;
  b0 <<= FP;
  
  int grd_offset = 0;
  
  // clipping
  
  if (grd_y0i < 0 && grd_y1i > 0) {
    //fixed s = rc_fp_div_luts_a(grd_y1 - 0, grd_y1 - grd_y0);
    //r0 = r0 + fp_mul(s, r1 - r0);
    fixed rc_v = -grd_y0i * grd_dy;
    r0 = r0 + fp_mul(dr_grd, rc_v);
    g0 = g0 + fp_mul(dg_grd, rc_v);
    b0 = b0 + fp_mul(db_grd, rc_v);
    grd_offset = ((rc_v & ((1 << FP) - 1)) * dy_grd) >> FP;
    grd_y0i = 0;
  } else
  if (grd_y1i <= 0) {
    grd_y1i = 0;
    r0 = r1 << FP;
    g0 = g1 << FP;
    b0 = b1 << FP;
  }
  
  if (grd_y0i < SCREEN_HEIGHT && grd_y1i > SCREEN_HEIGHT) {
    grd_y1i = SCREEN_HEIGHT - 1;
  }
  
  // draw the gradient
  
  fixed sr = r0;
  fixed sg = g0;
  fixed sb = b0;
  
  u32 color = dup16(((sb >> FP) << 10) | ((sg >> FP) << 5) | (sr >> FP));
  u16 *vid_y_offset = screen;
  
  int i = 0;
  while (i < SCREEN_HEIGHT) {
    if (i >= grd_y0i && i < grd_y1i) {
      while (grd_offset < dy_grd && i < SCREEN_HEIGHT) {
        memset32(vid_y_offset, color, SCREEN_HALF_WIDTH);

        vid_y_offset += SCREEN_WIDTH;
        grd_offset++;
        i++;
      }
      
      sr += dr_grd;
      sg += dg_grd;
      sb += db_grd;
      
      color = dup16(((sb >> FP) << 10) | ((sg >> FP) << 5) | (sr >> FP));
      grd_offset = 0;
    } else {
      memset16(vid_y_offset, color, SCREEN_WIDTH);
      
      vid_y_offset += SCREEN_WIDTH;
      i++;
    }
  }
}