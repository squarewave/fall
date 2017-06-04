#ifndef GEOMETRY_H__
#define GEOMETRY_H__

#include "platform.h"

reflectable struct vec2 {
  f32 x;
  f32 y;
};

reflectable struct ivec2 {
  i32 x;
  i32 y;
};

reflectable struct rect2 {
  vec2 bottom_left;
  vec2 top_right;
};

inline vec2 operator+(const vec2& lhs, const vec2& rhs) {
  return vec2 {lhs.x + rhs.x, lhs.y + rhs.y};
}

inline rect2 operator+(const rect2& lhs, const vec2& rhs) {
  return rect2 {lhs.bottom_left + rhs, lhs.top_right + rhs};
}

inline b32 is_in_rect(vec2 p, rect2 rect) {
  return p.x > rect.bottom_left.x && p.x < rect.top_right.x &&
         p.y > rect.bottom_left.y && p.y < rect.top_right.y;
}

#endif /* end of include guard: GEOMETRY_H__ */
