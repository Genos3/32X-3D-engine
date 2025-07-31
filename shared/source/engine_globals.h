extern char dbg_screen_output[20];

extern camera_t cam;
extern scene_t scn;
extern viewport_t vp;
extern config_t cfg;
extern object_t g_obj;
extern g_poly_t g_poly;
extern display_list_t dl;
extern ot_list_t g_ot_list;
extern const model_t *g_model;
extern const textures_t *g_textures;
extern const vx_model_t *g_vx_model;

// process_faces.h

extern int frame_cycles;
extern u32 frame_frt;
extern fixed fps, update_rate, delta_time, zfar_ratio, grid_tile_radius;
extern u32 r_scene_num_polys, r_scene_num_elements_ot, r_scene_num_objs, frame_counter;
extern u32 redraw_scene, model_id;
//extern u8 occlusion_buffer_bin[];

#if DRAW_NORMALS
  extern vec3_t tr_normals[SCN_VIS_PL_LIST_SIZE];
#endif
extern vec3_t scene_tr_vertices[SCN_VIS_VT_LIST_SIZE], model_tr_vertices[MDL_VIS_VT_LIST_SIZE], tr_sprite_vertices[NUM_VT_SPRITES];
extern u8 light_faces[MDL_PL_LIST_MAX_SIZE], clipping_pl_list[SCN_VIS_PL_LIST_SIZE], clipping_vt_list[SCN_VIS_VT_LIST_SIZE], faces_backface[MDL_PL_LIST_MAX_SIZE >> 3];
extern s16 ordering_table[SCN_OT_SIZE];
extern pl_list_t ot_pl_list[SCN_VIS_PL_LIST_SIZE];
extern u16 dl_pl_list[SCN_VIS_PL_LIST_SIZE], dl_vt_list[SCN_VIS_VT_LIST_SIZE], dl_obj_list[SCN_MAX_NUM_OBJECTS], dl_tr_vt_index[MDL_VT_LIST_MAX_SIZE];
#if ENABLE_TEXTURE_BUFFER
  extern u8 texture_buffer[TEXTURE_BUFFER_SIZE];
#endif
extern int profile_vars[3];
// extern const u8 *texture_image;
extern vec3_t *tr_vertices;

// drawing.h

#if RAM_FRAMEBUFFER
  extern u8 ram_framebuffer[320 * 224];
#endif

extern const fixed identity_matrix[12];
extern fixed camera_matrix[12], frustum_matrix[12], camera_sprite_matrix[12], model_matrix[12];

#if DRAW_FRUSTUM
  extern fixed view_frustum_matrix[12];
#endif

#if ENABLE_Z_BUFFER
  extern u32 z_buffer[SCREEN_WIDTH * SCREEN_HEIGHT + 1];
#endif

extern frustum_t frustum;
extern tr_frustum_t tr_frustum;

#if DRAW_FRUSTUM
  extern tr_frustum_t tr_view_frustum;
#endif

// frustum_cl_oc.h

#if ENABLE_GRID_FRUSTUM_CULLING
  extern u8 dl_visible_vt_list[MDL_VT_LIST_MAX_SIZE];
#endif

// scene.h

extern sky_t sky;

// debug

extern u8 dbg_show_poly_num, dbg_show_grid_tile_num;
extern int dbg_num_poly_dsp, dbg_num_grid_tile_dsp;