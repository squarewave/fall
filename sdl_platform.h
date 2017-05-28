#ifndef SDL_PLATFORM_H__
#define SDL_PLATFORM_H__

#include "SDL2/SDL.h"

#include "platform.h"

const i32 START_WIDTH = 1920;
const i32 START_HEIGHT = 1080;
const i32 FRAME_RATE = 60;

struct PlatformContext {
    SDL_GLContext gl_context;
    SDL_Renderer* renderer;
    SDL_Window* window;
    SDL_Texture* texture;
    SDL_GameController* controller_handle;

    PlatformInput* next_input;
    PlatformInput* prev_input;
};

struct PlatformAsyncFileHandle {
  SDL_atomic_t ready;
  PlatformEntireFile file;
};

struct PlatformFileIORequest {
  char* filename;
  PlatformAsyncFileHandle* result;
};

#endif /* end of include guard: SDL_PLATFORM_H__ */
