#ifndef GAME_MATH_H__
#define GAME_MATH_H__

#include "platform.h"

i32 round(f32 x) {
  return x > 0.0f ? (i32)(x + 0.5f) : (i32)(x - 0.5f);
}

i32 clamp(i32 x, i32 minimum, i32 maximum) {
  return x < minimum ? minimum : x > maximum ? maximum : x;
}

f32 clamp(f32 x, f32 minimum, f32 maximum) {
  return x < minimum ? minimum : x > maximum ? maximum : x;
}

#endif /* end of include guard: GAME_MATH_H__ */
