#include "common.h"

void init_sky() {
  // extract the channels
  #ifndef PC
    sky.start_color_ch.b = (sky.start_color >> 10) & 31;
  #else
    sky.start_color_ch.b = sky.start_color >> 10;
  #endif
  sky.start_color_ch.g = (sky.start_color >> 5) & 31;
  sky.start_color_ch.r = sky.start_color & 31;
  
  #ifndef PC
    sky.end_color_ch.b = (sky.end_color >> 10) & 31;
  #else
    sky.end_color_ch.b = sky.end_color >> 10;
  #endif
  sky.end_color_ch.g = (sky.end_color >> 5) & 31;
  sky.end_color_ch.r = sky.end_color & 31;
}

void set_obj_pos(object_t *obj, const model_t *model) {
  obj->pos.x = model->origin.x;
  obj->pos.y = model->origin.y;
  obj->pos.z = model->origin.z;
}