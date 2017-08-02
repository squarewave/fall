#pragma once

#include "platform.h"
#include "game.h"

reflectable struct AssetSpecEditState {
};

reflectable struct AssetEditor {
  b32 initialized;

  AssetSpec* asset_specs;
  AssetSpecEditState* asset_spec_edit_state;
  int asset_specs_count;
  f32 zoom_level;

  int asset_being_edited;
  vec2 dragging_start;
  b32 dragging;
};

void asset_editor_update_and_render();