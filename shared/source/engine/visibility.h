int check_backface_culling(vec3_t pos, int face_id, const model_t *model);
fixed calc_view_normal_z(vec3_t pos, int face_id, const model_t *model);
int check_frustum_clipping(g_poly_t *poly);
int test_tx_dist(g_poly_t *poly);
int test_poly_dist(g_poly_t *poly);
int test_poly_ratio(g_poly_t *poly);
int check_frustum_culling(vec3_t pos, fixed radius, u8 type);