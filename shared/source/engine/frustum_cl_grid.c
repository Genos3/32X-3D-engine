#include "common.h"

void get_frustum_aabb(aabb_t *aabb);
void check_map_tile(int y, int z, int x, vec3_t tile_pos, const model_t *model);
void add_tile_faces_to_dl(int tile_pnt, const model_t *model);
void draw_transformed_tile(vec3_t pos);
int check_occlusion(vec3_t *pos, u8 frustum_side, const model_t *model);
void add_face_ot_list(int i);
void transform_grid_cube(fixed tile_size);
void draw_grid_cube_lines(vec3_t pos, u16 color);

#define TS fix(0.5)
static fixed mdl_cube_matrix[12];
static vec3_t mdl_cube_tr_vertices[8]; // vertices pre transformed

static const vec3_s16_t mdl_cube_vertices[] = {
  {-TS,TS,-TS}, {TS,TS,-TS}, {TS,TS,TS}, {-TS,TS,TS}, {-TS,-TS,-TS}, {TS,-TS,-TS}, {TS,-TS,TS}, {-TS,-TS,TS}
};

static const u8 mdl_cube_lines[] = {
  0,1, 1,2, 2,3, 3,0,
  4,5, 5,6, 6,7, 7,4,
  0,4, 1,5, 2,6, 3,7
};

#define MDL_CUBE_NUM_VT 8
#define MDL_CUBE_LINES_SIZE sizeof(mdl_cube_lines)

#if 1 // !ENABLE_ASM
  static int tile_size_bits_fp;
  
  RAM_CODE void draw_grid(vec3_t obj_pos, const model_t *model) {
    vec3_t map_pos;
    map_pos.x = obj_pos.x + model->origin.x;
    map_pos.y = obj_pos.y + model->origin.y;
    map_pos.z = obj_pos.z + model->origin.z;
    
    // center of the first tile
    vec3_t tile_pos;
    tile_pos.x = map_pos.x + (model->grid.tile_size >> 1);
    tile_pos.y = map_pos.y + (model->grid.tile_size >> 1);
    tile_pos.z = map_pos.z + (model->grid.tile_size >> 1);
    
    #if SCENE_HAS_MAPS
      if (scn.curr_model == scn.cam_curr_map) {
        dl.num_tiles = 0;
      }
    #else
      dl.num_tiles = 0;
    #endif
    
    #if DRAW_GRID
      transform_grid_cube(model->grid.tile_size);
    #endif
    
    if (cfg.occlusion_cl_enabled) {
      dl.total_vertices = 0;
      
      // if (DRAW_GRID) {
      //   transform_grid_cube(model->grid.tile_size);
      // }
    } else {
      dl.num_faces = 0;
      dl.num_vertices = 0;
    }
    
    // - fixed grid_size_ratio = fp_div(1 << FP, model->grid.tile_size);
    // - fixed grid_tile_radius = fp_mul(model->grid.tile_size >> 1, DIAG_UNIT_DIST_RC);
    // fixed grid_tile_radius = DIAG_UNIT_DIST_RC << (model->grid.tile_size_bits - 1);
    
    aabb_t fr_aabb;
    // obtain the frustum aabb
    get_frustum_aabb(&fr_aabb);
    
    tile_size_bits_fp = model->grid.tile_size_bits + FP;
    
    // normalize the frustum aabb and the camera position against the grid size
    
    fr_aabb.min.x = (fr_aabb.min.x - map_pos.x) >> tile_size_bits_fp;
    fr_aabb.min.y = (fr_aabb.min.y - map_pos.y) >> tile_size_bits_fp;
    fr_aabb.min.z = (fr_aabb.min.z - map_pos.z) >> tile_size_bits_fp;
    fr_aabb.max.x = (fr_aabb.max.x - map_pos.x) >> tile_size_bits_fp;
    fr_aabb.max.y = (fr_aabb.max.y - map_pos.y) >> tile_size_bits_fp;
    fr_aabb.max.z = (fr_aabb.max.z - map_pos.z) >> tile_size_bits_fp;
    
    fr_aabb.min.x = clamp_i(fr_aabb.min.x, 0, model->grid.size_i.w - 1);
    fr_aabb.min.y = clamp_i(fr_aabb.min.y, 0, model->grid.size_i.h - 1);
    fr_aabb.min.z = clamp_i(fr_aabb.min.z, 0, model->grid.size_i.d - 1);
    fr_aabb.max.x = clamp_i(fr_aabb.max.x, 0, model->grid.size_i.w - 1);
    fr_aabb.max.y = clamp_i(fr_aabb.max.y, 0, model->grid.size_i.h - 1);
    fr_aabb.max.z = clamp_i(fr_aabb.max.z, 0, model->grid.size_i.d - 1);
    
    /* vec3i_t cam_tile_pos;
    cam_tile_pos.x = (cam.pos.x - map_pos.x) >> tile_size_bits_fp;
    cam_tile_pos.y = (cam.pos.y - map_pos.y) >> tile_size_bits_fp;
    cam_tile_pos.z = (cam.pos.z - map_pos.z) >> tile_size_bits_fp;
    
    // clamp the camera tile position within the frustum
    
    cam_tile_pos.x = clamp_i(cam_tile_pos.x, fr_aabb.min.x, fr_aabb.max.x);
    cam_tile_pos.y = clamp_i(cam_tile_pos.y, fr_aabb.min.y, fr_aabb.max.y);
    cam_tile_pos.z = clamp_i(cam_tile_pos.z, fr_aabb.min.z, fr_aabb.max.z); */
    
    // draw the map
    
    for (int i = fr_aabb.min.y; i <= fr_aabb.max.y; i++) {
      for (int j = fr_aabb.min.z; j <= fr_aabb.max.z; j++) {
        for (int k = fr_aabb.min.x; k <= fr_aabb.max.x; k++) {
          check_map_tile(k, i, j, tile_pos, model);
        }
      }
    }
  }
  
  RAM_CODE void get_frustum_aabb(aabb_t *aabb) {
    aabb->min.x = cam.pos.x;
    aabb->max.x = cam.pos.x;
    aabb->min.y = cam.pos.y;
    aabb->max.y = cam.pos.y;
    aabb->min.z = cam.pos.z;
    aabb->max.z = cam.pos.z;
    
    for (int i = 4; i < 8; i++) {
      aabb->min.x = min_c(aabb->min.x, tr_frustum.vertices[i].x);
      aabb->min.y = min_c(aabb->min.y, tr_frustum.vertices[i].y);
      aabb->min.z = min_c(aabb->min.z, tr_frustum.vertices[i].z);
      aabb->max.x = max_c(aabb->max.x, tr_frustum.vertices[i].x);
      aabb->max.y = max_c(aabb->max.y, tr_frustum.vertices[i].y);
      aabb->max.z = max_c(aabb->max.z, tr_frustum.vertices[i].z);
    }
  }
  
  RAM_CODE void check_map_tile(int x, int y, int z, vec3_t tile_pos, const model_t *model) {
    int tile_pnt = y * model->grid.size_i.d * model->grid.size_i.w + z * model->grid.size_i.w + x;
    
    // if the tile is empty return
    if (model->grid.grid_pnt[tile_pnt].pl < 0) return;
    
    // tile mid point
    tile_pos.x += x << tile_size_bits_fp;
    tile_pos.y += y << tile_size_bits_fp;
    tile_pos.z += z << tile_size_bits_fp;
    
    // check frustum culling
    // 0 = outside, 1 = partially inside, 2 = totally inside
    
    u8 frustum_side = check_frustum_culling(tile_pos, grid_tile_radius, 0);
    
    if (!frustum_side) return;
    
    #ifdef PC
      if (dbg_show_grid_tile_num && dbg_num_grid_tile_dsp != dl.num_tiles) {
        #if SCENE_HAS_MAPS
          if (scn.curr_model != scn.cam_curr_map) return;
        #else
          return;
        #endif
      }
    #endif
    
    #if DRAW_GRID
      u16 color;
      
      #ifdef PC
        if (dbg_show_grid_tile_num
          #if SCENE_HAS_MAPS
            && scn.curr_model == scn.cam_curr_map
          #endif
        ) {
          if (dbg_num_grid_tile_dsp == dl.num_tiles) {
            color = PAL_BLUE;
          } else return;
        } else {
          color = PAL_RED;
        }
      #else
        color = PAL_RED;
      #endif
      
      draw_grid_cube_lines(tile_pos, color);
    #endif
    
    add_tile_faces_to_dl(tile_pnt, model);
  }
  
  // add the tile faces to the display list
  
  #if ENABLE_GRID_FRUSTUM_CULLING
    RAM_CODE void add_tile_faces_to_dl(int tile_pnt, const model_t *model) {
      // add the vertices
      
      int pnt_vt = model->grid.grid_pnt[tile_pnt].vt;
      int vt_id = model->grid.vt_data[pnt_vt];
      
      while (vt_id >= 0) {
        // if the vertex is not already added
        if (!dl.visible_vt_list[vt_id]) {
          if (dl.num_vertices == SCN_VIS_VT_LIST_SIZE) break;
          dl.visible_vt_list[vt_id] = 1; // frustum_side
          dl.vt_list[dl.num_vertices] = vt_id;
          dl.tr_vt_index[vt_id] = dl.num_vertices; // order in which the vertices were stored in vertex_id
          dl.num_vertices++;
        }
        
        pnt_vt++;
        vt_id = model->grid.vt_data[pnt_vt];
      }
      
      // add the faces
      
      int pnt_pl = model->grid.grid_pnt[tile_pnt].pl;
      int pl_id = model->grid.pl_data[pnt_pl];
      
      while (pl_id >= 0) {
        if (dl.num_faces == SCN_VIS_PL_LIST_SIZE) break;
        //add_face_ot_list(pl_id);
        dl.pl_list[dl.num_faces] = pl_id;
        dl.num_faces++;
        
        pnt_pl++;
        pl_id = model->grid.pl_data[pnt_pl];
      }
      
      #if SCENE_HAS_MAPS
        if (scn.curr_model == scn.cam_curr_map) {
          dl.num_tiles++;
        }
      #else
        dl.num_tiles++;
      #endif
    }
  #endif
  
  #if 0
    void draw_transformed_tile(vec3_t pos) {
      transform_model_vis(model_matrix, dl.total_vertices);
      dl.total_vertices += dl.num_vertices;
      if (cfg.enable_sub_grid) {
        draw_sub_grid(x, y, z, map_pos, tile_pnt, g_model);
      } else {
        set_ot_list_vis(pos, 0, g_model, g_ot_list);
        draw_ot_list_vis(pos, 0, 0, g_model, g_ot_list);
      }
    }
  #endif
#endif

// add face to ordering table

void add_face_ot_list(int i) {
  int pt = g_model->face_group[i].face_index;
  fixed sz = tr_vertices[g_model->faces[pt]].z;
  
  for (int j = 1; j < g_model->face_group[i].face_num_vertices; j++) {
    fixed z = tr_vertices[g_model->faces[pt + j]].z;
    if (z > sz) {
      sz = z;
    }
  }
  
  if (sz < vp.z_near) return;
  u16 z_level = sz >> FP; //(sz-c_dist+(3<<FP))>>(FP-(5-3));
  if (z_level >= SCN_OT_SIZE) {
    z_level = SCN_OT_SIZE - 1;
  }
  
  if (ordering_table[z_level] >= 0) {
    ot_pl_list[i].pnt = ordering_table[z_level];
  }
  
  ordering_table[z_level] = i;
  r_scene_num_elements_ot++;
}

void transform_grid_cube(fixed tile_size) {
  set_matrix(mdl_cube_matrix, identity_matrix);
  scale_matrix(tile_size, tile_size, tile_size, mdl_cube_matrix);
  rotate_matrix(-cam.rot.x, -cam.rot.y, -cam.rot.z, mdl_cube_matrix, 1);
  // transform_matrix(mdl_cube_matrix, camera_matrix);
  transform_vertices(mdl_cube_matrix, mdl_cube_vertices, mdl_cube_tr_vertices, MDL_CUBE_NUM_VT);
}

void draw_grid_cube_lines(vec3_t pos, u16 color) {
  transform_vertex(pos, &pos, camera_matrix);
  set_matrix(mdl_cube_matrix, identity_matrix);
  translate_matrix(pos.x, pos.y, pos.z, mdl_cube_matrix);
  // transform_matrix(mdl_cube_matrix, camera_matrix);
  transform_vertices_pos(mdl_cube_matrix, mdl_cube_tr_vertices, tr_vertices, MDL_CUBE_NUM_VT);
  
  for (u32 i = 0; i < MDL_CUBE_LINES_SIZE; i += 2) {
    line_t line;
    line.p0.x = tr_vertices[mdl_cube_lines[i]].x;
    line.p0.y = tr_vertices[mdl_cube_lines[i]].y;
    line.p0.z = tr_vertices[mdl_cube_lines[i]].z;
    line.p1.x = tr_vertices[mdl_cube_lines[i + 1]].x;
    line.p1.y = tr_vertices[mdl_cube_lines[i + 1]].y;
    line.p1.z = tr_vertices[mdl_cube_lines[i + 1]].z;
    
    draw_line_3d(line, color);
  }
}