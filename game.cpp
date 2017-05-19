#include "platform.h"
#include "game.h"

#include "stb/stb_image.h"

void game_update_and_render() {
  static i32 counter = 0;
  if (!g_game_state->initialized) {
    g_game_state->initialized = true;

    g_platform.begin_read_entire_file("assets/img_test.png", &g_game_state->dummy_file_handle);
  }

  if (ImGui::InputText("", g_game_state->file_name_input, 64)) {
    ImGui::Text("Hello, world!");
  }

  if (g_platform.file_io_complete(&g_game_state->dummy_file_handle)) {
    g_game_state->dummy_file_result = g_platform.entire_file_result(&g_game_state->dummy_file_handle);

    PlatformEntireFile file = g_game_state->dummy_file_result;
    i32 x,y,n;
    char *data = (char*)stbi_load_from_memory(file.contents, file.content_size, &x, &y, &n, 0);
    g_platform.register_texture(data, x, y, n, &g_game_state->dummy_texture_handle);

    g_game_state->dummy_image_loaded = true;
    stbi_image_free(data);
  }

  if (g_game_state->dummy_image_loaded) {
    void* native_handle = g_platform.get_texture_native_handle(&g_game_state->dummy_texture_handle);
    ImGui::Image(native_handle, ImVec2(128,128));
  }

  if (g_game_state->dummy_file_result.contents) {
    ImGui::Text((const char *)g_game_state->dummy_file_result.contents);
  }
  ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
}
