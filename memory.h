#ifndef MEMORY_H__
#define MEMORY_H__

#include <assert.h>
#include <stdio.h>
#include <stdarg.h>

#include "platform.h"

#define MEMORY_SENTRY 0xDEADBEEF

reflectable struct Allocator {
  size_t capacity;
  size_t high_water;
  size_t used;
};

#define allocator_alloc(a, type) (type*)allocator_alloc_(a, sizeof(type))
#define allocator_alloc_array(a, type, count) (type*)allocator_alloc_(a, sizeof(type) * count)

inline size_t allocator_used(Allocator a) {
  return a.used;
}

inline void* allocator_alloc_(Allocator* a, size_t size, b32 allow_zero_size = false) {
  if (!size && !allow_zero_size) {
    return NULL;
  }
  if (a->used + size > a->capacity) {
    assert(false);
    return NULL;
  }
  char* base = (char*)(a + 1);

  char* result = base + a->used;
  a->used += size;

#if FALL_INTERNAL
  *(u32*)(result + size) = MEMORY_SENTRY;
  assert(a->used == size || *(u32*)(result - sizeof(u32)) == MEMORY_SENTRY);
  a->used += sizeof(u32);
#endif

  if (a->high_water < a->used) {
    a->high_water = a->used;
  }

  return result;
}

inline void reset_allocator(Allocator* a) {
  a->used = 0;
}

#define stretchy_buffer_init(a) (allocator_alloc_(a, 0, true))
#define stretchy_buffer_push(a, val) stretchy_buffer_push_(a, sizeof(val), &(val))
inline void* stretchy_buffer_grow_(Allocator* a, size_t size) {
  char* base = (char*)(a + 1);

  if (a->used + size > a->capacity) {
    assert(false);
    return NULL;
  }

#if FALL_INTERNAL
  char* result = a->used + base - sizeof(u32);
  if  (size) {
    *(u32*)(result + size) = MEMORY_SENTRY;
    assert(*(u32*)result == MEMORY_SENTRY);
    a->used += size;
  }
#else
  char* result = a->used + base;
  a->used += size;
#endif

  if (a->high_water < a->used) {
    a->high_water = a->used;
  }

  return result;
}

inline void* stretchy_buffer_tip(Allocator* a) {
  return stretchy_buffer_grow_(a, 0);
}

inline void* stretchy_buffer_push_(Allocator* a, size_t size, void* val) {
  auto result = stretchy_buffer_grow_(a, size);
  memcpy(result, val, size);
  return result;
}

char* sbprintf(Allocator* a, const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);

  int size = vsnprintf(NULL, 0, "%s", args);
  size_t aligned_size = size + 8;
  aligned_size &= ~0b0111;
  aligned_size |= 0b1000;
  char * result = (char*)stretchy_buffer_grow_(a, aligned_size);
  vsnprintf(result, aligned_size, "%s", args);

  va_end(args);

  return result;
}

#endif /* end of include guard: MEMORY_H__ */
