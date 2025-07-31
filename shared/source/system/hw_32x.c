#include <string.h>
#include "common.h"

void pri_vbi_handler();
void secondary();
void flush_ot(int bit);
void sec_dma1_handler();

#define NTSC_CLOCK_SPEED 23011360 // Hz
#define PAL_CLOCK_SPEED 22801467 // Hz
#define NTSC 1

volatile u32 mars_pwdt_ovf_count = 0;
volatile u32 mars_swdt_ovf_count = 0;
u32 mars_frtc2msec_frac = 0;

u16 page_index = 0;
u32 g_frame_index = 0;

u16 *screen;
u16 *hw_palette;

void hw_32x_init() {
  // Wait for the SH2 to gain access to the VDP
  while ((MARS_SYS_INTMSK & MARS_SH2_ACCESS_VDP) == 0);
  
  // Set 16-bit direct mode, 224 lines
  // MARS_VDP_DISPMODE = MARS_224_LINES | MARS_VDP_MODE_32K;
  
  // Set 8-bit paletted mode, 224 lines
  // 240 lines are not allowed on NTSC and only on PAL when the 240 lines mode on the MD is enabled
  MARS_VDP_DISPMODE = MARS_224_LINES | MARS_VDP_MODE_256;
  
  // init both framebuffers
  
  for (int page = 0; page < 2; page++) {
    page_flip();
    page_wait();
    
    volatile u16 *lineTable = &MARS_FRAMEBUFFER;
    u16 wordOffset = 0x100;
    
    for (int i = 0; i < 224; i++){
      lineTable[i] = wordOffset;
      
      #if !DOUBLED_LINES
        wordOffset += FRAME_WIDTH >> 1;
      #else
        if (i & 1) {
          wordOffset += FRAME_WIDTH >> 1;
        }
      #endif
    }
    
    page_clear();
  }
  
  SH2_WDT_WTCSR_TCNT = 0x5A00; /* WDT TCNT = 0 */
  SH2_WDT_WTCSR_TCNT = 0xA53E; /* WDT TCSR = clr OVF, IT mode, timer on, clksel = Fs/4096 */
  
  /* init hires timer system */
  SH2_WDT_VCR = (65 << 8) | (SH2_WDT_VCR & 0x00FF); // set exception vector for WDT
  SH2_INT_IPRA = (SH2_INT_IPRA & 0xFF0F) | 0x0020; // set WDT INT to priority 2
  
  // change 4096.0f to something else if WDT TCSR is changed!
  mars_frtc2msec_frac = (int)(4096.0f * 1000.0f / (NTSC ? NTSC_CLOCK_SPEED : PAL_CLOCK_SPEED) * 65536.0f);
  
  MARS_SYS_COMM4 = 0;
  
  screen = (u16*)&MARS_FRAMEBUFFER + 0x100;
  hw_palette = (u16*)&MARS_CRAM;
}

void page_flip() {
  page_index ^= 1;
  MARS_VDP_FBCTL = page_index;
}

void page_wait() {
  while ((MARS_VDP_FBCTL & MARS_VDP_FS) != page_index);
}

void vblank_wait() {
  while (!(MARS_VDP_FBCTL & MARS_VDP_VBLK));
}

void page_clear() {
  //fast_memset((u8*)&MARS_FRAMEBUFFER + 0x200, PAL_BLACK, (FRAME_WIDTH * FRAME_HEIGHT) >> 1);
  memset16((u16*)&MARS_FRAMEBUFFER + 0x100, dup8(PAL_BLACK), (FRAME_WIDTH * FRAME_HEIGHT) >> 2);
  // memset32((u16*)&MARS_FRAMEBUFFER + 0x100, quad8(PAL_BLACK), (FRAME_WIDTH * FRAME_HEIGHT) >> 2);
}

void os_set_palette(const u16* palette) {
  void *dst = (void*)&MARS_CRAM;
  memcpy(dst, palette, 256 * 2);
}

int get_frt_counter() {
  return (mars_pwdt_ovf_count << 8) | SH2_WDT_RTCNT;
}

int frt_counter2ms(int c) {
  return (c * mars_frtc2msec_frac) >> 16;
}

extern void pri_vbi_handler() {
  g_frame_index++;
}

extern void secondary() {
  // init DMA
  SH2_DMA_SAR0 = 0;
  SH2_DMA_DAR0 = 0;
  SH2_DMA_TCR0 = 0;
  SH2_DMA_CHCR0 = 0;
  SH2_DMA_DRCR0 = 0;
  SH2_DMA_SAR1 = 0;
  SH2_DMA_DAR1 = 0;
  SH2_DMA_TCR1 = 0;
  SH2_DMA_CHCR1 = 0;
  SH2_DMA_DRCR1 = 0;
  SH2_DMA_DMAOR = 1; // enable DMA
  
  SH2_DMA_VCR1 = 66; // set exception vector for DMA channel 1
  SH2_INT_IPRA = (SH2_INT_IPRA & 0xF0FF) | 0x0400;    // set DMA INT to priority 4
  
  ClearCache();
  
  while (1) {
    int cmd;
    while ((cmd = MARS_SYS_COMM4) == 0);
    
    switch (cmd) {            
      case MARS_CMD_CLEAR:
        page_clear();
        break;
      // case MARS_CMD_FLUSH:
      //   flush_ot(1);
      //   break;
      case MARS_CMD_DRAW_BG:
        if (scn.draw_sky) {
          ClearCache();
          #if PALETTE_MODE
            draw_sky_pal();
          #else
            draw_sky();
          #endif
        } else {
          memset16((u16*)&MARS_FRAMEBUFFER + 0x100, dup8(PAL_BG), SCREEN_WIDTH * SCREEN_HEIGHT);
        }
        break;
      #if USE_SECOND_CPU
        case MARS_CMD_DRAW_POLY_SCANLINE:
          ClearCache();
          draw_poly_scanline(&g_poly, &sec_cpu_tx_scanline);
          break;
        case MARS_CMD_DRAW_TX_AFF_SCANLINE:
          ClearCache();
          draw_tx_affine_scanline(&g_poly, &sec_cpu_tx_scanline);
          break;
        case MARS_CMD_DRAW_SPRITE_SCANLINE:
          ClearCache();
          draw_sprite_scanline(&g_poly, &sec_cpu_sp_scanline);
          break;
      #endif
    }
    
    MARS_SYS_COMM4 = 0;
  }
}

// extern void flush_ot(int bit) {};

extern void sec_dma1_handler() {
  SH2_DMA_CHCR1; // read TE
  SH2_DMA_CHCR1 = 0; // clear TE
}