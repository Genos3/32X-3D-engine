#include <stdarg.h>
#include "common.h"

void sprintf_c(char *dst, char *fmt, ...) {
  va_list arg;
  va_start(arg, fmt);
  
  while (*fmt) {
    if (*fmt == '%') {
      fmt++;
      switch (*fmt) {
        case '%':
          *dst++ = *fmt;
          break;
        case 'd': {
          char str[11];
          int n = va_arg(arg, int);
          inttostr(str, n);
          puts_c(&dst, str);
          break;
        }
      }
    } else {
      *dst++ = *fmt;
    }
    
    fmt++;
  }
  
  *dst = 0;
  va_end(arg);
}

void inttostr(char *str, int n) {
  //u8 num_offset=48;
  char str_out[11];
  char *pt = str_out;
  int neg = 0;
  int length = 0;
  *pt++ = 0;
  
  if (!n) {
    *pt++ = '0';
    length++;
  } else
  if (n < 0) {
    neg = 1;
    n = (~n) + 1;
  }
  
  while (n) {
    *pt++ = '0' + n % 10;
    n /= 10;
    length++;
  }
  
  if (neg) {
    *pt++ = '-';
    length++;
  }
  
  for (int i = 0; i <= length; i++) { //reverse the array
    str[i] = str_out[length - i];
  }
}

/* void putc(u8 *dst, u8 chr){
 *dst++=chr;
} */

void puts_c(char **dst, char *str) {
  while (*str) {
    *(*dst)++ = *str;
    // **dst=*str;
    // (*dst)++;
    str++;
  }
}

void draw_str(int x, int y, char *str, u16 color) {
  while (*str) {
    draw_char(x, y, *str, color);
    str++;
    x += CHAR_WIDTH;
  }
}

RAM_CODE void draw_char(int x, int y, char chr, u16 color) {
  #ifdef PC
    u16 *screen_offset = screen + y * FRAME_WIDTH + x;
  #else
    u8 *screen_offset = (u8*)screen + y * FRAME_WIDTH + x;
  #endif
  
  uint chr_offset = (chr - 32) * 2; //text_offset
  uint count = 0;
  
  for (int i = 0; i < CHAR_HEIGHT; i++) {
    #ifdef PC
      u16 *vid_pnt = screen_offset;
    #else
      u8 *vid_pnt = screen_offset;
    #endif
    
    for (int j = 0; j < CHAR_WIDTH; j++) {
      if (font[chr_offset + (count >> 5)] & 1 << (count & 31)) {
        *vid_pnt = color;
      }
      
      vid_pnt++;
      count++;
    }
    
    screen_offset += FRAME_WIDTH;
  }
}