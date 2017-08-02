#pragma once

#include "platform.h"

reflectable struct EntityEditor {
  i32 template_being_edited;
  f32 zoom_level;
  b32 initialized;
};

void entity_editor_update_and_render();