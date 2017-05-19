#include "platform.h"
#include "game.h"

void game_update_and_render() {
  static i32 counter = 0;
  if (!g_game_state->initialized) {
    g_game_state->initialized = true;
  }

  if (ImGui::InputText("", g_game_state->file_name_input, 64)) {
    ImGui::Text("Hello, world!");
    g_platform.begin_read_entire_file(g_game_state->file_name_input, &g_game_state->dummy_file_handle);
  }

  if (g_platform.file_io_complete(&g_game_state->dummy_file_handle)) {
    g_game_state->dummy_file_result = g_platform.entire_file_result(&g_game_state->dummy_file_handle);
  }

  if (g_game_state->dummy_file_result.contents) {
    ImGui::Text((const char *)g_game_state->dummy_file_result.contents);
  }
  ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
}
