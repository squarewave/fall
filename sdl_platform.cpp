#pragma comment(lib, "SDL2main.lib")
#pragma comment(lib, "SDL2.lib")

#include <assert.h>
#include <intrin.h>
#include <locale.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "gl/glew.h"
#include "gl/glu.h"
#include "SDL2/SDL.h"
#include <Windows.h>

#include "platform.h"
#include "sdl_platform.h"
#include "sdl_platform_input.h"

GameState* g_game_state;
TransientState* g_transient_state;
PlatformInput* g_platform_input;
PlatformServices g_platform_services;
PlatformRenderSettings g_platform_render_settings;

void exit_gracefully(int error_code) {
    exit(error_code);
}

DEBUG_PLATFORM_FREE_FILE_MEMORY(DEBUG_platform_free_file_memory) {
    free(memory);
}

DEBUG_PLATFORM_READ_ENTIRE_FILE(DEBUG_platform_read_entire_file) {
    PlatformEntireFile result = {};

    FILE *f = fopen(filename, "rb");

    if (f == 0) {
        printf("Failed to open file %s\n", filename);
        return result;
    }

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *string = (char *)malloc(fsize + 1);
    if (!fread(string, fsize, 1, f)) {
        printf("No results from fread\n");
    }
    fclose(f);

    string[fsize] = 0;

    result.contents = (u8 *)string;
    result.content_size = (u32) fsize;

    return result;
}

DEBUG_PLATFORM_WRITE_ENTIRE_FILE(DEBUG_platform_write_entire_file) {
    assert(false);
    return 0;
}

f32 get_seconds_elapsed(u64 old, u64 current) {
    return ((f32)(current - old) / (f32)(SDL_GetPerformanceFrequency()));
}

void check_sdl_error(int line = -1) {
    const char *error = SDL_GetError();
    if (*error != '\0')
    {
        printf("SDL Error: %s\n", error);
        if (line != -1)
            printf(" + line: %i\n", line);
        SDL_ClearError();
    }
}

#ifdef PLATFORM_WINDOWS
int CALLBACK WinMain(
  _In_ HINSTANCE hInstance,
  _In_ HINSTANCE hPrevInstance,
  _In_ LPSTR     lpCmdLine,
  _In_ int       nCmdShow
)
#else
#error
#endif
{
    setlocale(LC_NUMERIC, "");

    PlatformContext context = {0};
    g_platform_services.DEBUG_platform_read_entire_file = DEBUG_platform_read_entire_file;
    g_platform_services.DEBUG_platform_write_entire_file = DEBUG_platform_write_entire_file;
    g_platform_services.DEBUG_platform_free_file_memory = DEBUG_platform_free_file_memory;

    if (SDL_Init(SDL_INIT_EVERYTHING)) {
        printf("Unable to init SDL: %s\n", SDL_GetError());
        exit_gracefully(1);
    }

    check_sdl_error(__LINE__);


    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
    // SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    context.window = SDL_CreateWindow("Fall",
                                      SDL_WINDOWPOS_UNDEFINED,
                                      SDL_WINDOWPOS_UNDEFINED,
                                      START_WIDTH,
                                      START_HEIGHT,
                                      SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
    check_sdl_error(__LINE__);

    context.gl_context = SDL_GL_CreateContext(context.window);
    check_sdl_error(__LINE__);

    SDL_GL_MakeCurrent(context.window, context.gl_context);
    check_sdl_error(__LINE__);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        OutputDebugString("Error initializing GLEW");
        exit_gracefully(1);
    }

    if(SDL_GL_SetSwapInterval(1) < 0){
        check_sdl_error(__LINE__);
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glLineWidth(2.0f);

    glClearColor(0.784f, 0.8745f, 0.925f, 1.0f);

    size_t pixel_bytes = START_WIDTH * START_HEIGHT * 4;
    void* pixels = malloc(pixel_bytes);
    // void* pixels = aligned_alloc(64, pixel_bytes + (pixel_bytes % 64));

    const u64 one_gig = 1024LL * 1024LL * 1024LL;
    g_game_state = (GameState*)calloc(one_gig / 2, sizeof(u8));
    g_transient_state = (TransientState*)calloc(one_gig / 2, sizeof(u8));

    context.controller_handle = find_controller_handle();

    u32 frame_index = 0;
    u32 s = 0;
    bool running = true;

    PlatformInput prev_input = {};
    PlatformInput next_input = {};
    context.next_input = &next_input;
    context.prev_input = &prev_input;

    u64 last_counter = SDL_GetPerformanceCounter();
    const f32 target_seconds_per_frame = 1.0f / (f32)FRAME_RATE;
    while(running) {
        // TIMED_BLOCK(allotted_time);

        f32 elapsed = get_seconds_elapsed(last_counter, SDL_GetPerformanceCounter());
        if (elapsed < target_seconds_per_frame) {
            u32 sleep_time = ((target_seconds_per_frame - elapsed) * 1000) - 1;
            SDL_Delay(sleep_time);
            elapsed = get_seconds_elapsed(last_counter, SDL_GetPerformanceCounter());
            while (get_seconds_elapsed(last_counter, SDL_GetPerformanceCounter()) <
                   target_seconds_per_frame) { }
        }

        // TIMED_BLOCK(main_run_loop);

        f32 dt = get_seconds_elapsed(last_counter, SDL_GetPerformanceCounter());

        last_counter = SDL_GetPerformanceCounter();

        ZERO_STRUCT(next_input);

        for (int i = 0; i < ARRAY_SIZE(next_input.buttons); ++i) {
            next_input.buttons[i].ended_down = prev_input.buttons[i].ended_down;
        }

        for (int i = 0; i < ARRAY_SIZE(next_input.keyboard.buttons); ++i) {
            next_input.keyboard.buttons[i].ended_down = prev_input.keyboard.buttons[i].ended_down;
        }

        next_input.analog_l_trigger.value = prev_input.analog_l_trigger.value;
        next_input.analog_r_trigger.value = prev_input.analog_r_trigger.value;

        next_input.joystick_l.x = prev_input.joystick_l.x;
        next_input.joystick_r.x = prev_input.joystick_r.x;

        next_input.mouse.x = prev_input.mouse.x;
        next_input.mouse.y = prev_input.mouse.y;
        next_input.mouse.button_l.ended_down = prev_input.mouse.button_l.ended_down;
        next_input.mouse.button_r.ended_down = prev_input.mouse.button_r.ended_down;
        next_input.mouse.button_middle.ended_down = prev_input.mouse.button_middle.ended_down;

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (handle_sdl_event(&event, &context)) {
                running = false;
            }
        }

        next_input.analog_l_trigger.delta =
                next_input.analog_l_trigger.value -
                prev_input.analog_l_trigger.value;
        next_input.analog_r_trigger.delta =
                next_input.analog_r_trigger.value -
                prev_input.analog_r_trigger.value;

        next_input.joystick_r.dx =
                next_input.joystick_r.x -
                prev_input.joystick_r.x;
        next_input.joystick_r.dy =
                next_input.joystick_r.y -
                prev_input.joystick_r.y;
        next_input.joystick_l.dx =
                next_input.joystick_l.x -
                prev_input.joystick_l.x;
        next_input.joystick_l.dy =
                next_input.joystick_l.y -
                prev_input.joystick_l.y;

        g_platform_render_settings.width = START_WIDTH;
        g_platform_render_settings.height = START_HEIGHT;

        g_platform_input = &next_input;

        game_update_and_render();

        prev_input = next_input;

        SDL_GL_SwapWindow(context.window);

        // process_debug_log((tools_state_*)tools_state);
    }

    exit_gracefully(0);
}
