#ifndef MEMORY_H__
#define MEMORY_H__

#include <assert.h>
#include <stdio.h>
#include <stdarg.h>

#include "platform.h"
#include "game.h"

#define MEMORY_SENTRY 0xDEADBEEF

inline size_t game_memory_used() {
  char* base = (char*)(g_game_state + 1);
  i64* tip = (i64*)base;
  char* result = *tip + sizeof(*tip) + base;
  return (size_t)result - (size_t)g_game_state;
}

#define game_alloc(type) (type*)game_alloc_(sizeof(type))
#define game_alloc_array(type, count) (type*)game_alloc_(sizeof(type) * (count))
inline void* game_alloc_(size_t size) {
  if (!size) {
    return NULL;
  }
  char* base = (char*)(g_game_state + 1);
  i64* tip = (i64*)base;

  char* result = *tip + sizeof(*tip) + base;
  assert((size_t)result - (size_t)g_game_state < GAME_MEMORY_SIZE);
  *tip += size;

#if FALL_INTERNAL
  *(u32*)(result + size) = MEMORY_SENTRY;
  assert((i64)result - (i64)base == sizeof(*tip) || *(u32*)(result - sizeof(u32)) == MEMORY_SENTRY);
  *tip += sizeof(u32);
#endif

  return result;
}

inline size_t transient_memory_used() {
  char* base = (char*)(g_transient_state + 1);
  i64* tip = (i64*)base;
  char* result = *tip + sizeof(*tip) + base;
  return (size_t)result - (size_t)g_transient_state;
}


#define transient_alloc(type) (type*)transient_alloc_(sizeof(type))
#define transient_alloc_array(type, count) (type*)transient_alloc_(sizeof(type) * (count))
#define transient_realloc_array(base, old_count, new_count) (decltype(base))transient_realloc_((base), sizeof(*(base)) * (old_count), sizeof(*(base)) * (new_count))
inline void* transient_alloc_(size_t size) {
  char* base = (char*)(g_transient_state + 1);
  i64* tip = (i64*)base;

  char* result = *tip + sizeof(*tip) + base;
  assert((size_t)result - (size_t)g_transient_state < TRANSIENT_MEMORY_SIZE);
  *tip += size;

#if FALL_INTERNAL
  *(u32*)(result + size) = MEMORY_SENTRY;
  assert((i64)result - (i64)base == sizeof(*tip) || *(u32*)(result - sizeof(u32)) == MEMORY_SENTRY);
  *tip += sizeof(u32);
#endif

  return result;
}


inline void* transient_realloc_(void* base, size_t old_size, size_t new_size) {
  void* result;
#if FALL_INTERNAL
  if ((char*)base + old_size + sizeof(u32) == (char*)(g_transient_state + 1)) {
    assert(new_size - old_size >= sizeof(u32));
    transient_alloc_(new_size - old_size - sizeof(u32));
#else
  if ((char*)base + old_size == (char*)(g_transient_state + 1)) {
    transient_alloc_(new_size - old_size);
#endif
    result = base;
  } else {
    // discard the old memory - it will be cleaned up next frame
    auto new_base = transient_alloc_(new_size);
    memcpy(new_base, base, old_size);
    result = new_base;
  }
  return result;
}

#define stretchy_buffer_init() (transient_alloc_(0))
#define stretchy_buffer_push(val) stretchy_buffer_push_(sizeof(val), &(val))
inline void* stretchy_buffer_push_(size_t size, void* val) {
  char* base = (char*)(g_transient_state + 1);
  i64* tip = (i64*)base;

#if FALL_INTERNAL
  char* result = *tip + sizeof(*tip) + base - sizeof(u32);
  *(u32*)(result + size) = MEMORY_SENTRY;
  assert(*(u32*)result == MEMORY_SENTRY);
  *tip += size;
#else
  char* result = *tip + sizeof(*tip) + base;
  assert((i64)result - (i64)base < MAX_TRANSIENT_MEMORY);
  *tip += size;
#endif

  memcpy(result, val, size);
  return result;
}

inline void reset_transient_memory() {
  char* base = (char*)(g_transient_state + 1);
  i64* tip = (i64*)base;
  *tip = 0;
}

char* tprintf(const char* fmt, ...)
{
  va_list args;
  va_start(args, fmt);

  int size = snprintf(NULL, 0, "%d", 132);
  char * a = transient_alloc_array(char, size + 1);
  sprintf(a, "%d", 132);

  va_end(args);

  return a;
}

#endif /* end of include guard: MEMORY_H__ */
