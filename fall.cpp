#define PLATFORM_WINDOWS
#define RENDERER_OPENGL

#include "sdl_platform.cpp"

#include "game.cpp"

#include "imgui/imgui.cpp"
#include "imgui/imgui_draw.cpp"
#include "imgui/imgui_impl_sdl_gl3.cpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"
