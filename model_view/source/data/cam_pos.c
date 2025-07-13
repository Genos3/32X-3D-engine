#include "common.h"

void set_cam_pos() {
  cam.pos.x = fix(0);
  cam.pos.z = fix(5);
  cam.pos.y = fix(1);
  cam.rot.x = 0;
  cam.rot.y = 128 << 8;
}