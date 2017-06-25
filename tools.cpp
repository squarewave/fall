#include "platform.h"
#include "typeinfo.h"
#include "game.h"
#include "imgui/imgui.h"

using namespace ImGui;

void update_and_render_tools() {
  Begin("Tools");
  for (i32 i = 0; i < g_game_state->TMP_meat_space->template_collection.templates_count; i++) {
    PushID(i);
    auto t = g_game_state->TMP_meat_space->template_collection.templates + i;
    Text(enum_member_name(MeatSpaceEntityTemplateId, t->id));
    Checkbox("Edit", (bool*)&t->editing);
    if (t->editing) {
      for (i32 j = 0; j < g_game_state->TMP_meat_space->entities_count; j++) {
        auto entity = g_game_state->TMP_meat_space->entities + j;
        if (entity->template_id == t->id) {
          draw_collision_volumes(g_game_state->TMP_meat_space, entity);
        }
      }
    }
    PopID();
  }
  End();
}