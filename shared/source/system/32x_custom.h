// RAM_CODE u32 dup8(u16 x);
u32 key_is_down(u32 key);
u32 key_hit(u32 key);

extern u16 key_curr, key_prev;

#define KEY_UP SEGA_CTRL_UP
#define KEY_DOWN SEGA_CTRL_DOWN
#define KEY_LEFT SEGA_CTRL_LEFT
#define KEY_RIGHT SEGA_CTRL_RIGHT
#define KEY_A SEGA_CTRL_A
#define KEY_B SEGA_CTRL_B
#define KEY_C SEGA_CTRL_C
#define KEY_X SEGA_CTRL_X
#define KEY_Y SEGA_CTRL_Y
#define KEY_Z SEGA_CTRL_Z
#define KEY_START SEGA_CTRL_START
#define KEY_MODE SEGA_CTRL_MODE

#define KEY_ANY 0x0FFF  // any key pressed
#define KEY_DIR 0x000F  // any direction pressed