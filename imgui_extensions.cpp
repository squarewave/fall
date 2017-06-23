#include "platform.h"
#include "assets.h"
#include "imgui_extensions.h"

IMGUI_API void ImGui::Image(PlatformTexture texture, float scale) {
  Image(texture.handle, ImVec2(texture.width * scale, texture.height * scale));
}

IMGUI_API void ImGui::Image(TextureAsset texture, float scale) {
  Image(texture.handle,
        ImVec2(texture.px_width * 4 * scale, texture.px_height * 4 * scale),
        ImVec2(texture.left, texture.top),
        ImVec2(texture.right, texture.bottom));
}
