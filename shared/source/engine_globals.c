#include "defines.h"
#include "structs.h"
#include "engine_defines.h"
#include "scene_defines.h"
#include "engine_globals.h"

char dbg_screen_output[20];

camera_t cam;
scene_t scn;
viewport_t vp;
config_t cfg;
object_t g_obj;
g_poly_t g_poly;
display_list_t dl;
ot_list_t g_ot_list;
const model_t *g_model;
const textures_t *g_textures;
const vx_model_t *g_vx_model;

// process_faces.h

int frame_cycles;
u32 frame_frt;
fixed fps, update_rate, delta_time, zfar_ratio, grid_tile_radius;
u32 r_scene_num_polys, r_scene_num_elements_ot, r_scene_num_objs, frame_counter;
u32 redraw_scene, model_id;
//extern u8 occlusion_buffer_bin[];

#if DRAW_NORMALS
  vec3_t tr_normals[SCN_VIS_PL_LIST_SIZE];
#endif
vec3_t scene_tr_vertices[SCN_VIS_VT_LIST_SIZE], model_tr_vertices[MDL_VIS_VT_LIST_SIZE], tr_sprite_vertices[NUM_VT_SPRITES];
u8 light_faces[MDL_PL_LIST_MAX_SIZE], clipping_pl_list[SCN_VIS_PL_LIST_SIZE], clipping_vt_list[SCN_VIS_VT_LIST_SIZE], faces_backface[MDL_PL_LIST_MAX_SIZE >> 3];
s16 ordering_table[SCN_OT_SIZE];
pl_list_t ot_pl_list[SCN_VIS_PL_LIST_SIZE];
u16 dl_pl_list[SCN_VIS_PL_LIST_SIZE], dl_vt_list[SCN_VIS_VT_LIST_SIZE], dl_obj_list[SCN_MAX_NUM_OBJECTS], dl_tr_vt_index[MDL_VT_LIST_MAX_SIZE];
#if ENABLE_TEXTURE_BUFFER
  u16 texture_buffer[TEXTURE_BUFFER_SIZE];
#endif
int profile_vars[3];
// const u8 *texture_image;
vec3_t *tr_vertices;

// drawing.h

#if RAM_FRAMEBUFFER
  u8 ram_framebuffer[320 * 224];
#endif

// columns: origin, vec_x, vec_y, vec_z
// rows: x, y, z

// array order:
// 0 1 2 3
// 4 5 6 7
// 8 9 10 11

// 0 1 0 0
// 0 0 1 0
// 0 0 0 1

// x, y, z (origin, vec_x, vec_y, vec_z)

// origin.x, vec_x.x, vec_y.x, vec_z.x,
// origin.y, vec_x.y, vec_y.y, vec_z.y,
// origin.z, vec_x.z, vec_y.z, vec_z.z

const fixed identity_matrix[] = {0,1 << FP,0,0, 0,0,1 << FP,0, 0,0,0,1 << FP};

fixed camera_matrix[12], frustum_matrix[12], camera_sprite_matrix[12], model_matrix[12];
#if DRAW_FRUSTUM
  fixed view_frustum_matrix[12];
#endif

#if ENABLE_Z_BUFFER
  u32 z_buffer[SCREEN_WIDTH * SCREEN_HEIGHT + 1];
#endif

frustum_t frustum;
tr_frustum_t tr_frustum;

#if DRAW_FRUSTUM
  tr_frustum_t tr_view_frustum;
#endif

// frustum_cl_oc.h

#if ENABLE_GRID_FRUSTUM_CULLING
  u8 dl_visible_vt_list[MDL_VT_LIST_MAX_SIZE];
#endif

// scene.h

sky_t sky;

// debug

u8 dbg_show_poly_num, dbg_show_grid_tile_num;
int dbg_num_poly_dsp, dbg_num_grid_tile_dsp;