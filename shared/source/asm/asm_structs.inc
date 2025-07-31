// flags

// g_poly
.equ poly.is_backface.bit, 1
.equ poly.has_texture.bit, (1 << 1)
.equ poly.has_transparency.bit, (1 << 2)
.equ poly.is_visible.bit, (1 << 3)

// model
.equ model.has_normals.bit, 1
.equ model.has_grid.bit, (1 << 1)
.equ model.has_textures.bit, (1 << 2)

// config
.equ cfg.draw_lines.bit, 1
.equ cfg.draw_polys.bit, (1 << 1)
.equ cfg.draw_textures.bit, (1 << 2)
.equ cfg.draw_textured_poly.bit, (1 << 3)
.equ cfg.perspective_enabled.bit, (1 << 4)
.equ cfg.tx_perspective_mapping_enabled.bit, (1 << 5)
.equ cfg.static_light.bit, (1 << 6)
.equ cfg.directional_lighting.bit, (1 << 7)
.equ cfg.draw_normals.bit, (1 << 8)
.equ cfg.draw_z_buffer.bit, (1 << 9)
.equ cfg.enable_far_plane_clipping.bit, (1 << 10)
.equ cfg.draw_grid.bit, (1 << 11)
.equ cfg.tx_alpha_cr.bit, (1 << 12)
.equ cfg.debug.bit, (1 << 13)
.equ cfg.animation_play.bit, (1 << 14)
.equ cfg.occlusion_cl_enabled.bit, (1 << 15)
.equ cfg.clean_screen.bit, (1 << 16)
.equ cfg.face_enable_persp_tx.bit, (1 << 17)

// g_poly
.struct 0
  g_poly_t:
    // array
    poly.vertices: // 32 bit
      .space (8 * 5 * 4)  // x, y, z, u, v
    
    // int
    poly.color: // u16
      .space 4
    poly.flags: // is_backface, has_texture, has_transparency, is_visible // u8
      .space 4
    poly.num_vertices: // u8
      .space 4
    poly.texture_size_wh_s: // u16 (w | h)
      .space 4
    poly.texture_width_bits: // u8
      .space 4
    poly.frustum_clip_sides: // u8
      .space 4
    poly.face_type: // u8
      .space 4
    poly.final_light_factor: // u16
      .space 4
    poly.face_id: // u16
      .space 4
    
    // pointers
    poly.texture_image: // const u16*
      .space 4
    poly.cr_palette_tx_idx: // const u16*
      .space 4
    
.previous

// grid_pnt
.struct 0
  grid_pnt_t:
    grid_pnt.pl: // const s16*
      .space 4
    grid_pnt.vt: // const s16*
      .space 4
    
.previous

// grid
.struct 0
  grid_t:
    // pointers
    grid.grid_pnt: // const grid_pnt_t*
      .space 4
    grid.pl_data: // const s16*
      .space 4
    grid.vt_data: // const s16*
      .space 4
    
    // array
    grid.size_i: // size3_i (w, d, h)
      .space (3 * 4)
    
    // int
    grid.num_tiles: // u32
      .space 4
    grid.tile_size: // fixed
      .space 4
    grid.tile_size_bits: // u8
      .space 4
    
    grid_t_size:
.previous

// face_group
.struct 0
  face_group_t:
    face_group.num_vertices: // u8
      .space 1
    face_group.face_materials: // u8
      .space 1
    face_group.face_types: // u8
      .space 2
    face_group.face_index: // u16
      .space 2
    face_group.tx_face_index: // u16
      .space 2
    face_group.sprite_face_index: // s16
      .space 4
  
.previous

// model
.struct 0
  model_t:
    // int
    model.num_vertices: // u16
      .space 2
    model.num_faces: // u16
      .space 2
    // model.faces_size: // u32
      // .space 4
    model.num_txcoords: // u16
      .space 2
    model.num_tx_faces: // u16
      .space 2
    // model.tx_faces_size: // u32
      // .space 4
    model.num_objects: // u16
      .space 2
    model.num_materials: // u8
      .space 1
    model.num_sprites: // u8
      .space 1
    model.num_sprite_vertices: // u8
      .space 1
    model.flags: // has_normals, has_grid, has_textures // u8
      .space 3
    
    // array
    model.origin: // vec3_s16_t
      .space (3 * 4) // x, y, z
    model.size: // size3_u16_t
      .space (3 * 4) // w, h, d
    
    // pointers
    model.vertices: // const vec3_s16_t*
      .space 4
    model.faces: // const u16*
      .space 4
    model.txcoords: // const vec2_tx_u16_t*
      .space 4
    model.tx_faces: // const u16*
      .space 4
    model.normals: // const vec3_s16_t*
      .space 4
    model.face_group: // const face_group_t*
      .space 4
    model.sprite_vertices: // const vec3_s16_t*
      .space 4
    model.sprite_faces: // const u8*
      .space 4
    model.object_face_index: // const u16*
      .space 4
    model.object_num_faces: // const u16*
      .space 4
    model.mtl_textures: // const u8*
      .space 4
    model.lines: // const u8*
      .space 4
    
    model.grid:
      .space grid_t_size
    
.previous

// tx_group
.struct 0
  tx_group_t:
    tx_group.texture_sizes_padded_wh: // u32 (w | h)
      .space 4
    tx_group.texture_width_bits: // u8
      .space 1
    tx_group.tx_animation_id: // s8
      .space 3
    tx_group.tx_index: // u32
      .space 4
    
.previous

// textures
.struct 0
  textures_t:
    // int
    textures.num_textures: // u8
      .space 1
    textures.num_animations: // u8
      .space 1
    textures.pal_size: // u8
      .space 1
    textures.pal_num_colors: // u8
      .space 1
    textures.pal_size_tx: // u8
      .space 1
    textures.pal_tx_num_colors: // u8
      .space 1
    textures.lightmap_levels: // u8
      .space 2
    textures.texture_data_total_size: // u32
      .space 4
    
    // pointers
    textures.material_colors: // const u8*
      .space 4
    textures.cr_palette_idx: // const u16*
      .space 4
    textures.material_colors_tx: // const u8*
      .space 4
    textures.cr_palette_tx_idx: // const u16*
      .space 4
    textures.tx_group: // const tx_group_t*
      .space 4
    textures.texture_data: // const u16*
      .space 4
    
.previous

// camera
.struct 0
  camera_t:
    // int
    cam.acc: // fixed
      .space 4
    cam.rot_speed: // fixed
      .space 4
    
    // array
    cam.pos: // vec3_t
      .space (3 * 4)
    cam.speed: // vec3_t
      .space (3 * 4)
    cam.rot: // vec3_u16_t
      .space (3 * 2) + 2
    
.previous

// scene
.struct 0
  scene_t:
    // array
    scene.lightdir: // vec3_t
      .space (3 * 4)
    scene.lightdir_n: // vec3_t
      .space (3 * 4)
    
    // int
    scene.bg_color: // u16
      .space 2
    
    scene.bitfield: // directional_lighting_enabled, draw_sky // u8
      .space 2
    
    scene.curr_model: // u8
      .space 1
    scene.cam_curr_map: // u8
      .space 3
    
.previous

// viewport
.struct 0
  viewport_t:
    vp.z_near: // fixed
      .space 4
    vp.z_far: // fixed
      .space 4
    // vp.aspect_ratio:
    //   .space 4
    vp.focal_length: // fixed
      .space 4
    vp.focal_length_i: // fixed
      .space 4
    vp.hfov: // fixed
      .space 4
    vp.vfov: // fixed
      .space 4
    vp.half_hfov: // fixed
      .space 4
    vp.half_vfov: // fixed
      .space 4
    vp.screen_side_x_dt: // fixed
      .space 4
    vp.screen_side_y_dt: // fixed
      .space 4
    
.previous

// transformed frustum
.struct 0
  tr_frustum_t:
    tr_frustum.vertices: // vec3_t
      .space (8 * 3 * 4)
    tr_frustum.normals: // vec4_nm_t
      .space (6 * 4 * 4)

.previous

// display lists
.struct 0
  display_list_t:
    // pointers
    dl.pl_list: // u16*
      .space 4
    dl.vt_list: // u16*
      .space 4
    dl.obj_list: // u16*
      .space 4
    dl.tr_vt_index: // u16*
      .space 4
    dl.visible_vt_list: // u8*
      .space 4
    
    // int
    dl.num_tiles: // u16
      .space 2
    dl.num_faces: // u16
      .space 2
    dl.num_vertices: // u16
      .space 2
    dl.num_objects: // u16
      .space 2
    dl.total_vertices: // u32
      .space 4
    
.previous