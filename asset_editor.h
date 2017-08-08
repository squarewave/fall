#pragma once

#include "platform.h"
#include "game.h"

reflectable struct AssetSpecEditState {
};

reflectable struct AssetEditor {
  b32 initialized;

  reflect_member(array) AssetSpec* asset_specs;
  i32 asset_specs_count;

  AssetSpecEditState* asset_spec_edit_state;
  f32 zoom_level;

  int asset_being_edited;
  vec2 dragging_start;
  b32 dragging;
};

void asset_editor_update_and_render();