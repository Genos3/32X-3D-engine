#include "common.h"

extern u8 animation_frames[];

// set the ordering table

#if 1 // !ENABLE_ASM
  RAM_CODE void set_ot_list_vis(vec3_t pos, int dl_mode, const model_t *model, ot_list_t *ot_list) {
    int num_faces;
    if (!dl_mode) {
      // ot_size = MDL_OT_SIZE;
      num_faces = model->num_faces;
    } else {
      // ot_size = SCN_OT_SIZE;
      num_faces = dl.num_faces;
    }
    
    // init the ordering table
    // memset32(ot_list->pnt, -1, ot_list->size >> 1);
    
    for (int i = 0; i < num_faces; i++) {
      int face_id;
      if (!dl_mode) {
        face_id = i;
      } else {
        face_id = dl.pl_list[i];
      }
      
      int face_index = model->face_group[face_id].face_index;
      int face_type = model->face_group[face_id].face_types;
      
      // backface culling
      
      if (!(face_type & (SPRITE | BACKFACE))) {
        if (calc_view_normal_z(pos, face_id, model) >= 0) continue;
      }
      
      fixed far_z;
      if (!dl_mode) {
        far_z = tr_vertices[model->faces[face_index]].z;
      } else {
        far_z = tr_vertices[dl.tr_vt_index[model->faces[face_index]]].z;
      }
      
      int outside_frustum = 0xff;
      int clip_sides = 0;
      
      if (face_type & SPRITE) { // sprite
        int pt_vt;
        if (!dl_mode) {
          pt_vt = model->faces[face_index];
        } else {
          pt_vt = dl.tr_vt_index[model->faces[face_index]];
        }
        
        vec3_t vt;
        vt.x = tr_vertices[pt_vt].x;
        vt.y = tr_vertices[pt_vt].y;
        vt.z = tr_vertices[pt_vt].z;
        
        if (vt.z > vp.z_far) continue;
        if (vt.z < vp.z_near) continue;
        
        fixed w = fp_mul(vt.z, vp.screen_side_x_dt);
        fixed h = fp_mul(vt.z, vp.screen_side_y_dt);
        
        int sprite_face_index = model->face_group[face_id].sprite_face_index;
        
        for (int j = 0; j < model->face_group[face_id].face_num_vertices; j++) {
          vec2_t sp_vt;
          int pt_sp = model->sprite_faces[sprite_face_index + j];
          sp_vt.x = vt.x + tr_sprite_vertices[pt_sp].x;
          sp_vt.y = vt.y + tr_sprite_vertices[pt_sp].y;
          
          // compare the sprite vertices against the frustum planes
          
          int plane_sides = 0;
          
          plane_sides |= (sp_vt.x < -w) << 2; // left
          plane_sides |= (sp_vt.x > w) << 3; // right
          plane_sides |= (sp_vt.y < -h) << 4; // top
          plane_sides |= (sp_vt.y > h) << 5; // bottom
          
          outside_frustum &= plane_sides;
          clip_sides |= plane_sides;
        }
      } else { // normal face
        for (int j = 0; j < model->face_group[face_id].face_num_vertices; j++) {
          int pt_vt; 
          if (!dl_mode) {
            pt_vt = model->faces[face_index + j];
          } else {
            pt_vt = dl.tr_vt_index[model->faces[face_index + j]];
          }
          
          fixed vt_z = tr_vertices[pt_vt].z;
          
          if (vt_z > far_z) {
            far_z = vt_z;
          }
          
          int plane_sides = 0;
          
          fixed w = fp_mul(tr_vertices[pt_vt].z, vp.screen_side_x_dt);
          fixed h = fp_mul(tr_vertices[pt_vt].z, vp.screen_side_y_dt);
          
          plane_sides |= (tr_vertices[pt_vt].z < vp.z_near); // near
          plane_sides |= (tr_vertices[pt_vt].z > vp.z_far) << 1; // far
          plane_sides |= (tr_vertices[pt_vt].x < -w) << 2; // left
          plane_sides |= (tr_vertices[pt_vt].x > w) << 3; // right
          plane_sides |= (tr_vertices[pt_vt].y < -h) << 4; // top
          plane_sides |= (tr_vertices[pt_vt].y > h) << 5; // bottom
          
          // int plane_sides = clipping_vt_list[pt_vt];
          
          outside_frustum &= plane_sides;
          clip_sides |= plane_sides;
        }
      }
      
      // frustum culling
      
      if (outside_frustum) continue; // the face is outside the frustum
      
      u32 z_level = fp_mul(far_z, zfar_ratio) >> FP; // (far_z - c_dist + (3 << FP)) >> (FP - (5 - 3));
      
      if (z_level >= ot_list->size) {
        z_level = ot_list->size - 1;
      }
      
      if (ot_list->pnt[z_level] >= 0) {
        ot_list->pl_list[r_scene_num_elements_ot].pnt = ot_list->pnt[z_level];
      } else {
        ot_list->pl_list[r_scene_num_elements_ot].pnt = -1;
      }
      
      ot_list->pl_list[r_scene_num_elements_ot].id = i;
      ot_list->pl_list[r_scene_num_elements_ot].type = OT_FACE_TYPE;
      ot_list->pnt[z_level] = r_scene_num_elements_ot;
      clipping_pl_list[i] = clip_sides;
      r_scene_num_elements_ot++;
    }
  }
  
  RAM_CODE void set_ot_list_obj(ot_list_t *ot_list) {
    for (int i = 0; i < dl.num_objects; i++) {
      fixed pos_z = scn.obj_tr_pos[i].z;
      u32 z_level = fp_mul(pos_z, zfar_ratio) >> FP;
      
      if (z_level >= ot_list->size) {
        z_level = ot_list->size - 1;
      }
      
      if (ot_list->pnt[z_level] >= 0) {
        ot_list->pl_list[r_scene_num_elements_ot].pnt = ot_list->pnt[z_level];
      } else {
        ot_list->pl_list[r_scene_num_elements_ot].pnt = -1;
      }
      
      ot_list->pl_list[r_scene_num_elements_ot].id = dl.obj_list[i];
      ot_list->pl_list[r_scene_num_elements_ot].type = OT_OBJECT_TYPE;
      ot_list->pnt[z_level] = r_scene_num_elements_ot;
      r_scene_num_elements_ot++;
    }
  }
  
  RAM_CODE void draw_ot_list_vis(vec3_t pos, int draw_dir, int dl_mode, const model_t *model, ot_list_t *ot_list) {
    /* int ot_size;
    if (!cfg.occlusion_cl_enabled) {
      ot_size = SCN_OT_SIZE;
    } else {
      ot_size = GRID_CELL_OT_SIZE;
    } */
    
    if (!draw_dir) { // draw from back to front
      for (int i = ot_list->size - 1; i >= 0; i--) {
        int pnt = ot_list->pnt[i];
        
        while (pnt >= 0) {
          int id = ot_list->pl_list[pnt].id;
          u8 type = ot_list->pl_list[pnt].type;
          
          if (type == OT_FACE_TYPE) {
            draw_face_vis(pos, id, dl_mode, model);
          } else {
            draw_object(pos, id, draw_dir, &scn);
          }
          
          pnt = ot_list->pl_list[pnt].pnt;
        }
      }
    } else { // draw from front to back
      for (u32 i = 0; i < ot_list->size; i++) {
        int pnt = ot_list->pnt[i];
        
        while (pnt >= 0) {
          int id = ot_list->pl_list[pnt].id;
          u8 type = ot_list->pl_list[pnt].type;
          
          if (type == OT_FACE_TYPE) {
            draw_face_vis(pos, id, dl_mode, model);
          } else {
            draw_object(pos, id, draw_dir, &scn);
          }
          
          pnt = ot_list->pl_list[pnt].pnt;
        }
      }
    }
  }
  
  /* void make_model_display_list(model_t model) {
    dl.num_faces = 0;
    dl.num_vertices = 0;
    
    for (int i = 0; i < model->num_vertices; i++) {
      if (dl.total_vertices + dl.num_vertices == SCN_VIS_VT_LIST_SIZE) break;
      dl.vt_list[dl.num_vertices] = i;
      dl.tr_vt_index[i] = dl.total_vertices + dl.num_vertices;
      dl.num_vertices++;
    }
    
    for (int i = 0; i < model->num_faces; i++) {
      if (dl.num_faces == SCN_VIS_PL_LIST_SIZE) break;
      dl.pl_list[dl.num_faces] = i;
      dl.num_faces++;
    }
  } */
  
  RAM_CODE void draw_object(vec3_t pos, int obj_id, int draw_dir, scene_t *scene) {
    int obj_type = scene->obj_list[obj_id].type;
    
    if (obj_type == OBJ_SPRITE) {
      const model_t *model_pnt = scene->obj_list[obj_id].mdl_pnt;
      
      draw_face_vis(pos, 0, 0, model_pnt);
    } else if (obj_type == OBJ_VOXEL) {
      const vx_model_t *model_pnt = scene->obj_list[obj_id].mdl_pnt;
      
      //transform_vx_model(model_matrix, model_pnt);
      transform_vertices(model_matrix, model_pnt->vertices, tr_vertices, model_pnt->num_vertices);
      
      // TODO: add frustum culling
      int draw_affine_model = 1;
      
      for (int i = 0; i < 8; i++) {
        if (tr_vertices[i].z < vp.z_near) {
          draw_affine_model = 0;
          break;
        }
      }
      
      if (draw_affine_model) {
        draw_voxel_model_affine(model_pnt);
      } else {
        draw_voxel_model_ps(model_pnt);
      }
    } else { // normal model
      const model_t *model_pnt = scene->obj_list[obj_id].mdl_pnt;
      
      transform_model_vis(model_matrix, 0, 0, model_pnt);
      set_ot_list_vis(pos, 0, model_pnt, &g_ot_list);
      draw_ot_list_vis(pos, draw_dir, 0, model_pnt, &g_ot_list);
    }
  }
  
  RAM_CODE void draw_face_vis(vec3_t pos, int dl_face_id, int dl_mode, const model_t *model) {
    int face_id;
    if (!dl_mode) {
      face_id = dl_face_id;
    } else {
      face_id = dl.pl_list[dl_face_id];
    }
    
    int face_index = model->face_group[face_id].face_index;
    g_poly.face_type = model->face_group[face_id].face_types;
    g_poly.flags.has_transparency = !!(g_poly.face_type & HAS_ALPHA);
    g_poly.num_vertices = model->face_group[face_id].face_num_vertices;
    g_poly.frustum_clip_sides = clipping_pl_list[dl_face_id];
    g_poly.face_id = face_id;
    
    if (model->flags.has_textures && cfg.draw_textures && !(g_poly.face_type & UNTEXTURED)) {
      g_poly.flags.has_texture = 1;
    } else {
      g_poly.flags.has_texture = 0;
    }
    
    // load the polygon vertices
    
    if (g_poly.face_type & SPRITE) {
      int pt_vt;
      if (!dl_mode) {
        pt_vt = model->faces[face_index];
      } else {
        pt_vt = dl.tr_vt_index[model->faces[face_index]];
      }
      
      int sprite_face_index = model->face_group[face_id].sprite_face_index;
      
      vec3_t vt;
      vt.x = tr_vertices[pt_vt].x;
      vt.y = tr_vertices[pt_vt].y;
      vt.z = tr_vertices[pt_vt].z;
      
      if (!g_poly.flags.has_texture) {
        for (int i = 0; i < 4; i++) {
          int pt_sp = model->sprite_faces[sprite_face_index + i];
          g_poly.vertices[i].x = vt.x + tr_sprite_vertices[pt_sp].x;
          g_poly.vertices[i].y = vt.y + tr_sprite_vertices[pt_sp].y;
          g_poly.vertices[i].z = vt.z + tr_sprite_vertices[pt_sp].z;
        }
      } else {
        int tx_face_index = model->face_group[face_id].tx_face_index;
        
        for (int i = 0; i < 4; i++) {
          int pt_sp = model->sprite_faces[sprite_face_index + i];
          int pt_tx = model->tx_faces[tx_face_index + i];
          g_poly.vertices[i].x = vt.x + tr_sprite_vertices[pt_sp].x;
          g_poly.vertices[i].y = vt.y + tr_sprite_vertices[pt_sp].y;
          g_poly.vertices[i].z = vt.z + tr_sprite_vertices[pt_sp].z;
          g_poly.vertices[i].u = model->txcoords[pt_tx].u;
          g_poly.vertices[i].v = model->txcoords[pt_tx].v;
        }
      }
      
      g_poly.flags.is_visible = 1;
      g_poly.flags.is_backface = 0;
    } else { // normal face
      // backface culling
      if (!check_backface_culling(pos, face_id, model)) {
        if (g_poly.face_type & BACKFACE) {
          g_poly.flags.is_backface = 1;
          g_poly.flags.is_visible = 1;
        } else {
          g_poly.flags.is_backface = 0;
          g_poly.flags.is_visible = 0;
          
          if (!cfg.draw_lines) return;
        }
      } else {
        g_poly.flags.is_backface = 0;
        g_poly.flags.is_visible = 1;
      }
      
      if (!g_poly.flags.has_texture) {
        for (int i = 0; i < g_poly.num_vertices; i++) {
          int pt_vt;
          if (!dl_mode) {
            pt_vt = model->faces[face_index + i];
          } else {
            pt_vt = dl.tr_vt_index[model->faces[face_index + i]];
          }
          
          g_poly.vertices[i].x = tr_vertices[pt_vt].x;
          g_poly.vertices[i].y = tr_vertices[pt_vt].y;
          g_poly.vertices[i].z = tr_vertices[pt_vt].z;
        }
      } else {
        int tx_face_index = model->face_group[face_id].tx_face_index;
        
        for (int i = 0; i < g_poly.num_vertices; i++) {
          int pt_vt;
          if (!dl_mode) {
            pt_vt = model->faces[face_index + i];
          } else {
            pt_vt = dl.tr_vt_index[model->faces[face_index + i]];
          }
          
          int pt_tx = model->tx_faces[tx_face_index + i];
          
          g_poly.vertices[i].x = tr_vertices[pt_vt].x;
          g_poly.vertices[i].y = tr_vertices[pt_vt].y;
          g_poly.vertices[i].z = tr_vertices[pt_vt].z;
          g_poly.vertices[i].u = model->txcoords[pt_tx].u;
          g_poly.vertices[i].v = model->txcoords[pt_tx].v;
        }
      }
    }
    
    // make the face semitransparent for the fog
    
    #if ENABLE_FOG
      if (test_poly_dist(&g_poly, vp.z_far - fix(1))) {
        g_poly.flags.dithered_alpha = 0;
      } else {
        g_poly.flags.dithered_alpha = 1;
      }
    #endif
    
    int tx_id = model->mtl_textures[model->face_group[face_id].face_materials];
    // int tx_id = model->face_group[face_id].face_materials;
    
    if (g_poly.flags.has_texture) {
      g_poly.texture_width_bits = g_textures->tx_group[tx_id].texture_width_bits;
      g_poly.texture_size_wh_s = g_textures->tx_group[tx_id].texture_sizes_padded_wh;
      
      #if ENABLE_TEXTURE_BUFFER
        #if ENABLE_ANIMATIONS
          if (cfg.animation_play && g_poly.face_type & ANIMATED) {
            //- custom_face_animation(face_id);
            int tx_animation_id = animation_frames[g_textures->tx_group[tx_id].tx_animation_id];
            g_poly.texture_image = texture_buffer + g_textures->tx_group[tx_id + tx_animation_id].tx_index;
          } else {
            g_poly.texture_image = texture_buffer + g_textures->tx_group[tx_id].tx_index;
          }
        #else
          g_poly.texture_image = texture_buffer + g_textures->tx_group[tx_id].tx_index;
        #endif
      #else
        #if ENABLE_ANIMATIONS
          if (cfg.animation_play && g_poly.face_type & ANIMATED) {
            //- custom_face_animation(face_id);
            int tx_animation_id = animation_frames[g_textures->tx_group[tx_id].tx_animation_id];
            g_poly.texture_image = g_textures->texture_data + g_textures->tx_group[tx_id + tx_animation_id].tx_index;
          } else {
            g_poly.texture_image = g_textures->texture_data + g_textures->tx_group[tx_id].tx_index;
          }
        #else
          g_poly.texture_image = g_textures->texture_data + g_textures->tx_group[tx_id].tx_index;
        #endif
      #endif
      
      g_poly.cr_palette_tx_idx = g_textures->cr_palette_tx_idx;
    }
    
    if (cfg.static_light) {
      int light_grd;
      if (cfg.directional_lighting) {
        light_grd = light_faces[face_id];
      } else {
        light_grd = LIGHT_GRD - 1;
      }
      
      if (g_poly.flags.has_texture) {
        #ifdef PC
          g_poly.final_light_factor = light_grd;
        #else
          g_poly.final_light_factor = dup8(light_grd);
        #endif
      } else {
        #if PALETTE_MODE
          g_poly.color = (g_textures->material_colors[tx_id] << LIGHT_GRD_BITS) + light_grd;
          #ifndef PC
            g_poly.color = quad8(g_poly.color);
          #endif
        #else
          g_poly.color = g_textures->cr_palette_idx[(g_textures->material_colors[tx_id] << LIGHT_GRD_BITS) + light_grd];
          g_poly.color = dup16(g_poly.color);
        #endif
      }
    } else {
      calc_face_light(face_id, model);
    }
    
    if (g_poly.flags.has_texture && !(model->face_group[face_id].face_types & SPRITE)) {
      if (cfg.tx_perspective_mapping_enabled) {
        #if !TX_PERSP_MODE
          cfg.face_enable_persp_tx = test_poly_ratio(&g_poly);
          
          if (cfg.face_enable_persp_tx) {
            #if !SUBDIV_MODE
              subdiv_poly(&g_poly, face_id);
            #else
              subdiv_poly_edge_lut(&g_poly, face_id);
            #endif
          } else {
            #if DRAW_TRIANGLES
              subdiv_poly_tris(&g_poly, face_id);
            #else
              draw_face_tr(&g_poly, face_id);
            #endif
          }
        #elif TX_PERSP_MODE == 1
          draw_face_tr(&g_poly, face_id);
        #else
          subdiv_poly_clip(&g_poly, face_id);
        #endif
      } else {
        cfg.face_enable_persp_tx = 0;
        #if DRAW_TRIANGLES
          subdiv_poly_tris(&g_poly, face_id);
        #else
          draw_face_tr(&g_poly, face_id);
        #endif
      }
    } else {
      cfg.face_enable_persp_tx = 0;
      draw_face_tr(&g_poly, face_id);
    }
    
    #if DRAW_NORMALS
      if (!(g_poly.face_type & SPRITE)) {
        line_t line;
        line.p0.x = tr_vertices[model->faces[face_index]].x;
        line.p0.y = tr_vertices[model->faces[face_index]].y;
        line.p0.z = tr_vertices[model->faces[face_index]].z;
        line.p1.x = line.p0.x + tr_normals[dl_face_id].x;
        line.p1.y = line.p0.y + tr_normals[dl_face_id].y;
        line.p1.z = line.p0.z + tr_normals[dl_face_id].z;
        
        draw_line_3d(line, PAL_RED);
      }
    #endif
  }
#endif

/* void mgba_printfc(int a) {
  char str[12];
  //siprintf(str, "%d", a);
  sprintf_c(str, "%d", a);
  mgba_printf(3, str);
} */