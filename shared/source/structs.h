typedef struct {
  vec3_t min, max;
} aabb_t;

typedef struct {
  vec3i_t min, max;
} aabb_i_t;

typedef struct {
  vec2_t min, max;
} aabb_2d_t;

typedef struct {
  vec3_t p0, p1;
} line_t;

typedef struct {
  vec3_t vertices[8];
  int num_vertices;
} poly_t;

typedef struct {
  vec5_t vertices[8];
  int num_vertices;
} poly_tx_t;

typedef struct {
  vec5_t vertices[8];
  u32 color; // u16
  
  struct {
    u32 _pad : 28;
    u32 is_visible : 1;
    u32 has_transparency : 1;
    u32 has_texture : 1;
    u32 is_backface : 1;
  } flags;
  
  int num_vertices; // u8
  u32 texture_size_wh_s; // u16 (w | h)
  u32 texture_width_bits; // u8
  u32 frustum_clip_sides; // u8
  u32 face_type; // u8
  u32 final_light_factor; // u16
  u32 face_id; // u32
  const u16 *texture_image;
  const u16 *cr_palette_tx_idx;
} g_poly_t;

typedef struct {
  const s16 pl;
  const s16 vt;
} grid_pnt_t;

typedef struct {
  const grid_pnt_t *grid_pnt;
  const s16 *pl_data;
  const s16 *vt_data;
  const size3i_t size_i;
  const u32 num_tiles;
  const fixed tile_size;
  const u8 tile_size_bits;
} grid_t;

typedef struct {
  const u8 face_num_vertices;
  const u8 face_materials;
  const u8 face_types;
  const u16 face_index;
  const u16 tx_face_index;
  const s16 sprite_face_index;
} face_group_t;

typedef struct {
  const u16 num_vertices; // u16
  const u16 num_faces; // u16
  const u16 num_txcoords; // u16
  const u16 num_tx_faces; // u16
  const u16 num_objects; // u16
  const u8 num_materials; // u8
  const u8 num_sprites; // u8
  const u8 num_sprite_vertices; // u8
  
  struct {
    const u8 _pad : 5;
    const u8 has_textures : 1;
    const u8 has_grid : 1;
    const u8 has_normals : 1;
  } flags;
  
  const vec3_s16_t origin;
  const size3_u16_t size;
  
  const vec3_s16_t *vertices;
  const u16 *faces;
  const vec2_tx_u16_t *txcoords;
  const u16 *tx_faces;
  const vec3_s16_t *normals;
  const face_group_t *face_group;
  // const u8 *type_vt;
  const vec3_s16_t *sprite_vertices;
  const u8 *sprite_faces;
  const u16 *object_face_index;
  const u16 *object_num_faces;
  const u8 *mtl_textures;
  const u8 *lines;
  const grid_t grid;
} model_t;

typedef struct {
  const u32 texture_sizes_padded_wh;
  const u8 texture_width_bits;
  const s8 tx_animation_id;
  const u32 tx_index;
} tx_group_t;

typedef struct {
  const u8 num_textures; // u8
  const u8 num_animations; // u8
  const u8 pal_size; // u8
  const u8 pal_num_colors; // u8
  const u8 pal_size_tx; // u8
  const u8 pal_tx_num_colors; // u8
  const u8 lightmap_levels; // u8
  const u32 texture_data_total_size;
  
  const u8 *material_colors;
  const u16 *cr_palette_idx;
  const u8 *material_colors_tx;
  const u16 *cr_palette_tx_idx;
  // const size2i_u16_t *texture_sizes;
  const tx_group_t *tx_group;
  const u16 *texture_data;
} textures_t;

typedef struct {
  fixed acc;
  fixed rot_speed;
  vec3_t pos;
  vec3_t speed;
  vec3_u16_t rot;
} camera_t;

typedef struct {
  fixed z_near;
  fixed z_far;
  // fixed aspect_ratio;
  fixed focal_length;
  fixed focal_length_i;
  fixed hfov;
  fixed vfov;
  fixed half_hfov;
  fixed half_vfov;
  fixed screen_side_x_dt;
  fixed screen_side_y_dt;
} viewport_t;

typedef struct {
  u32 _pad : 14;
  u32 face_enable_persp_tx : 1;
  u32 clean_screen : 1;
  u32 occlusion_cl_enabled : 1;
  u32 animation_play : 1;
  u32 debug : 1;
  u32 tx_alpha_cr : 1;
  u32 draw_grid : 1;
  u32 enable_far_plane_clipping : 1;
  u32 draw_z_buffer : 1;
  u32 draw_normals : 1;
  u32 directional_lighting : 1;
  u32 static_light : 1;
  u32 tx_perspective_mapping_enabled : 1;
  u32 perspective_enabled : 1;
  u32 draw_textured_poly : 1;
  u32 draw_textures : 1;
  u32 draw_polys : 1;
  u32 draw_lines : 1;
} config_t;

typedef struct {
  vec3_t vertices[8];
  vec3_t normals[6];
} frustum_t;

typedef struct {
  vec3_t vertices[8];
  vec4_nm_t normals[6];
} tr_frustum_t;

typedef struct {
  u16 *pl_list;
  u16 *vt_list;
  u16 *obj_list;
  u16 *tr_vt_index; // order in which the vertices were stored in vertex_id
  u8 *visible_vt_list; // visibility in the frustum of the tile containing the vertex
  
  u16 num_tiles; // u16
  u16 num_faces; // u16
  u16 num_vertices; // u16
  u16 num_objects; // u16
  u32 total_vertices; // check size
} display_list_t;

typedef struct {
  u16 id;
  s16 pnt;
  u8 type;
} pl_list_t;

typedef struct {
  s16 *pnt;
  pl_list_t *pl_list;
  u32 size;
} ot_list_t;

/* typedef struct {
  vec3_t pos;
  u8 type;
  void *model;
} object_t; */

typedef struct {
  int pnt;
  int length;
} rle_grid_t;

typedef struct {
  u8 color;
  u8 vis;
  u8 length;
} rle_column_t;

typedef struct {
  const size3i_t size_i;
  const int pal_size;
  const int num_vertices;
  const fixed voxel_radius;
  const fixed model_radius;
  const size3_t size;
  
  const vec3_s16_t *vertices;
  const rle_grid_t *rle_grid; // - VX_MDL_GRID_MAX_SIZE
  const rle_column_t *rle_columns; // - VX_MDL_CLM_MAX_SIZE
  const u16 *palette;
  
  vec3_t *tr_vertices;
} vx_model_t;