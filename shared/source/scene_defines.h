#define UPDATE_RATE 30

#define SKY_NUM_COLOR_GRADIENT_BITS 3
#define SKY_NUM_COLOR_GRADIENT (1 << SKY_NUM_COLOR_GRADIENT_BITS)
#define PAL_SKY_GRD_OFFSET (240 - SKY_NUM_COLOR_GRADIENT)

#define OT_FACE_TYPE 0
#define OT_OBJECT_TYPE 1

#define OBJ_SPRITE 0
#define OBJ_VOXEL 1
#define OBJ_MODEL 2

#define SCN_MAX_NUM_OBJECTS 256  // maximum number of objects in the scene

typedef struct {
  u16 start_color, end_color;
  rgb_t start_color_ch;
  rgb_t end_color_ch;
  u16 start_angle, end_angle;
  fixed fp_deg2px;
} sky_t;

typedef struct {
  vec3_t pos;
  vec3_u16_t rot;
  u8 static_pos, static_rot;
  const void *mdl_pnt;
  u8 type;
} object_t;

typedef struct {
  vec3_t lightdir;
  vec3_t lightdir_n;
  u16 bg_color;
  u8 directional_lighting_enabled: 1;
  u8 draw_sky: 1;
  u8 curr_model; // u8
  u8 cam_curr_map; // u8
  
  object_t obj_list[SCN_MAX_NUM_OBJECTS];
  vec3_t obj_tr_pos[SCN_MAX_NUM_OBJECTS];
} scene_t;