#ifndef GAME_H__
#define GAME_H__

#include "platform.h"

struct MeatSpace;

struct GameState {
  b32 initialized;

  MeatSpace* TMP_meat_space;
};

struct AssetManager;
struct TransientState {
  AssetManager* asset_manager;
};

#endif /* end of include guard: GAME_H__ */
