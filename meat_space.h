#ifndef MEAT_SPACE_H__
#define MEAT_SPACE_H__

#include "platform.h"
#include "assets.h"
#include "geometry.h"

reflectable struct MeatSpaceEntity {
  // NOTE(doug): these have been thought about
  i64 id;
  vec2 p;

  // TODO(doug): should these be stored directly in here, or
  // derived from other state?
  AssetType asset_type;
  AssetAttributes asset_attributes;

  // TODO(doug): these haven't been thought much about.
  b32 selected;
  rect2 selection_bounds;
};

reflectable enum MeatSpaceBrainType {
  MeatSpaceBrainType_none = 0,
  MeatSpaceBrainType_player,

  MeatSpaceBrainType_count
};

#define MAX_SELECTED_ENTITIES 256

reflectable struct MeatSpaceBrain_player {
  i32 selected_entities_count;
  i64* selected_entities;
  MeatSpaceEntity* mouse_down_entity;

  b32 selecting;
  vec2 selection_start;
};

reflectable struct MeatSpaceBrain {
  MeatSpaceBrainType type;
  union {
    MeatSpaceBrain_player player;
  };
};

reflectable struct MeatSpaceCamera {
  vec2 position;
  vec2 scale;

  f32 viewport_top;
  f32 viewport_bottom;
  f32 viewport_left;
  f32 viewport_right;
};

reflectable struct MeatSpace {
  MeatSpaceEntity entities[4096];
  i32 entities_count;

  MeatSpaceBrain brains[1024];
  i32 brains_count;

  MeatSpaceCamera camera;
};

void meat_space_update_and_render(MeatSpace* meat_space);

#endif /* end of include guard: MEAT_SPACE_H__ */
