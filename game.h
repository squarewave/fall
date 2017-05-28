#ifndef GAME_H__
#define GAME_H__

#include "platform.h"

struct GameState {
  b32 initialized;
  b32 dummy_image_loaded;
  PlatformTexture dummy_texture;
  PlatformEntireFile dummy_file_result;
  void* dummy_file_handle;
  char* dummy_texture_bitmap;
  int dummy_texture_bitmap_size;
  char file_name_input[64];
};

#endif /* end of include guard: GAME_H__ */
