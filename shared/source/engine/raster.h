void draw_line(fixed x0, fixed y0, fixed x1, fixed y1, u16 color);
void draw_line_zb(fixed x0, fixed y0, fixed z0, fixed x1, fixed y1, fixed z1, u16 color);
void set_pixel(int x, int y, u16 color);
void fill_rect(int x, int y, int width, int height, u16 color);
void draw_sprite(g_poly_t *poly);
void draw_poly(g_poly_t *poly);
void draw_poly_tx_affine(g_poly_t *poly);
void draw_poly_tx_sub_ps(g_poly_t *poly);
void draw_tri_tx_affine(g_poly_t *poly);