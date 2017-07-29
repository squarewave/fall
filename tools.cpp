#include "platform.h"
#include "typeinfo.h"
#include "game.h"
#include "render_commands.h"
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

          f32 left = entity->p.x - 0.5;
          f32 right = entity->p.x + 0.5;
          f32 bottom = entity->p.y - 0.5;
          f32 top = entity->p.y + 0.5;
          f32 z = entity->p.y + entity->z_bias - 0.1f;
          u32 color = 0xffffaaff;
          push_quad_frame(g_render_commands,
                          left, bottom, z, color,
                          left, top, z, color,
                          right, top, z, color,
                          right, bottom, z, color);
        }
      }
    }
    PopID();
  }
  End();
}