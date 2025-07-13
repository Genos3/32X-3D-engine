#include "asm_common.inc"

.section .sdram
.global memset8_asm
.global memset16_asm
.global memset32_asm
.global memcpy16_asm
.global memcpy32_asm
.type memset8_asm STT_FUNC
.type memset16_asm STT_FUNC
.type memset32_asm STT_FUNC
.type memcpy16_asm STT_FUNC
.type memcpy32_asm STT_FUNC

// void memset8_asm(void *dst, u32 val, uint count)

memset8_asm:
  // r4: *dst (arg 0)
  // r5: val (arg 1)
  // r6: count (arg 2), start_pnt
  
  // r1: end_offset
  
  cmp/pl r6  // if (count)
  bf .main_loop_exit_ms8
  
  add r4, r6  // start_pnt = dst + count
  
  mov r4, r1
  add #8, r1  // end_offset = dst + 4
  
  cmp/ge r6, r1  // if (start_pnt >= end_offset)
  bt .loop_start_ms8
  
  .loop_remainder_ms8:
    mov r1, r0
    sub r6, r0  // pc_offset = end_offset - start_pnt
    braf r0  // pc += pc_offset
    nop
    
    .x_loop_start_ms8:
      mov.b r5, @-r6
      mov.b r5, @-r6
      mov.b r5, @-r6
      mov.b r5, @-r6
      mov.b r5, @-r6
      mov.b r5, @-r6
      mov.b r5, @-r6
      mov.b r5, @-r6  // *--start_pnt = val
    
    cmp/ge r6, r1  // if (start_pnt >= end_offset)
    bt .x_loop_start_ms8
  
  cmp/gt r6, r4  // if (start_pnt > dst)
  bt .loop_remainder_ms8
  
  rts
  nop

// void memset16_asm(void *dst, u32 val, uint count)

memset16_asm:
  // r4: *dst (arg 0)
  // r5: val (arg 1)
  // r6: count (arg 2), start_pnt
  
  // r1: end_offset
  
  cmp/pl r6  // if (count)
  bf .main_loop_exit_ms16
  
  shll1 r6
  add r4, r6  // start_pnt = dst + (count << 1)
  
  mov r4, r1
  add #(4 * 2), r1  // end_offset = dst + 4
  
  cmp/gt r6, r1  // if (start_pnt > end_offset)
  bt .loop_start_ms16
  
  .loop_remainder_ms16:
    mov r1, r0
    sub r6, r0  // pc_offset = end_offset - start_pnt
    braf r0  // pc += pc_offset
    nop
    
    .x_loop_start_ms16:
      mov.w r5, @-r6
      mov.w r5, @-r6
      mov.w r5, @-r6
      mov.w r5, @-r6  // *--start_pnt = val
    
    cmp/gt r6, r1  // if (start_pnt > end_offset)
    bt .x_loop_start_ms16
  
  cmp/gt r6, r4  // if (start_pnt > dst)
  bt .loop_remainder_ms16
  
  rts
  nop

// void memset32_asm(void *dst, u32 val, uint count)

memset32_asm:
  // r4: *dst (arg 0)
  // r5: val (arg 1)
  // r6: count (arg 2), start_pnt
  
  // r1: end_offset
  
  cmp/pl r6  // if (count)
  bf .main_loop_exit_ms32
  
  shll2 r6
  add r4, r6  // start_pnt = dst + (count << 2)
  
  mov r4, r1
  add #(4 * 4), r1  // end_offset = dst + 4
  
  cmp/ge r6, r1  // if (start_pnt >= end_offset)
  bt .loop_start_ms32
  
  .loop_remainder_ms32:
    mov r1, r0
    sub r6, r0  // pc_offset = end_offset - start_pnt
    braf r0  // pc += pc_offset
    nop
    
    .x_loop_start_ms32:
      mov.l r5, @-r6
      mov.l r5, @-r6
      mov.l r5, @-r6
      mov.l r5, @-r6  // *--start_pnt = val
    
    cmp/ge r6, r1  // if (start_pnt >= end_offset)
    bt .x_loop_start_ms32
  
  cmp/gt r6, r4  // if (start_pnt > dst)
  bt .loop_remainder_ms32
  
  rts
  nop

.section data

// void memcpy16_asm(void *dst, const void *src, int count)

memcpy16_asm:
  // r4: *dst (arg 0)
  // r5: *src (arg 1)
  // r6: count (arg 2), end_pnt
  
  // r1: end_offset
  
  cmp/pl r6  // if (count)
  bf .main_loop_exit_mc16
  
  shll1 r6
  add r4, r6  // end_pnt = dst + (count << 1)
  
  mov r6, r1
  add #-(4 * 2), r1  // end_offset = end_pnt - 4
  
  cmp/gt r4, r1  // if (dst <= end_offset)
  bf .loop_start_mc16
  
  .loop_remainder_mc16:
    mov r4, r0
    sub r1, r0  // pc_offset = dst - end_offset
    braf r0  // pc += pc_offset
    nop
    
    .x_loop_start_mc16:
      mov.w @r5+, r0
      mov.w r0, @r4
      mov.w @r5+, r0
      mov.w r0, @(2, r4)
      mov.w @r5+, r0
      mov.w r0, @(4, r4)
      mov.w @r5+, r0
      mov.w r0, @(6, r4)  // *dst = *src++
      add #8, r4  // dst += 4
    
    cmp/gt r4, r1  // if (dst <= end_offset)
    bf .x_loop_start_mc16
  
  cmp/ge r4, r6  // if (dst < end_pnt)
  bf .loop_remainder_mc16
  
  rts
  nop

// void memcpy32_asm(void *dst, const void *src, int count)

memcpy32_asm:
  // r4: *dst (arg 0)
  // r5: *src (arg 1)
  // r6: count (arg 2), end_pnt
  
  // r1: end_offset
  
  cmp/pl r6  // if (count)
  bf .main_loop_exit_mc32
  
  shll2 r6
  add r4, r6  // end_pnt = dst + (count << 2)
  
  mov r6, r1
  add #-(4 * 4), r1  // end_offset = end_pnt - 4
  
  cmp/gt r4, r1  // if (dst <= end_offset)
  bf .loop_start_mc32
  
  .loop_remainder_mc32:
    mov r4, r0
    sub r1, r0  // pc_offset = dst - end_offset
    braf r0  // pc += pc_offset
    nop
    
    .x_loop_start_mc32:
      mov.l @r5+, r0
      mov.l r0, @r4
      mov.l @r5+, r0
      mov.l r0, @(4, r4)
      mov.l @r5+, r0
      mov.l r0, @(8, r4)
      mov.l @r5+, r0
      mov.l r0, @(16, r4)  // *dst = *src++
      add #16, r4  // dst += 4
    
    cmp/gt r4, r1  // if (dst <= end_offset)
    bf .x_loop_start_mc32
  
  cmp/ge r4, r6  // if (dst < end_pnt)
  bf .loop_remainder_mc32
  
  rts
  nop