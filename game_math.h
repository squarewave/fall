#ifndef GAME_MATH_H__
#define GAME_MATH_H__

#include "platform.h"

inline i32 _abs(i32 x) {
  return x < 0 ? -x : x;
}

inline i32 _floor(f32 x) {
  i32 i = (i32)x;
  return i - (i > x);
}

inline i32 _ceil(f32 x) {
  i32 i = (i32)x;
  return i + (i < x);
}

inline i32 _round(f32 x) {
  return _floor(x + 0.5f);
}

inline i32 _clamp(i32 x, i32 minimum, i32 maximum) {
  return x < minimum ? minimum : x > maximum ? maximum : x;
}

inline f32 _clamp(f32 x, f32 minimum, f32 maximum) {
  return x < minimum ? minimum : x > maximum ? maximum : x;
}

#endif /* end of include guard: GAME_MATH_H__ */
