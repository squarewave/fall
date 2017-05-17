#ifndef GAME_H__
#define GAME_H__

#include "platform.h"

struct GameState {
  b32 initialized;
  PlatformEntireFile dummy_file_result;
  PlatformAsyncFileHandle dummy_file_handle;
  char file_name_input[64];
};

#endif /* end of include guard: GAME_H__ */
