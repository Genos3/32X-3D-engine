#include "asm_common.inc"

.section .sdram
.global draw_sprite_asm
.type draw_sprite_asm STT_FUNC

// pixel macro

// r0: su_i (scratch)
// r2: *vid_pnt
// r3: *end_pnt
// r4: su
// r5: sv_i
// r6: tx_color (scratch)
// r7: texture_image
// r8: su_l
// r10: dudx

// r13: cr_palette_tx_idx
// r14: final_light_factor

.macro set_pixel
  mov r4, r0
  shlr8 r0  // uint su_i = su >> 8
  
  add r5, r0
  mov.b @(r0, r12), r6  // u16 tx_color = texture_image[(sv_i << texture_width_bits) + su_i];
  
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
      mov.w @(r6, r13), r6   // textures->cr_palette_tx_idx[tx_color];
      mov.w r6, @r2  // *vid_pnt = tx_color;
    #endif
  1:
  
  add #-2, r2  // vid_pnt--;
  
  add r10, r4  // su += dudx;
.endm

// void draw_sprite_asm(poly_t *poly)

draw_sprite_asm:
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
    
    height:
      .space 4
    vid_y_offset:
      .space 4
    
    stack_size:
  .previous
  
  add -stack_size, sp
  
  mov r4, r0
  
  mov.l r0, @(poly, sp)
  
  // obtain the top/left vertex
  
  // read only:
  // r0: poly (arg 0)
  
  // volatile:
  // r1: i
  // r2: vertices + j
  // r3: vertices + vt_0
  // r4: vertices[j].x
  // r5: vertices[j].y
  // r6: vertices[vt_0].x
  // r7: vertices[vt_0].y
  
  mov #3, r1  // i = 3
  mov r0, r2  // j = vertices
  mov r0, r3  // vt_0 = vertices
  
  mov @r3, r6  // vertices[vt_0].x
  mov @(4, r3), r7  // vertices[vt_0].y
  
  .loop_sort_x_y:
    add #(5 * 4), r2  // j++
    mov.l @r2, r4  // vertices[j].x
    mov.l @(4, r2), r4  // vertices[j].y
    
    cmp/gt r4, r6  // if (vertices[j].x <= vertices[vt_0].x)
    cmp/gt r5, r7  // if (vertices[j].y <= vertices[vt_0].y)
    bf 1f
      mov r4, r6  // vertices[j].x = vertices[vt_0].x
      mov r5, r7  // vertices[j].y = vertices[vt_0].y
      mov r2, r3  // vt_0 = j
    1:
    
    dt r1  // i--
    bf .loop_sort_x_y  // i != 0
  
  // r3: vertices + vt_0
  // r4: vertices + vt_1
  // r5: vertices + 4
  
  mov r0, r5
  add #(4 * 5 * 4), r5  // vertices + 4
  
  mov r3, r4
  add #(2 * 5 * 4), r4  // int vt_1 = vt_0 + 2
  cmp/ge r4, r5  // if (vt_1 >= 4)
  bf 1f
    add #-(4 * 5 * 4), r4  // vt_1 -= 4
  1:
  
  // calculate the deltas
  
  // prev:
  // r3: vertices + vt_0
  // r4: vertices + vt_1
  // r5: vertices + 4
  // r6: vertices[vt_0].x
  // r7: vertices[vt_0].y
  
  // curr:
  // r7: y0_i
  // r8: su_l
  // r9: su_r
  // r10: dudx
  // r11: dvdy
  // r12: dx
  // r13: dy / height
  
  // r14: div_lut
  
  mov.l @(r4), r8  // vertices[vt_1].x
  
  shlr16 r8
  shlr16 r6
  mov r8, r12
  sub r6, r12  // dx = (vertices[vt_1].x >> FP) - (vertices[vt_0].x >> FP);
  
  mov.l @(12, r3), r1  // vertices[vt_0].u
  mov.l @(12, r4), r10  // vertices[vt_1].u
  
  sub r1, r10  // du = vertices[vt_1].u - vertices[vt_0].u
  
  mov.l div_lutr_ptr, r14
  
  // unsigned integer division
  
  mov.l r12, r0
  shll2 r0
  mov.l @(r0, r14), r0  // rdx = div_luts(dx);  // .16 result
  
  dmulu.l r10, r0
  sts mach r0
  sts macl r10
  xtrct r0, r10  // dudx = fp_mul(du, rdx);
  
  mov.l @(4, r4), r8  // vertices[vt_1].y
  
  shlr16 r8
  shlr16 r7
  mov r8, r13
  sub r7, r13  // dy = (vertices[vt_1].y >> FP) - (vertices[vt_0].y >> FP);
  
  mov.l @(12, r3), r2  // vertices[vt_0].v
  mov.l @(12, r4), r11  // vertices[vt_1].v
  
  sub r2, r11  // dv = vertices[vt_1].v - vertices[vt_0].v
  
  // unsigned integer division
  
  mov.l r11, r0
  shll2 r0
  mov.l @(r0, r14), r0 // rdy = div_luts(dy);  // .16 result
  
  dmulu.l r11, r0
  sts mach r0
  sts macl r11
  xtrct r0, r11  // dvdy = fp_mul(dv, rdy);
  
  // initialize the side variables
  
  mov r1, r8  // su_l = vertices[0].u
  mov r2, r9  // sv_l = vertices[0].v
  
  // set the screen offset for the start of the first scanline
  
  mov.w frame_width_ptr, r0
  mov.l screen_ptr, r1
  mov.l @r1, r1
  mul r7, r0
  sts macl, r14
  add r1, r14
  add r6, r14  // u16 *vid_y_offset = screen + y0_i * SCREEN_WIDTH + x0_i;
  
  shll r12  // dx <<= 1
  
  // Y loop
  
  // prev:
  // r8: su_l
  // r9: sv_l
  // r10: dudx
  // r11: dvdy
  // r12: dx
  
  // r13: height
  // r14: vid_y_offset
  
  // curr:
  // r0: su_i (scratch)
  // r2: *vid_pnt
  // r3: *end_pnt
  // r4: su
  // r5: sv_i
  
  .y_loop:
    mov r6, r4  // su = su_l
    mov r7, r5
    shlr8 r5  // sv_i = sv_l >> 8
    
    // set the pointers for the start and the end of the scanline
    
    mov r14, r2
    add r12, r2  // vid_pnt = vid_y_offset + dx;
    mov r14, r3  // end_pnt = vid_y_offset
    
    // r6: texture_width_bits
    // r7: texture_image
    // r13: cr_palette_tx_idx
    // r14: final_light_factor
    
    mov.l @(poly, sp), r0
    mov.l @(poly.texture_width_bits, r0), r6
    mov.l @(poly.texture_image, r0), r7
    mov.l @(poly.cr_palette_tx_idx, r0), r13
    mov.l @(poly.final_light_factor, r0), r14
    
    cmp/eq #3, r6  // if (texture_width_bits == 3)
    bf 1f
      shll2 r5
      shll r5  // sv_i <<= 3
      bra 2f
      nop
    1:
    cmp/eq #4, r6  // if (texture_width_bits == 4)
    bf 2f
      shll2 r5
      shll2 r5  // sv_i <<= 4
    2:
    
    // scanline loop
    
    // r0: scratch
    // r1: end_offset
    // r2: *vid_pnt
    // r3: *end_pnt
    // r4: su
    // r5: sv_i
    // r6: scratch
    // r7: texture_image
    // r8: su_l
    // r9: sv_l
    // r10: dudx
    // r11: dvdy
    // r12: dx
    
    // r13: cr_palette_tx_idx
    // r14: final_light_factor
    
    .x_loop:
      cmp/gt r2, r3  // if (vid_pnt > end_pnt)
      bf .x_loop_exit
        
        mov r3, r1
        add #(4 * 2), r1  // end_offset = end_pnt + 4
        
        cmp/ge r2, r1  // if (vid_pnt >= end_offset)
        bt .set_pixel_0
        
        .x_loop_remainder:
          mov r1, r0
          sub r2, r0  // pc_offset = end_offset - vid_pnt
          shll r0
          mova .loop_table, r6
          mov.l @(r0, r6), r0
          jmp @r0  // pc = *(pc + (pc_offset << 1))
          nop
          
          .align 4
          .loop_table:
            .long .set_pixel_0
            .long .set_pixel_1
            .long .set_pixel_2
            .long .set_pixel_3
          
          .set_pixel_0:
            set_pixel
          .set_pixel_1:
            set_pixel
          .set_pixel_2:
            set_pixel
          .set_pixel_3:
            set_pixel
          
          cmp/ge r2, r1  // while (vid_pnt >= end_offset)
          bt .set_pixel_0
        
        cmp/gt r2, r3  // if (vid_pnt > end_pnt)
        bt .x_loop_remainder
    
    .x_loop_exit:
    
    // r11: dvdy
    // r9: sv_l
    
    // r13: height
    // r14: vid_y_offset
    
    add r11, r9  // sv_l += dvdy
    
    mov @(height, sp), r13
    mov @(vid_y_offset, sp), r14
    
    mov.w frame_width_ptr, r0
    add r0, r14  // vid_y_offset += SCREEN_WIDTH;
    dt r13  // height--;
    
    mov r13, @(height, sp)
    mov r14, @(vid_y_offset, sp)
    
    bf .y_loop  // while (height)
  
  .y_loop_exit:
  
  add stack_size, sp
  pop r8
  pop r9
  pop r10
  pop r11
  pop r12
  pop r13
  rts
  pop r14