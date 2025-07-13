// RAM_CODE void draw_line_asm(fixed x0, fixed y0, fixed x1, fixed y1, u16 color);
RAM_CODE void draw_poly_asm(g_poly_t *poly);
RAM_CODE void draw_poly_tx_affine_asm(g_poly_t *poly);
// RAM_CODE void draw_sprite_asm(g_poly_t *poly);
// RAM_CODE void transform_model_vis_asm(fixed matrix[], int dl_vt_offset, int dl_mode, const model_t *model);
// RAM_CODE void clip_poly_plane_asm(g_poly_t *poly, int plane);
// RAM_CODE void set_ot_list_vis_asm(vec3_t pos, int dl_mode, int offset, const model_t *model); // not made yet
// RAM_CODE void draw_grid_asm(vec3_t map_pos, const model_t *model); // not made yet
// RAM_CODE void draw_ot_list_vis_asm(vec3_t pos, int draw_dir, int dl_mode, const model_t *model); // not made yet
// RAM_CODE void memset32_asm(void *dst, u32 wd, uint wcount);