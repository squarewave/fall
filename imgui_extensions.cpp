#include "platform.h"
#include "imgui_extensions.h"

IMGUI_API void ImGui::Image(PlatformTexture texture) {
  ImGui::Image(texture.handle, ImVec2(texture.width, texture.height));
}
