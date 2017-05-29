#include <assert.h>

#include "imgui/imgui.h"
#include "stb/stb_image.h"

#include "assets.h"
#include "asset_manager.h"
#include "debug.h"
#include "game.h"
#include "imgui_extensions.h"
#include "imgui_memory_editor.h"
#include "platform.h"
#include "render_commands.h"

GameState* g_game_state;
TransientState* g_transient_state;
PlatformInput* g_input;
PlatformServices g_platform;
RenderCommands* g_render_commands;
char* g_debug_print_ring_buffer;
i32* g_debug_print_ring_buffer_write_head;

GAME_UPDATE_AND_RENDER(game_update_and_render) {
  g_game_state = memory->game_state;
  g_transient_state = memory->transient_state;
  g_input = memory->input;
  g_platform = memory->platform;
  g_render_commands = memory->render_commands;
  g_debug_print_ring_buffer = memory->debug_print_ring_buffer;
  g_debug_print_ring_buffer_write_head = memory->debug_print_ring_buffer_write_head;

  if (!g_game_state->initialized) {
    assets_init();
    g_game_state->initialized = true;
    set_camera_position(g_render_commands, 0.0f, 0.0f);
    set_camera_scale(g_render_commands, 320.0f, 180.0f);
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
  push_textured_quad(g_render_commands,
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

#ifdef FALL_INTERNAL
GAME_DEBUG_END_FRAME(game_debug_end_frame) {
    show_debug_log();
}

GAME_IMGUI_GET_IO(game_imgui_get_io) {
  return ImGui::GetIO();
}

GAME_IMGUI_NEW_FRAME(game_imgui_new_frame) {
  ImGui::NewFrame();
}

GAME_IMGUI_SHUTDOWN(game_imgui_shutdown) {
  ImGui::Shutdown();
}

GAME_IMGUI_RENDER(game_imgui_render) {
  ImGui::Render();
}

GAME_IMGUI_GET_TEX_DATA_AS_RGBA32(game_imgui_get_tex_data_as_rgba32) {
  ImGui::GetIO().Fonts->GetTexDataAsRGBA32(pixels, width, height);
}
#endif

