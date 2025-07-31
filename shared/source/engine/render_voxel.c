#include "common.h"

#define CHECK_FRUSTUM_CULLING 1

typedef struct {
  fixed x, y, r;
} vec3_r_t;

// order of the vertices for every viewing direction

static const u8 vertex_order_table[8][8] = {
  {0, 1, 2, 3, 4, 5, 6, 7},
  {1, 0, 3, 2, 5, 4, 7, 6},
  {3, 2, 1, 0, 7, 6, 5, 4},
  {2, 3, 0, 1, 6, 7, 4, 5},
  {4, 5, 6, 7, 0, 1, 2, 3},
  {5, 4, 7, 6, 1, 0, 3, 2},
  {7, 6, 5, 4, 3, 2, 1, 0},
  {6, 7, 4, 5, 2, 3, 0, 1},
};

// draws voxels by bilinearly interpolating a 3D volume by traversing a 2D grid on x and z from back to front and drawing RLE stored columns

void draw_voxel_model_affine(const vx_model_t *model) {
  u8 face_dir_bits = 0;
  
  if (model_matrix[VEC_X_Z] < 0) {
    face_dir_bits |= 1;
  }
  
  if (model_matrix[VEC_Z_Z] > 0) {
    face_dir_bits |= 2;
  }
  
  if (model_matrix[VEC_Y_Z] < 0) {
    face_dir_bits |= 4;
  }
  
  const u8 *vertex_order = vertex_order_table[face_dir_bits];
  
  vec3_r_t model_vt[8];
  
  // perspective division
  
  for (int i = 0; i < 8; i++) {
    #if PERSPECTIVE_ENABLED
      // unsigned fixed point division
      #if !DIV_LUT_ENABLE
        fixed_u rc_z = fp_div(vp.focal_length, tr_vertices[i].z);
      #else
        fixed_u rc_z = div_lut[(fixed_u)tr_vertices[i].z >> 8] << 8; // .16 result
        rc_z = vp.focal_length_i * rz;
      #endif
      model_vt[i].x = fp_mul(tr_vertices[i].x, rc_z);
      model_vt[i].y = fp_mul(tr_vertices[i].y, rc_z);
      model_vt[i].r = fp_mul(model->voxel_radius << 1, rc_z);
    #else
      model_vt[i].x = tr_vertices[i].x;
      model_vt[i].y = tr_vertices[i].y;
      model_vt[i].r = model->voxel_radius << 1;
    #endif
    
    // convert to screen coordinates
    
    #if 0 && DOUBLED_PIXELS
      model_vt[i].x >>= 1;
    #endif
    
    model_vt[i].x += SCREEN_WIDTH_FP >> 1;
    model_vt[i].y += SCREEN_HEIGHT_FP >> 1;
  }
  
  // calculate the deltas for the z edges in the cube
  
  vec3_r_t dt_z_0, dt_z_1, dt_z_2, dt_z_3;
  
  // unsigned integer division
  #if !DIV_LUT_ENABLE
    fixed rc_width = fp_div(1, model->size_i.w);
    fixed rc_depth = fp_div(1, model->size_i.d);
    fixed rc_height = fp_div(1, model->size_i.h);
  #else
    fixed rc_width = div_lut[model->size_i.w]; // .16 result
    fixed rc_depth = div_lut[model->size_i.d]; // .16 result
    fixed rc_height = div_lut[model->size_i.h]; // .16 result
  #endif
  
  fixed dx = model_vt[vertex_order[3]].x - model_vt[vertex_order[0]].x;
  fixed dy = model_vt[vertex_order[3]].y - model_vt[vertex_order[0]].y;
  fixed dr = model_vt[vertex_order[3]].r - model_vt[vertex_order[0]].r;
  dt_z_0.x = fp_mul(dx, rc_depth);
  dt_z_0.y = fp_mul(dy, rc_depth);
  dt_z_0.r = fp_mul(dr, rc_depth);
  
  dx = model_vt[vertex_order[2]].x - model_vt[vertex_order[1]].x;
  dy = model_vt[vertex_order[2]].y - model_vt[vertex_order[1]].y;
  dr = model_vt[vertex_order[2]].r - model_vt[vertex_order[1]].r;
  dt_z_1.x = fp_mul(dx, rc_depth);
  dt_z_1.y = fp_mul(dy, rc_depth);
  dt_z_1.r = fp_mul(dr, rc_depth);
  
  dx = model_vt[vertex_order[7]].x - model_vt[vertex_order[4]].x;
  dy = model_vt[vertex_order[7]].y - model_vt[vertex_order[4]].y;
  dr = model_vt[vertex_order[7]].r - model_vt[vertex_order[4]].r;
  dt_z_2.x = fp_mul(dx, rc_depth);
  dt_z_2.y = fp_mul(dy, rc_depth);
  dt_z_2.r = fp_mul(dr, rc_depth);
  
  dx = model_vt[vertex_order[6]].x - model_vt[vertex_order[5]].x;
  dy = model_vt[vertex_order[6]].y - model_vt[vertex_order[5]].y;
  dr = model_vt[vertex_order[6]].r - model_vt[vertex_order[5]].r;
  dt_z_3.x = fp_mul(dx, rc_depth);
  dt_z_3.y = fp_mul(dy, rc_depth);
  dt_z_3.r = fp_mul(dr, rc_depth);
  
  // set the start point for the z interpolants in the middle of the block
  
  vec3_r_t pos_z_0, pos_z_1, pos_z_2, pos_z_3;
  
  pos_z_0.x = model_vt[vertex_order[0]].x + (dt_z_0.x >> 1);
  pos_z_0.y = model_vt[vertex_order[0]].y + (dt_z_0.y >> 1);
  pos_z_0.r = model_vt[vertex_order[0]].r + (dt_z_0.r >> 1);
  
  pos_z_1.x = model_vt[vertex_order[1]].x + (dt_z_1.x >> 1);
  pos_z_1.y = model_vt[vertex_order[1]].y + (dt_z_1.y >> 1);
  pos_z_1.r = model_vt[vertex_order[1]].r + (dt_z_1.r >> 1);
  
  pos_z_2.x = model_vt[vertex_order[4]].x + (dt_z_2.x >> 1);
  pos_z_2.y = model_vt[vertex_order[4]].y + (dt_z_2.y >> 1);
  pos_z_2.r = model_vt[vertex_order[4]].r + (dt_z_2.r >> 1);
  
  pos_z_3.x = model_vt[vertex_order[5]].x + (dt_z_3.x >> 1);
  pos_z_3.y = model_vt[vertex_order[5]].y + (dt_z_3.y >> 1);
  pos_z_3.r = model_vt[vertex_order[5]].r + (dt_z_3.r >> 1);
  
  // set the start and end points on the grid in back to front order
  
  int x_start, x_end, z_start, z_end, x_inc, z_inc;
  u8 model_dir_y;
  u8 model_face_vis = 0;
  
  if (!(face_dir_bits & 1)) {
    x_start = 0;
    x_end = model->size_i.w;
    x_inc = 1;
    model_face_vis |= 1; // -x
  } else {
    x_start = model->size_i.w - 1;
    x_end = -1;
    x_inc = -1;
    model_face_vis |= 2; // +x
  }
  
  if (!(face_dir_bits & 2)) {
    z_start = model->size_i.d - 1;
    z_end = -1;
    z_inc = -1;
    model_face_vis |= 4; // -z
  } else {
    z_start = 0;
    z_end = model->size_i.d;
    z_inc = 1;
    model_face_vis |= 8; // +z
  }
  
  if (!(face_dir_bits & 4)) {
    model_dir_y = 0;
    model_face_vis |= 16; // -y
  } else {
    model_dir_y = 1;
    model_face_vis |= 32; // +y
  }
  
  int grid_pnt_z = z_start * model->size_i.w + x_start;
  
  // traverse the grid from back to front
  
  for (int i = z_start; i != z_end; i += z_inc) {
    vec3_r_t dt_x_0, dt_x_1;
    
    dx = pos_z_1.x - pos_z_0.x;
    dy = pos_z_1.y - pos_z_0.y;
    dr = pos_z_1.r - pos_z_0.r;
    dt_x_0.x = fp_mul(dx, rc_width);
    dt_x_0.y = fp_mul(dy, rc_width);
    dt_x_0.r = fp_mul(dr, rc_width);
    
    dx = pos_z_3.x - pos_z_2.x;
    dy = pos_z_3.y - pos_z_2.y;
    dr = pos_z_3.r - pos_z_2.r;
    dt_x_1.x = fp_mul(dx, rc_width);
    dt_x_1.y = fp_mul(dy, rc_width);
    dt_x_1.r = fp_mul(dr, rc_width);
    
    vec3_r_t pos_x_0, pos_x_1;
    
    pos_x_0.x = pos_z_0.x + (dt_x_0.x >> 1);
    pos_x_0.y = pos_z_0.y + (dt_x_0.y >> 1);
    pos_x_0.r = pos_z_0.r + (dt_x_0.r >> 1);
    
    pos_x_1.x = pos_z_2.x + (dt_x_1.x >> 1);
    pos_x_1.y = pos_z_2.y + (dt_x_1.y >> 1);
    pos_x_1.r = pos_z_2.r + (dt_x_1.r >> 1);
    
    int grid_pnt = grid_pnt_z;
    
    for (int j = x_start; j != x_end; j += x_inc) {
      int column_pnt = model->rle_grid[grid_pnt].pnt;
      
      if (column_pnt >= 0) {   
        vec3_r_t dt_y;
        
        dx = pos_x_0.x - pos_x_1.x;
        dy = pos_x_0.y - pos_x_1.y;
        dr = pos_x_0.r - pos_x_1.r;
        dt_y.x = fp_mul(dx, rc_height);
        dt_y.y = fp_mul(dy, rc_height);
        dt_y.r = fp_mul(dr, rc_height);
        
        vec3_r_t pos_y;
        
        pos_y.x = pos_x_1.x + (dt_y.x >> 1);
        pos_y.y = pos_x_1.y + (dt_y.y >> 1);
        pos_y.r = pos_x_1.r + (dt_y.r >> 1);
        
        int y_start, y_end, y_inc;
        
        if (!model_dir_y) {
          y_start = 0;
          y_end = model->rle_grid[grid_pnt].length;
          y_inc = 1;
        } else {
          y_start = model->rle_grid[grid_pnt].length - 1;
          y_end = -1;
          y_inc = -1;
        }
        
        for (int k = y_start; k != y_end; k += y_inc) {
          u8 color = model->rle_columns[column_pnt + k].color;
          u8 face_vis = model->rle_columns[column_pnt + k].vis;
          u8 length = model->rle_columns[column_pnt + k].length;
          
          // backface culling
          if (!color || !(face_vis & model_face_vis)) {
          // if (!color || !face_vis) {
            pos_y.x += dt_y.x * length;
            pos_y.y += dt_y.y * length;
            pos_y.r += dt_y.r * length;
            
            continue;
          }
          
          for (int l = 0; l < length; l++) {
            int x = (pos_y.x - pos_y.r) >> FP;
            int y = (pos_y.y - pos_y.r) >> FP;
            int w = (pos_y.r >> FP) + 1;
            int h = w;
            
            // clipping
            
            if (x < 0) {
              w += x;
              x = 0; 
            }
            
            if (y < 0) {
              h += y;
              y = 0; 
            }
            
            if (x + w >= SCREEN_WIDTH) {
              w = SCREEN_WIDTH - x;
            }
            
            if (y + h >= SCREEN_HEIGHT) {
              h = SCREEN_HEIGHT - y;
            }
            
            #ifdef PC
              fill_rect(x, y, w, h, model->palette[color]);
            #else
              fill_rect(x, y, w, h, dup8(color));
            #endif
            
            pos_y.x += dt_y.x;
            pos_y.y += dt_y.y;
            pos_y.r += dt_y.r;
          }
        }
      }
      
      pos_x_0.x += dt_x_0.x;
      pos_x_0.y += dt_x_0.y;
      pos_x_0.r += dt_x_0.r;
      pos_x_1.x += dt_x_1.x;
      pos_x_1.y += dt_x_1.y;
      pos_x_1.r += dt_x_1.r;
      
      grid_pnt += x_inc;
    }
    
    pos_z_0.x += dt_z_0.x;
    pos_z_0.y += dt_z_0.y;
    pos_z_0.r += dt_z_0.r;
    pos_z_1.x += dt_z_1.x;
    pos_z_1.y += dt_z_1.y;
    pos_z_1.r += dt_z_1.r;
    pos_z_2.x += dt_z_2.x;
    pos_z_2.y += dt_z_2.y;
    pos_z_2.r += dt_z_2.r;
    pos_z_3.x += dt_z_3.x;
    pos_z_3.y += dt_z_3.y;
    pos_z_3.r += dt_z_3.r;
    
    grid_pnt_z += z_inc * model->size_i.w;
  }
}

void draw_voxel_model_ps(const vx_model_t *model) {
  u8 face_dir_bits = 0;
  
  if (model_matrix[VEC_X_Z] < 0) {
    face_dir_bits |= 1;
  }
  
  if (model_matrix[VEC_Z_Z] > 0) {
    face_dir_bits |= 2;
  }
  
  if (model_matrix[VEC_Y_Z] < 0) {
    face_dir_bits |= 4;
  }
  
  const u8 *vertex_order = vertex_order_table[face_dir_bits];
  
  // calculate the deltas for the z edges in the cube
  
  vec3_t dt_z_0, dt_z_1, dt_z_2, dt_z_3;
  
  // unsigned integer division
  #if !DIV_LUT_ENABLE
    fixed rc_width = fp_div(1, model->size_i.w);
    fixed rc_depth = fp_div(1, model->size_i.d);
    fixed rc_height = fp_div(1, model->size_i.h);
  #else
    fixed rc_width = div_lut[model->size_i.w]; // .16 result
    fixed rc_depth = div_lut[model->size_i.d]; // .16 result
    fixed rc_height = div_lut[model->size_i.h]; // .16 result
  #endif
  
  fixed dx = tr_vertices[vertex_order[3]].x - tr_vertices[vertex_order[0]].x;
  fixed dy = tr_vertices[vertex_order[3]].y - tr_vertices[vertex_order[0]].y;
  fixed dz = tr_vertices[vertex_order[3]].z - tr_vertices[vertex_order[0]].z;
  dt_z_0.x = fp_mul(dx, rc_depth);
  dt_z_0.y = fp_mul(dy, rc_depth);
  dt_z_0.z = fp_mul(dz, rc_depth);
  
  dx = tr_vertices[vertex_order[2]].x - tr_vertices[vertex_order[1]].x;
  dy = tr_vertices[vertex_order[2]].y - tr_vertices[vertex_order[1]].y;
  dz = tr_vertices[vertex_order[2]].z - tr_vertices[vertex_order[1]].z;
  dt_z_1.x = fp_mul(dx, rc_depth);
  dt_z_1.y = fp_mul(dy, rc_depth);
  dt_z_1.z = fp_mul(dz, rc_depth);
  
  dx = tr_vertices[vertex_order[7]].x - tr_vertices[vertex_order[4]].x;
  dy = tr_vertices[vertex_order[7]].y - tr_vertices[vertex_order[4]].y;
  dz = tr_vertices[vertex_order[7]].z - tr_vertices[vertex_order[4]].z;
  dt_z_2.x = fp_mul(dx, rc_depth);
  dt_z_2.y = fp_mul(dy, rc_depth);
  dt_z_2.z = fp_mul(dz, rc_depth);
  
  dx = tr_vertices[vertex_order[6]].x - tr_vertices[vertex_order[5]].x;
  dy = tr_vertices[vertex_order[6]].y - tr_vertices[vertex_order[5]].y;
  dz = tr_vertices[vertex_order[6]].z - tr_vertices[vertex_order[5]].z;
  dt_z_3.x = fp_mul(dx, rc_depth);
  dt_z_3.y = fp_mul(dy, rc_depth);
  dt_z_3.z = fp_mul(dz, rc_depth);
  
  // set the start point for the z interpolants in the middle of the block
  
  vec3_t pos_z_0, pos_z_1, pos_z_2, pos_z_3;
  
  pos_z_0.x = tr_vertices[vertex_order[0]].x + (dt_z_0.x >> 1);
  pos_z_0.y = tr_vertices[vertex_order[0]].y + (dt_z_0.y >> 1);
  pos_z_0.z = tr_vertices[vertex_order[0]].z + (dt_z_0.z >> 1);
  
  pos_z_1.x = tr_vertices[vertex_order[1]].x + (dt_z_1.x >> 1);
  pos_z_1.y = tr_vertices[vertex_order[1]].y + (dt_z_1.y >> 1);
  pos_z_1.z = tr_vertices[vertex_order[1]].z + (dt_z_1.z >> 1);
  
  pos_z_2.x = tr_vertices[vertex_order[4]].x + (dt_z_2.x >> 1);
  pos_z_2.y = tr_vertices[vertex_order[4]].y + (dt_z_2.y >> 1);
  pos_z_2.z = tr_vertices[vertex_order[4]].z + (dt_z_2.z >> 1);
  
  pos_z_3.x = tr_vertices[vertex_order[5]].x + (dt_z_3.x >> 1);
  pos_z_3.y = tr_vertices[vertex_order[5]].y + (dt_z_3.y >> 1);
  pos_z_3.z = tr_vertices[vertex_order[5]].z + (dt_z_3.z >> 1);
  
  // set the start and end points on the grid in back to front order
  
  int x_start, x_end, z_start, z_end, x_inc, z_inc;
  u8 model_dir_y;
  u8 model_face_vis = 0;
  
  if (!(face_dir_bits & 1)) {
    x_start = 0;
    x_end = model->size_i.w;
    x_inc = 1;
    model_face_vis |= 1;
  } else {
    x_start = model->size_i.w - 1;
    x_end = -1;
    x_inc = -1;
    model_face_vis |= 2;
  }
  
  if (!(face_dir_bits & 2)) {
    z_start = model->size_i.d - 1;
    z_end = -1;
    z_inc = -1;
    model_face_vis |= 4;
  } else {
    z_start = 0;
    z_end = model->size_i.d;
    z_inc = 1;
    model_face_vis |= 8;
  }
  
  if (!(face_dir_bits & 4)) {
    model_dir_y = 0;
    model_face_vis |= 16;
  } else {
    model_dir_y = 1;
    model_face_vis |= 32;
  }
  
  int grid_pnt_z = z_start * model->size_i.w + x_start;
  
  // traverse the grid from back to front
  
  for (int i = z_start; i != z_end; i += z_inc) {
    vec3_t dt_x_0, dt_x_1;
    
    dx = pos_z_1.x - pos_z_0.x;
    dy = pos_z_1.y - pos_z_0.y;
    dz = pos_z_1.z - pos_z_0.z;
    dt_x_0.x = fp_mul(dx, rc_width);
    dt_x_0.y = fp_mul(dy, rc_width);
    dt_x_0.z = fp_mul(dz, rc_width);
    
    dx = pos_z_3.x - pos_z_2.x;
    dy = pos_z_3.y - pos_z_2.y;
    dz = pos_z_3.z - pos_z_2.z;
    dt_x_1.x = fp_mul(dx, rc_width);
    dt_x_1.y = fp_mul(dy, rc_width);
    dt_x_1.z = fp_mul(dz, rc_width);
    
    vec3_t pos_x_0, pos_x_1;
    
    pos_x_0.x = pos_z_0.x + (dt_x_0.x >> 1);
    pos_x_0.y = pos_z_0.y + (dt_x_0.y >> 1);
    pos_x_0.z = pos_z_0.z + (dt_x_0.z >> 1);
    
    pos_x_1.x = pos_z_2.x + (dt_x_1.x >> 1);
    pos_x_1.y = pos_z_2.y + (dt_x_1.y >> 1);
    pos_x_1.z = pos_z_2.z + (dt_x_1.z >> 1);
    
    int grid_pnt = grid_pnt_z;
    
    for (int j = x_start; j != x_end; j += x_inc) {
      int column_pnt = model->rle_grid[grid_pnt].pnt;
      
      if (column_pnt >= 0) {   
        vec3_t dt_y;
        
        dx = pos_x_0.x - pos_x_1.x;
        dy = pos_x_0.y - pos_x_1.y;
        dz = pos_x_0.z - pos_x_1.z;
        dt_y.x = fp_mul(dx, rc_height);
        dt_y.y = fp_mul(dy, rc_height);
        dt_y.z = fp_mul(dz, rc_height);
        
        vec3_t pos_y;
        
        pos_y.x = pos_x_1.x + (dt_y.x >> 1);
        pos_y.y = pos_x_1.y + (dt_y.y >> 1);
        pos_y.z = pos_x_1.z + (dt_y.z >> 1);
        
        int y_start, y_end, y_inc;
        
        if (!model_dir_y) {
          y_start = 0;
          y_end = model->rle_grid[grid_pnt].length;
          y_inc = 1;
        } else {
          y_start = model->rle_grid[grid_pnt].length - 1;
          y_end = -1;
          y_inc = -1;
        }
        
        for (int k = y_start; k != y_end; k += y_inc) {
          u8 color = model->rle_columns[column_pnt + k].color;
          u8 face_vis = model->rle_columns[column_pnt + k].vis;
          u8 length = model->rle_columns[column_pnt + k].length;
          
          if (!color || !(face_vis & model_face_vis)) {
          // if (!color || !face_vis) {
            pos_y.x += dt_y.x * length;
            pos_y.y += dt_y.y * length;
            pos_y.z += dt_y.z * length;
            
            continue;
          }
          
          for (int l = 0; l < length; l++) {
            if (pos_y.z <= vp.z_near << 1) goto skip_voxel;
            if (pos_y.z >= vp.z_far) goto skip_voxel;
            
            // check frustum culling
            // 0 = outside, 1 = partially inside, 2 = totally inside
            
            #if CHECK_FRUSTUM_CULLING
              u8 frustum_side = check_frustum_culling(pos_y, model->voxel_radius, 1);
              
              if (!frustum_side) goto skip_voxel;
            #endif
            
            vec3_r_t sc_pos_y;
          
            // perspective division
            
            #if PERSPECTIVE_ENABLED
              // unsigned fixed point division
              #if !DIV_LUT_ENABLE
                fixed_u rc_z = fp_div(vp.focal_length, pos_y.z);
              #else
                fixed_u rc_z = div_lut[(fixed_u)pos_y.z >> 8] << 8; // .16 result
                rc_z = vp.focal_length_i * rc_z;
              #endif
              sc_pos_y.x = fp_mul(pos_y.x, rc_z);
              sc_pos_y.y = fp_mul(pos_y.y, rc_z);
              sc_pos_y.r = fp_mul(model->voxel_radius << 1, rc_z);
            #else
              sc_pos_y.x = pos_y.x;
              sc_pos_y.y = pos_y.y;
              sc_pos_y.r = model->voxel_radius << 1;
            #endif
            
            // convert to screen coordinates
            
            #if 0 && DOUBLED_PIXELS
              sc_pos_y.x >>= 1;
            #endif
            
            sc_pos_y.x += SCREEN_WIDTH_FP >> 1;
            sc_pos_y.y += SCREEN_HEIGHT_FP >> 1;
            
            int x = (sc_pos_y.x - sc_pos_y.r) >> FP;
            int y = (sc_pos_y.y - sc_pos_y.r) >> FP;
            int w = (sc_pos_y.r >> FP) + 1;
            int h = w;
            
            if (frustum_side == 1) {
              // clipping
              
              if (x < 0) {
                w += x;
                x = 0; 
              }
              
              if (y < 0) {
                h += y;
                y = 0; 
              }
              
              if (x + w >= SCREEN_WIDTH) {
                w = SCREEN_WIDTH - x;
              }
              
              if (y + h >= SCREEN_HEIGHT) {
                h = SCREEN_HEIGHT - y;
              }
            }
            
            #ifdef PC
              fill_rect(x, y, w, h, model->palette[color]);
            #else
              fill_rect(x, y, w, h, dup8(color));
            #endif
            
            skip_voxel:
            
            pos_y.x += dt_y.x;
            pos_y.y += dt_y.y;
            pos_y.z += dt_y.z;
          }
        }
      }
      
      pos_x_0.x += dt_x_0.x;
      pos_x_0.y += dt_x_0.y;
      pos_x_0.z += dt_x_0.z;
      pos_x_1.x += dt_x_1.x;
      pos_x_1.y += dt_x_1.y;
      pos_x_1.z += dt_x_1.z;
      
      grid_pnt += x_inc;
    }
    
    pos_z_0.x += dt_z_0.x;
    pos_z_0.y += dt_z_0.y;
    pos_z_0.z += dt_z_0.z;
    pos_z_1.x += dt_z_1.x;
    pos_z_1.y += dt_z_1.y;
    pos_z_1.z += dt_z_1.z;
    pos_z_2.x += dt_z_2.x;
    pos_z_2.y += dt_z_2.y;
    pos_z_2.z += dt_z_2.z;
    pos_z_3.x += dt_z_3.x;
    pos_z_3.y += dt_z_3.y;
    pos_z_3.z += dt_z_3.z;
    
    grid_pnt_z += z_inc * model->size_i.w;
  }
}