#include <assert.h>

#include "imgui/imgui.h"
#include "stb/stb_image.h"

#include "assets.h"
#include "asset_manager.h"
#include "debug.h"
#include "game.h"
#include "meat_space.h"
#include "memory.h"
#include "platform.h"
#include "render_commands.h"

#ifdef FALL_INTERNAL
#include "imgui_extensions.h"
#include "imgui_memory_editor.h"
#endif

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

    auto meat_space = g_game_state->TMP_meat_space = game_alloc(MeatSpace);

    {
      MeatSpaceEntity crew;
      crew.p = vec2 {0.0f, 0.0f};
      crew.id = 1;
      crew.asset_type = AssetType_crew;
      crew.asset_attributes.asset_class = AssetClass_science;
      crew.asset_attributes.color = AssetColor_dark;
      crew.asset_attributes.direction = AssetDirection_forward;
      crew.asset_attributes.move_state = AssetMoveState_standing;
      crew.asset_attributes.tracking_id = crew.id;
      crew.selected = false;
      meat_space->entities[meat_space->entities_count++] = crew;
    }

    {
      MeatSpaceEntity crew;
      crew.p = vec2 {32.0f, 0.0f};
      crew.id = 2;
      crew.asset_type = AssetType_crew;
      crew.asset_attributes.asset_class = AssetClass_science;
      crew.asset_attributes.color = AssetColor_dark;
      crew.asset_attributes.direction = AssetDirection_forward;
      crew.asset_attributes.move_state = AssetMoveState_standing;
      crew.asset_attributes.tracking_id = crew.id;
      crew.selected = false;
      meat_space->entities[meat_space->entities_count++] = crew;
    }

    {
      MeatSpaceBrain player = {};
      player.type = MeatSpaceBrainType_player;
      player.player.selected_entities = game_alloc_array(i64, MAX_SELECTED_ENTITIES);
      meat_space->brains[meat_space->brains_count++] = player;

      meat_space->camera.position = vec2 {0.0f, 0.0f};
      meat_space->camera.scale = vec2 {1920.0f / 4.0f, 1080.0f / 4.0f};
      meat_space->camera.viewport_left = 0.0f;
      meat_space->camera.viewport_right = g_render_commands->screen_width;
      meat_space->camera.viewport_bottom = 0.0f;
      meat_space->camera.viewport_top = g_render_commands->screen_height;
    }

    g_game_state->initialized = true;
  } else {
    assets_refresh();
  }

  meat_space_update_and_render(g_game_state->TMP_meat_space);
}

#ifdef FALL_INTERNAL
GAME_DEBUG_END_FRAME(game_debug_end_frame) {
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
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

