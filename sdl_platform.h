#ifndef SDL_PLATFORM_H__
#define SDL_PLATFORM_H__

#include "SDL2/SDL.h"

#include "platform.h"

const i32 START_WIDTH = 1920;
const i32 START_HEIGHT = 1080;
const i32 FRAME_RATE = 60;

// TODO(doug): ensure this isn't used from user-facing code
#define FILEPATH_SIZE MAX_PATH

struct PlatformContext {
    SDL_GLContext gl_context;
    SDL_Renderer* renderer;
    SDL_Window* window;
    SDL_Texture* texture;
    SDL_GameController* controller_handle;

    PlatformInput* next_input;
    PlatformInput* prev_input;

    char exe_filepath[FILEPATH_SIZE];
};

struct PlatformAsyncFileHandle {
  SDL_atomic_t ready;
  PlatformEntireFile file;
};

struct PlatformFileIORequest {
  char* filename;
  PlatformAsyncFileHandle* result;
};

struct GameCode {
#ifdef PLATFORM_WINDOWS
  FILETIME last_write_time;
  HMODULE dll;
#endif

  b32 is_valid;

#ifdef FALL_INTERNAL
  GameUpdateAndRender* update_and_render;
  GameImguiGetIO* imgui_get_io;
  GameImguiNewFrame* imgui_new_frame;
  GameImguiShutdown* imgui_shutdown;
  GameImguiRender* imgui_render;
  GameImguiGetTexDataAsRGBA32* imgui_get_tex_data_as_rgba32;
  GameDebugEndFrame* debug_end_frame;
#endif
};

extern GameCode g_game_code;

#endif /* end of include guard: SDL_PLATFORM_H__ */
