// #define fp_trunc(x) (((x) >> 16) << 16)

.macro fp_trunc, src
  shlr16 \src
  shll16 \src
.endm

// #define fp_ceil(x) ((((x) + (1 << 16) - 1) >> 16) << 16)

.macro fp_ceil, src, dst
  mov.l #0xFFFF, \dst
  add \src, \dst
  shlr16 \dst
  shll16 \dst
.endm

// #define fp_mul(a, b) (int)(((s64)(a) * (b)) >> 16)

.macro fp_muls, a, b, t, dst
  dmuls.l \a, \b
  sts mach \t
  sts macl \dst
  xtrct \t, \dst
.endm

// #define fp_mul32(a, b) (((a) * (b)) >> 16)

.macro fp_mul32, a, b, dst
  muls \a, \b
  sts macl \dst
.endm

// #define min_c(a, b) ((a) < (b) ? (a) : (b))

.macro min_c, a, b, dst
  cmp/gt \b, \a
  bf/s 1f
    mov \a, \dst
    mov \b, \dst
  1:
.endm

// #define max_c(a, b) ((a) > (b) ? (a) : (b))

.macro max_c, a, b, dst
  cmp/gt \b, \a
  bt/s 1f
    mov \a, \dst
    mov \b, \dst
  1:
.endm

// int clamp_i(int x, int min, int max)

.macro clamp_i, src, min, max, dst
  cmp/ge \min, \src
  bf/s 1f
    mov \min, \dst
  cmp/gt \min, \src
  bt/s 1f
    mov \max, \dst
    mov \src, \dst
  1:
.endm

// #define abs_c(x) ((x) < 0 ? -(x) : (x))

.macro abs_c, src, dst
  cmp/pz \src
  bt 1f
    neg \src, \dst
  1:
.endm

// #define dup8(x) ((x) | ((x) << 8))

.macro dup8, src, dst
  mov \src, \dst
  shll8 \dst
  or \src, \dst
.endm

// #define dup16(x) ((x) | ((x) << 16))

.macro dup16, src, dst
  mov \src, \dst
  shll16 \dst
  or \src, \dst
.endm