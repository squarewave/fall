#pragma once
#include "imgui/imgui.h"

#include "platform.h"
#include "assets.h"

namespace ImGui {
  IMGUI_API void Image(PlatformTexture texture);
  IMGUI_API void Image(TextureAsset texture);
}
