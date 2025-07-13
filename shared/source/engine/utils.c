#include "defines.h"
#include "utils.h"

void memset8(void *dst, u8 val, int count) {
  u8 *_dst = dst;
  
  for (int i = 0; i < count; i++) {
    *_dst++ = val;
  }
}

void memset16(void *dst, u16 val, int count) {
  u16 *_dst = dst;
  
  for (int i = 0; i < count; i++) {
    *_dst++ = val;
  }
}

// use fast_memset instead

void memset32(void *dst, u32 val, int count) {
  u32 *_dst = dst;
  
  for (int i = 0; i < count; i++) {
    *_dst++ = val;
  }
}

void memcpy16(void *dst, const void *src, int count) {
  u16 *_dst = dst;
  const u16 *_src = src;
  
  for (int i = 0; i < count; i++) {
    *_dst++ = *_src++;
  }
}

// use fast_memcpy instead

void memcpy32(void *dst, const void *src, int count) {
  u32 *_dst = dst;
  const u32 *_src = src;
  
  for (int i = 0; i < count; i++) {
    *_dst++ = *_src++;
  }
}

int clamp_i(int x, int min, int max) {
  if (x < min) {
    x = min;
  } else
  if (x > max) {
    x = max;
  }
  
  return x;
}

int count_bits(int n) {
  int i = 0;
  
  while (n) {
    n >>= 1;
    i++;
  }
  
  return i;
}

int log2_c(int n) {
  int i = 0;
  
  while (n >>= 1) {
    i++;
  }
  
  return i;
}