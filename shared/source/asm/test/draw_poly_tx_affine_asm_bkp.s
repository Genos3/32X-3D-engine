#include "asm_common.inc"

.section .sdram
.global draw_poly_tx_affine_asm
.type draw_poly_tx_affine_asm STT_FUNC

// pixel macro

// r0: sv_i (scratch)
// r2: *vid_pnt
// r3: *end_pnt
// r4: su_sv
// r5: dudx_dvdx
// r6: su_i, tx_color (scratch)

// r11: texture_size_s
// r12: texture_image
// r13: cr_palette_tx_idx
// r14: final_light_factor

.macro set_pixel_not_transparent_8
  mov r4, r0
  shlr8 r0  // uint su_sv_i = su_sv >> 8
  and r11, r0  // uint su_sv_i &= texture_size_s
  mov.w r0, r6  // uint su_i = su_sv_i & 15
  shlr16 r0  // uint sv_i = su_sv_i >> 16
  
  shll2 r0
  shll r0
  add r6, r0
  mov.b @(r0, r12), r6  // u16 tx_color = texture_image[(sv_i << 3) + su_i];
  
  #if ASM_HAS_LIGHTING
    #if PALETTE_MODE
      shll2 r6  // LIGHT_GRD_BITS
      add r14, r6  // txcolor = (txcolor << LIGHT_GRD_BITS) + final_light_factor; 16-bit
    #else
      shll2 r6
      shll r6  // LIGHT_GRD_BITS + 1
      add r14, r6  // txcolor = (txcolor << (LIGHT_GRD_BITS + 1)) + final_light_factor; 16-bit
    #endif
  #endif
  
  #if PALETTE_MODE
    mov r6, r0
    shll8 r0
    or r0, r6
    mov.w r6, @-r2  // *--vid_pnt = dup8(tx_color);
  #else
    mov.w @(r6, r13), r6  // textures->cr_palette_tx_idx[tx_color];
    mov.w r6, @-r2  // *--vid_pnt = tx_color;
  #endif
  
  add r5, r4  // su_sv += dudx_dvdx;
.endm

.macro set_pixel_not_transparent_16
  mov r4, r0
  shlr8 r0  // uint su_sv_i = su_sv >> 8
  and r11, r0  // uint su_sv_i &= texture_size_s
  mov.w r0, r6  // uint su_i = su_sv_i & 15
  shlr16 r0  // uint sv_i = su_sv_i >> 16
  
  shll2 r0
  shll2 r0
  add r6, r0
  mov.b @(r0, r12), r6  // u16 tx_color = texture_image[(sv_i << 4) + su_i];
  
  #if ASM_HAS_LIGHTING
    #if PALETTE_MODE
      shll2 r6  // LIGHT_GRD_BITS
      add r14, r6  // txcolor = (txcolor << LIGHT_GRD_BITS) + final_light_factor; 16-bit
    #else
      shll2 r6
      shll r6  // LIGHT_GRD_BITS + 1
      add r14, r6  // txcolor = (txcolor << (LIGHT_GRD_BITS + 1)) + final_light_factor; 16-bit
    #endif
  #endif
  
  #if PALETTE_MODE
    mov r6, r0
    shll8 r0
    or r0, r6
    mov.w r6, @-r2  // *--vid_pnt = dup8(tx_color);
  #else
    mov.w @(r6, r13), r6  // textures->cr_palette_tx_idx[tx_color];
    mov.w r6, @-r2  // *--vid_pnt = tx_color;
  #endif
  
  add r5, r4  // su_sv += dudx_dvdx;
.endm

.macro set_pixel_not_transparent_32
  mov r4, r0
  shlr8 r0  // uint su_sv_i = su_sv >> 8
  and r11, r0  // uint su_sv_i &= texture_size_s
  mov.w r0, r6  // uint su_i = su_sv_i & 15
  shlr16 r0  // uint sv_i = su_sv_i >> 16
  
  shll2 r0
  shll2 r0
  shll r0
  add r6, r0
  mov.b @(r0, r12), r6  // u16 tx_color = texture_image[(sv_i << 5) + su_i];
  
  #if ASM_HAS_LIGHTING
    #if PALETTE_MODE
      shll2 r6  // LIGHT_GRD_BITS
      add r14, r6  // txcolor = (txcolor << LIGHT_GRD_BITS) + final_light_factor; 16-bit
    #else
      shll2 r6
      shll r6  // LIGHT_GRD_BITS + 1
      add r14, r6  // txcolor = (txcolor << (LIGHT_GRD_BITS + 1)) + final_light_factor; 16-bit
    #endif
  #endif
  
  #if PALETTE_MODE
    mov r6, r0
    shll8 r0
    or r0, r6
    mov.w r6, @-r2  // *--vid_pnt = dup8(tx_color);
  #else
    mov.w @(r6, r13), r6  // textures->cr_palette_tx_idx[tx_color];
    mov.w r6, @-r2  // *--vid_pnt = tx_color;
  #endif
  
  add r5, r4  // su_sv += dudx_dvdx;
.endm

.macro set_pixel_has_transparency_8
  mov r4, r0
  shlr8 r0  // uint su_sv_i = su_sv >> 8
  and r11, r0  // uint su_sv_i &= texture_size_s
  mov.w r0, r6  // uint su_i = su_sv_i & 15
  shlr16 r0  // uint sv_i = su_sv_i >> 16
  
  shll2 r0
  shll r0
  add r6, r0
  mov.b @(r0, r12), r6  // u16 tx_color = texture_image[(sv_i << 3) + su_i];
  
  cmp/eq #0, r6  // if (tx_color)
  bt 1f
    #if ASM_HAS_LIGHTING
      #if PALETTE_MODE
        shll2 r6  // LIGHT_GRD_BITS
        add r14, r6  // txcolor = (txcolor << LIGHT_GRD_BITS) + final_light_factor; 16-bit
      #else
        shll2 r6
        shll r6  // LIGHT_GRD_BITS + 1
        add r14, r6  // txcolor = (txcolor << (LIGHT_GRD_BITS + 1)) + final_light_factor; 16-bit
      #endif
    #endif
    
    #if PALETTE_MODE
      mov r6, r0
      shll8 r0
      or r0, r6
      mov.w r6, @r2  // *vid_pnt = dup8(tx_color);
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
  and r11, r0  // su_sv_i &= texture_size_s
  mov.w r0, r6  // su_i = su_sv_i & 15
  shlr16 r0  // sv_i = su_sv_i >> 16
  
  shll2 r0
  shll2 r0
  add r6, r0
  mov.b @(r0, r12), r6  // u16 tx_color = texture_image[(sv_i << 4) + su_i];
  
  cmp/eq #0, r6  // if (tx_color)
  bt 1f
    #if ASM_HAS_LIGHTING
      #if PALETTE_MODE
        shll2 r6  // LIGHT_GRD_BITS
        add r14, r6  // txcolor = (txcolor << LIGHT_GRD_BITS) + final_light_factor; 16-bit
      #else
        shll2 r6
        shll r6  // LIGHT_GRD_BITS + 1
        add r14, r6  // txcolor = (txcolor << (LIGHT_GRD_BITS + 1)) + final_light_factor; 16-bit
      #endif
    #endif
    
    #if PALETTE_MODE
      mov r6, r0
      shll8 r0
      or r0, r6
      mov.w r6, @r2  // *vid_pnt = dup8(tx_color);
    #else
      mov.w @(r6, r13), r6  // textures->cr_palette_tx_idx[tx_color];
      mov.w r6, @r2  // *vid_pnt = tx_color;
    #endif
  1:
  
  add #-2, r2  // vid_pnt--;
  
  add r5, r4  // su_sv += dudx_dvdx;
.endm

frame_width_ptr:
.word (SCREEN_WIDTH * 2)
.space 2

screen_width_ptr:
.word SCREEN_WIDTH_C
.space 2

screen_ptr:
.long screen

div_lutr_ptr:
.long div_lut

// void draw_poly_tx_affine_asm(poly_t *poly)

draw_poly_tx_affine_asm:
  push r8
  push r9
  push r10
  push r11
  push r12
  push r13
  push r14
  
  // stack, local variables
  
  .struct 0
    poly:
      .space 4
    
    num_vertices_s_ptr:
      .space 4  // vertices + num_vertices - 1
    
    curr_vt_l:
      .space 4
    curr_vt_r:
      .space 4
    height_l:
      .space 4
    height_r:
      .space 4
    height:
      .space 4
    vid_y_offset:
      .space 4
    
    sx_l_sx_r:
      .space 4
    su_l_sv_l:
      .space 4
    su_r_sv_r:
      .space 4
    dxdy_l_dxdy_r:
      .space 4
    dudy_l_dvdy_l:
      .space 4
    dudy_r_dvdy_r:
      .space 4
    
    stack_size:
  .previous
  
  add -stack_size, sp
  
  mov r4, r0
  
  mov.l r0, @(poly, sp)
  
  mov.l @(poly.bitfield, r0), r14  // poly.bitfield
  mov.l @(poly.num_vertices, r0), r13  // poly.num_vertices
  
  // obtain the top and bottom vertices
  
  // read only:
  // r0: poly
  // r13: num_vertices
  // r14: bitfield
  
  // volatile:
  // r1: i
  // r2: vertices + j
  // r3: vertices + sup_vt
  // r4: vertices[j].y
  // r5: vertices[sup_vt].y
  // r6: vertices[inf_vt].y, inf_y
  
  add #-1, r13
  mov r13, r1  // i = num_vertices - 1
  mov r0, r2  // j = vertices
  mov r0, r3  // sup_vt = vertices
  
  mov.l @(4, r2), r5  // vertices[sup_vt].y
  #if CHECK_POLY_HEIGHT
    mov r5, r6  // vertices[inf_vt].y
  #endif
  
  .loop_sort_y:
    add #(5 * 4), r2  // j++
    mov.l @(4, r2), r4  // vertices[j].y
    
    cmp/ge r4, r5  // if (vertices[j].y < vertices[sup_vt].y)
    bt 1f
      mov r2, r3  // sup_vt = j
      mov r4, r5  // vertices[sup_vt].y = vertices[j].y
    1:
    
    #if CHECK_POLY_HEIGHT
      cmp/gt r4, r6  // if (vertices[j].y > vertices[inf_vt].y)
      bf 1f
        mov r4, r6  // vertices[inf_vt].y = vertices[j].y
      1:
    #endif
    
    dt r1  // i--
    bf .loop_sort_y  // i != 0
  
  // if the polygon doesn't have height return
  
  #if CHECK_POLY_HEIGHT
    shlr16 r5
    shlr16 r6
    cmp/eq r5, r6
    bt .return  // if (poly->vertices[sup_vt].y >> FP == poly->vertices[inf_vt].y >> FP) return;
  #endif
  
  // initialize the edge variables
  
  // prev:
  // r3: vertices + i
  
  // curr:
  // r3: *curr_vt_l
  // r4: *curr_vt_r
  // r5: height_l
  // r6: height_r
  
  // r14: vid_y_offset
  
  mov r13, r1
  shll2 r1
  add r13, r1  // num_vertices * 5 (x, y, z, u, v)
  shll2 r1
  add r0, r1  // vertices + num_vertices - 1
  mov.l r1, @(num_vertices_s_ptr, sp)
  
  // mov r3, r3  // int *curr_vt_l = vertices;
  mov r3, r4  // int *curr_vt_r = curr_vt_l;
  
  mov.l @(curr_vt_l, sp), r3
  mov.l @(curr_vt_r, sp), r4
  
  mov #0, r5  // int height_l = 0;
  mov #0, r6  // int height_r = 0;
  
  // set the screen offset for the start of the first scanline
  
  mov.l @(4, r4), r0
  shlr16 r0  // y0_i = vertices[sup_vt]..y >> FP;
  
  mov.w frame_width_ptr, r1
  mov.l screen_ptr, r2
  mov.l @r2, r2
  mul r0, r1
  sts macl, r14
  add r2, r14  // u16 *vid_y_offset = screen + y0_i * SCREEN_WIDTH;
  
  mov r14, @(vid_y_offset, sp)
  
  // main Y loop
  
  .y_main_loop:
    // next edge found on the left side
    
    cmp/eq #0, r4  // if (!height_l)
    bf .left_edge_exit
      
      // r0: scratch
      // r1: vertices[curr_vt_l].y, dty
      // r2: *next_vt_l, rdy
      
      // r3: *curr_vt_l
      // r4: height_l
      // r5: height_r
      
      // r6: dx, dxdy_l
      // r7: du, dudy_l
      // r8: dv, dvdy_l
      // r9: dxdy_l_dxdy_r
      // r10: su_r_sv_r
      // r11: su_l_sv_l
      // r12: sx_l_sx_r
      
      // r13: div_lut
      
      mov.l @(curr_vt_l, sp), r3
      mov.l @(poly, sp), r0
      
      .left_edge_loop_repeat:
      
      mov r3, r2
      add #-(5 * 4), r2   // int *next_vt_l = curr_vt_l - 1;
      cmp/ge r2, r0  // if (next_vt_l < vertices)
      bt 1f
        mov.l @(num_vertices_s_ptr, sp), r2  // next_vt_l = vertices + poly->num_vertices - 1;
      1:
      
      mov.l @(4, r3), r1  // vertices[curr_vt_l].y
      mov.l @(4, r2), r4  // vertices[next_vt_l].y
      
      mov r1, r0
      shlr16 r4
      shlr16 r0
      sub r0, r4  // height_l = (vertices[next_vt_l].y >> FP) - (vertices[curr_vt_l].y >> FP);
      
      cmp/pz r4
      bf .return  // if (height_l < 0) return;
      
      cmp/eq #0 r4  // if (height_l)
      bf 1f
        bra .left_edge_loop_repeat  // while (!height_l)
        mov r2, r3  // curr_vt_l = next_vt_l; // the delay slot makes this execute before the jump
      1:
        // calculate the edge deltas
        
        mov.l @r3, r0  // vertices[curr_vt_l].x
        mov.l @r2, r6  // vertices[next_vt_l].x
        sub r0, r6  // fixed dx = vertices[next_vt_l].x - vertices[curr_vt_l].x;
        
        mov.l @(8, r3), r0  // vertices[curr_vt_l].u
        mov.l @(8, r2), r7  // vertices[next_vt_l].u
        sub r0, r7  // fixed du = vertices[next_vt_l].u - vertices[curr_vt_l].u;
        
        mov.l @(12, r3), r0  // vertices[curr_vt_l].v
        mov.l @(12, r2), r8  // vertices[next_vt_l].v
        sub r0, r8  // fixed dv = vertices[next_vt_l].v - vertices[curr_vt_l].v;
        
        mov.l r2, @(curr_vt_l, sp)  // curr_vt_l = next_vt_l;
        
        mov.l div_lutr_ptr, r13
        
        // unsigned integer division
        
        mov.l r4, r0
        shll2 r0
        mov.l @(r0, r13), r2  // fixed rdy = div_luts(height_l);  // .16 result
        
        dmulu.l r6, r2
        sts mach r0
        sts macl r6
        xtrct r0, r6  // dxdy_l = fp_mul(dx, rdy);
        
        mov.w r6, r9  // dxdy_l_dxdy_r
        
        mov.l r9, @(dxdy_l_dxdy_r, sp)
        
        dmulu.l r7, r2
        sts mach r0
        sts macl r7
        xtrct r0, r7  // dudy_l = fp_mul(du, rdy);
        
        dmulu.l r8, r2
        sts mach r0
        sts macl r8
        xtrct r0, r8  // dvdy_l = fp_mul(dv, rdy);
        
        mov r8, r0
        shll16 r0
        or r7, r0  // dudy_l_dvdy_l = (dvdy_l << 16) | dudy_l
        
        mov.l r0, @(dudy_l_dvdy_l, sp)
        
        // initialize the side variables
        
        // sub-pixel precision
        
        mov r1, r0
        fp_trunc r0  // fp_trunc(vertices[curr_vt_l].y)
        sub r0, r1  // fixed dty = vertices[curr_vt_l].y - fp_trunc(vertices[curr_vt_l].y);
        
        dmuls.l r6, r1
        sts mach r0
        sts macl r1
        xtrct r0, r1
        add r1, r6  // sx_l = vertices[curr_vt_l].x + fp_mul(dxdy_l, dty);
        
        mov.w r6, r12  // sx_l_sx_r
        
        mov r12, @(sx_l_sx_r, sp)
        
        dmuls.l r7, r1
        sts mach r0
        sts macl r1
        xtrct r0, r1
        add r1, r7  // su_l = vertices[curr_vt_l].u + fp_mul(dudy_l, dty);
        
        dmuls.l r8, r1
        sts mach r0
        sts macl r1
        xtrct r0, r1
        add r1, r8  // sv_l = vertices[curr_vt_l].v + fp_mul(dvdy_l, dty);
        
        shll16 r8
        or r8, r7  // su_l_sv_l = (sv_l << 16) | su_l
        
        mov r7, @(su_l_sv_l, sp)
    
    .left_edge_exit:
    
    // next edge found on the right side
    
    cmp/eq #0, r5  // if (!height_r)
    bf .right_edge_exit
      
      // r0: scratch
      // r1: vertices[curr_vt_r].y, dty
      // r2: *next_vt_r, rdy
      
      // r3: *curr_vt_r
      // r4: height_l
      // r5: height_r
      
      // r6: dx, dxdy_r
      // r7: du, dudy_r
      // r8: dv, dvdy_r
      // r9: dxdy_l_dxdy_r
      // r10: su_r_sv_r
      // r11: su_l_sv_l
      // r12: sx_l_sx_r
      
      // r13: div_lut
      
      mov.l @(curr_vt_r, sp), r3
      
      .right_edge_loop_repeat:
      
      mov r3, r2
      add #(5 * 4), r2   // int *next_vt_r = curr_vt_r + 1;
      mov.l @(num_vertices_s_ptr, sp), r1
      cmp/gt r2, r1  // if (next_vt_r > poly->num_vertices - 1)
      bf 1f
        mov.l @(poly, sp), r2  // next_vt_r = poly;
      1:
      
      mov.l @(4, r3), r1  // vertices[curr_vt_r].y
      mov.l @(4, r2), r5  // vertices[next_vt_r].y
      
      mov r1, r0
      shlr16 r5
      shlr16 r0
      sub r0, r5  // height_r = (vertices[next_vt_r].y >> FP) - (vertices[curr_vt_r].y >> FP);
      
      cmp/pz r5
      bf .return  // if (height_r < 0) return;
      
      cmp/eq #0 r5  // if (height_r)
      bf 1f
        bra .right_edge_loop_repeat  // while (!height_r)
        mov r2, r3  // curr_vt_r = next_vt_r; // the delay slot makes this execute before the jump
      1:
        // calculate the edge deltas
        
        mov.l @r3, r0  // vertices[curr_vt_r].x
        mov.l @r2, r6  // vertices[next_vt_r].x
        sub r0, r6  // fixed dx = vertices[next_vt_r].x - vertices[curr_vt_r].x;
        
        mov.l @(8, r3), r0  // vertices[curr_vt_r].u
        mov.l @(8, r2), r7  // vertices[next_vt_r].u
        sub r0, r7  // fixed du = vertices[next_vt_r].u - vertices[curr_vt_r].u;
        
        mov.l @(12, r3), r0  // vertices[curr_vt_r].v
        mov.l @(12, r2), r8  // vertices[next_vt_r].v
        sub r0, r8  // fixed dv = vertices[next_vt_r].v - vertices[curr_vt_r].v;
        
        mov.l r2, @(curr_vt_r, sp)  // curr_vt_r = next_vt_r;
        
        mov.l div_lutr_ptr, r13
        
        // unsigned integer division
        
        mov.l r5, r0
        shll2 r0
        mov.l @(r0, r13), r2  // fixed rdy = div_luts(height_r);  // .16 result
        
        dmulu.l r6, r2
        sts mach r0
        sts macl r6
        xtrct r0, r6  // dxdy_r = fp_mul(dx, rdy);
        
        swap.w r9
        mov.w r6, r9
        swap.w r9  // dxdy_l_dxdy_r
        
        mov.l r9, @(dxdy_l_dxdy_r, sp)
        
        dmulu.l r7, r2
        sts mach r0
        sts macl r7
        xtrct r0, r7  // dudy_r = fp_mul(du, rdy);
        
        dmulu.l r8, r2
        sts mach r0
        sts macl r8
        xtrct r0, r8  // dvdy_r = fp_mul(dv, rdy);
        
        mov r8, r0
        shll16 r0
        or r7, r0  // dudy_r_dvdy_r = (dvdy_r << 16) | dudy_r
        
        mov.l r0, @(dudy_r_dvdy_r, sp)
        
        // initialize the side variables
        
        // sub-pixel precision
        
        mov r1, r0
        fp_trunc r0  // fp_trunc(vertices[curr_vt_r].y)
        sub r0, r1  // fixed dty = vertices[curr_vt_r].y - fp_trunc(vertices[curr_vt_r].y);
        
        dmuls.l r6, r1
        sts mach r0
        sts macl r1
        xtrct r0, r1
        add r1, r6  // sx_r = vertices[curr_vt_r].x + fp_mul(dxdy_r, dty);
        
        swap.w r12
        mov.w r6, r12
        swap.w r12  // sx_l_sx_r
        
        mov r12, @(sx_l_sx_r, sp)
        
        dmuls.l r7, r1
        sts mach r0
        sts macl r1
        xtrct r0, r1
        add r1, r7  // su_r = vertices[curr_vt_r].u + fp_mul(dudy_r, dty);
        
        dmuls.l r8, r1
        sts mach r0
        sts macl r1
        xtrct r0, r1
        add r1, r8  // sv_r = vertices[curr_vt_r].v + fp_mul(dvdy_r, dty);
        
        shll16 r8
        or r8, r7  // su_r_sv_r = (sv_r << 16) | su_r
        
        mov r7, @(su_r_sv_r, sp)
    
    .right_edge_exit:
    
    // if the polygon doesn't have height return
    
    mov r4, r0
    or r5, r0
    tst r0, r0
    bt .return  // if (!height_l && !height_r) return;
    
    // obtain the height to the next vertex on Y
    
    min_c r13, r4, r5  // int height = min_c(height_l, height_r);
    
    sub r13, r4  // height_l -= height;
    sub r13, r5  // height_r -= height;
    
    mov.l r4, @(height_l, sp)
    mov.l r5, @(height_r, sp)
    mov.l r13, @(height, sp)
    
    // second Y loop
    
    // prev:
    // r10: su_r_sv_r
    // r11: su_l_sv_l
    // r12: sx_l_sx_r
    
    // r14: vid_y_offset
    
    // curr:
    // r1: rdx
    // r2: sx_r_i, *vid_pnt
    // r3: sx_l_i, *end_pnt
    // r4: su_sv
    // r5: dudx_dvdx
    
    // r13: div_lut
    
    .y_second_loop:
      shlr8 r12  // sx_l_sx_r >>= 8
      mov.b r12, r2  // sx_l_i = sx_l_sx_r & 0xFF
      mov r12, r3
      shlr16 r3  // sx_r_i = sx_l_sx_r >> 16
      
      // initialize the scanline variables
      
      mov r11, r4  // su_sv = su_l_sv_l
      
      // mov r8, r4  // su = su_l
      // mov r9, r0  // sv = sv_l
      // shll16 r0
      // or r0, r4  // su_sv = (sv << 16) | su
      
      // calculate the scanline deltas
      
      mov.w r11, r5  // su_l
      shlr16 r11  // sv_l
      
      mov.w r10, r6  // su_r
      shlr16 r10  // sv_r
      
      sub r5, r6  // du = su_r - su_l
      sub r11, r10  // dv = sv_r - sv_l
      
      mov r3, r1
      sub r2, r1  // int dx = sx_r_i - sx_l_i
      
      mov.l div_lutr_ptr, r13
      
      // unsigned integer division
      
      div_luts r1, r1, r13  // fixed rdx = div_luts(dx);
      
      dmuls.l r6, r1
      sts mach r0
      sts macl r5
      xtrct r0, r5  // dudx = fp_mul(du, rdx);
      
      dmuls.l r10, r1
      sts mach r0
      sts macl r6
      xtrct r0, r6  // dvdx = fp_mul(dv, rdx);
      
      shll16 r6
      or r6, r5  // dudx_dvdx = (dvdx << 16) | dudx
      
      // X clipping
      
      #if POLY_X_CLIPPING
        cmp/ge #0, r3  // if (sx_l_i < 0)
        bt 1f
          mov.l #0, r3  // sx_l_i = 0;
        1:
        
        mov.w screen_width_ptr, r0
        cmp/gt r2, r0  // if (sx_r_i > SCREEN_WIDTH_C)
        bt 1f
          mov r0, r2  // sx_r_i = SCREEN_WIDTH_C;
        1:
      #endif
      
      // set the pointers for the start and the end of the scanline
      
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
      
      // r8: bitfield
      // r9: texture_width_bits
      // r10: texture_width_s
      // r11: texture_height_s
      // r12: texture_image
      // r13: cr_palette_tx_idx
      // r14: final_light_factor
      
      mov.l @(poly, sp), r0
      mov.l @(poly.bitfield, r0), r8
      mov.l @(poly.texture_width_bits, r0), r9
      mov.l @(poly.texture_width_s, r0), r10
      mov.l @(poly.texture_height_s, r0), r11
      mov.l @(poly.texture_image, r0), r12
      mov.l @(poly.cr_palette_tx_idx, r0), r13
      mov.l @(poly.final_light_factor, r0), r14
      
      tst poly.has_transparency.bit, r8
      bt .transparent_face
        
        cmp/eq #3, r9  // if (texture_width_bits == 3)
        bt .x_loop_not_transparent_8
        cmp/eq #4, r9  // if (texture_width_bits == 4)
        bt .x_loop_not_transparent_16
        cmp/eq #5, r9  // if (texture_width_bits == 5)
        bt .x_loop_not_transparent_32
        bra .x_loop_exit
        nop
        
        // r0: pc_offset (scratch)
        // r1: end_offset
        // r8: .loop_table
        
        .x_loop_not_transparent_8:
          cmp/gt r2, r3  // if (vid_pnt > end_pnt)
          bf .x_loop_exit
            
            mov r3, r1
            add #(4 * 2), r1  // end_offset = end_pnt + 4
            
            cmp/ge r2, r1  // if (vid_pnt >= end_offset)
            bt .set_pixel_0_nt_8
            
            .x_loop_remainder_nt_8:
              mov r1, r0
              sub r2, r0  // pc_offset = end_offset - vid_pnt
              shll r0
              mova .loop_table_nt_8, r8
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
              
              cmp/ge r2, r1  // while (vid_pnt >= end_offset)
              bt .set_pixel_0_nt_8
            
            cmp/gt r2, r3  // if (vid_pnt > end_pnt)
            bt .x_loop_remainder_nt_8
          
          bra .x_loop_exit
          nop
        
        .x_loop_not_transparent_16:
          cmp/gt r2, r3  // if (vid_pnt > end_pnt)
          bf .x_loop_exit
            
            mov r3, r1
            add #(4 * 2), r1  // end_offset = end_pnt + 4
            
            cmp/ge r2, r1  // if (vid_pnt >= end_offset)
            bt .set_pixel_0_nt_16
            
            .x_loop_remainder_nt_16:
              mov r1, r0
              sub r2, r0  // pc_offset = end_offset - vid_pnt
              shll r0
              mova .loop_table_nt_16, r8
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
              
              cmp/ge r2, r1  // while (vid_pnt >= end_offset)
              bt .set_pixel_0_nt_16
              
            cmp/gt r2, r3  // if (vid_pnt > end_pnt)
            bt .x_loop_remainder_nt_16
          
          bra .x_loop_exit
          nop
        
        .x_loop_not_transparent_32:
          cmp/gt r2, r3  // if (vid_pnt > end_pnt)
          bf .x_loop_exit
            
            mov r3, r1
            add #(4 * 2), r1  // end_offset = end_pnt + 4
            
            cmp/ge r2, r1  // if (vid_pnt >= end_offset)
            bt .set_pixel_0_nt_32
            
            .x_loop_remainder_nt_32:
              mov r1, r0
              sub r2, r0  // pc_offset = end_offset - vid_pnt
              shll r0
              mova .loop_table_nt_32, r8
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
              
              cmp/ge r2, r1  // while (vid_pnt >= end_offset)
              bt .set_pixel_0_nt_32
            
            cmp/gt r2, r3  // if (vid_pnt > end_pnt)
            bt .x_loop_remainder_nt_32
      
      .x_loop_exit:
      
      sys_mode
      
      // increment the left and right side variables
      
      // r0: scratch
      
      // r7: dudy_r_dvdy_r
      // r8: dudy_l_dvdy_l
      // r9: dxdy_l_dxdy_r
      
      // r10: sv_l_sv_r
      // r11: su_l_su_r
      // r12: sx_l_sx_r
      
      // r13: height
      // r14: vid_y_offset
      
      mov.l @(dxdy_l_dxdy_r, sp), r9
      mov.l @(dudy_l_dvdy_l, sp), r8
      mov.l @(dudy_r_dvdy_r, sp), r7
      
      mov.l @(sx_l_sx_r, sp), r12
      mov.l @(su_r_sv_r, sp), r11
      mov.l @(su_l_sv_l, sp), r10
      
      add r9, r12  // sx_l_sx_r += dxdy_l_dxdy_r
      add r8, r11  // su_l_sv_l += dudy_l_dvdy_l
      add r7, r10  // su_r_sv_r += dudy_r_dvdy_r
      
      mov.l r12, @(sx_l_sx_r, sp)
      mov.l r11, @(su_l_sv_l, sp)
      mov.l r10, @(su_r_sv_r, sp)
      
      mov @(height, sp), r13
      mov @(vid_y_offset, sp), r14
      
      mov.w frame_width_ptr, r0
      add r0, r14  // vid_y_offset += SCREEN_WIDTH;
      dt r13  // height--;
      
      mov r13, @(height, sp)
      mov r14, @(vid_y_offset, sp)
      
      bf .y_second_loop  // while (height)
    
    mov.l @(height_l, sp), r4
    bra .y_main_loop
    mov.l @(height_r, sp), r5

.return:
  add stack_size, sp
  pop r8
  pop r9
  pop r10
  pop r11
  pop r12
  pop r13
  rts
  pop r14

.transparent_face:  // face has transparency
  cmp/eq #3, r9
  bt .x_loop_has_transparency_8
  cmp/eq #4, r9
  bt .x_loop_has_transparency_16
  bra .x_loop_exit
  nop
  
  .x_loop_has_transparency_8:
    cmp/gt r2, r3  // if (vid_pnt > end_pnt)
    bf .x_loop_exit
      
      mov r3, r1
      add #(4 * 2), r1  // end_offset = end_pnt + 4
      
      cmp/ge r2, r1  // if (vid_pnt >= end_offset)
      bt .set_pixel_0_tr_8
      
      .x_loop_remainder_tr_8:
        mov r1, r0
        sub r2, r0  // pc_offset = end_offset - vid_pnt
        shll r0
        mova .loop_table_tr_8, r8
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
        
        cmp/ge r2, r1  // while (vid_pnt >= end_offset)
        bt .set_pixel_0_tr_8
      
      cmp/gt r2, r3  // if (vid_pnt > end_pnt)
      bt .x_loop_remainder_tr_8
    
    bra .x_loop_exit
    nop
  
  .x_loop_has_transparency_16:
    cmp/ge r2, r3  // if (vid_pnt > end_pnt)
    bt .x_loop_exit
      
      mov r3, r1
      add #-(4 * 2), r1  // end_offset = end_pnt + 4
      
      cmp/gt r2, r1  // if (vid_pnt >= end_offset)
      bf .set_pixel_0_tr_16
      
      .x_loop_remainder_tr_16:
        mov r1, r0
        sub r2, r0  // pc_offset = end_offset - vid_pnt
        shll r0
        mova .loop_table_tr_16, r8
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
        
        cmp/ge r2, r1  // while (vid_pnt >= end_offset)
        bt .set_pixel_0_tr_16
      
      cmp/gt r2, r3  // if (vid_pnt > end_pnt)
      bt .x_loop_remainder_tr_16
  
  bra .x_loop_exit
  nop