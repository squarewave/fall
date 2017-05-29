#include "assert.h"

#include "imgui/imgui.h"
#include "stb/stb_image.h"

#include "platform.h"
#include "game.h"
#include "imgui_extensions.h"
#include "render_commands.h"
#include "imgui_memory_editor.h"
#include "assets.h"
#include "asset_manager.h"

void game_update_and_render() {
  if (!g_game_state->initialized) {
    assets_init();
    g_game_state->initialized = true;
    set_camera_position(&g_render_commands, 0.0f, 0.0f);
    set_camera_scale(&g_render_commands, 320.0f, 180.0f);
  } else {
    assets_refresh();
  }

  AssetAttributes attrs = {};
  attrs.asset_class = AssetClass_science;
  attrs.color = AssetColor_dark;
  attrs.direction = AssetDirection_forward;
  attrs.move_state = AssetMoveState_standing;
  attrs.tracking_id = 1;
  // TextureAsset tex = assets_get_atlas(AssetType_crew);
  TextureAsset tex = assets_get_texture(AssetType_crew, attrs);
  push_textured_quad(&g_render_commands,
                     tex.handle,
                     tex.left, tex.bottom, 0.0f, 0.0f, 0xffffffff,
                     tex.right, tex.bottom, tex.px_width, 0.0f, 0xffffffff,
                     tex.left, tex.top, 0.0f, tex.px_height, 0xffffffff,
                     tex.right, tex.top, tex.px_width, tex.px_height, 0xffffffff);

  if (ImGui::InputText("", g_game_state->file_name_input, 64)) {
    ImGui::Text("Hello, world!");
  }

  ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
}
