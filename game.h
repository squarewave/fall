#ifndef GAME_H__
#define GAME_H__

#include "platform.h"
#include "meat_space.h"

const float PX_PER_PIXEL = 4.0f;

struct MeatSpace;
struct AssetManager;
struct Editor;

reflectable struct GameState {
  b32 initialized;

  i64 highest_entity_id;

  MeatSpace* TMP_meat_space;
  AssetManager* asset_manager;
  f32 perlin_scale;

#ifdef FALL_INTERNAL
  Editor* editor;
  PlatformTexture dummy_texture;
  b32 show_grid;
  b32 show_occupied_spaces;
  b32 editor_mode;
  MeatSpaceEntityTemplateId pending_entity_placement_id;
  b32 draw_path_finding;
  b32 draw_collision_volumes;
  b32 step_through_frames;
  SelectionGroup pending_entity_placement_selection_group;
  u32 pending_entity_variation_number;
#endif
};

reflectable struct TransientState {
};

#endif /* end of include guard: GAME_H__ */
