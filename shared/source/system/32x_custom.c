#include "defines.h"
#include "32x.h"
#include "32x_custom.h"

u16 key_curr, key_prev;

u32 key_is_down(u32 key) {
  return key_curr & key;
}

u32 key_hit(u32 key) {
  return (key_curr & ~key_prev) & key;
}