#include "defines.h"
#include "engine_defines.h"
#include "engine/lut.h"
#include "engine/div_lut.h"

int lu_lerp32a(const u32 lut[], u32 x, const uint shift);

// unsigned division
// b has to be positive and less than the max size of the lut

inline fixed div_luts(int b) {
  return div_lut[b];
}

// signed division
// b can be negative and has to be less than the max size of the lut

inline fixed div_luts_s(int b) {
  int c = abs_c(b);
  fixed r = div_lut[c];
  
  if (b < 0) {
    return -r;
  } else {
    return r;
  }
}

// unsigned shifted division
// b has to be positive and can be larger than the max size of the lut

inline fixed div_luts_sh(int b) {
  int n = 0;
  
  while (b >= DIV_LUT_SIZE_R) {
    b >>= 2;
    n += 2;
  }
  
  return div_lut[b] >> n;
}

// signed shifted division
// b can be negative and can be larger than the max size of the lut

inline fixed div_luts_sh_s(int b) {
  int n = 0;
  int c = abs_c(b);
  
  while (c >= DIV_LUT_SIZE_R) {
    c >>= 2;
    n += 2;
  }
  
  fixed r = div_lut[c] >> n;
  
  if (b < 0) {
    return -r;
  } else {
    return r;
  }
}

// taken from Tonc
// lut interpolation

int lu_lerp32a(const u32 lut[], u32 x, const uint shift) {
  u32 xa = x >> shift;
  u32 ya = lut[xa];
  u32 yb = lut[xa + 1];
  return ya + ((yb - ya) * (x - (xa << shift)) >> shift);
}

// unsigned division

fixed fp_div_refine(fixed b) {
  // Get initial approximation from LUT
  fixed x = div_lut[b] >> FP;

  // Perform 1-2 Newton-Raphson iterations for higher precision
  for (int i = 0; i < 2; i++) {
    fixed bx = fp_mul(b, x);
    
    // Update x using Newton-Raphson iteration: x = x * (2 - b * x)
    x = fp_mul(x, (2 << FP) - bx);
  }
  
  return x;
}