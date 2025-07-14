#include "asm_common.inc"

.section .sdram
.global clip_poly_plane_asm
.type clip_poly_plane_asm STT_FUNC
.type clip_poly_line_plane_asm STT_FUNC

// void clip_poly_plane_asm(g_poly_t *poly, int plane)

clip_poly_plane_asm:
  push r8
  push r9
  push r10
  push r11
  push r12
  push r13
  push r14
  
  // stack, local variables

  .struct 0
    poly_cl.vertices:
      .space (8 * 5 * 4)  // x, y, z, u, v
    
    stack_size:
  .previous

  add -stack_size, sp
  
  // r4: poly (arg 0)
  // r5: plane (arg 1)
  
  // r6: dist_1
  // r8: vp
  // r9: poly->num_vertices
  // r10: poly->vertices + num_vt
  
  mov.l @(poly.num_vertices, r4), r9
  mov r9, r10
  add #-1, r10  // num_vt = poly->num_vertices - 1;
  shll2 r10
  add r10, r10  // num_vt *= 5 (x, y, z, u, v)
  shll2 r10
  add r4, r10  // poly->vertices + num_vt * 4
  
  // calculate the distance to the plane
  
  mov.l #0, r6  // dist_1 = 0; distance to border, positive is inside
  
  mov.l vp_ptr, r8
  
  // switch statement
  mov r5, r0
  shll2 r0
  mova .switch_table_0, r1
  mov.l @(r0, r1), r0
  jmp @r0  // pc = *(pc + (r0 << 2))
  nop
  
  .align 4
  .switch_table_0:
    .long .switch_0_case_0
    .long .switch_0_case_1
    .long .switch_0_case_2
    .long .switch_0_case_3
    .long .switch_0_case_4
    .long .switch_0_case_5
  
  .switch_0_case_0:  // near
    mov.l @(vp.z_near, r8), r8
    mov.l @(2 * 4, r10), r6  // poly->vertices[num_vt].z
    bra .switch_0_exit
    sub r8, r6  // dist_1 = poly->vertices[num_vt].z - vp.z_near;
  
  .switch_0_case_1:  // far
    mov.l @(vp.z_far, r8), r8
    mov.l @(2 * 4, r10), r3  // poly->vertices[num_vt].z
    mov r8, r6
    bra .switch_0_exit
    sub r3, r6  // dist_1 = vp.z_far - poly->vertices[num_vt].z;
  
  .switch_0_case_2:  // left
    mov.l @(vp.screen_side_x_dt, r8), r8
    mov.l @(2 * 4, r10), r3  // poly->vertices[num_vt].z
    mov @r10, r2  // poly->vertices[num_vt].x
    dmuls.l r3, r8
    sts mach r0
    sts macl r6
    xtrct r0, r6
    bra .switch_0_exit
    add r2, r6  // dist_1 = poly->vertices[num_vt].x + fp_mul(poly->vertices[num_vt].z, vp.screen_side_x_dt);
  
  .switch_0_case_3:  // right
    mov.l @(vp.screen_side_x_dt, r8), r8
    mov.l @(2 * 4, r10), r3  // poly->vertices[num_vt].z
    mov @r10, r2  // poly->vertices[num_vt].x
    dmuls.l r3, r8
    sts mach r0
    sts macl r6
    xtrct r0, r6
    bra .switch_0_exit
    sub r2, r6  // dist_1 = fp_mul(poly->vertices[num_vt].z, vp.screen_side_x_dt) - poly->vertices[num_vt].x;
  
  .switch_0_case_4:  // top
    mov.l @(vp.screen_side_x_dt, r8), r8
    mov.l @(2 * 4, r10), r3  // poly->vertices[num_vt].z
    mov @(4, r10), r2  // poly->vertices[num_vt].y
    dmuls.l r3, r8
    sts mach r0
    sts macl r6
    xtrct r0, r6
    bra .switch_0_exit
    add r2, r6  // dist_1 = poly->vertices[num_vt].y + fp_mul32(poly->vertices[num_vt].z, vp.screen_side_y_dt);
  
  .switch_0_case_5:  // bottom
    mov.l @(vp.screen_side_x_dt, r8), r8
    mov.l @(2 * 4, r10), r3  // poly->vertices[num_vt].z
    mov @(4, r10), r2  // poly->vertices[num_vt].y
    dmuls.l r3, r8
    sts mach r0
    sts macl r6
    xtrct r0, r6
    sub r2, r6  // dist_1 = fp_mul32(poly->vertices[num_vt].z, vp.screen_side_y_dt) - poly->vertices[num_vt].y;
  
  .switch_0_exit:
  
  // r4: poly
  // r5: plane
  
  // r6: dist_1
  // r7: dist_2
  // r8: vp.z_near / vp.z_far / vp.screen_side_x_dt / vp.screen_side_y_dt
  
  // r9: poly->num_vertices
  // r10: poly->vertices + num_vt
  // r11: poly->vertices + i
  // r12: poly_cl_num_vt
  // r13: poly_cl->vertices
  // r14: poly->bitfield
  
  // curr:
  // r2: poly->vertices[i].x / poly->vertices[i].y
  // r3: poly->vertices[i].r
  
  mov.l @(poly.bitfield, r4), r14  // poly->bitfield
  mov r4, r11  // i = poly->vertices
  
  mov sp, r13
  add poly_cl.vertices, r13  // poly_cl->vertices
  mov.l #0, r12  // poly_cl_num_vt = 0
  
  cmp/eq #0, r9  // if (poly->num_vertices)
  bt .loop_clip_lines_exit
  
  .loop_clip_lines:
    // switch statement
    mov r5, r0
    shll2 r0
    mova .switch_table_0, r1
    mov.l @(r0, r1), r0
    jmp @r0  // pc = *(pc + (r0 << 2))
    nop
    
    .align 4
    .switch_table_1:
      .long .switch_1_case_0
      .long .switch_1_case_1
      .long .switch_1_case_2
      .long .switch_1_case_3
      .long .switch_1_case_4
      .long .switch_1_case_5
    
    .switch_1_case_0:  // near
      mov.l @(2 * 4, r11), r7  // poly->vertices[i].z
      bra .switch_0_exit
      sub r8, r7  // dist_2 = poly->vertices[i].z - vp.z_near;
    
    .switch_1_case_1:  // far
      mov.l @(2 * 4, r11), r3  // poly->vertices[i].z
      mov r8, r7
      bra .switch_0_exit
      sub r3, r7  // dist_2 = vp.z_far - poly->vertices[i].z;
    
    .switch_1_case_2:  // left
      mov.l @(2 * 4, r11), r3  // poly->vertices[i].z
      mov @r11, r2  // poly->vertices[i].x
      dmuls.l r3, r8
      sts mach r0
      sts macl r7
      xtrct r0, r7
      bra .switch_0_exit
      add r2, r7  // dist_2 = poly->vertices[i].x + fp_mul(poly->vertices[i].z, vp.screen_side_x_dt);
    
    .switch_1_case_3:  // right
      mov.l @(2 * 4, r11), r3  // poly->vertices[i].z
      mov @r11, r2  // poly->vertices[i].x
      dmuls.l r3, r8
      sts mach r0
      sts macl r7
      xtrct r0, r7
      bra .switch_0_exit
      sub r2, r7  // dist_2 = fp_mul(poly->vertices[i].z, vp.screen_side_x_dt) - poly->vertices[i].x;
    
    .switch_1_case_4:  // top
      mov.l @(2 * 4, r11), r3  // poly->vertices[i].z
      mov @(4, r11), r2  // poly->vertices[i].y
      dmuls.l r3, r8
      sts mach r0
      sts macl r7
      xtrct r0, r7
      bra .switch_0_exit
      add r2, r7  // dist_2 = poly->vertices[i].y + fp_mul32(poly->vertices[i].z, vp.screen_side_y_dt);
    
    .switch_1_case_5:  // bottom
      mov.l @(2 * 4, r11), r3  // poly->vertices[i].z
      mov @(4, r11), r2  // poly->vertices[i].y
      dmuls.l r3, r8
      sts mach r0
      sts macl r7
      xtrct r0, r7
      sub r2, r7  // dist_2 = fp_mul32(poly->vertices[i].z, vp.screen_side_y_dt) - poly->vertices[i].y;
    
    .switch_1_exit:
    
    // r4: poly
  
    // r6: dist_1
    // r7: dist_2
    // r8: vp.z_near / vp.z_far / vp.screen_side_x_dt / vp.screen_side_y_dt
    
    // r9: poly->num_vertices
    // r10: poly->vertices + num_vt
    // r11: poly->vertices + i
    // r12: poly_cl_num_vt
    // r13: poly_cl->vertices
    // r14: poly->bitfield
    
    cmp/ge #0, r7  // if (dist_2 >= 0)  // inside
    blt .point_2_is_outside
      
      cmp/ge #0, r6  // if (dist_1 < 0)  // outside
      bge .point_1_is_inside
        
        mov r11, r0  // poly->vertices[i]
        mov r10, r1  // poly->vertices[num_vt]
        mov r7, r2  // dist_2
        mov r6, r3  // dist_1
        
        bsr clip_poly_line_plane_asm  // clip_poly_line_plane_asm(&poly->vertices[i], &poly->vertices[num_vt], dist_2, dist_1, &poly_cl->vertices[poly_cl_num_vt], poly->bitfield)
        nop
        
        add #(5 * 4), r13  // &poly_cl->vertices++
        add #1, r12  // poly_cl_num_vt++
      
      .point_1_is_inside:
      
      tst poly.has_texture.bit, r14
      bt 1f
        mov.l @r11, r0
        mov.l r0, @r13
        mov.l @(4, r11), r0
        mov.l r0, @(4, r13)
        mov.l @(8, r11), r0  // poly->vertices[i] (x, y, z)
        bra 2f
        mov.l r0, @(8, r13)  // poly_cl->vertices[poly_cl_num_vt] (x, y, z)
      1:
        mov.l @r11, r0
        mov.l r0, @r13
        mov.l @(4, r11), r0
        mov.l r0, @(4, r13)
        mov.l @(8, r11), r0
        mov.l r0, @(8, r13)
        mov.l @(12, r11), r0
        mov.l r0, @(12, r13)
        mov.l @(16, r11), r0  // poly->vertices[i] (x, y, z, u, v)
        mov.l r0, @(16, r13)  // poly_cl->vertices[poly_cl_num_vt] (x, y, z, u, v)
      2:
      
      add #(5 * 4), r13  // &poly_cl->vertices++
      bra .point_2_is_inside_exit
      add #1, r12  // poly_cl_num_vt++
    
    
    .point_2_is_outside:
      
      cmp/ge #0, r6  // if (dist_1 >= 0)  // inside
      bf .point_2_is_inside_exit
        
        mov r10, r0  // poly->vertices[num_vt]
        mov r11, r1  // poly->vertices[i]
        mov r6, r2  // dist_1
        mov r7, r3  // dist_2
        
        bsr clip_poly_line_plane_asm  // clip_poly_line_plane_asm(&poly->vertices[num_vt], &poly->vertices[i], dist_1, dist_2, &poly_cl->vertices[poly_cl_num_vt], poly->bitfield)
        nop
        
        add #(5 * 4), r13  // &poly_cl->vertices++
        add #1, r12  // poly_cl_num_vt++
    
    .point_2_is_inside_exit:
    
    mov r11, r10  // &poly->vertices[num_vt] = &poly->vertices[i]
    mov r7, r6  // dist_1 = dist_2
    
    add #(5 * 4), r11  // &poly->vertices[i]++
    
    dt r9  // i--
    bf .loop_clip_lines
  
  .loop_clip_lines_exit:
  
  // r4: poly
  // r14: poly->bitfield
  
  // curr:
  // r5: poly_cl->vertices
  // r12: i = poly_cl_num_vt
  
  mov sp, r5
  add #poly_cl.vertices, r5
  
  mov.l r12, @(poly.num_vertices, r4)  // poly->num_vertices = poly_cl_num_vt
  
  cmp/eq #0, r12  // if (poly_cl_num_vt)
  bt .not_textured_loop_exit
  
  tst poly.has_texture.bit, r14
  bf .has_texture_loop
  .not_textured_loop:
    mov.l @r5, r0
    mov.l r0, @r4
    mov.l @(4, r5), r0
    mov.l r0, @(4, r4)
    mov.l @(8, r5), r0  // poly_cl->vertices[i] (x, y, z)
    mov.l r0, @(8, r4)  // poly->vertices[i] (x, y, z)
    
    add #(5 * 4), r5
    add #(5 * 4), r4
    
    dt r12  // i--
    bf .not_textured_loop
    
    bra .not_textured_loop_exit
    nop
  
  .has_texture_loop:
    mov.l @r5+, r0
    mov.l r0, @r4
    mov.l @r5+, r0
    mov.l r0, @(4, r4)
    mov.l @r5+, r0
    mov.l r0, @(8, r4)
    mov.l @r5+, r0
    mov.l r0, @(12, r4)
    mov.l @r5+, r0  // poly_cl->vertices[i] (x, y, z, u, v)
    mov.l r0, @(16, r4)  // poly->vertices[i] (x, y, z, u, v)
    
    add #(5 * 4), r4
    
    dt r12  // i--
    bf .has_texture_loop
  
  .not_textured_loop_exit:
  
  add #-stack_size, sp
  pop r8
  pop r9
  pop r10
  pop r11
  pop r12
  pop r13
  rts
  pop r14

div_lutr_ptr:
.long div_lut

// void clip_poly_line_plane_asm(const vec5_t *vt_0, const vec5_t *vt_1, fixed dist_1, fixed dist_2, vec5_t *i_vt, int poly->bitfield)

clip_poly_line_plane_asm:
  push r4
  push r5
  
  // r0: *vt_0 (arg 0)
  // r1: *vt_1 (arg 1)
  // r2: dist_1 (arg 2)
  // r3: dist_2 (arg 3), s
  // r4: length (scratch)
  // r5: div_lut (scratch)
  // r13: *i_vt (arg 4)
  // r14: poly->bitfield (arg 5)
  
  mov r2, r4
  sub r3, r4  // length = dist_1 - dist_2
  
  cmp/eq #0, r4  // if (length == 0), result will always be positive
  bt .length_is_zero
    
    mov.l div_lutr_ptr, r5
    
    // unsigned fixed point division
    
    shlr8 r4
    div_luts r3, r4, r5  // s = div_luts(length >> 8) >> 8; // .16 result
    shlr8 r3
    
    dmuls.l r2, r3
    sts mach r2
    sts macl r3
    xtrct r2, r3  // s = fp_mul(dist_1, s);
  
  .length_is_zero_return:
  
  // obtain the intersecting point
  
  // r0: *vt_0
  // r1: *vt_1
  // r3: s
  // r13: *i_vt
  // r14: poly->bitfield
  
  mov.l @r0, r3  // vt_0->x
  mov.l @r1, r4  // vt_1->x
  sub r3, r4
  
  dmuls.l r3, r2
  sts mach r5
  sts macl r4
  xtrct r5, r4
  add r4, r3
  
  mov r3, @r13  // i_vt->x = vt_0->x + fp_mul((vt_1->x - vt_0->x), s)
  
  mov.l @(4, r0), r3  // vt_0->y
  mov.l @(4, r1), r4  // vt_1->y
  sub r3, r4
  
  dmuls.l r3, r2
  sts mach r5
  sts macl r4
  xtrct r5, r4
  add r4, r3
  
  mov r3, @(4, r13)  // i_vt->y = vt_0->y + fp_mul((vt_1->y - vt_0->y), s)
  
  mov.l @(8, r0), r3  // vt_0->z
  mov.l @(8, r1), r4  // vt_1->z
  sub r3, r4
  
  dmuls.l r3, r2
  sts mach r5
  sts macl r4
  xtrct r5, r4
  add r4, r3
  
  mov r3, @(8, r13)  // i_vt->z = vt_0->z + fp_mul((vt_1->z - vt_0->z), s)
  
  tst poly.has_texture.bit, r14
  bf 1f
    mov.l @(12, r0), r3  // vt_0->u
    mov.l @(12, r1), r4  // vt_1->u
    sub r3, r4
    
    dmuls.l r3, r2
    sts mach r5
    sts macl r4
    xtrct r5, r4
    add r4, r3
    
    mov r3, @(12, r13)  // i_vt->u = vt_0->u + fp_mul((vt_1->u - vt_0->u), s)
    
    mov.l @(16, r0), r3  // vt_0->v
    mov.l @(16, r1), r4  // vt_1->v
    sub r3, r4
    
    dmuls.l r3, r2
    sts mach r5
    sts macl r4
    xtrct r5, r4
    add r4, r3
    
    mov r3, @(16, r13)  // i_vt->v = vt_0->v + fp_mul((vt_1->v - vt_0->v), s)
  1:
  
  pop r4
  rts
  pop r5
  
  .length_is_zero:
    mov #1, r3
    shll16 r3  // s = 1 << 16