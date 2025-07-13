#include "../../shared/source/defines.h"
#include "../../shared/source/structs.h"
#include "../../shared/source/engine_defines.h"

extern const model_t model_0;
extern const textures_t textures_0;
extern const model_t model_cube;

const void *model_list[1];
const textures_t *textures_list[1];

void init_model_list() {
  model_list[0] = &model_0;
  textures_list[0] = &textures_0;
}