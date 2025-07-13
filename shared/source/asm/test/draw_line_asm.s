#include "asm_common.inc"

.section .sdram
.global draw_line_asm
.type draw_line_asm STT_FUNC

// pixel macro

// r1: dxdy
// r2: vid_y_inc
// r3: vid_y_offset
// r4: sx_i
// r5: sx
// r7: height
// r8: color

.macro set_pixel_y
  mov r5, r0
  shlr8 r0  // uint sx_i = sx >> 8
  mov.w r8, @(r0, r3)  // vid_y_offset + sx_i = color
  
  add r1, r5  // sx += dxdy
  add r2, r3  // vid_y_offset += vid_y_inc
  
  dt r7  // height--
.endm

// r1: dydx
// r2: vid_y_inc
// r3: vid_y_offset
// r4: vid_pnt
// r5: sy
// r6: width
// r8: color

.macro set_pixel_x
  mov.w r8, @r4+  // *vid_pnt++ = color;
  
  addc r1, r5  // sy += dydx;
  bf/s 1f
  dt r6 // width--;
    add r2, r8  // vid_pnt += vid_y_inc;
  1:
  
.endm

frame_width_ptr:
.long (SCREEN_WIDTH * 2)

screen_ptr:
.long screen

div_lutr_ptr:
.long div_lut

// void draw_line_asm(fixed x0, fixed y0, fixed x1, fixed y1, u16 color)

// args:
// r4: x0 (arg 0)
// r5: y0 (arg 1)
// r6: x1 (arg 2)
// r7: y1 (arg 3)
// r8: color (arg 4)

draw_line_asm:
  push r8
  
  mov.w @(4, sp), r8  // color (r8)
  
  // if x0 > x1 exchange the vertices in order to always draw from left to right
  
  bpl .dx_is_positive
    mov r4, r0  // t = x0;
    mov r6, r4  // x0 = x1;
    mov r0, r6  // x1 = t;
    mov r5, r0  // t = y0;
    mov r7, r5  // y0 = y1;
    mov r0, r7  // y1 = t;
  .dx_is_positive:
  
  mov r4, r0
  shlr16 r0
  shlr16 r6
  sub r0, r6  // adx_i = (x1 >> FP) - (x0 >> FP);
  
  mov r5, r1
  shlr16 r1
  shlr16 r7
  sub r1, r7  // dy_i = (y1 >> FP) - (y0 >> FP);
  
  // r1: y0_i
  // r2: vid_y_inc
  // r3: vid_y_offset
  // r4: x0
  // r5: y0
  // r6: adx_i
  // r7: ady_i
  // r8: color
  
  mov r2, r0
  or r3, r0
  tst r0, r0
  bt .return  // if (!adx_i && !dy_i) return;
  
  mov.l @(frame_width_ptr), r2
  shll r2  // vid_y_inc = SCREEN_WIDTH << 1
  
  mov.l screen_ptr, r0
  mov.l @r0, r0
  mul r1, r2
  sts macl, r3
  add r0, r3  // u16 *vid_y_offset = screen + y0_i * SCREEN_WIDTH;
  
  cmp/ge #0 r7  // if (dy_i >= 0)
  bt 1f
    neg r2  // vid_y_inc = -SCREEN_WIDTH
    neg r7  // int ady_i = -dy
  1:
  
  cmp/ge r7, r6  // if (ady_i >= adx_i)
  bf .x_is_larger
    // r1: rdy, dxdy
    // r2: vid_y_inc
    // r3: vid_y_offset
    // r4: x0, sx_i
    // r5: y0, dty, sx
    // r6: adx_i
    // r7: ady_i, height
    // r8: color
    
    cmp/eq #0, r7
    cmp/eq #0, r6
    bt .skip_div_a  // if (!ady_i || !adx_i)
      
      mov.l div_lutr_ptr, r1
      
      // unsigned integer division
      
      mov.l r7, r0
      shll2 r0
      mov.l @(r0, r1), r1  // fixed rdy = div_luts(ady_i)  // .16 result
      
      mulu.l r6, r1
      sts macl r1  // dxdy = adx_i * rdy  // always positive
    
    .skip_div_a_return:
    
    // sub-pixel precision
    
    mov r5, r0
    fp_trunc r0  // fp_trunc(y0)
    sub r0, r5  // fixed dty = y0 - fp_trunc(y0);
    
    dmuls.l r1, r5
    sts mach r0
    sts macl r5
    xtrct r0, r5
    add r4, r5  // sx = x0 + fp_mul(dxdy, dty);
    
    add #1, r7  // height++
    
    // unrolled y loop
    
    cmp/ge r7, #4  // if (height >= 4)
    bt .set_pixel_y_0
    
    .y_loop_remainder:
      mov #4, r0
      sub r7, r0  // height_offset = height - 4
      shll r0
      mova .loop_table_y, r1
      mov.l @(r0, r1), r0
      jmp @r0  // pc = *(pc + (height_offset << 1))
      nop
      
      .align 4
      .loop_table_y:
        .long .set_pixel_y_0
        .long .set_pixel_y_1
        .long .set_pixel_y_2
        .long .set_pixel_y_3
      
      .set_pixel_y_0:
        set_pixel_y
      .set_pixel_y_1:
        set_pixel_y
      .set_pixel_y_2:
        set_pixel_y
      .set_pixel_y_3:
        set_pixel_y
      
      cmp/ge r7, #4
      bt .set_pixel_y_0
      
      cmp/gt r7, #0
      bt .y_loop_remainder
    
    bra .return
    nop
  
  .x_is_larger:
    // r1: rdx, dydx
    // r2: vid_y_inc
    // r3: vid_y_offset
    // r4: x0, xo_i, vid_pnt
    // r5: y0, sy
    // r6: adx_i, width
    // r7: ady_i, dtx
    // r8: color
    
    cmp/eq #0, r7
    cmp/eq #0, r6
    bt .skip_div_b  // if (!ady_i || !adx_i)
      
      mov.l div_lutr_ptr, r1
      
      // unsigned integer division
      
      mov.l r6, r0
      shll2 r0
      mov.l @(r0, r1), r1  // fixed rdx = div_luts(adx_i)  // .16 result
      
      mulu.l r7, r1
      sts macl r1  // dydx = ady_i * rdx  // always positive
    
    .skip_div_b_return:
    
    // sub-pixel precision
    
    mov r4, r0
    fp_trunc r0  // fp_trunc(x0)
    sub r0, r7  // fixed dtx = x0 - fp_trunc(x0);
    
    dmuls.l r1, r7
    sts mach r0
    sts macl r1
    xtrct r0, r1
    add r1, r5  // sy = y0 + fp_mul(dydx, dtx);
    
    shlr16 r0  // x0_i = x0 >> FP;
    shll r0
    add r7, r0  // vid_pnt = vid_y_offset + (x0_i << 1)
    
    // unrolled y loop
    
    cmp/ge r7, #4  // if (height >= 4)
    bt .set_pixel_x_0
    
    .x_loop_remainder:
      mov #4, r0
      sub r7, r0  // height_offset = height - 4
      shll r0
      mova .loop_table_x, r1
      mov.l @(r0, r1), r0
      jmp @r0  // pc = *(pc + (height_offset << 1))
      nop
      
      .align 4
      .loop_table_x:
        .long .set_pixel_x_0
        .long .set_pixel_x_1
        .long .set_pixel_x_2
        .long .set_pixel_x_3
      
      .set_pixel_x_0:
        set_pixel_x
      .set_pixel_x_1:
        set_pixel_x
      .set_pixel_x_2:
        set_pixel_x
      .set_pixel_x_3:
        set_pixel_x
      
      cmp/ge r7, #4
      bt .set_pixel_x_0
      
      cmp/gt r7, #0
      bt .x_loop_remainder
  
  .return
    rts
    pop r8
  
  .skip_div_a:
    bra .skip_div_a_return
    mov #0, r1  // dxdy = 0
  
  .skip_div_b:
    bra .skip_div_b_return
    mov #0, r1  // dydx = 0