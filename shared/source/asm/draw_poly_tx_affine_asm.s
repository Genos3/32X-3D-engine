#include "asm_common.inc"

#define UNROLL_SCANLINE_LOOP 0

.section .sdram
.global _draw_poly_tx_affine_asm
.type _draw_poly_tx_affine_asm STT_FUNC

.extern _screen
.extern _div_lut

// pixel macro

// r0: sv_i (scratch)
// r2: *vid_pnt
// r3: *end_pnt
// r4: su_sv
// r5: dudx_dvdx
// r6: su_i, tx_color (scratch)

// r11: texture_size_wh_s
// r12: texture_image
// r13: cr_palette_tx_idx
// r14: final_light_factor

.macro set_pixel_not_transparent_8
  mov r4, r0
  shlr8 r0  // u32 su_sv_i = su_sv >> 8
  and r11, r0  // su_sv_i &= texture_size_wh_s
  mov r0, r6  // u32 su_i = su_sv_i & 0xFF
  shlr16 r0  // u32 sv_i = su_sv_i >> 16
  
  shll2 r0
  shll r0
  add r6, r0
  shll r0
  mov.w @(r0, r12), r6  // u16 tx_color = texture_image[(sv_i << 3) + su_i];
  
  #if ASM_HAS_LIGHTING
    #if PALETTE_MODE
      shll2 r6  // LIGHT_GRD_BITS
      add r14, r6  // txcolor = (txcolor << LIGHT_GRD_BITS) + final_light_factor; // 16-bit
    #else
      shll2 r6
      shll r6  // LIGHT_GRD_BITS + 1
      add r14, r6  // txcolor = (txcolor << (LIGHT_GRD_BITS + 1)) + final_light_factor; // 16-bit
    #endif
  #endif
  
  #if PALETTE_MODE
    mov.w r6, @-r2  // *--vid_pnt = tx_color;
  #else
    mov.w @(r6, r13), r6  // textures->cr_palette_tx_idx[tx_color];
    mov.w r6, @-r2  // *--vid_pnt = tx_color;
  #endif
  
  add r5, r4  // su_sv += dudx_dvdx;
.endm

.macro set_pixel_not_transparent_16
  mov r4, r0
  shlr8 r0  // uint su_sv_i = su_sv >> 8
  and r11, r0  // uint su_sv_i &= texture_size_wh_s
  mov r0, r6  // uint su_i = su_sv_i & 0xFF
  shlr16 r0  // uint sv_i = su_sv_i >> 16
  
  shll2 r0
  shll2 r0
  add r6, r0
  shll r0
  mov.w @(r0, r12), r6  // u16 tx_color = texture_image[(sv_i << 4) + su_i];
  
  #if ASM_HAS_LIGHTING
    #if PALETTE_MODE
      shll2 r6  // LIGHT_GRD_BITS
      add r14, r6  // txcolor = (txcolor << LIGHT_GRD_BITS) + final_light_factor; // 16-bit
    #else
      shll2 r6
      shll r6  // LIGHT_GRD_BITS + 1
      add r14, r6  // txcolor = (txcolor << (LIGHT_GRD_BITS + 1)) + final_light_factor; // 16-bit
    #endif
  #endif
  
  #if PALETTE_MODE
    mov.w r6, @-r2  // *--vid_pnt = tx_color;
  #else
    mov.w @(r6, r13), r6  // textures->cr_palette_tx_idx[tx_color];
    mov.w r6, @-r2  // *--vid_pnt = tx_color;
  #endif
  
  add r5, r4  // su_sv += dudx_dvdx;
.endm

.macro set_pixel_not_transparent_32
  mov r4, r0
  shlr8 r0  // uint su_sv_i = su_sv >> 8
  and r11, r0  // uint su_sv_i &= texture_size_wh_s
  extu.b r0, r6  // uint su_i = su_sv_i & 0xFF
  shlr16 r0  // uint sv_i = su_sv_i >> 16
  
  shll2 r0
  shll2 r0
  shll r0
  add r6, r0
  shll r0
  mov.w @(r0, r12), r6  // u16 tx_color = texture_image[(sv_i << 5) + su_i];
  
  #if ASM_HAS_LIGHTING
    #if PALETTE_MODE
      shll2 r6  // LIGHT_GRD_BITS
      add r14, r6  // txcolor = (txcolor << LIGHT_GRD_BITS) + final_light_factor; // 16-bit
    #else
      shll2 r6
      shll r6  // LIGHT_GRD_BITS + 1
      add r14, r6  // txcolor = (txcolor << (LIGHT_GRD_BITS + 1)) + final_light_factor; // 16-bit
    #endif
  #endif
  
  #if PALETTE_MODE
    mov.w r6, @-r2  // *--vid_pnt = tx_color;
  #else
    mov.w @(r6, r13), r6  // textures->cr_palette_tx_idx[tx_color];
    mov.w r6, @-r2  // *--vid_pnt = tx_color;
  #endif
  
  add r5, r4  // su_sv += dudx_dvdx;
.endm

.macro set_pixel_has_transparency_8
  mov r4, r0
  shlr8 r0  // uint su_sv_i = su_sv >> 8
  and r11, r0  // uint su_sv_i &= texture_size_wh_s
  extu.b r0, r6  // uint su_i = su_sv_i & 0xFF
  shlr16 r0  // uint sv_i = su_sv_i >> 16
  
  shll2 r0
  shll r0
  add r6, r0
  shll r0
  mov.w @(r0, r12), r6  // u16 tx_color = texture_image[(sv_i << 3) + su_i];
  
  cmp/pl r6  // if (tx_color)
  bf 1f
    #if ASM_HAS_LIGHTING
      #if PALETTE_MODE
        shll2 r6  // LIGHT_GRD_BITS
        add r14, r6  // txcolor = (txcolor << LIGHT_GRD_BITS) + final_light_factor; // 16-bit
      #else
        shll2 r6
        shll r6  // LIGHT_GRD_BITS + 1
        add r14, r6  // txcolor = (txcolor << (LIGHT_GRD_BITS + 1)) + final_light_factor; // 16-bit
      #endif
    #endif
    
    #if PALETTE_MODE
      mov.w r6, @r2  // *vid_pnt = tx_color;
    #else
      mov.w @(r6, r13), r6  // textures->cr_palette_tx_idx[tx_color];
      mov.w r6, @r2  // *vid_pnt = tx_color;
    #endif
  1:
  
  add #-2, r2  // vid_pnt--;
  
  add r5, r4  // su_sv += dudx_dvdx;
.endm

.macro set_pixel_has_transparency_16
  mov r4, r0
  shlr8 r0  // su_sv_i = su_sv >> 8
  and r11, r0  // su_sv_i &= texture_size_wh_s
  extu.b r0, r6  // su_i = su_sv_i & 0xFF
  shlr16 r0  // sv_i = su_sv_i >> 16
  
  shll2 r0
  shll2 r0
  add r6, r0
  shll r0
  mov.w @(r0, r12), r6  // u16 tx_color = texture_image[(sv_i << 4) + su_i];
  
  cmp/pl r6  // if (tx_color)
  bf 1f
    #if ASM_HAS_LIGHTING
      #if PALETTE_MODE
        shll2 r6  // LIGHT_GRD_BITS
        add r14, r6  // txcolor = (txcolor << LIGHT_GRD_BITS) + final_light_factor; // 16-bit
      #else
        shll2 r6
        shll r6  // LIGHT_GRD_BITS + 1
        add r14, r6  // txcolor = (txcolor << (LIGHT_GRD_BITS + 1)) + final_light_factor; // 16-bit
      #endif
    #endif
    
    #if PALETTE_MODE
      mov.w r6, @r2  // *vid_pnt = tx_color;
    #else
      mov.w @(r6, r13), r6  // textures->cr_palette_tx_idx[tx_color];
      mov.w r6, @r2  // *vid_pnt = tx_color;
    #endif
  1:
  
  add #-2, r2  // vid_pnt--;
  
  add r5, r4  // su_sv += dudx_dvdx;
.endm

.macro set_pixel_has_transparency_32
  mov r4, r0
  shlr8 r0  // su_sv_i = su_sv >> 8
  and r11, r0  // su_sv_i &= texture_size_wh_s
  extu.b r0, r6  // su_i = su_sv_i & 0xFF
  shlr16 r0  // sv_i = su_sv_i >> 16
  
  shll2 r0
  shll2 r0
  shll r0
  add r6, r0
  shll r0
  mov.w @(r0, r12), r6  // u16 tx_color = texture_image[(sv_i << 5) + su_i];
  
  cmp/pl r6  // if (tx_color)
  bf 1f
    #if ASM_HAS_LIGHTING
      #if PALETTE_MODE
        shll2 r6  // LIGHT_GRD_BITS
        add r14, r6  // txcolor = (txcolor << LIGHT_GRD_BITS) + final_light_factor; // 16-bit
      #else
        shll2 r6
        shll r6  // LIGHT_GRD_BITS + 1
        add r14, r6  // txcolor = (txcolor << (LIGHT_GRD_BITS + 1)) + final_light_factor; // 16-bit
      #endif
    #endif
    
    #if PALETTE_MODE
      mov.w r6, @r2  // *vid_pnt = tx_color;
    #else
      mov.w @(r6, r13), r6  // textures->cr_palette_tx_idx[tx_color];
      mov.w r6, @r2  // *vid_pnt = tx_color;
    #endif
  1:
  
  add #-2, r2  // vid_pnt--;
  
  add r5, r4  // su_sv += dudx_dvdx;
.endm

// void draw_poly_tx_affine_asm(poly_t *poly)

_draw_poly_tx_affine_asm:
  mov.l r8, @-sp
  mov.l r9, @-sp
  mov.l r10, @-sp
  mov.l r11, @-sp
  mov.l r12, @-sp
  mov.l r13, @-sp
  mov.l r14, @-sp
  
  // stack, local variables
  
  .struct 0
    st.poly:
      .space 4
    
    st.num_vertices_s_ptr:
      .space 4  // vertices + num_vertices - 1
    
    st.curr_vt_l:
      .space 4
    st.curr_vt_r:
      .space 4
    st.height_l:
      .space 4
    st.height_r:
      .space 4
    st.height:
      .space 4
    st.vid_y_offset:
      .space 4
    
    st.sx_l:
      .space 4
    st.sx_r:
      .space 4
    st.su_l_sv_l:
      .space 4
    st.su_r_sv_r:
      .space 4
    st.dxdy_l:
      .space 4
    st.dxdy_r:
      .space 4
    st.dudy_l_dvdy_l:
      .space 4
    st.dudy_r_dvdy_r:
      .space 4
    
    // 0x10
    st.poly_flags:
      .space 4
    
    stack_size:
  .previous
  
  add #-stack_size, sp
  
  mov.l r4, @sp  // poly
  
  mov.l pool.poly_flags_offset, r0
  add r4, r0  // poly + poly_flags_offset
  mov.l @r0, r14  // poly.flags
  mov.l @(poly.num_vertices - poly.flags, r0), r13  // poly.num_vertices
  
  mov #st.poly_flags, r0
  mov.l r14, @(r0, sp)
  
  // obtain the top and bottom vertices
  
  // read only:
  // r4: poly
  // r13: num_vertices
  // r14: flags
  
  // volatile:
  // r1: i
  // r2: vertices + j
  // r3: vertices + sup_vt
  // r5: vertices[j].y
  // r6: vertices[sup_vt].y
  // r7: vertices[inf_vt].y, inf_y
  
  add #-1, r13  // num_vertices - 1
  mov r13, r1  // i = num_vertices - 1
  mov r4, r2  // j = vertices
  mov r4, r3  // sup_vt = vertices
  
  mov.l @(4, r2), r6  // vertices[sup_vt].y
  #if CHECK_POLY_HEIGHT
    mov r6, r7  // vertices[inf_vt].y
  #endif
  
  .loop_sort_y:
    add #(5 * 4), r2  // j++
    mov.l @(4, r2), r5  // vertices[j].y
    
    cmp/ge r6, r5  // if (vertices[j].y < vertices[sup_vt].y)
    bt 1f
      mov r2, r3  // sup_vt = j
      mov r5, r6  // vertices[sup_vt].y = vertices[j].y
    1:
    
    #if CHECK_POLY_HEIGHT
      cmp/gt r7, r5  // if (vertices[j].y > vertices[inf_vt].y)
      bf 1f
        mov r5, r7  // vertices[inf_vt].y = vertices[j].y
      1:
    #endif
    
    dt r1  // i--
    bf .loop_sort_y  // i != 0
  
  // if the polygon doesn't have height return
  
  #if CHECK_POLY_HEIGHT
    shlr16 r6
    shlr16 r7
    cmp/eq r7, r6
    bf 1f
      bra return  // if (poly->vertices[sup_vt].y >> FP == poly->vertices[inf_vt].y >> FP) return;
      nop
    1:
  #endif
  
  // initialize the edge variables
  
  // prev:
  // r3: vertices + i
  // r4: poly
  
  // curr:
  // r3: *curr_vt_l / *curr_vt_r
  // r4: height_l
  // r5: height_r
  
  // r14: vid_y_offset
  
  mov r13, r1
  shll2 r1
  add r13, r1  // num_vertices * 5 (x, y, z, u, v)
  shll2 r1
  add r4, r1  // vertices + num_vertices - 1
  mov.l r1, @(st.num_vertices_s_ptr, sp)
  
  // mov r3, r3  // int *curr_vt_l = sup_vt;
  
  mov.l r3, @(st.curr_vt_l, sp)
  mov.l r3, @(st.curr_vt_r, sp)
  
  mov #0, r4  // int height_l = 0;
  mov #0, r5  // int height_r = 0;
  
  // set the screen offset for the start of the first scanline
  
  mov.l @(4, r3), r0
  shlr16 r0  // y0_i = vertices[sup_vt].y >> FP;
  
  mov.l pool.frame_width, r1
  mov.l pool.screen, r2
  mov.l @r2, r2
  mulu r0, r1
  sts macl, r14
  add r2, r14  // u16 *vid_y_offset = screen + y0_i * SCREEN_WIDTH;
  
  mov.l r14, @(st.vid_y_offset, sp)
  
  // main Y loop
  
  .y_main_loop:
    // next edge found on the left side
    
    cmp/pl r4  // while (!height_l)
    bt .left_edge_exit
      
      // r1: vertices[curr_vt_l].y, dty
      // r2: *next_vt_l, rdy
      
      // r3: *curr_vt_l
      // r4: height_l
      // r5: height_r
      
      // r6: dx, dxdy_l
      // r7: du, dudy_l
      // r8: dv, dvdy_l
      // r9: sx_l
      // r10: sx_r
      // r11: su_l_sv_l
      // r12: su_r_sv_r
      
      mov.l @(st.curr_vt_l, sp), r3
      
      .left_edge_loop_repeat:
      
      mov r3, r2
      mov #st.poly_flags, r0
      mov.l @(r0, sp), r0
      tst #poly.is_backface.bit, r0  // if (poly->flags.is_backface)
      bt 1f
        add #(5 * 4), r2   // int *next_vt_l = curr_vt_l + 1;
        mov.l @(st.num_vertices_s_ptr, sp), r0
        cmp/gt r0, r2  // if (next_vt_l > poly->num_vertices - 1)
        bf 2f
          mov.l @sp, r2  // next_vt_l = poly;
        bra 2f
        nop
      1:
        add #-(5 * 4), r2   // int *next_vt_l = curr_vt_l - 1;
        mov.l @sp, r0
        cmp/ge r0, r2  // if (next_vt_l < vertices)
        bt 2f
          mov.l @(st.num_vertices_s_ptr, sp), r2  // next_vt_l = vertices + poly->num_vertices - 1;
      2:  
      
      mov.l @(4, r3), r1  // vertices[curr_vt_l].y
      mov.l @(4, r2), r4  // vertices[next_vt_l].y
      
      mov r1, r0
      shlr16 r4
      shlr16 r0
      sub r0, r4  // height_l = (vertices[next_vt_l].y >> FP) - (vertices[curr_vt_l].y >> FP);
      
      cmp/pz r4  // if (height_l < 0) return;
      bt 1f
        bra return
        nop
      1:
      
      cmp/pl r4
      bt 1f
        bra .left_edge_loop_repeat  // if (!height_l)
        mov r2, r3  // curr_vt_l = next_vt_l; // the delay slot makes this execute before the jump
      1:
        // if (height_l)
        
        // calculate the edge deltas
        
        mov.l @r3, r9  // vertices[curr_vt_l].x
        mov.l @r2, r6  // vertices[next_vt_l].x
        sub r9, r6  // fixed dx = vertices[next_vt_l].x - vertices[curr_vt_l].x;
        
        mov.l @((3 * 4), r3), r11  // vertices[curr_vt_l].u
        mov.l @((3 * 4), r2), r7  // vertices[next_vt_l].u
        sub r11, r7  // fixed du = vertices[next_vt_l].u - vertices[curr_vt_l].u;
        
        mov.l @((4 * 4), r3), r13  // vertices[curr_vt_l].v
        mov.l @((4 * 4), r2), r8  // vertices[next_vt_l].v
        sub r13, r8  // fixed dv = vertices[next_vt_l].v - vertices[curr_vt_l].v;
        
        mov.l r2, @(st.curr_vt_l, sp)  // curr_vt_l = next_vt_l;
        
        // unsigned integer division
        
        #if !DIV_LUT_ENABLE
          mov #1, r2
          shll16 r2
          mov r4, r0
          shll16 r0
          
          div0u
          
          .rept 16
            div1 r0, r2
          .endr
          
          rotcl r2
          extu.w r2, r2
        #else
          mov.l pool.div_lut, r3
          
          mov r4, r0
          shll r0  // u16
          mov.l @(r0, r3), r2  // fixed rdy = div_lut[height_l]; // .16 result
        #endif
        
        dmuls.l r6, r2
        sts mach, r0
        sts macl, r6
        xtrct r0, r6  // dxdy_l = fp_mul(dx, rdy);
        
        mov.l r6, @(st.dxdy_l, sp)
        
        dmuls.l r7, r2
        sts mach, r0
        sts macl, r7
        xtrct r0, r7  // dudy_l = fp_mul(du, rdy);
        
        dmuls.l r8, r2
        sts mach, r0
        sts macl, r8
        xtrct r0, r8  // dvdy_l = fp_mul(dv, rdy);
        
        mov r8, r0
        shll16 r0
        extu.w r7, r2
        or r2, r0  // dudy_l_dvdy_l = (dvdy_l << 16) | (u16)dudy_l
        
        mov.l r0, @(st.dudy_l_dvdy_l, sp)
        
        // initialize the side variables
        
        // sub-pixel precision
        
        mov r1, r0
        fp_trunc r0  // fp_trunc(vertices[curr_vt_l].y)
        sub r0, r1  // fixed dty = vertices[curr_vt_l].y - fp_trunc(vertices[curr_vt_l].y);
        
        dmuls.l r6, r1
        sts mach, r0
        sts macl, r2
        xtrct r0, r2
        add r2, r9  // sx_l = vertices[curr_vt_l].x + fp_mul(dxdy_l, dty);
        
        mov.l r9, @(st.sx_l, sp)
        
        dmuls.l r7, r1
        sts mach, r0
        sts macl, r2
        xtrct r0, r2
        add r2, r11  // su_l = vertices[curr_vt_l].u + fp_mul(dudy_l, dty);
        
        dmuls.l r8, r1
        sts mach, r0
        sts macl, r2
        xtrct r0, r2
        add r2, r13  // sv_l = vertices[curr_vt_l].v + fp_mul(dvdy_l, dty);
        
        shll16 r13
        or r13, r11  // su_l_sv_l = (sv_l << 16) | su_l
        
        mov.l r11, @(st.su_l_sv_l, sp)
    
    .left_edge_exit:
    
    // next edge found on the right side
    
    cmp/pl r5  // while (!height_r)
    bt .right_edge_exit
      
      // r1: vertices[curr_vt_r].y, dty
      // r2: *next_vt_r, rdy
      
      // r3: *curr_vt_r
      // r4: height_l
      // r5: height_r
      
      // r6: dx, dxdy_r
      // r7: du, dudy_r
      // r8: dv, dvdy_r
      // r9: sx_l
      // r10: sx_r
      // r11: su_l_sv_l
      // r12: su_r_sv_r
      
      mov.l @(st.curr_vt_r, sp), r3
      
      .right_edge_loop_repeat:
      
      mov r3, r2
      mov #st.poly_flags, r0
      mov.l @(r0, sp), r0
      tst #poly.is_backface.bit, r0  // if (poly->flags.is_backface)
      bt 1f
        add #-(5 * 4), r2   // int *next_vt_r = curr_vt_r - 1;
        mov.l @sp, r0
        cmp/ge r0, r2  // if (next_vt_r < vertices)
        bt 2f
          mov.l @(st.num_vertices_s_ptr, sp), r2  // next_vt_r = vertices + poly->num_vertices - 1;
        bra 2f
        nop
      1:
        add #(5 * 4), r2   // int *next_vt_r = curr_vt_r + 1;
        mov.l @(st.num_vertices_s_ptr, sp), r0
        cmp/gt r0, r2  // if (next_vt_r > poly->num_vertices - 1)
        bf 2f
          mov.l @sp, r2  // next_vt_r = poly;
      2:
      
      mov.l @(4, r3), r1  // vertices[curr_vt_r].y
      mov.l @(4, r2), r5  // vertices[next_vt_r].y
      
      mov r1, r0
      shlr16 r5
      shlr16 r0
      sub r0, r5  // height_r = (vertices[next_vt_r].y >> FP) - (vertices[curr_vt_r].y >> FP);
      
      cmp/pz r5  // if (height_r < 0) return;
      bt 1f
        bra return
        nop
      1:
      
      cmp/pl r5
      bt 1f
        bra .right_edge_loop_repeat  // if (!height_r)
        mov r2, r3  // curr_vt_r = next_vt_r; // the delay slot makes this execute before the jump
      1:
        // if (height_r)
        
        // calculate the edge deltas
        
        mov.l @r3, r10  // vertices[curr_vt_r].x
        mov.l @r2, r6  // vertices[next_vt_r].x
        sub r10, r6  // fixed dx = vertices[next_vt_r].x - vertices[curr_vt_r].x;
        
        mov.l @((3 * 4), r3), r12  // vertices[curr_vt_r].u
        mov.l @((3 * 4), r2), r7  // vertices[next_vt_r].u
        sub r12, r7  // fixed du = vertices[next_vt_r].u - vertices[curr_vt_r].u;
        
        mov.l @((4 * 4), r3), r13  // vertices[curr_vt_r].v
        mov.l @((4 * 4), r2), r8  // vertices[next_vt_r].v
        sub r13, r8  // fixed dv = vertices[next_vt_r].v - vertices[curr_vt_r].v;
        
        mov.l r2, @(st.curr_vt_r, sp)  // curr_vt_r = next_vt_r;
        
        // unsigned integer division
        
        #if !DIV_LUT_ENABLE
          mov #1, r2
          shll16 r2
          mov r5, r0
          shll16 r0
          
          div0u
          
          .rept 16
            div1 r0, r2
          .endr
          
          rotcl r2
          extu.w r2, r2
        #else
          mov.l pool.div_lut, r3
          
          mov r5, r0
          shll r0  // u16
          mov.l @(r0, r3), r2  // fixed rdy = div_lut[height_r]; // .16 result
        #endif
        
        dmuls.l r6, r2
        sts mach, r0
        sts macl, r6
        xtrct r0, r6  // dxdy_r = fp_mul(dx, rdy);
        
        mov.l r6, @(st.dxdy_r, sp)
        
        dmuls.l r7, r2
        sts mach, r0
        sts macl, r7
        xtrct r0, r7  // dudy_r = fp_mul(du, rdy);
        
        dmuls.l r8, r2
        sts mach, r0
        sts macl, r8
        xtrct r0, r8  // dvdy_r = fp_mul(dv, rdy);
        
        mov r8, r0
        shll16 r0
        extu.w r7, r2
        or r2, r0  // dudy_r_dvdy_r = (dvdy_r << 16) | (u16)dudy_r
        
        mov.l r0, @(st.dudy_r_dvdy_r, sp)
        
        // initialize the side variables
        
        // sub-pixel precision
        
        mov r1, r0
        fp_trunc r0  // fp_trunc(vertices[curr_vt_r].y)
        sub r0, r1  // fixed dty = vertices[curr_vt_r].y - fp_trunc(vertices[curr_vt_r].y);
        
        dmuls.l r6, r1
        sts mach, r0
        sts macl, r2
        xtrct r0, r2
        add r2, r10  // sx_r = vertices[curr_vt_r].x + fp_mul(dxdy_r, dty);
        
        mov.l r10, @(st.sx_r, sp)
        
        dmuls.l r7, r1
        sts mach, r0
        sts macl, r2
        xtrct r0, r2
        add r2, r12  // su_r = vertices[curr_vt_r].u + fp_mul(dudy_r, dty);
        
        dmuls.l r8, r1
        sts mach, r0
        sts macl, r2
        xtrct r0, r2
        add r2, r13  // sv_r = vertices[curr_vt_r].v + fp_mul(dvdy_r, dty);
        
        shll16 r13
        or r13, r12  // su_r_sv_r = (sv_r << 16) | su_r
        
        mov.l r12, @(st.su_r_sv_r, sp)
    
    .right_edge_exit:
    
    // if the polygon doesn't have height return
    
    mov r4, r0
    or r5, r0
    tst r0, r0
    bt return  // if (!height_l && !height_r) return;
    
    // obtain the height to the next vertex on Y
    
    min_c r4, r5, r13  // int height = min_c(height_l, height_r);
    
    sub r13, r4  // height_l -= height;
    sub r13, r5  // height_r -= height;
    
    mov.l r4, @(st.height_l, sp)
    mov.l r5, @(st.height_r, sp)
    mov.l r13, @(st.height, sp)
    
    // second Y loop
    
    // prev:
    // r9: sx_l
    // r10: sx_r
    // r11: su_l_sv_l
    // r12: su_r_sv_r
    
    // r14: vid_y_offset
    
    // curr:
    // r1: dx, rdx
    // r2: sx_r_i, *vid_pnt
    // r3: sx_l_i, *end_pnt
    // r4: su_sv
    
    // r5: su_l, du, dudx, dudx_dvdx
    // r6: su_r
    // r11: sv_l, dv, dvdx
    // r12: sv_r
    
    // r13: div_lut
    
    .y_second_loop:
      mov r10, r2  // sx_r_i = sx_r
      shlr16 r2  // sx_r_i >>= 16
      mov r9, r3  // sx_l_i = sx_l
      shlr16 r3  // sx_l_i >>= 16
      
      // initialize the scanline variables
      
      mov r12, r4  // su_sv = su_r_sv_r
      
      // calculate the scanline deltas
      
      extu.w r11, r5  // su_l = su_l_sv_l & 0xFFFF
      shlr16 r11  // sv_l = su_l_sv_l >> 16
      
      extu.w r12, r6  // su_r = su_r_sv_r & 0xFFFF
      shlr16 r12  // sv_r = su_r_sv_r >> 16
      
      mov r2, r1
      sub r3, r1
      mov #1, r0
      subv r0, r1  // int dx = sx_r_i - sx_l_i - 1
      bf 1f
        mov #0, r5  // dudx = 0;
        bra 2f
        mov #0, r6  // dvdx = 0;
      1:
      
      sub r6, r5  // du = su_l - su_r
      sub r12, r11  // dv = sv_l - sv_r
      
      // unsigned integer division
      
      #if !DIV_LUT_ENABLE
        mov r1, r0
        shll16 r0
        mov #1, r1
        shll16 r1
        
        div0u
        
        .rept 16
        div1 r0, r1
        .endr
        
        rotcl r1
        extu.w r1, r1
      #else
        mov.l pool.div_lut, r13
        
        mov r1, r0
        shll r0  // u16
        mov.l @(r0, r13), r1  // fixed rdx = div_lut[dx]; // .16 result
      #endif
      
      dmuls.l r5, r1
      sts mach, r0
      sts macl, r5
      xtrct r0, r5  // dudx = fp_mul(du, rdx);
      
      dmuls.l r11, r1
      sts mach, r0
      sts macl, r6
      xtrct r0, r6  // dvdx = fp_mul(dv, rdx);
      2:
      
      shll16 r6
      extu.w  r5, r5
      or r6, r5  // dudx_dvdx = (dvdx << 16) | (u16)dudx
      
      // X clipping
      
      #if POLY_X_CLIPPING
        cmp/pz r3  // if (sx_l_i < 0)
        bt 1f
          mov #0, r3  // sx_l_i = 0;
        1:
        
        mov.l pool.screen_width, r0
        cmp/gt r0, r2  // if (sx_r_i > SCREEN_WIDTH_C)
        bf 1f
          mov r0, r2  // sx_r_i = SCREEN_WIDTH_C;
        1:
      #endif
      
      // set the pointers for the start and the end of the scanline
      // the loop goes from back to front on the sh-2
      
      shll r2
      add r14, r2  // u16 *vid_pnt = vid_y_offset + sx_r_i;
      shll r3
      add r14, r3  // u16 *end_pnt = vid_y_offset + sx_l_i;
      
      // scanline loop unrolled
      
      // curr:
      // r2: *vid_pnt
      // r3: *end_pnt
      // r4: su_sv
      // r5: dudx_dvdx
      
      // r9: flags
      // r10: texture_width_bits
      // r11: texture_size_wh_s
      // r12: texture_image
      // r13: cr_palette_tx_idx
      // r14: final_light_factor
      
      mov.l @sp, r0  // poly
      mov.l pool.poly_flags_offset, r1
      add r1, r0
      mov.l @r0, r9  // poly.flags
      mov.l @(poly.texture_width_bits - poly.flags, r0), r10
      mov.l @(poly.texture_size_wh_s - poly.flags, r0), r11
      mov.l @(poly.texture_image - poly.flags, r0), r12
      #if !PALETTE_MODE
        mov.l @(poly.cr_palette_tx_idx - poly.flags, r0), r13
      #endif
      mov.l @(poly.final_light_factor - poly.flags, r0), r14
      
      mov r9, r0
      tst #poly.has_transparency.bit, r0
      bt scanline_loop
      bra transparent_face
      nop
      
      .scanline_loop_exit:
      
      // increment the left and right side variables
      
      // r5: dxdy_l
      // r6: dxdy_r
      // r7: dudy_l_dvdy_l
      // r8: dudy_r_dvdy_r
      
      // r9: sx_l
      // r10: sx_r
      // r11: su_l_sv_l
      // r12: su_r_sv_r
      
      // r13: height
      // r14: vid_y_offset
      
      mov.l @(st.dxdy_l, sp), r5
      mov.l @(st.dxdy_r, sp), r6
      mov.l @(st.dudy_l_dvdy_l, sp), r7
      mov.l @(st.dudy_r_dvdy_r, sp), r8
      mov.l @(st.sx_l, sp), r9
      mov.l @(st.sx_r, sp), r10
      mov.l @(st.su_l_sv_l, sp), r11
      mov.l @(st.su_r_sv_r, sp), r12
      
      add r5, r9  // sx_l += dxdy_l
      add r6, r10  // sx_r += dxdy_r
      add r7, r11  // su_l_sv_l += dudy_l_dvdy_l
      add r8, r12  // su_r_sv_r += dudy_r_dvdy_r
      
      mov.l r9, @(st.sx_l, sp)
      mov.l r10, @(st.sx_r, sp)
      mov.l r11, @(st.su_l_sv_l, sp)
      mov.l r12, @(st.su_r_sv_r, sp)
      
      mov.l @(st.height, sp), r13
      mov.l @(st.vid_y_offset, sp), r14
      
      mov.l pool.frame_width, r0
      dt r13  // height--;
      add r0, r14  // vid_y_offset += SCREEN_WIDTH;
      
      mov.l r13, @(st.height, sp)
      mov.l r14, @(st.vid_y_offset, sp)
      
      bf .y_second_loop  // while (height)
    
    mov.l @(st.height_l, sp), r4
    bra .y_main_loop
    mov.l @(st.height_r, sp), r5  // delay slot

return:
  add #stack_size, sp
  mov.l @sp+, r8
  mov.l @sp+, r9
  mov.l @sp+, r10
  mov.l @sp+, r11
  mov.l @sp+, r12
  mov.l @sp+, r13
  rts
  mov.l @sp+, r14

// memory pool

.align 4

pool.poly_flags_offset:
  .long poly.flags

pool.frame_width:
  .long FRAME_WIDTH

#if POLY_X_CLIPPING
  pool.screen_width:
    .long SCREEN_WIDTH
#endif

pool.screen:
  .long _screen

pool.div_lut:
  .long _div_lut

scanline_loop:  
  cmp/gt r3, r2  // if (vid_pnt > end_pnt)
  bf .scanline_loop_exit
  
  mov r3, r1
  add #(4 * 2), r1  // end_offset = end_pnt + 4
  
  mov r10, r0
  cmp/eq #3, r0  // if (texture_width_bits == 3)
  bt .x_loop_not_transparent_8
  cmp/eq #4, r0  // if (texture_width_bits == 4)
  bt .x_loop_not_transparent_16
  cmp/eq #5, r0  // if (texture_width_bits == 5)
  bf 1f
    bra .x_loop_not_transparent_32
    nop
  1:
  bra .scanline_loop_exit
  nop
  
  // r0: .loop_table (scratch)
  // r1: end_offset
  // r2: *vid_pnt
  // r3: *end_pnt
  // r8: pc_offset
  
  .x_loop_not_transparent_8:
    #if UNROLL_SCANLINE_LOOP
      cmp/ge r1, r2  // if (vid_pnt >= end_offset)
      bt .set_pixel_0_nt_8
      
      .x_loop_remainder_nt_8:
        mov r1, r8
        sub r2, r8  // pc_offset = end_offset - vid_pnt
        shll r8
        mova .loop_table_nt_8, r0
        mov.l @(r0, r8), r0
        jmp @r0  // pc = *(pc + (pc_offset << 1))
        nop
        
        .align 4
        .loop_table_nt_8:
          .long .set_pixel_0_nt_8
          .long .set_pixel_1_nt_8
          .long .set_pixel_2_nt_8
          .long .set_pixel_3_nt_8
        
        .set_pixel_0_nt_8:
          set_pixel_not_transparent_8
        .set_pixel_1_nt_8:
          set_pixel_not_transparent_8
        .set_pixel_2_nt_8:
          set_pixel_not_transparent_8
        .set_pixel_3_nt_8:
          set_pixel_not_transparent_8
        
        cmp/ge r1, r2  // while (vid_pnt >= end_offset)
        bt .set_pixel_0_nt_8
      
      cmp/gt r3, r2  // if (vid_pnt > end_pnt)
      bt .x_loop_remainder_nt_8
    #else
      cmp/gt r3, r2  // while (vid_pnt > end_pnt)
      bf 1f
        bra .x_loop_not_transparent_8
        set_pixel_not_transparent_8
      1:
    #endif
  
  bra .scanline_loop_exit
  nop
  
  .x_loop_not_transparent_16:
    #if UNROLL_SCANLINE_LOOP
      cmp/ge r1, r2  // if (vid_pnt >= end_offset)
      bt .set_pixel_0_nt_16
      
      .x_loop_remainder_nt_16:
        mov r1, r8
        sub r2, r8  // pc_offset = end_offset - vid_pnt
        shll r8
        mova .loop_table_nt_16, r0
        mov.l @(r0, r8), r0
        jmp @r0  // pc = *(pc + (pc_offset << 1))
        nop
        
        .align 4
        .loop_table_nt_16:
          .long .set_pixel_0_nt_16
          .long .set_pixel_1_nt_16
          .long .set_pixel_2_nt_16
          .long .set_pixel_3_nt_16
        
        .set_pixel_0_nt_16:
          set_pixel_not_transparent_16
        .set_pixel_1_nt_16:
          set_pixel_not_transparent_16
        .set_pixel_2_nt_16:
          set_pixel_not_transparent_16
        .set_pixel_3_nt_16:
          set_pixel_not_transparent_16
        
        cmp/ge r1, r2  // while (vid_pnt >= end_offset)
        bt .set_pixel_0_nt_16
        
      cmp/gt r3, r2  // if (vid_pnt > end_pnt)
      bt .x_loop_remainder_nt_16
    #else
      cmp/gt r3, r2  // while (vid_pnt > end_pnt)
      bf 1f
        bra .x_loop_not_transparent_16
        set_pixel_not_transparent_16
      1:
    #endif
  
  bra .scanline_loop_exit
  nop
  
  .x_loop_not_transparent_32:
    #if UNROLL_SCANLINE_LOOP
      cmp/ge r1, r2  // if (vid_pnt >= end_offset)
      bt .set_pixel_0_nt_32
      
      .x_loop_remainder_nt_32:
        mov r1, r8
        sub r2, r8  // pc_offset = end_offset - vid_pnt
        shll r8
        mova .loop_table_nt_32, r0
        mov.l @(r0, r8), r0
        jmp @r0  // pc = *(pc + (pc_offset << 1))
        nop
        
        .align 4
        .loop_table_nt_32:
          .long .set_pixel_0_nt_32
          .long .set_pixel_1_nt_32
          .long .set_pixel_2_nt_32
          .long .set_pixel_3_nt_32
        
        .set_pixel_0_nt_32:
          set_pixel_not_transparent_32
        .set_pixel_1_nt_32:
          set_pixel_not_transparent_32
        .set_pixel_2_nt_32:
          set_pixel_not_transparent_32
        .set_pixel_3_nt_32:
          set_pixel_not_transparent_32
        
        cmp/ge r1, r2  // while (vid_pnt >= end_offset)
        bt .set_pixel_0_nt_32
      
      cmp/gt r3, r2  // if (vid_pnt > end_pnt)
      bt .x_loop_remainder_nt_32
    #else
      cmp/gt r3, r2  // while (vid_pnt > end_pnt)
      bf 1f
        bra .x_loop_not_transparent_32
        set_pixel_not_transparent_32
      1:
    #endif
  
  bra .scanline_loop_exit
  nop

transparent_face:  // face has transparency
  cmp/gt r3, r2  // if (vid_pnt > end_pnt)
  bt 1f
    bra .scanline_loop_exit
    nop
  1:
  
  mov r3, r1
  add #(4 * 2), r1  // end_offset = end_pnt + 4
  
  mov r10, r0
  cmp/eq #3, r0  // if (texture_width_bits == 3)
  bt .x_loop_has_transparency_8
  cmp/eq #4, r0  // if (texture_width_bits == 4)
  bt .x_loop_has_transparency_16
  cmp/eq #5, r0  // if (texture_width_bits == 5)
  bf 1f
    bra .x_loop_has_transparency_32
    nop
  1:
  bra .scanline_loop_exit
  nop
  
  // r0: .loop_table (scratch)
  // r1: end_offset
  // r8: pc_offset
  
  .x_loop_has_transparency_8:
    #if UNROLL_SCANLINE_LOOP
      cmp/ge r1, r2  // if (vid_pnt >= end_offset)
      bt .set_pixel_0_tr_8
      
      .x_loop_remainder_tr_8:
        mov r1, r8
        sub r2, r8  // pc_offset = end_offset - vid_pnt
        shll r8
        mova .loop_table_tr_8, r0
        mov.l @(r0, r8), r0
        jmp @r0  // pc = *(pc + (pc_offset << 1))
        nop
        
        .align 4
        .loop_table_tr_8:
          .long .set_pixel_0_tr_8
          .long .set_pixel_1_tr_8
          .long .set_pixel_2_tr_8
          .long .set_pixel_3_tr_8
        
        .set_pixel_0_tr_8:
          set_pixel_has_transparency_8
        .set_pixel_1_tr_8:
          set_pixel_has_transparency_8
        .set_pixel_2_tr_8:
          set_pixel_has_transparency_8
        .set_pixel_3_tr_8:
          set_pixel_has_transparency_8
        
        cmp/ge r1, r2  // while (vid_pnt >= end_offset)
        bt .set_pixel_0_tr_8
      
      cmp/gt r3, r2  // if (vid_pnt > end_pnt)
      bt .x_loop_remainder_tr_8
    #else
      cmp/gt r3, r2  // while (vid_pnt > end_pnt)
      bf 1f
        bra .x_loop_has_transparency_8
        set_pixel_has_transparency_8
      1:
    #endif
  
  bra .scanline_loop_exit
  nop
  
  .x_loop_has_transparency_16:
    #if UNROLL_SCANLINE_LOOP
      cmp/gt r1, r2  // if (vid_pnt >= end_offset)
      bt .set_pixel_0_tr_16
      
      .x_loop_remainder_tr_16:
        mov r1, r8
        sub r2, r8  // pc_offset = end_offset - vid_pnt
        shll r8
        mova .loop_table_tr_16, r0
        mov.l @(r0, r8), r0
        jmp @r0  // pc = *(pc + (pc_offset << 1))
        nop
        
        .align 4
        .loop_table_tr_16:
          .long .set_pixel_0_tr_16
          .long .set_pixel_1_tr_16
          .long .set_pixel_2_tr_16
          .long .set_pixel_3_tr_16
        
        .set_pixel_0_tr_16:
          set_pixel_has_transparency_16
        .set_pixel_1_tr_16:
          set_pixel_has_transparency_16
        .set_pixel_2_tr_16:
          set_pixel_has_transparency_16
        .set_pixel_3_tr_16:
          set_pixel_has_transparency_16
        
        cmp/ge r1, r2  // while (vid_pnt >= end_offset)
        bt .set_pixel_0_tr_16
      
      cmp/gt r3, r2  // if (vid_pnt > end_pnt)
      bt .x_loop_remainder_tr_16
    #else
      cmp/gt r3, r2  // while (vid_pnt > end_pnt)
      bf 1f
        bra .x_loop_has_transparency_16
        set_pixel_has_transparency_16
      1:
    #endif
  
  bra .scanline_loop_exit
  nop
  
  .x_loop_has_transparency_32:
    #if UNROLL_SCANLINE_LOOP
      cmp/gt r1, r2  // if (vid_pnt >= end_offset)
      bt .set_pixel_0_tr_32
      
      .x_loop_remainder_tr_32:
        mov r1, r8
        sub r2, r8  // pc_offset = end_offset - vid_pnt
        shll r8
        mova .loop_table_tr_32, r0
        mov.l @(r0, r8), r0
        jmp @r0  // pc = *(pc + (pc_offset << 1))
        nop
        
        .align 4
        .loop_table_tr_32:
          .long .set_pixel_0_tr_32
          .long .set_pixel_1_tr_32
          .long .set_pixel_2_tr_32
          .long .set_pixel_3_tr_32
        
        .set_pixel_0_tr_32:
          set_pixel_has_transparency_32
        .set_pixel_1_tr_32:
          set_pixel_has_transparency_32
        .set_pixel_2_tr_32:
          set_pixel_has_transparency_32
        .set_pixel_3_tr_32:
          set_pixel_has_transparency_32
        
        cmp/ge r1, r2  // while (vid_pnt >= end_offset)
        bt .set_pixel_0_tr_32
      
      cmp/gt r3, r2  // if (vid_pnt > end_pnt)
      bt .x_loop_remainder_tr_32
    #else
      cmp/gt r3, r2  // while (vid_pnt > end_pnt)
      bf 1f
        bra .x_loop_has_transparency_32
        set_pixel_has_transparency_32
      1:
    #endif
  
  bra .scanline_loop_exit
  nop
