#include "common.h"

void load_model_palette() {
  if (g_obj.type == OBJ_MODEL) {
    if (g_model->flags.has_textures && cfg.draw_textures) {
      load_palette(1);
    } else {
      load_palette(0);
    }
  } else if (g_obj.type == OBJ_VOXEL) {
    load_vx_model_palette(g_vx_model);
  }
}

void load_palette(int pal_type) {
  if (!pal_type) {
    for (int i = 0; i < g_textures->pal_size; i++) {
      hw_palette[i] = g_textures->cr_palette_idx[i];
    }
  } else {
    // memcpy16(hw_palette, textures->cr_palette_tx_idx, textures->pal_size_tx);
    for (int i = 0; i < g_textures->pal_size_tx; i++) {
      hw_palette[i] = g_textures->cr_palette_tx_idx[i];
    }
  }
}

void load_vx_model_palette(const vx_model_t *model) {
  for (int i = 0; i < model->pal_size; i++) {
    hw_palette[i] = model->palette[i];
  }
}

void calc_model_light(const model_t *model) {
  for (int i = 0; i < model->num_faces; i++) {
    vec3_t normal;
    normal.x = model->normals[i].x << 8;
    normal.y = model->normals[i].y << 8;
    normal.z = model->normals[i].z << 8;
    fixed dp = dot(&normal, &scn.lightdir_n); // dot product
    //if (dp < 0){dp /= 4;}
    
    if (model->face_group[i].face_types & BACKFACE) {
      dp = abs_c(dp);
    } else if (dp < 0) {
      dp = 0;
    }
    
    fixed_u final_color = fp_mul(dp, fix(0.4)) + fix(0.6);
    
    if (final_color >= 1 << FP) {
      final_color = (1 << FP) - 1;
    }
    
    light_faces[i] = (final_color << LIGHT_GRD_BITS) >> FP;
  }
}

void calc_face_light(int face_id, const model_t *model) {
  vec3_t normal;
  normal.x = model->normals[face_id].x << 8;
  normal.y = model->normals[face_id].y << 8;
  normal.z = model->normals[face_id].z << 8;
  
  fixed dp = dot(&normal, &scn.lightdir_n);
  //if (dp<0){dp/=4;}
  
  if (model->face_group[face_id].face_types & BACKFACE) {
    dp = abs_c(dp);
  } else if (dp < 0) {
    dp = 0;
  }
  
  #if SPECULAR_LIGHT
    vec3_t viewdir, viewdir_n, halfdir_n;
    viewdir.x = cam.pos.x - normal.x;
    viewdir.y = cam.pos.y - normal.y;
    viewdir.z = cam.pos.z - normal.z;
    normalize(&viewdir, &viewdir_n);
    viewdir_n.x += scene.lightdir_n.x;
    viewdir_n.y += scene.lightdir_n.y;
    viewdir_n.z += scene.lightdir_n.z;
    normalize(&viewdir_n, &halfdir_n);
    fixed dps = dot(&normal, &halfdir_n);
    
    if (model->face_types[face_index] & BACKFACE) {
      dps = abs_c(dps);
    } else if (dps < 0) {
      dps = 0;
    }
    
    pow8(&dps);
    //final_color=fp_mul(dps,fix(0.4))+fp_mul(dp,fix(0.4))+fix(0.6);
    fixed_u spec = fp_mul(dps, fix(0.4));
    fixed_u diff = fp_mul(dp, fix(0.4)) + fix(0.6);
    g_poly.final_light_factor = ((spec + diff) << LIGHT_GRD_BITS) >> FP;
  #else
    fixed_u final_color = fp_mul(dp, fix(0.4)) + fix(0.6);
    
    if (final_color >= 1 << FP) {
      final_color = (1 << FP) - 1;
    }
    
    g_poly.final_light_factor = (final_color << LIGHT_GRD_BITS) >> FP;
  #endif
  
  if (!g_poly.flags.has_texture) {
    // final_color = g_textures->cr_palette_idx[model->face_group[face_id].face_materials * LIGHT_GRD + g_poly.final_light_factor];
    final_color = (g_textures->material_colors[model->face_group[face_id].face_materials] << LIGHT_GRD_BITS) + g_poly.final_light_factor;
  }
}

void change_model(int id) {
  g_model = model_list[id];
  //memcpy32(g_model, model_list[id], sizeof(g_model) >> 2);
}

void change_texture(int id) {
  g_textures = textures_list[id];
  //memcpy32(g_textures, texture_list[id], sizeof(texture) >> 2);
  #if ENABLE_TEXTURE_BUFFER
    if (g_textures->texture_data_total_size < TEXTURE_BUFFER_SIZE) {
      memcpy32(texture_buffer, g_textures->texture_data, g_textures->texture_data_total_size);
    }
  #endif
}