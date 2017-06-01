#ifndef GEOMETRY_H__
#define GEOMETRY_H__

#include "platform.h"

struct vec2 {
  f32 x, y;
};

struct ivec2 {
  i32 x, y;
};

struct rect2 {
  vec2 bottom_left;
  vec2 top_right;
};

b32 is_in_rect(vec2 v, rect2 rect) {
  return v.x > rect.bottom_left.x && v.x < rect.top_right.x &&
         v.y > rect.bottom_left.y && v.y < rect.top_right.y;
}

#endif /* end of include guard: GEOMETRY_H__ */
