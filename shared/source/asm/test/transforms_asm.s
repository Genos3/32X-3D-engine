#include "asm_common.inc"

.section .sdram
.global transform_model_asm
.global transform_model_vis_asm
.global transform_vertex_asm
.type transform_model_asm STT_FUNC
.type transform_model_vis_asm STT_FUNC
.type transform_vertex_asm STT_FUNC

// void transform_model_asm(fixed matrix[restrict], const model_t *model)

// args:
// r4: matrix (arg 0)
// r5: model (arg 1)

transform_model_asm:
  // r0: scratch, dl.vertex_id[i]
  
  // r1: matrix
  // r2: vt
  // r3: tr_vertices
  // r4: i (num_vertices)
  
  // r5: model
  // r7: model->vertices
  // r8: *dl.vertex_id
  
  mov r4, r1
  
  mov.l tr_vertices_ptr, r3
  mov.l @r3, r3
  
  mov.l @(model.num_vertices, r5), r4  // i = model->num_vertices
  mov.l @(model.vertices, r5), r2  // vt
  
  .loop_num_vertices:
    cmp/eq #0, r4  // while (i != 0)
    bt .loop_num_vertices_exit
    
    // transform the vertices
    
    // r0: p
    // r1: matrix
    // r2: vt
    // r3: tr_vertices
    // r4: i (num_vertices)
    // r5: *vt
    // r6: *matrix (0)
    
    mov.l @r2+, r6  // *matrix (0); matrix++
    
    // p.x = matrix[0] + ((vt->x * matrix[1]) + (vt->y * matrix[2]) + (vt->z * matrix[3])) >> FP;
    
    clrmac
    mac.l @r1+, @r2+
    mac.l @r1+, @r2+
    mac.l @r1+, @r2+  // vt += 3; matrix += 3
    sts mach r5
    sts macl r0
    xtrct r5, r0
    add r6, r0
    
    mov r0, @r3+  // tr_vertices.x = p.x; tr_vertices++
    
    mov.l @r2+, r6  // *matrix (4); matrix++
    
    // p.y = matrix[4] + ((vt->x * matrix[5]) + (vt->y * matrix[6]) + (vt->z * matrix[7])) >> FP;
    
    clrmac
    mac.l @r1+, @r2+
    mac.l @r1+, @r2+
    mac.l @r1+, @r2+  // vt += 3; matrix += 3
    sts mach r5
    sts macl r0
    xtrct r5, r0
    add r6, r0
    
    mov r0, @r3+  // tr_vertices.y = p.y; tr_vertices++
    
    mov.l @r2+, r6  // *matrix (8); matrix++
    
    // p.z = matrix[8] + ((vt->x * matrix[9]) + (vt->y * matrix[10]) + (vt->z * matrix[11])) >> FP;
    
    clrmac
    mac.l @r1+, @r2+
    mac.l @r1+, @r2+
    mac.l @r1+, @r2+  // vt += 3; matrix += 3
    sts mach r5
    sts macl r0
    xtrct r5, r0
    add r6, r0
    
    mov r0, @r3+  // tr_vertices.z = p.z; tr_vertices++
    
    add #-(12 * 4), r3  // matrix -= 12
    
    bra .loop_num_vertices
    dt r4  // i--
  
  .loop_num_vertices_exit:
  
  rts
  nop

// void transform_model_vis_asm(fixed matrix[restrict], const model_t *model)

// args:
// r4: matrix (arg 0)
// r5: model (arg 1)

transform_model_vis_asm:
  push r8
  
  // r0: scratch, dl.vertex_id[i]
  
  // r1: matrix
  // r2: vt
  // r3: tr_vertices
  // r4: i (num_vertices)
  
  // r5: model
  // r7: model->vertices
  // r8: *dl.vertex_id
  
  mov r4, r1
  
  mov.l tr_vertices_ptr, r3
  mov.l @r3, r3
  
  // if (!dl_mode)
  // mov.l @(model.num_vertices, r12), r11  // i = model->num_vertices
  // bra 2f
  // mov.l @(model.vertices, r12), r1  // model->vertices
  
  mov.l dl_ptr, r0
  mov.l @(dl.num_vertices, r0), r4  // i = dl.num_vertices
  mov.l @(model.vertices, r5), r7  // model->vertices
  mov.l @(dl.vertex_id, r0), r8
  
  .loop_num_vertices:
    cmp/eq #0, r4  // while (i != 0)
    bt .loop_num_vertices_exit
    
    mov.w @r8+, r0  // dl.vertex_id[i]; i++
    shll r0
    add r0, r0  // r9 *= 3
    shll2 r0
    mov r7, r2
    add r0, r2  // vt = &model->vertices[dl.vertex_id[i]]
    
    // transform the vertices
    
    // r0: p
    // r1: matrix
    // r2: vt
    // r3: tr_vertices
    // r4: i (num_vertices)
    // r5: *vt
    // r6: *matrix (0)
    
    mov.l @r2+, r6  // *matrix (0); matrix++
    
    // p.x = matrix[0] + ((vt->x * matrix[1]) + (vt->y * matrix[2]) + (vt->z * matrix[3])) >> FP;
    
    clrmac
    mac.l @r1+, @r2+
    mac.l @r1+, @r2+
    mac.l @r1+, @r2+  // vt += 3; matrix += 3
    sts mach r5
    sts macl r0
    xtrct r5, r0
    add r6, r0
    
    mov r0, @r3+  // tr_vertices.x = p.x; tr_vertices++
    
    mov.l @r2+, r6  // *matrix (4); matrix++
    
    // p.y = matrix[4] + ((vt->x * matrix[5]) + (vt->y * matrix[6]) + (vt->z * matrix[7])) >> FP;
    
    clrmac
    mac.l @r1+, @r2+
    mac.l @r1+, @r2+
    mac.l @r1+, @r2+  // vt += 3; matrix += 3
    sts mach r5
    sts macl r0
    xtrct r5, r0
    add r6, r0
    
    mov r0, @r3+  // tr_vertices.y = p.y; tr_vertices++
    
    mov.l @r2+, r6  // *matrix (8); matrix++
    
    // p.z = matrix[8] + ((vt->x * matrix[9]) + (vt->y * matrix[10]) + (vt->z * matrix[11])) >> FP;
    
    clrmac
    mac.l @r1+, @r2+
    mac.l @r1+, @r2+
    mac.l @r1+, @r2+  // vt += 3; matrix += 3
    sts mach r5
    sts macl r0
    xtrct r5, r0
    add r6, r0
    
    mov r0, @r3+  // tr_vertices.z = p.z; tr_vertices++
    
    add #-(12 * 4), r3  // matrix -= 12
    
    bra .loop_num_vertices
    dt r4  // i--
  
  .loop_num_vertices_exit:
  
  rts
  pop r8

// void transform_vertex_asm(vec3_t *p0, vec3_t *p1, fixed matrix[restrict])

transform_vertex_asm:
  // r4: *p0 (arg 0)
  // r5: *p0 (arg 1)
  // r6: *matrix (arg 3)
  
  mov.l @r6+, r2  // *matrix (0); matrix++
  
  // p.x = matrix[0] + ((vt->x * matrix[1]) + (vt->y * matrix[2]) + (vt->z * matrix[3])) >> FP;
  
  clrmac
  mac.l @r4+, @r6+
  mac.l @r4+, @r6+
  mac.l @r4+, @r6+  // vt += 3; matrix += 3
  sts mach r1
  sts macl r0
  xtrct r1, r0
  add r2, r0
  
  mov r0, @r5+  // tr_vertices.x = p.x; tr_vertices++
  
  add #-(3 * 4), r4  // vt -= 3
  mov.l @r6+, r2  // *matrix (4); matrix++
  
  // p.y = matrix[4] + ((vt->x * matrix[5]) + (vt->y * matrix[6]) + (vt->z * matrix[7])) >> FP;
  
  clrmac
  mac.l @r4+, @r6+
  mac.l @r4+, @r6+
  mac.l @r4+, @r6+  // vt += 3; matrix += 3
  sts mach r1
  sts macl r0
  xtrct r1, r0
  add r2, r0
  
  mov r0, @r5+  // tr_vertices.y = p.y; tr_vertices++
  
  add #-(3 * 4), r4  // vt -= 3
  mov.l @r6+, r2  // *matrix (8); matrix++
  
  // p.z = matrix[8] + ((vt->x * matrix[9]) + (vt->y * matrix[10]) + (vt->z * matrix[11])) >> FP;
  
  clrmac
  mac.l @r4+, @r6+
  mac.l @r4+, @r6+
  mac.l @r4+, @r6+  // vt += 3; matrix += 3
  sts mach r1
  sts macl r0
  xtrct r1, r0
  add r2, r0
  
  rts
  mov r0, @r5+  // tr_vertices.z = p.z; tr_vertices++