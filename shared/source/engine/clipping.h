void clip_poly_line_plane(const vec5_t *vt_0, const vec5_t *vt_1, fixed dist_1, fixed dist_2, vec5_t *i_vt, int has_texture);
void clip_poly_plane(g_poly_t *poly, int plane);
int check_clip_line(line_t *line);

#define NEAR_PLANE 1
#define FAR_PLANE 2
#define LEFT_PLANE 4
#define RIGHT_PLANE 8
#define TOP_PLANE 16
#define BOTTOM_PLANE 32