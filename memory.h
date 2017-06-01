#ifndef MEMORY_H__
#define MEMORY_H__

#include <assert.h>

#include "platform.h"
#include "game.h"

#define MAX_GAME_MEMORY 1024 * 1024 * 512
#define MAX_TRANSIENT_MEMORY 1024 * 1024 * 512

#define MEMORY_SENTRY 0xDEADBEEF

#define game_alloc(type) (type*)game_alloc_(sizeof(type))
#define game_alloc_array(type, count) (type*)game_alloc_(sizeof(type) * (count))
inline void* game_alloc_(size_t size) {
  char* base = (char*)(g_game_state + 1);
  i64* tip = (i64*)base;

  char* result = *tip + sizeof(*tip) + base;
  assert((i64)result - (i64)base < MAX_GAME_MEMORY);
  *tip += size;

#if FALL_INTERNAL
  *(u32*)(result + size) = MEMORY_SENTRY;
  assert((i64)result - (i64)base == sizeof(*tip) || *(u32*)(result - sizeof(u32)) == MEMORY_SENTRY);
  *tip += sizeof(u32);
#endif

  return result;
}

#define transient_alloc(type) (type*)transient_alloc_(sizeof(type))
#define transient_alloc_array(type, count) (type*)transient_alloc_(sizeof(type) * (count))
inline void* transient_alloc_(size_t size) {
  char* base = (char*)(g_transient_state + 1);
  i64* tip = (i64*)base;

  char* result = *tip + sizeof(*tip) + base;
  assert((i64)result - (i64)base < MAX_TRANSIENT_MEMORY);
  *tip += size;

#if FALL_INTERNAL
  *(u32*)(result + size) = MEMORY_SENTRY;
  assert((i64)result - (i64)base == sizeof(*tip) || *(u32*)(result - sizeof(u32)) == MEMORY_SENTRY);
  *tip += sizeof(u32);
#endif

  return result;
}

#endif /* end of include guard: MEMORY_H__ */
