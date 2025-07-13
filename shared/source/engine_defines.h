// #ifdef PROJECT_DIR
//   #include PROJECT_DIR
// #endif
#include "custom_defines.h"

#ifdef PC
  #define RAM_CODE
  #define fptofl(x) ((float)(x) / (1 << FP))
  #define NO_OPTIMIZE
  #define ENABLE_ASM 0
  #define OVERWRITE_IMAGE 0
  #define DIV_LUT_ENABLE 0
  #define SQRT_SUPPORT 1
#else
  #define RAM_CODE __attribute__((section(".sdram"), aligned(16)))
  #define USE_SECOND_CPU 0
  
  #ifdef DBG
    #define NO_OPTIMIZE __attribute__((optimize("O0")))
  #else
    #define NO_OPTIMIZE
  #endif
  
  #define ENABLE_ASM 1
  #define OVERWRITE_IMAGE 0
  #define DIV_LUT_ENABLE 0
  #define SQRT_SUPPORT 0
#endif

#define ALWAYS_INLINE __attribute__((always_inline))

#ifdef PC
  #define PC_SCREEN_WIDTH 320
  #define PC_SCREEN_HEIGHT 224
  #define PC_SCREEN_SCALE_SIZE 2
  
  #define FRAME_WIDTH PC_SCREEN_WIDTH
  #define FRAME_HEIGHT PC_SCREEN_HEIGHT
  #define SCREEN_WIDTH PC_SCREEN_WIDTH
  #define SCREEN_HEIGHT PC_SCREEN_HEIGHT
  
  #define PALETTE_MODE 0
  #define DOUBLED_PIXELS 0
  #define DOUBLED_LINES 0
  #define AUTO_FILL 0
#else
  #define PALETTE_MODE 1
  #define DOUBLED_PIXELS 1
  #define DOUBLED_LINES 1
  #define AUTO_FILL 0  // auto fill requires a 512 width framebuffer
  
  #define FRAME_WIDTH 320
  #define FRAME_HEIGHT 224
  #define SCREEN_WIDTH (FRAME_WIDTH >> DOUBLED_PIXELS)
  #define SCREEN_HEIGHT (FRAME_HEIGHT >> DOUBLED_LINES)
#endif

#define FRAME_HALF_WIDTH (FRAME_WIDTH >> 1)

#define SCREEN_HALF_WIDTH (SCREEN_WIDTH >> 1)
#define SCREEN_HALF_HEIGHT (SCREEN_HEIGHT >> 1)
#define SCREEN_WIDTH_FP (SCREEN_WIDTH << FP)
#define SCREEN_HEIGHT_FP (SCREEN_HEIGHT << FP)

#define RAM_FRAMEBUFFER 0

#define ENABLE_SUB_TEXEL_ACC 0
#define CHECK_POLY_HEIGHT 1
#define POLY_X_CLIPPING 1

// init 3d

#define TX_ALPHA_CR 0  // texture transparency color index
#define DIAG_UNIT_DIST fix(0.70710678)  // sin(45 * PI_fl / 180)
#define DIAG_UNIT_DIST_RC fix(1.41421356)  // 1 / DIAG_UNIT_DIST

// screen resolution has to be a multiple of the segment size
#define OC_SEG_SIZE_BITS 3
#define OC_SEG_SIZE (1 << OC_SEG_SIZE_BITS)
#define OC_SEG_SIZE_S (OC_SEG_SIZE - 1)
#define OC_SEG_FULL_SIZE_S ((1 << OC_SEG_SIZE) - 1)
#define OC_SEG_SCREEN_WIDTH (SCREEN_WIDTH >> OC_SEG_SIZE_BITS)

#define DRAW_OC_BIN_BUFFER_GRID 0
#define DRAW_OC_MAP_GRID 0

// face type
#define UNTEXTURED 1
#define BACKFACE 2
#define BILLBOARD_N 4
#define BILLBOARD_V 8
#define SPRITE (4 | 8)
#define ANIMATED 16
#define HAS_ALPHA 32

// matrix
#define ORIG_X 0
#define VEC_X_X 1
#define VEC_Y_X 2
#define VEC_Z_X 3

#define ORIG_Y 4
#define VEC_X_Y 5
#define VEC_Y_Y 6
#define VEC_Z_Y 7

#define ORIG_Z 8
#define VEC_X_Z 9
#define VEC_Y_Z 10
#define VEC_Z_Z 11

// palette

#define RGB15(a, b, c) (((c) << 10) | ((b) << 5) | (a))

#define CLR_WHITE RGB15(30, 30, 30)
#define CLR_BLACK RGB15(0, 0, 0)
#define CLR_RED RGB15(30, 0, 0)
#define CLR_GREEN RGB15(0, 30, 0)
#define CLR_BLUE RGB15(0, 0, 30)
#define CLR_YELLOW RGB15(30, 30, 0)
#define CLR_MAGENTA RGB15(30, 0 ,30)
#define CLR_BG RGB15(18, 26, 28)  // RGB15(2, 2, 2)

#if PALETTE_MODE
  #define PAL_WHITE 248
  #define PAL_BLACK 249
  #define PAL_RED 250
  #define PAL_GREEN 251
  #define PAL_BLUE 252
  #define PAL_YELLOW 253
  #define PAL_MAGENTA 254
  #define PAL_BG 255
#else
  #define PAL_WHITE CLR_WHITE
  #define PAL_BLACK CLR_BLACK
  #define PAL_RED CLR_RED
  #define PAL_GREEN CLR_GREEN
  #define PAL_BLUE CLR_BLUE
  #define PAL_YELLOW CLR_YELLOW
  #define PAL_MAGENTA CLR_MAGENTA
  #define PAL_BG CLR_BG
#endif