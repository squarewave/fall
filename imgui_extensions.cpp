#include "platform.h"
#include "assets.h"
#include "imgui_extensions.h"

IMGUI_API void ImGui::Image(PlatformTexture texture) {
  Image(texture.handle, ImVec2(texture.width, texture.height));
}

IMGUI_API void ImGui::Image(TextureAsset texture) {
  Image(texture.handle,
        ImVec2(texture.px_width * 4, texture.px_height * 4),
        ImVec2(texture.left, texture.top),
        ImVec2(texture.right, texture.bottom));
}
