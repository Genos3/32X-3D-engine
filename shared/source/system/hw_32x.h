void hw_32x_init();
void page_flip();
void page_wait();
void vblank_wait();
void page_clear();
void os_set_palette(const u16* palette);
int get_frt_counter();
int frt_counter2ms(int c);

extern u16 page_index;
extern u32 g_frame_index;

extern u16 *screen;
extern u16 *hw_palette;

enum MarsCmd {
  MARS_CMD_NONE = 0,
  MARS_CMD_CLEAR,
  MARS_CMD_FLUSH,
  MARS_CMD_DRAW_BG,
  /* MARS_CMD_DRAW_POLY0,
  MARS_CMD_DRAW_POLY1,
  MARS_CMD_DRAW_POLY2,
  MARS_CMD_DRAW_POLY3 */
};

/* extern char seg_lock;

inline void R_LockSeg(void)
{
  int res;
  do {
    __asm volatile (\
      "tas.b %1\n\t" \
      "movt %0\n\t" \
      : "=&r" (res) \
      : "m" (seg_lock) \
    );
  } while (res == 0);
}

inline void R_UnlockSeg(void)
{
  seg_lock = 0;
} */