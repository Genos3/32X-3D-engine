#include "defines.h"

#ifdef PC
  #include <math.h>
  #include "pc/pc.h"
#endif

#include "engine_defines.h"
#include "engine/div_lut.h"
#include "engine/lut.h"
#include "engine/math_c.h"

void calc_normal(vec3_t *v0, vec3_t *v1, vec3_t *v2, vec3_t *nm) {
  vec3_t d0, d1;
  d0.x = v2->x - v0->x;
  d0.y = v2->y - v0->y;
  d0.z = v2->z - v0->z;
  d1.x = v1->x - v0->x;
  d1.y = v1->y - v0->y;
  d1.z = v1->z - v0->z;
  
  cross(&d0, &d1, nm);
}

fixed calc_normal_z(vec3_t *v0, vec3_t *v1, vec3_t *v2) {
  vec3_t d0, d1;
  d0.x = v2->x - v0->x;
  d0.y = v2->y - v0->y;
  d1.x = v1->x - v0->x;
  d1.y = v1->y - v0->y;
  
  return cross_2d(&d0, &d1);
}

fixed calc_dp(vec3_t *v0, vec3_t *v1, vec3_t *v2) {
  vec3_t d0, d1;
  d0.x = v2->x - v0->x;
  d0.y = v2->y - v0->y;
  d0.z = v2->z - v0->z;
  d1.x = v1->x - v0->x;
  d1.y = v1->y - v0->y;
  d1.z = v1->z - v0->z;
  
  return dot(&d0, &d1);
}

void calc_2d_normal(vec2_t *v0, vec2_t *v1, vec2_t *nm) {
  vec2_t d;
  d.x = v1->x - v0->x;
  d.y = v1->y - v0->y;
  
  nm->x = -d.y;
  nm->y = d.x;
}

fixed dot(vec3_t *v0, vec3_t *v1) {
  return dp3(v0->x, v1->x, v0->y, v1->y, v0->z, v1->z);
}

void cross(vec3_t *v0, vec3_t *v1, vec3_t *nm) {
  nm->x = (((s64)v0->y * v1->z) - ((s64)v0->z * v1->y)) >> FP;
  nm->y = (((s64)v0->z * v1->x) - ((s64)v0->x * v1->z)) >> FP;
  nm->z = (((s64)v0->x * v1->y) - ((s64)v0->y * v1->x)) >> FP;
}

fixed cross_2d(vec3_t *v0, vec3_t *v1) {
  return (((s64)v0->x * v1->y) - ((s64)v0->y * v1->x)) >> FP;
}

u32 sqrt_i(u32 x) {
  u32 m = 0x40000000;
  u32 y = 0;
  u32 z = 0;
  
  do {
    y += m;
    
    if (y > x) {
      y = z;
    } else {
      x -= y;
      y = z + m;
    }
    
    z = y >> 1;
  } while (m >>= 2);
  
  return y;
}

void normalize(vec3_t *v, vec3_t *u) {
  fixed_u length = sqrt_i(dp3(v->x, v->x, v->y, v->y, v->z, v->z)) << (FP >> 1);
  
  if (length) {
    // unsigned fixed point division
    #if !DIV_LUT_ENABLE
      length = fp_div(1 << FP, length);
    #else
      length = div_lut[length >> 8] << 8; // .16 result
    #endif
  }
  
  u->x = fp_mul(v->x, length);
  u->y = fp_mul(v->y, length);
  u->z = fp_mul(v->z, length);
}

inline void normalize_2d(vec2_t *v, vec2_t *u) {
  fixed_u length = sqrt_i(dp2(v->x, v->x, v->y, v->y)) << (FP >> 1);
  
  if (length) {
    // unsigned fixed point division
    #if !DIV_LUT_ENABLE
      length = fp_div(1 << FP, length);
    #else
      length = div_lut[length >> 8] << 8; // .16 result
    #endif
  }
  
  u->x = fp_mul(v->x, length);
  u->y = fp_mul(v->y, length);
}

void normalize_fast(vec3_t *v, vec3_t *u) {
  *u = *v;
  
  while (abs_c(u->x) > (1 << FP) ||
    abs_c(u->y) > (1 << FP) ||
    abs_c(u->z) > (1 << FP)) {
    u->x >>= 1;
    u->y >>= 1;
    u->z >>= 1;
  }
}

void normalize_2d_fast(vec2_t *v, vec2_t *u) {
  *u = *v;
  
  while (abs_c(u->x) > (1 << FP) ||
    abs_c(u->y) > (1 << FP)) {
    u->x >>= 1;
    u->y >>= 1;
  }
}

void pow8(fixed *n) {
  fixed sq = fp_mul(*n, *n);
  fixed sq4 = fp_mul(sq, sq);
  *n = fp_mul(sq4, sq4);
}

u16 atan2_c(fixed y, fixed x) {
  if (x > 0) {
    return atan_lut[fp_div(y, x) >> 8];
  } else if (x < 0 && y >= 0) {
    return atan_lut[fp_div(y, x) >> 8] + (128 << 8);
  } else if (x < 0 && y < 0) {
    return atan_lut[fp_div(y, x) >> 8] - (128 << 8);
  } else if (x == 0 && y > 0) {
    return (64 << 8);
  } else if (x == 0 && y < 0) {
    return (192 << 8);
  }
  
  return 0; // (0,0) case: Undefined, but returning 0
}

fixed calc_vec_length(vec3_t p) {
  return sqrt_i(dp3(p.x, p.x, p.y, p.y, p.z, p.z)) << (FP >> 1);
}