#include "platform.h"
#include "typeinfo.h"
#include "game.h"
#include "render_commands.h"
#include "imgui/imgui.h"

using namespace ImGui;

void update_and_render_tools() {
  Begin("Tools");
  for (i32 i = 0; i < g_game_state->meat_space_entity_templates.templates_count; i++) {
    PushID(i);
    auto t = g_game_state->meat_space_entity_templates.templates + i;
    Text(enum_member_name(MeatSpaceEntityTemplateId, t->id));
    PopID();
  }
  End();
}