extern const s32 sin_lut[]; // 512 elements 32 bit 16.16
extern const u16 atan_lut[]; // 512 elements 16 bit 8.8
extern const u16 div_lut[]; // 4096 elements 32 bit 0.16
#define DIV_LUT_SIZE_R 4096
#define DIV_LUT_BITS 16 // 24
#define PROJ_SHIFT 4