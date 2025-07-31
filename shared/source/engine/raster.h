void draw_line(fixed x0, fixed y0, fixed x1, fixed y1, u16 color);
void draw_line_zb(fixed x0, fixed y0, fixed z0, fixed x1, fixed y1, fixed z1, u16 color);
void set_pixel(int x, int y, u16 color);
void fill_rect(int x, int y, int width, int height, u16 color);
void draw_sprite(g_poly_t *poly);
void draw_poly(g_poly_t *poly);
void draw_poly_tx_affine(g_poly_t *poly);
void draw_poly_tx_sub_ps(g_poly_t *poly);
void draw_tri_tx_affine(g_poly_t *poly);

typedef struct {
  u16 *vid_pnt;
  u16 *end_pnt;
  u32 su_sv;
  u32 dudx_dvdx;
  u32 y_line;
} scanline_tx_t;

typedef struct {
  u16 *vid_pnt;
  u16 *end_pnt;
  u32 sv_i;
  fixed_u su;
  fixed dudx;
  u32 y_line;
} scanline_sp_t;

void draw_sprite_scanline(g_poly_t *poly, scanline_sp_t *scanline);
void draw_poly_scanline(g_poly_t *poly, scanline_tx_t *scanline);
void draw_tx_affine_scanline(g_poly_t *poly, scanline_tx_t *scanline);

#if USE_SECOND_CPU
  extern scanline_tx_t sec_cpu_tx_scanline;
  extern scanline_sp_t sec_cpu_sp_scanline;
#endif