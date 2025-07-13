#define DIV_LUT_SIZE_R 4096
#define DIV_LUT_BITS 16
#define PROJ_SHIFT 4

// dst returns a value in 32 bit size

// r0 should be free
// src should be positive and less than the lut size
// lut should have div_lut
// src can use r0

.macro div_luts dst, src, lut
  mov.l \src, r0
  shll2 r0
  mov.l @(r0, \lut), \dst  // 32 bit
.endm

// r0 should be free
// src can be negative and should be less than the lut size
// lut should have div_lut
// src can use r0

.macro div_luts_s dst, src, lut
  cmp/pz \src
  bt/s 1f
  mov.l \src, r0
    neg r0, r0
  1:
  
  shll2 r0
  bt/s 1f
  mov.l @(r0, \lut), \dst  // 32 bit
    neg \dst, \dst
  1:
.endm