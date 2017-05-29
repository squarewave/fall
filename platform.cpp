#include "sdl_platform.cpp"

#include "imgui_extensions.cpp"
#include "imgui/imgui.cpp"
#include "imgui/imgui_draw.cpp"
#include "imgui/imgui_impl_sdl_gl3.cpp"

#define JOBTHIEF_IMPLEMENTATION
#include "jobthief.h"

#ifdef RENDERER_OPENGL
#include "renderer_opengl.cpp"
#endif
