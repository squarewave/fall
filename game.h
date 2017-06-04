#ifndef GAME_H__
#define GAME_H__

#include "platform.h"

const float PX_PER_PIXEL = 4.0f;

struct MeatSpace;
struct AssetManager;

reflectable struct GameState {
  b32 initialized;

  MeatSpace* TMP_meat_space;
  AssetManager* asset_manager;
};

reflectable struct TransientState {
};

#endif /* end of include guard: GAME_H__ */
