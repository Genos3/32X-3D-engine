u32 key_is_down(u32 key);
u32 key_hit(u32 key);
fixed ArcTan(fixed x);

#define PI_FL 3.14159265

// #define vid_page screen
#define hw_palette pc_palette
// #define Sqrt(x) (int)sqrt(x)
#define memset32_asm memset32

#define KEY_UP           0x0001
#define KEY_DOWN         0x0002
#define KEY_LEFT         0x0004
#define KEY_RIGHT        0x0008
#define KEY_B            0x0010
#define KEY_C            0x0020
#define KEY_A            0x0040
#define KEY_START        0x0080
#define KEY_Z            0x0100
#define KEY_Y            0x0200
#define KEY_X            0x0400
#define KEY_MODE         0x0800

#define KEY_DIR          0x000F
#define KEY_ANY          0x0FFF

// #define PC_SCREEN_WIDTH 320
// #define PC_SCREEN_HEIGHT 240
// #define PC_SCREEN_SCALE_SIZE 2

extern u16 pc_palette[256];
extern u16 key_curr, key_prev;
extern u16 *screen;