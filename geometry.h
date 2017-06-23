#ifndef GEOMETRY_H__
#define GEOMETRY_H__

#include "platform.h"
#include <math.h>

#define PI 3.14159265358979323846

reflectable struct vec2 {
  f32 x;
  f32 y;
};

reflectable struct ivec2 {
  i32 x;
  i32 y;
};

reflectable struct vec3 {
  f32 x;
  f32 y;
  f32 z;
};

reflectable struct ivec3 {
  i32 x;
  i32 y;
  i32 z;
};

reflectable struct rect2 {
  vec2 bottom_left;
  vec2 top_right;
};

inline f32 lerp(f32 a, f32 b, f32 x) {
  assert(x >= 0.0f && x <= 1.0f);
  return a + x * (b - a);
}

inline vec2 operator+(const vec2& lhs, const vec2& rhs) {
  return vec2{ lhs.x + rhs.x, lhs.y + rhs.y };
}

inline vec2 operator-(const vec2& lhs, const vec2& rhs) {
  return vec2{ lhs.x - rhs.x, lhs.y - rhs.y };
}

inline vec2 operator*(const vec2& lhs, const f32 rhs) {
  return vec2{ lhs.x * rhs, lhs.y * rhs };
}

inline b32 operator==(const vec2& lhs, const vec2& rhs) {
  return lhs.x == rhs.x && lhs.y == rhs.y;
}

inline f32 magnitude_sq(vec2 v) {
  return v.x * v.x + v.y * v.y;
}

inline f32 magnitude(vec2 v) {
  return sqrtf(magnitude_sq(v));
}

inline vec2 normalize(vec2 v) {
  return v * (1.0f / magnitude(v));
}

inline f32 dot(vec2 a, vec2 b) {
  return a.x * b.x + a.y * b.y;
}

inline vec2 move_toward(vec2 source, vec2 target, f32 distance, f32* remaining = NULL) {
  vec2 dp = target - source;
  f32 mag = magnitude(dp);
  if (remaining) {
    *remaining = distance - mag;
  }
  if (mag < distance) {
    return target;
  } else {
    return source + dp * (distance / mag);
  }
}

inline f32 dot(vec3 a, vec3 b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline rect2 operator+(const rect2& lhs, const vec2& rhs) {
  return rect2 {lhs.bottom_left + rhs, lhs.top_right + rhs};
}

inline b32 is_in_rect(vec2 p, rect2 rect) {
  return p.x > rect.bottom_left.x && p.x < rect.top_right.x &&
         p.y > rect.bottom_left.y && p.y < rect.top_right.y;
}

#endif /* end of include guard: GEOMETRY_H__ */
