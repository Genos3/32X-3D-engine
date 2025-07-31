#include "common.h"

#if ENABLE_Z_BUFFER
  void draw_line_zb(fixed x0, fixed y0, fixed z0, fixed x1, fixed y1, fixed z1, u16 color) {
    // unsigned fixed point division
    #if !DIV_LUT_ENABLE
      z0 = fp_div(1 << FP, z0);
      z1 = fp_div(1 << FP, z1);
    #else
      z0 = div_lut[(fixed_u)z0 >> 8] << 8; // .16 result
      z1 = div_lut[(fixed_u)z1 >> 8] << 8; // .16 result
    #endif
    
    // if x0 > x1 exchange the vertices in order to always draw from left to right
    
    if (x0 > x1) {
      int t = x1;
      x1 = x0;
      x0 = t;
      t = y1;
      y1 = y0;
      y0 = t;
      t = z1;
      z1 = z0;
      z0 = t;
    }
    
    if (x0 >= SCREEN_WIDTH_FP) x0--;
    if (x1 >= SCREEN_WIDTH_FP) x1--;
    if (y0 >= SCREEN_HEIGHT_FP) y0--;
    if (y1 >= SCREEN_HEIGHT_FP) y1--;
    
    uint adx_i = (x1 >> FP) - (x0 >> FP);
    int dy_i = (y1 >> FP) - (y0 >> FP);
    fixed dz = z1 - z0;
    
    if (!adx_i && !dy_i) return;
    
    int vid_y_inc;
    if (dy_i >= 0) {
      vid_y_inc = SCREEN_WIDTH;
    } else {
      vid_y_inc = -SCREEN_WIDTH;
    }
    
    uint ady_i = abs_c(dy_i);
    fixed sz = z0;
    
    int y0_i = y0 >> FP;
    
    int screen_y_offset = y0_i * SCREEN_WIDTH;
    u16 *vid_y_offset = screen + screen_y_offset;
    
    if (ady_i >= adx_i) { // dy > dx
      fixed dxdy, dzdy;
      if (ady_i && adx_i) {
        // unsigned integer division
        #if !DIV_LUT_ENABLE
          fixed rdy = fp_div(1, ady_i);
        #else
          fixed rdy = div_lut[ady_i]; // .16 result
        #endif
        dxdy = adx_i * rdy; // always positive
        dzdy = fp_mul(dz, rdy);
      } else {
        dxdy = 0;
        dzdy = 0;
      }
      
      int height = ady_i;
      
      fixed dty = y0 - fp_trunc(y0); // sub-pixel precision
      fixed sx = x0 + fp_mul(dxdy, dty);
      
      while (height >= 0) {
        int sx_i = sx >> FP;
        
        if (sx_i >= SCREEN_WIDTH) {
          sx_i = SCREEN_WIDTH - 1;
        }
        
        if (sz > z_buffer[screen_y_offset + sx_i]) {
          *(vid_y_offset + sx_i) = color;
          z_buffer[screen_y_offset + sx_i] = sz;
        }
        
        sx += dxdy;
        sz += dzdy;
        screen_y_offset += vid_y_inc;
        vid_y_offset += vid_y_inc;
        
        height--;
      }
    } else { // dx > dy
      fixed dydx, dzdx;
      if (ady_i && adx_i) {
        // unsigned integer division
        #if !DIV_LUT_ENABLE
          fixed rdx = fp_div(1, adx_i);
        #else
          fixed rdx = div_lut[adx_i]; // .16 result
        #endif
        dydx = ady_i * rdx; // always positive
        dzdx = fp_mul(dz, rdx);
      } else {
        dydx = 0;
        dzdx = 0;
      }
      
      int width = adx_i;
      
      fixed dtx = x0 - fp_trunc(x0); // sub-pixel precision
      fixed sy = 0 + fp_mul(dydx, dtx);
      
      int x0_i = x0 >> FP;
      
      u16 *vid_pnt = vid_y_offset + x0_i;
      int screen_offset = screen_y_offset + x0_i;
      
      while (width > 0) {
        if (sz > z_buffer[screen_offset]) {
          *vid_pnt = color;
          z_buffer[screen_offset] = sz;
        }
        
        vid_pnt++;
        screen_offset++;
        sy += dydx;
        sz += dzdx;
        
        if (sy >= 1 << FP) {
          sy -= 1 << FP;
          screen_offset += vid_y_inc;
          vid_pnt += vid_y_inc;
        }
        
        width--;
      }
    }
  }
  
  inline void set_pixel_zb(int x, int y, fixed z, u16 color) {
    // if (x < 0 || x >= vp.screen_width || y < 0 || y >= vp.screen_height || z < 0) return;
    // unsigned fixed point division
    #if !DIV_LUT_ENABLE
      z = fp_div(1 << FP, z);
    #else
      z = div_lut[(fixed_u)z >> 8] << 8; // .16 result
    #endif
    
    int screen_offset = y * SCREEN_WIDTH + x;
    if (z <= z_buffer[screen_offset]) return;
    
    #if defined(PC) && PALETTE_MODE
      screen[screen_offset] = pc_palette[color];
    #else
      screen[screen_offset] = color;
    #endif
    z_buffer[screen_offset] = z;
  }
  
  void draw_sprite_zb(g_poly_t *poly) {
    // obtain the top/left vertex
    
    u32 vt_0 = 0;
    
    for (int i = 1; i < 4; i++) {
      if (poly->vertices[i].y <= poly->vertices[vt_0].y && poly->vertices[i].x <= poly->vertices[vt_0].x) {
        vt_0 = i;
      }
    }
    
    fixed vt_rz[8];
    
    for (int i = 0; i < 4; i++) {
      // unsigned fixed point division
      #if !DIV_LUT_ENABLE
        vt_rz[i] = fp_div(1 << FP, poly->vertices[i].z);
      #else
        vt_rz[i] = div_lut[poly->vertices[i].z << 8] >> 8;  // .16 result
      #endif
    }
    
    u32 vt_1 = vt_0 + 2;
    if (vt_1 >= 4) {
      vt_1 -= 4;
    }
    
    // calculate the deltas
    
    u32 dx = (poly->vertices[vt_1].x >> FP) - (poly->vertices[vt_0].x >> FP);
    fixed du = poly->vertices[vt_1].u - poly->vertices[vt_0].u;
    
    // unsigned integer division
    #if !DIV_LUT_ENABLE
      fixed dudx = du / dx;
    #else
      fixed rdx = div_lut[dx]; // + 1 // .16 result
      fixed dudx = fp_mul(du, rdx);
    #endif
    
    u32 dy = (poly->vertices[vt_1].y >> FP) - (poly->vertices[vt_0].y >> FP);
    fixed dv = poly->vertices[vt_1].v - poly->vertices[vt_0].v;
    
    // unsigned integer division
    #if !DIV_LUT_ENABLE
      fixed dvdy = dv / dy;
    #else
      fixed rdy = div_lut[dy]; // .16 result
      fixed dvdy = fp_mul(dv, rdy);
    #endif
    
    // initialize the side variables
    
    #if ENABLE_SUB_TEXEL_ACC
      fixed dtx = poly->vertices[vt_0].x - fp_trunc(poly->vertices[vt_0].x);
      fixed_u su_l = poly->vertices[vt_0].u + fp_mul(dudx, dtx);
    #else
      fixed_u su_l = poly->vertices[vt_0].u;
    #endif
    fixed_u sv_l = poly->vertices[vt_0].v; // + fp_mul(dvdy, dty);
    
    fixed sz = vt_rz[0];
    
    u32 height = dy;
    
    #if POLY_X_CLIPPING
      if (poly->vertices[vt_0].x < 0) {
        dx += poly->vertices[vt_0].x >> FP;
        poly->vertices[vt_0].x = 0;
      }
      if (poly->vertices[vt_1].x > SCREEN_WIDTH_FP) {
        dx -= (poly->vertices[vt_1].x >> FP) - SCREEN_WIDTH;
      }
    #endif
    
    // set the screen offset for the start of the first scanline
    
    u32 y0_i = poly->vertices[vt_0].y >> FP;
    u32 x0_i = poly->vertices[vt_0].x >> FP;
    
    u32 screen_y_offset = y0_i * SCREEN_WIDTH + x0_i;
    u16 *vid_y_offset = screen + screen_y_offset;
    
    // Y loop
    
    while (height > 0) {
      fixed_u su = su_l;
      u32 sv_i = sv_l >> FP;
      
      // set the pointers for the start and the end of the scanline
      
      u16 *vid_pnt = vid_y_offset;
      u16 *end_pnt = vid_y_offset + dx;
      u32 screen_offset = screen_y_offset;
      
      // scanline loop
      
      while (vid_pnt < end_pnt) {
        if (sz > z_buffer[screen_offset]) {
          int su_i = su >> FP;
          u8 tx_color = (u8)poly->texture_image[(sv_i << poly->texture_width_bits) + su_i];
          
          if (tx_color) { // tx_color != cfg.tx_alpha_cr
            tx_color = (tx_color << LIGHT_GRD_BITS) + poly->final_light_factor;
            *vid_pnt = poly->cr_palette_tx_idx[tx_color];
            
            z_buffer[screen_offset] = sz;
          }
        }
        
        vid_pnt++;
        screen_offset++;
        su += dudx;
      }
      
      sv_l += dvdy;
      vid_y_offset += SCREEN_WIDTH;
      screen_y_offset += SCREEN_WIDTH;
      height--;
    }
  }
  
  void draw_poly_zb(g_poly_t *poly) {
    u16 color = poly->color;
    
    // obtain the top and bottom vertices
    
    int sup_vt = 0;
    #if CHECK_POLY_HEIGHT
      int inf_vt = 0;
    #endif
    
    for (int i = 1; i < poly->num_vertices; i++) {
      if (poly->vertices[i].y < poly->vertices[sup_vt].y) {
        sup_vt = i;
      }
      #if CHECK_POLY_HEIGHT
        if (poly->vertices[i].y > poly->vertices[inf_vt].y) {
          inf_vt = i;
        }
      #endif
    }
    
    // if the polygon doesn't have height return
    
    #if CHECK_POLY_HEIGHT
      if (poly->vertices[sup_vt].y >> FP == poly->vertices[inf_vt].y >> FP) return;
    #endif
    
    fixed vt_rz[8];
    
    for (int i = 0; i < poly->num_vertices; i++) {
      // unsigned fixed point division
      #if !DIV_LUT_ENABLE
        vt_rz[i] = fp_div(1 << FP, poly->vertices[i].z);
      #else
        vt_rz[i] = div_lut[poly->vertices[i].z << 8] >> 8; // .16 result
      #endif
    }
    
    // initialize the edge variables
    
    fixed sx_l, sz_l, sx_r, sz_r;
    fixed dxdy_l, dzdy_l, dxdy_r, dzdy_r;
    
    int curr_vt_l = sup_vt;
    int curr_vt_r = sup_vt;
    
    int height_l = 0;
    int height_r = 0;
    
    // set the screen offset for the start of the first scanline
    
    u32 y0_i = poly->vertices[sup_vt].y >> FP;
    
    u32 screen_y_offset = y0_i * SCREEN_WIDTH;
    u16 *vid_y_offset = screen + screen_y_offset;
    
    // main Y loop
    
    while (1) {
      // next edge found on the left side
      
      while (!height_l) {
        int next_vt_l;
        
        if (!poly->flags.is_backface) {
          next_vt_l = curr_vt_l - 1;
        
          if (next_vt_l < 0) {
            next_vt_l = poly->num_vertices - 1;
          }
        } else {
          next_vt_l = curr_vt_l + 1;
          
          if (next_vt_l == poly->num_vertices) {
            next_vt_l = 0;
          }
        }
        
        height_l = (poly->vertices[next_vt_l].y >> FP) - (poly->vertices[curr_vt_l].y >> FP);
        
        if (height_l < 0) return;
        
        if (height_l) {
          // calculate the edge deltas
          
          fixed dx = poly->vertices[next_vt_l].x - poly->vertices[curr_vt_l].x;
          fixed dz = vt_rz[next_vt_l] - vt_rz[curr_vt_l];
          
          // unsigned integer division
          #if !DIV_LUT_ENABLE
            fixed rdy = fp_div(1, height_l);
          #else
            fixed rdy = div_lut[height_l]; // .16 result
          #endif
          dxdy_l = fp_mul(dx, rdy);
          dzdy_l = fp_mul(dz, rdy);
          
          // initialize the side variables
          
          fixed dty = poly->vertices[curr_vt_l].y - fp_trunc(poly->vertices[curr_vt_l].y); // sub-pixel precision
          sx_l = poly->vertices[curr_vt_l].x + fp_mul(dxdy_l, dty);
          sz_l = vt_rz[curr_vt_l] + fp_mul(dzdy_l, dty);
        }
        
        curr_vt_l = next_vt_l;
      }
      
      // next edge found on the right side
      
      while (!height_r) {
        int next_vt_r;
        
        if (!poly->flags.is_backface) {
          next_vt_r = curr_vt_r + 1;
          
          if (next_vt_r == poly->num_vertices) {
            next_vt_r = 0;
          }
        } else {
          next_vt_r = curr_vt_r - 1;
        
          if (next_vt_r < 0) {
            next_vt_r = poly->num_vertices - 1;
          }
        }
        
        height_r = (poly->vertices[next_vt_r].y >> FP) - (poly->vertices[curr_vt_r].y >> FP);
        
        if (height_r < 0) return;
        
        if (height_r) {
          // calculate the edge deltas
          
          fixed dx = poly->vertices[next_vt_r].x - poly->vertices[curr_vt_r].x;
          fixed dz = vt_rz[next_vt_r] - vt_rz[curr_vt_r];
          
          // unsigned integer division
          #if !DIV_LUT_ENABLE
            fixed rdy = fp_div(1, height_r);
          #else
            fixed rdy = div_lut[height_r]; // .16 result
          #endif
          dxdy_r = fp_mul(dx, rdy);
          dzdy_r = fp_mul(dz, rdy);
          
          // initialize the side variables
          
          fixed dty = poly->vertices[curr_vt_r].y - fp_trunc(poly->vertices[curr_vt_r].y); // sub-pixel precision
          sx_r = poly->vertices[curr_vt_r].x + fp_mul(dxdy_r, dty);
          sz_r = vt_rz[curr_vt_r] + fp_mul(dzdy_r, dty);
        }
        
        curr_vt_r = next_vt_r;
      }
      
      // if the polygon doesn't have height return
      
      if (!height_l && !height_r) return;
      
      // obtain the height to the next vertex on Y
      
      u32 height = min_c(height_l, height_r);
      
      height_l -= height;
      height_r -= height;
      
      // second Y loop
      
      while (height > 0) {
        int sx_l_i = sx_l >> FP;
        int sx_r_i = sx_r >> FP;
        // int sx_r_i = fp_ceil_i(sx_r);
        
        if (sx_l_i >= sx_r_i) goto segment_loop_exit;
        
        // calculate the scanline deltas
        
        fixed dzdx;
        u32 dx = sx_r_i - sx_l_i - 1;
        
        if (dx > 0) {
          // unsigned integer division
          #if !DIV_LUT_ENABLE
            dzdx = fp_div(sz_r - sz_l, dx);
          #else
            fixed rdx = div_lut[dx]; // .16 result
            dzdx = fp_mul(sz_r - sz_l, rdx);
          #endif
        } else {
          dzdx = 0;
        }
        
        // initialize the scanline variables
        
        #if ENABLE_SUB_TEXEL_ACC
          fixed dt = fp_ceil(sx_l) - sx_l; // sub-texel precision
          fixed sz = sz_l + fp_mul(dzdx, dt);
        #else
          fixed sz = sz_l;
        #endif
        
        // X clipping
        
        #if POLY_X_CLIPPING
          if (sx_l_i < 0) {
            // sz += dzdx * (-sx_l_i);
            sx_l_i = 0;
          }
          if (sx_r_i > SCREEN_WIDTH) {
            sx_r_i = SCREEN_WIDTH;
          }
        #endif
        
        // set the pointers for the start and the end of the scanline
        
        u16 *vid_pnt = vid_y_offset + sx_l_i;
        u16 *end_pnt = vid_y_offset + sx_r_i;
        u32 screen_offset = screen_y_offset + sx_l_i;
        
        // scanline loop
        
        while (vid_pnt < end_pnt) {
          if (sz > z_buffer[screen_offset]) {
            *vid_pnt = color;
            z_buffer[screen_offset] = sz;
          }
          
          vid_pnt++;
          screen_offset++;
          sz += dzdx;
        }
        
        segment_loop_exit:
        
        // increment the left and right side variables
        
        sx_l += dxdy_l;
        sx_r += dxdy_r;
        sz_l += dzdy_l;
        sz_r += dzdy_r;
        vid_y_offset += SCREEN_WIDTH;
        screen_y_offset += SCREEN_WIDTH;
        height--;
      }
    }
  }
  
  void draw_poly_tx_affine_zb(g_poly_t *poly) {
    // obtain the top and bottom vertices
    
    int sup_vt = 0;
    #if CHECK_POLY_HEIGHT
      int inf_vt = 0;
    #endif
    
    for (int i = 1; i < poly->num_vertices; i++) {
      if (poly->vertices[i].y < poly->vertices[sup_vt].y) {
        sup_vt = i;
      }
      #if CHECK_POLY_HEIGHT
        if (poly->vertices[i].y > poly->vertices[inf_vt].y) {
          inf_vt = i;
        }
      #endif
    }
    
    // if the polygon doesn't have height return
    
    #if CHECK_POLY_HEIGHT
      if (poly->vertices[sup_vt].y >> FP == poly->vertices[inf_vt].y >> FP) return;
    #endif
    
    fixed vt_rz[8];
    
    for (int i = 0; i < poly->num_vertices; i++) {
      // unsigned fixed point division
      #if !DIV_LUT_ENABLE
        vt_rz[i] = fp_div(1 << FP, poly->vertices[i].z);
      #else
        vt_rz[i] = div_lut[poly->vertices[i].z << 8] >> 8; // .16 result
      #endif
    }
    
    // initialize the edge variables
    
    fixed sx_l, sz_l, sx_r, sz_r;
    fixed dxdy_l, dzdy_l, dxdy_r, dzdy_r;
    u32 su_l_sv_l, su_r_sv_r, dudy_l_dvdy_l, dudy_r_dvdy_r;
    
    int curr_vt_l = sup_vt;
    int curr_vt_r = sup_vt;
    
    int height_l = 0;
    int height_r = 0;
    
    // set the screen offset for the start of the first scanline
    
    u32 y0_i = poly->vertices[sup_vt].y >> FP;
    
    u32 screen_y_offset = y0_i * SCREEN_WIDTH;
    u16 *vid_y_offset = screen + screen_y_offset;
    
    // main Y loop
    
    while (1) {
      // next edge found on the left side
      
      while (!height_l) {
        int next_vt_l;
        
        if (!poly->flags.is_backface) {
          next_vt_l = curr_vt_l - 1;
        
          if (next_vt_l < 0) {
            next_vt_l = poly->num_vertices - 1;
          }
        } else {
          next_vt_l = curr_vt_l + 1;
          
          if (next_vt_l == poly->num_vertices) {
            next_vt_l = 0;
          }
        }
        
        height_l = (poly->vertices[next_vt_l].y >> FP) - (poly->vertices[curr_vt_l].y >> FP);
        
        if (height_l < 0) return;
        
        if (height_l) {
          // calculate the edge deltas
          
          fixed dx = poly->vertices[next_vt_l].x - poly->vertices[curr_vt_l].x;
          fixed du = poly->vertices[next_vt_l].u - poly->vertices[curr_vt_l].u;
          fixed dv = poly->vertices[next_vt_l].v - poly->vertices[curr_vt_l].v;
          fixed dz = vt_rz[next_vt_l] - vt_rz[curr_vt_l];
          
          // unsigned integer division
          #if !DIV_LUT_ENABLE
            fixed rdy = fp_div(1, height_l);
          #else
            fixed rdy = div_lut[height_l]; // .16 result
          #endif
          dxdy_l = fp_mul(dx, rdy);
          dzdy_l = fp_mul(dz, rdy);
          fixed dudy_l = fp_mul(du, rdy);
          fixed dvdy_l = fp_mul(dv, rdy);
          
          dudy_l_dvdy_l = (dvdy_l << 16) | (u16)dudy_l;
          
          // initialize the side variables
          
          fixed dty = poly->vertices[curr_vt_l].y - fp_trunc(poly->vertices[curr_vt_l].y); // sub-pixel precision
          sx_l = poly->vertices[curr_vt_l].x + fp_mul(dxdy_l, dty);
          sz_l = vt_rz[curr_vt_l] + fp_mul(dzdy_l, dty);
          fixed su_l = poly->vertices[curr_vt_l].u + fp_mul(dudy_l, dty);
          fixed sv_l = poly->vertices[curr_vt_l].v + fp_mul(dvdy_l, dty);
          
          su_l_sv_l = (sv_l << 16) | su_l;
        }
        
        curr_vt_l = next_vt_l;
      }
      
      // next edge found on the right side
      
      while (!height_r) {
        int next_vt_r;
        
        if (!poly->flags.is_backface) {
          next_vt_r = curr_vt_r + 1;
          
          if (next_vt_r == poly->num_vertices) {
            next_vt_r = 0;
          }
        } else {
          next_vt_r = curr_vt_r - 1;
        
          if (next_vt_r < 0) {
            next_vt_r = poly->num_vertices - 1;
          }
        }
        
        height_r = (poly->vertices[next_vt_r].y >> FP) - (poly->vertices[curr_vt_r].y >> FP);
        
        if (height_r < 0) return;
        
        if (height_r) {
          // calculate the edge deltas
          
          fixed dx = poly->vertices[next_vt_r].x - poly->vertices[curr_vt_r].x;
          fixed du = poly->vertices[next_vt_r].u - poly->vertices[curr_vt_r].u;
          fixed dv = poly->vertices[next_vt_r].v - poly->vertices[curr_vt_r].v;
          fixed dz = vt_rz[next_vt_r] - vt_rz[curr_vt_r];
          
          // unsigned integer division
          #if !DIV_LUT_ENABLE
            fixed rdy = fp_div(1, height_r);
          #else
            fixed rdy = div_lut[height_r]; // .16 result
          #endif
          dxdy_r = fp_mul(dx, rdy);
          dzdy_r = fp_mul(dz, rdy);
          fixed dudy_r = fp_mul(du, rdy);
          fixed dvdy_r = fp_mul(dv, rdy);
          
          dudy_r_dvdy_r = (dvdy_r << 16) | (u16)dudy_r;
          
          // initialize the side variables
          
          fixed dty = poly->vertices[curr_vt_r].y - fp_trunc(poly->vertices[curr_vt_r].y); // sub-pixel precision
          sx_r = poly->vertices[curr_vt_r].x + fp_mul(dxdy_r, dty);
          sz_r = vt_rz[curr_vt_r] + fp_mul(dzdy_r, dty);
          fixed su_r = poly->vertices[curr_vt_r].u + fp_mul(dudy_r, dty);
          fixed sv_r = poly->vertices[curr_vt_r].v + fp_mul(dvdy_r, dty);
          
          su_r_sv_r = (sv_r << 16) | su_r;
        }
        
        curr_vt_r = next_vt_r;
      }
      
      // if the polygon doesn't have height return
      
      if (!height_l && !height_r) return;
      
      // obtain the height to the next vertex on Y
      
      u32 height = min_c(height_l, height_r);
      
      height_l -= height;
      height_r -= height;
      
      // second Y loop
      
      while (height > 0) {
        int sx_l_i = sx_l >> FP; // ceil
        int sx_r_i = sx_r >> FP;
        
        if (sx_l_i >= sx_r_i) goto segment_loop_exit;
        
        fixed su_l = su_l_sv_l & 0xFFFF;
        fixed sv_l = su_l_sv_l >> 16;
        
        fixed su_r = su_r_sv_r & 0xFFFF;
        fixed sv_r = su_r_sv_r >> 16;
        
        // calculate the scanline deltas
        
        fixed dudx, dvdx, dzdx;
        u32 dx = sx_r_i - sx_l_i - 1;
        
        if (dx > 0) {
          // unsigned integer division
          #if !DIV_LUT_ENABLE
            fixed rdx = fp_div(1, dx);
          #else
            fixed rdx = div_lut[dx]; // .16 result
          #endif
          dudx = fp_mul(su_r - su_l, rdx);
          dvdx = fp_mul(sv_r - sv_l, rdx);
          dzdx = fp_mul(sz_r - sz_l, rdx);
        } else {
          dudx = 0;
          dvdx = 0;
          dzdx = 0;
        }
        
        u32 dudx_dvdx = (dvdx << 16) | (u16)dudx;
        
        // initialize the scanline variables
        
        #if ENABLE_SUB_TEXEL_ACC
          fixed dtx = sx_l - fp_trunc(sx_l); // sub-texel precision
          fixed_u su = su_l + fp_mul(dudx, dtx);
          fixed_u sv = sv_l + fp_mul(dvdx, dtx);
          fixed_u sz = sz_l + fp_mul(dzdx, dt);
          u32 su_sv = (sv << 16) | su;
        #else
          fixed_u sz = sz_l;
          u32 su_sv = su_l_sv_l;
        #endif
        
        // X clipping
        
        #if POLY_X_CLIPPING
          if (sx_l_i < 0) {
            // su += dudx * (-sx_l_i);
            // sv += dvdx * (-sx_l_i);
            // sz += dzdx * (-sx_l_i);
            sx_l_i = 0;
          }
          if (sx_r_i > SCREEN_WIDTH) {
            sx_r_i = SCREEN_WIDTH;
          }
        #endif
        
        // set the pointers for the start and the end of the scanline
        
        u16 *vid_pnt = vid_y_offset + sx_l_i;
        u16 *end_pnt = vid_y_offset + sx_r_i;
        u32 screen_offset = screen_y_offset + sx_l_i;
        
        // scanline loop
        
        if (!poly->flags.has_transparency) {
          while (vid_pnt < end_pnt) {
            if (sz > z_buffer[screen_offset]) {
              u32 su_sv_i = su_sv >> 8;
              su_sv_i &= poly->texture_size_wh_s;
              u32 su_i = su_sv_i & 0xFF;
              u32 sv_i = su_sv_i >> 16;
              
              u16 tx_color = poly->texture_image[(sv_i << poly->texture_width_bits) + su_i];
              tx_color = (tx_color << LIGHT_GRD_BITS) + poly->final_light_factor;
              
              *vid_pnt = poly->cr_palette_tx_idx[(u8)tx_color];
              z_buffer[screen_offset] = sz;
            }
            
            vid_pnt++;
            screen_offset++;
            su_sv += dudx_dvdx;
            sz += dzdx;
          }
        } else {
          while (vid_pnt < end_pnt) {
            if (sz > z_buffer[screen_offset]) {
              u32 su_sv_i = su_sv >> 8;
              su_sv_i &= poly->texture_size_wh_s;
              u32 su_i = su_sv_i & 0xFF;
              u32 sv_i = su_sv_i >> 16;
              
              u16 tx_color = poly->texture_image[(sv_i << poly->texture_width_bits) + su_i];
              
              if (tx_color) {
                tx_color = (tx_color << LIGHT_GRD_BITS) + poly->final_light_factor;
                
                *vid_pnt = poly->cr_palette_tx_idx[(u8)tx_color];
                z_buffer[screen_offset] = sz;
              }
            }
            
            vid_pnt++;
            screen_offset++;
            su_sv += dudx_dvdx;
            sz += dzdx;
          }
        }
        
        segment_loop_exit:
        
        // increment the left and right side variables
        
        sx_l += dxdy_l;
        sz_l += dzdy_l;
        sx_r += dxdy_r;
        sz_r += dzdy_r;
        su_l_sv_l += dudy_l_dvdy_l;
        su_r_sv_r += dudy_r_dvdy_r;
        vid_y_offset += SCREEN_WIDTH;
        screen_y_offset += SCREEN_WIDTH;
        height--;
      }
    }
  }
#endif