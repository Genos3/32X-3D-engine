#include "asm_common.inc"

#define UNROLL_SCANLINE_LOOP 1

.section .sdram
.global _draw_poly_asm
.type _draw_poly_asm STT_FUNC

.extern _screen
.extern _div_lut

// void draw_poly_asm(poly_t *poly)

_draw_poly_asm:
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
    
    st.poly_flags:
      .space 4
    
    stack_size:
  .previous
  
  add #-stack_size, sp
  
  mov.l r4, @sp  // poly
  
  mov.l pool.poly_color_offset, r0
  add r4, r0  // poly + poly_color_offset
  mov.l @r0, r14  // poly.color
  mov.l @(poly.flags - poly.color, r0), r13  // poly.flags
  mov.l @(poly.num_vertices - poly.color, r0), r12  // poly.num_vertices
  
  mov.l r13, @(st.poly_flags, sp)
  
  // obtain the top and bottom vertices
  
  // read only:
  // r4: poly
  // r12: num_vertices
  // r13: poly.flags
  // r14: color
  
  // volatile:
  // r1: i
  // r2: vertices + j
  // r3: vertices + sup_vt
  // r5: vertices[j].y
  // r6: vertices[sup_vt].y
  // r7: vertices[inf_vt].y
  
  add #-1, r12  // num_vertices -= 1
  mov r12, r1  // i = num_vertices
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
  // r3: vertices + sup_vt
  
  // curr:
  // r8: *curr_vt_l
  // r9: *curr_vt_r
  // r10: height_l
  // r11: height_r
  
  // r13: vid_y_offset
  
  mov r12, r1
  shll2 r1
  add r12, r1  // (num_vertices - 1) * 5 (x, y, z, u, v)
  shll2 r1
  add r4, r1  // vertices + num_vertices - 1
  mov.l r1, @(st.num_vertices_s_ptr, sp)
  
  mov r3, r8  // int *curr_vt_l = sup_vt;
  mov r8, r9  // int *curr_vt_r = curr_vt_l;
  
  mov #0, r10  // int height_l = 0;
  mov #0, r11  // int height_r = 0;
  
  // set the screen offset for the start of the first scanline
  
  mov.l @(4, r9), r0
  shlr16 r0  // y0_i = vertices[sup_vt].y >> FP;
  
  mov.l pool.frame_width, r1
  mov.l pool.screen, r2
  mov.l @r2, r2
  mulu r0, r1
  sts macl, r13
  add r2, r13  // u16 *vid_y_offset = screen + y0_i * SCREEN_WIDTH;
  
  // main Y loop
  
  .y_main_loop:
    // next edge found on the left side
    
    cmp/pl r10  // while (!height_l)
    bt .left_edge_exit
      
      // r1: vertices[curr_vt_l].y, dty
      // r2: *next_vt_l, rdy
      // r3: div_lut
      
      // r4: vertices[curr_vt_l].x, sx_l
      // r5: sx_r
      // r6: dx, dxdy_l
      // r7: dxdy_r
      
      // r8: *curr_vt_l
      // r9: *curr_vt_r
      // r10: height_l
      // r11: height_r
      
      .left_edge_loop_repeat:
      
      mov r8, r2
      mov.l @(st.poly_flags, sp), r0
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
      
      mov.l @(4, r8), r1  // vertices[curr_vt_l].y
      mov.l @(4, r2), r10  // vertices[next_vt_l].y
      
      mov r1, r0
      shlr16 r10
      shlr16 r0
      sub r0, r10  // height_l = (vertices[next_vt_l].y >> FP) - (vertices[curr_vt_l].y >> FP);
      
      cmp/pz r10  // if (height_l < 0) return;
      bt 1f
        bra return
        nop
      1:
      
      cmp/pl r10
      bt 1f
        bra .left_edge_loop_repeat  // if (!height_l)
        mov r2, r8  // curr_vt_l = next_vt_l; // the delay slot makes this execute before the jump
      1:
        // if (height_l)
        
        mov.l @r8, r4  // vertices[curr_vt_l].x
        mov.l @r2, r6  // vertices[next_vt_l].x
        
        mov r2, r8  // curr_vt_l = next_vt_l;
        
        sub r4, r6  // fixed dx = vertices[next_vt_l].x - vertices[curr_vt_l].x;
        
        // unsigned integer division
        
        #if !DIV_LUT_ENABLE
          mov #1, r2
          shll16 r2
          mov r10, r0
          shll16 r0
          
          div0u
          
          .rept 16
            div1 r0, r2
          .endr
          
          rotcl r2
          extu.w r2, r2
        #else
          mov.l pool.div_lut, r3
          
          mov r10, r0
          shll r0  // u16
          mov.w @(r0, r3), r2  // fixed rdy = div_lut[height_l];  // .16 result
        #endif
        
        dmuls.l r2, r6
        sts mach, r0
        sts macl, r6
        xtrct r0, r6  // dxdy_l = fp_mul(dx, rdy);
        
        // initialize the side variables
        
        // sub-pixel precision
        
        mov r1, r0
        fp_trunc r0  // fp_trunc(vertices[curr_vt_l].y)
        sub r0, r1  // fixed dty = vertices[curr_vt_l].y - fp_trunc(vertices[curr_vt_l].y);
        
        dmuls.l r6, r1
        sts mach, r0
        sts macl, r1
        xtrct r0, r1
        add r1, r4  // sx_l = vertices[curr_vt_l].x + fp_mul(dxdy_l, dty);
    
    .left_edge_exit:
    
    // next edge found on the right side
    
    cmp/pl r11  // while (!height_r)
    bt .right_edge_exit
      
      // r1: vertices[curr_vt_r].y, dty
      // r2: *next_vt_r, rdy
      // r3: div_lut
      
      // r4: sx_l
      // r5: vertices[curr_vt_r].x, sx_r
      // r6: dxdy_l
      // r7: dx, dxdy_r
      
      // r8: *curr_vt_l
      // r9: *curr_vt_r
      // r10: height_l
      // r11: height_r
      
      .right_edge_loop_repeat:
      
      mov r9, r2
      mov.l @(st.poly_flags, sp), r0
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
      
      mov.l @(4, r9), r1  // vertices[curr_vt_r].y
      mov.l @(4, r2), r11  // vertices[next_vt_r].y
      
      mov r1, r0
      shlr16 r11
      shlr16 r0
      sub r0, r11  // height_r = (vertices[next_vt_r].y >> FP) - (vertices[curr_vt_r].y >> FP);
      
      cmp/pz r11
      bf return  // if (height_r < 0) return;
      
      cmp/pl r11
      bt 1f
        bra .right_edge_loop_repeat  // if (!height_r)
        mov r2, r9  // curr_vt_r = next_vt_r; // the delay slot makes this execute before the jump
      1:
        // if (height_r)
        
        mov.l @r9, r5  // vertices[curr_vt_r].x
        mov.l @r2, r7  // vertices[next_vt_r].x
        
        mov r2, r9  // curr_vt_r = next_vt_r;
        
        sub r5, r7  // fixed dx = vertices[next_vt_r].x - vertices[curr_vt_r].x;
        
        // unsigned integer division
        
        #if !DIV_LUT_ENABLE
          mov #1, r2
          shll16 r2
          mov r11, r0
          shll16 r0
          
          div0u
          
          .rept 16
            div1 r0, r2
          .endr
          
          rotcl r2
          extu.w r2, r2
        #else
          mov.l pool.div_lut, r3
          
          mov r11, r0
          shll r0  // u16
          mov.w @(r0, r3), r2  // fixed rdy = div_lut[height_r];  // .16 result
        #endif
        
        dmuls.l r2, r7
        sts mach, r0
        sts macl, r7
        xtrct r0, r7  // dxdy_r = fp_mul(dx, rdy);
        
        // initialize the side variables
        
        // sub-pixel precision
        
        mov r1, r0
        fp_trunc r0  // fp_trunc(vertices[curr_vt_r].y)
        sub r0, r1  // fixed dty = vertices[curr_vt_r].y - fp_trunc(vertices[curr_vt_r].y);
        
        dmuls.l r7, r1
        sts mach, r0
        sts macl, r1
        xtrct r0, r1
        add r1, r5  // sx_r = vertices[curr_vt_r].x + fp_mul(dxdy_r, dty);
    
    .right_edge_exit:
    
    // if the polygon doesn't have height return
    
    mov r10, r0
    or r11, r0
    tst r0, r0
    bt return  // if (!height_l && !height_r) return;
    
    // obtain the height to the next vertex on Y
    
    min_c r10, r11, r12  // int height = min_c(height_l, height_r);
    
    sub r12, r10  // height_l -= height;
    sub r12, r11  // height_r -= height;
    
    // second Y loop
    
    // r2: sx_r_i, *vid_pnt
    // r3: sx_l_i, *end_pnt
    
    // r4: sx_l
    // r5: sx_r
    // r6: dxdy_l
    // r7: dxdy_r
    
    // r8: vertices + curr_vt_l
    // r9: vertices + curr_vt_r
    // r10: height_l
    // r11: height_r
    
    // r12: height
    // r13: vid_y_offset
    
    // skips the first line if it doesn't have width
    cmp/ge r5, r4  // if (sx_l >= sx_r)
    bt .x_loop_exit
    
    .y_second_loop:
      mov r5, r2
      shlr16 r2  // int sx_r_i = sx_r >> FP;
      mov r4, r3
      shlr16 r3  // int sx_l_i = sx_l >> FP;
      
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
      add r13, r2  // u16 *vid_pnt = vid_y_offset + sx_r_i;
      shll r3
      add r13, r3  // u16 *end_pnt = vid_y_offset + sx_l_i;
      
      // scanline loop unrolled
      
      // r1: end_offset
      // r2: *vid_pnt
      // r3: *end_pnt
      
      // r14: color
      
      #if UNROLL_SCANLINE_LOOP
        cmp/gt r3, r2  // if (vid_pnt > end_pnt)
        bf .x_loop_exit
          
          mov r3, r1
          add #(4 * 2), r1  // end_offset = end_pnt + 4
          
          cmp/ge r1, r2  // if (vid_pnt >= end_offset)
          bt .x_loop_start
          
          .x_loop_remainder:
            mov r1, r0
            sub r2, r0  // pc_offset = end_offset - vid_pnt
            braf r0  // pc += pc_offset
            nop
            
            .x_loop_start:
              mov.w r14, @-r2
              mov.w r14, @-r2
              mov.w r14, @-r2
              mov.w r14, @-r2
            
            cmp/ge r1, r2  // while (vid_pnt >= end_offset)
            bt .x_loop_start
          
          cmp/gt r3, r2  // if (vid_pnt > end_pnt)
          bt .x_loop_remainder
      #else
        .x_loop:
          cmp/gt r3, r2  // while (vid_pnt > end_pnt)
          bf 1f
            bra .x_loop
            mov.w r14, @-r2
          1:
      #endif
      
      .x_loop_exit:
      
      // increment the left and right side variables
      
      // r4: sx_l
      // r5: sx_r
      // r6: dxdy_l
      // r7: dxdy_r
      
      // r12: height
      // r13: vid_y_offset
      
      add r6, r4  // sx_l += dxdy_l
      add r7, r5  // sx_r += dxdy_r
      
      mov.l pool.frame_width, r0
      add r0, r13  // vid_y_offset += SCREEN_WIDTH
      dt r12  // height--;
      
      bf .y_second_loop  // while (height)
    
    bra .y_main_loop
    nop

// memory pool

.align 4

pool.poly_color_offset:
  .long poly.color

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

return:
  add #stack_size, sp
  mov.l @sp+, r14
  mov.l @sp+, r13
  mov.l @sp+, r12
  mov.l @sp+, r11
  mov.l @sp+, r10
  mov.l @sp+, r9
  rts
  mov.l @sp+, r8
