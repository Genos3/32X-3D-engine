#include <stdio.h>
#include <string.h>
#include <math.h>
#include "defines.h"
#include "pc/pc.h"

u16 pc_palette[256];
u16 key_curr, key_prev;
u16 *screen;

u32 key_is_down(u32 key) {
  return key_curr &key;
}

u32 key_hit(u32 key) {
  return (key_curr & ~key_prev) &key;
}

// in: .14, out: .8

fixed ArcTan(fixed x) {
  return (int)((double)atan((double)x / (1 << 14)) * 180 / PI_FL * 256 / 360 * (1 << 8));
}