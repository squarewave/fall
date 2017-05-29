#include "platform.h"
#include "imgui/imgui.h"

char debug_print_copy_buffer[DEBUG_PRINT_RING_BUFFER_SIZE + 1] = {0};

void show_debug_log() {
  ImGui::SetNextWindowSize(ImVec2(200,100), ImGuiSetCond_FirstUseEver);
  if (ImGui::Begin("Output")) {
    if (*g_debug_print_ring_buffer_write_head < DEBUG_PRINT_RING_BUFFER_SIZE) {
      ImGui::Text("%s", (const char*)g_debug_print_ring_buffer);
    } else {
      i32 write_head = *g_debug_print_ring_buffer_write_head & DEBUG_PRINT_RING_BUFFER_MASK;
      strcpy(debug_print_copy_buffer, g_debug_print_ring_buffer + write_head + 1);
      strcpy(debug_print_copy_buffer + DEBUG_PRINT_RING_BUFFER_SIZE - write_head - 1, g_debug_print_ring_buffer);
      char* end_of_first_line = strchr(debug_print_copy_buffer, '\n');
      if (end_of_first_line) {
          ImGui::Text("%s", (const char*)end_of_first_line + 1);
      } else {
          ImGui::Text("%s", (const char*)debug_print_copy_buffer);
      }
    }
  }
  ImGui::End();
}
