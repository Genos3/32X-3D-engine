#include "common.h"

#define UPDATE_TIME ((1 << FP) / UPDATE_RATE) // 33 ms
#define UPDATE_FRAMES (60 / UPDATE_RATE)

#ifndef PC
  int main() {
    u32 ticks = g_frame_index;
    fps = 60;
    redraw_scene = 1;
    key_curr = MARS_SYS_COMM8 & ((1 << 12) - 1);
    key_prev = key_curr;
    
    hw_32x_init();
    
     init_3d();
    
    while (1) {
      key_curr = MARS_SYS_COMM8 & ((1 << 12) - 1);
      
      while (ticks < g_frame_index) {
        update_state(UPDATE_TIME);
        ticks += UPDATE_FRAMES;
        key_prev = key_curr;
      }
      
      page_wait();
      
      if (redraw_scene) {
        u32 frame_frt;
        redraw_scene = 0;
        if (cfg.debug) {
          frame_frt = get_frt_counter();
        }
        
        draw();
        
        if (cfg.debug) {
          frame_frt = get_frt_counter() - frame_frt;
          frame_cycles = frame_frt * 4096;
          delta_time = frt_counter2ms(frame_frt);
          fps = 1000 / delta_time;
          if (fps > 60) fps = 60;
          draw_dbg();
        }
        
        page_flip();
      }
    }
    
    return 0;
  }
#endif