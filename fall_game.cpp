#include "game.cpp"
#include "render_commands.cpp"
#include "asset_manager.cpp"
#include "meat_space.cpp"

#ifdef FALL_INTERNAL
#include "debug.cpp"
#endif

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

#include "imgui_extensions.cpp"
#include "imgui/imgui.cpp"
#include "imgui/imgui_draw.cpp"
