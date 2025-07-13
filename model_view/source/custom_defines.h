#pragma once

#define AXIS_VECTOR_SIZE (1 << FP)
#define PERSPECTIVE_ENABLED 1 // perspective view
#define ENABLE_FAR_PLANE_CLIPPING 0
#define DRAW_GRID 0
#define VIEW_PERSP_SUB 0
#define VIEW_TEST_DIST 1
#define ENABLE_GRID_FRUSTUM_CULLING 1
#define TX_PERSP_MODE 2 // 0 = subdivided faces, 1 = sub affine per scanline (Quake method), 2 = subdivide by clipping against a plane
#define TX_PERSP_DIST (4 << FP) // fix(4)
#define TX_PERSP_RATIO fix(1.5)
#define SUBDIV_MODE 1 // 0 = subdivide the face by a midpoint, 1 = subdivide by edge
#define DRAW_TRIANGLES 0
#define DRAW_FRUSTUM 0
#define ADVANCE_FRUSTUM 0
#define ENABLE_ANIMATIONS 0
#define ENABLE_Z_BUFFER 0
#define DRAW_DEBUG_BUFFERS 0
#define DRAW_Z_BUFFER 0
#define DRAW_NORMALS 0

#define SCENE_HAS_MAPS 0

#define LIGHT_GRD_BITS 2
#define LIGHT_GRD (1 << LIGHT_GRD_BITS)
#define SPECULAR_LIGHT 0
#define LIGHT_GRD_T (LIGHT_GRD << SPECULAR_LIGHT)

#define ENABLE_TEXTURE_BUFFER 0
#define TEXTURE_BUFFER_SIZE 16384

#define MDL_PL_LIST_MAX_SIZE 10000 // max num model faces
#define MDL_VT_LIST_MAX_SIZE 10000 // max num model vertices
#define SCN_VIS_PL_LIST_SIZE 1000 // max num scene visible faces
#define SCN_VIS_VT_LIST_SIZE 1000 // max num scene visible vertices
#define MDL_VIS_VT_LIST_SIZE 256 // model max num visible vertices
#define SCN_OT_SIZE 256 // 256
#define MDL_OT_SIZE 16
#define NUM_SPRITES 32 // num unique sprites
#define NUM_VT_SPRITES (NUM_SPRITES * 2)