#include "sdl_platform.cpp"

#include "imgui_impl_sdl_gl3.cpp"

#define JOBTHIEF_IMPLEMENTATION
#include "jobthief.h"

#ifdef RENDERER_OPENGL
#include "renderer_opengl.cpp"
#endif
