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
#ifdef PLATFORM_WINDOWS
#include <Windows.h>
#endif

#ifdef RENDERER_OPENGL
#include "libs/include/GL/glew.h"
#include "GL/glu.h"
#include "GL/gl.h"
#endif

#include "libs/include/SDL2/SDL.h"
#include "imgui/imgui.h"
#include "imgui_impl_sdl_gl3.h"
#include "jobthief.h"

#include "platform.h"
#include "sdl_platform.h"
#include "sdl_platform_input.h"
#include "render_commands.h"
#include "debug.h"

#ifdef RENDERER_OPENGL
#include "renderer_opengl.h"
#endif

GameState* g_game_state;
TransientState* g_transient_state;
PlatformInput* g_input;
PlatformServices g_platform;
RenderCommands* g_render_commands;
PlatformRenderSettings g_platform_render_settings;
char* g_debug_print_ring_buffer;
i32* g_debug_print_ring_buffer_write_head;
GameCode g_game_code;

void exit_gracefully(int error_code) {
  ImGui_ImplSdlGL3_Shutdown();
  exit(error_code);
}

DEBUG_PLATFORM_FREE_FILE_MEMORY(DEBUG_platform_free_file_memory) {
  free(memory);
}

DEBUG_PLATFORM_READ_ENTIRE_FILE(DEBUG_platform_read_entire_file) {
  PlatformEntireFile result = {};

  FILE *f = fopen(filename, "rb");

  if (f == 0) {
    LOG("Failed to open file %s\n", filename);
    return result;
  }

  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
  fseek(f, 0, SEEK_SET);

  char *string = (char *)malloc(fsize + 1);
  if (!fread(string, fsize, 1, f)) {
    LOG("No results from fread\n");
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

#ifdef PLATFORM_WINDOWS

inline FILETIME get_file_last_write_time(char* filepath) {
  FILETIME last_write_time = {};

  WIN32_FILE_ATTRIBUTE_DATA data;
  if(GetFileAttributesEx(filepath, GetFileExInfoStandard, &data)) {
      last_write_time = data.ftLastWriteTime;
  }

  return last_write_time;
}

inline b32 file_has_been_touched(char* filepath, FILETIME* filetime) {
  FILETIME last_write = get_file_last_write_time(filepath);
  b32 result = CompareFileTime(&last_write, filetime) != 0;
  *filetime = last_write;
  return result;
}

PLATFORM_GET_LAST_WRITE_TIME(platform_get_last_write_time) {
  assert(sizeof(PlatformFileLastWriteTime) <= sizeof(FILETIME));
  union {
    FILETIME from;
    PlatformFileLastWriteTime to;
  } u;
  u.from = get_file_last_write_time(filename);
  return u.to;
}

PLATFORM_FILE_HAS_BEEN_TOUCHED(platform_file_has_been_touched) {
  assert(sizeof(PlatformFileLastWriteTime) <= sizeof(FILETIME));
  return file_has_been_touched(filename, (FILETIME*)(last_write_time));
}

void readfile_job(jt_job_data* job) {
  auto request = (PlatformFileIORequest*)job->padding;
  char* filename = request->filename;

  FILE *f = fopen(filename, "rb");

  if (f == 0) {
    LOG("Failed to open file %s\n", filename);
    exit(1);
  }

  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
  fseek(f, 0, SEEK_SET);

  char* string = (char*)malloc(fsize + 1);
  if (!fread(string, fsize, 1, f)) {
    LOG("No results from fread\n");
  }
  fclose(f);

  string[fsize] = 0;

  PlatformAsyncFileHandle* result = request->result;
  result->file.contents = (u8 *)string;
  result->file.content_size = (u32)fsize;

  SDL_AtomicSet(&result->ready, true);
}

PLATFORM_BEGIN_READ_ENTIRE_FILE(platform_begin_read_entire_file) {
  PlatformFileIORequest request;
  request.filename = (char*)filename;
  request.result = (PlatformAsyncFileHandle*)calloc(1, sizeof(PlatformAsyncFileHandle));
  auto job = jt_create_job(readfile_job, (void*)&request, sizeof(PlatformFileIORequest));
  jt_run_job(job, false);
  return request.result;
}

PLATFORM_FILE_IO_COMPLETE(platform_file_io_complete) {
  PlatformAsyncFileHandle* result = (PlatformAsyncFileHandle*)handle;
  return SDL_AtomicGet(&result->ready);
}

PLATFORM_ENTIRE_FILE_RESULT(platform_entire_file_result) {
  PlatformAsyncFileHandle* result = (PlatformAsyncFileHandle*)handle;
  assert(SDL_AtomicGet(&result->ready));
  return result->file;
}

PLATFORM_FREE_FILE_MEMORY(platform_free_file_memory) {
  PlatformAsyncFileHandle* result = (PlatformAsyncFileHandle*)handle;
  free(result->file.contents);
  free(result);
}

void get_exe_filepath(char* buffer, i32 buffer_size) {
  GetModuleFileNameA(0, buffer, buffer_size);
}

void unload_game_code(GameCode* game) {
  if(game->dll)
  {
      FreeLibrary(game->dll);
      game->dll = 0;
  }

  game->is_valid = false;
  game->update_and_render = 0;
}

GameCode load_game_code(char *source_dll_name, char *tmp_dll_name, char *lock_file_name) {
  GameCode result = {};

  WIN32_FILE_ATTRIBUTE_DATA ignored;
  if(!GetFileAttributesEx(lock_file_name, GetFileExInfoStandard, &ignored)) {
    result.last_write_time = get_file_last_write_time(source_dll_name);

    CopyFile(source_dll_name, tmp_dll_name, FALSE);

    result.dll = LoadLibraryA(tmp_dll_name);
    if(result.dll) {
      result.update_and_render = (GameUpdateAndRender*)GetProcAddress(result.dll, "game_update_and_render");
      result.imgui_get_io = (GameImguiGetIO*)GetProcAddress(result.dll, "game_imgui_get_io");
      result.imgui_new_frame = (GameImguiNewFrame*)GetProcAddress(result.dll, "game_imgui_new_frame");
      result.imgui_shutdown = (GameImguiShutdown*)GetProcAddress(result.dll, "game_imgui_shutdown");
      result.imgui_render = (GameImguiRender*)GetProcAddress(result.dll, "game_imgui_render");
      result.imgui_get_tex_data_as_rgba32 = (GameImguiGetTexDataAsRGBA32*)GetProcAddress(result.dll, "game_imgui_get_tex_data_as_rgba32");
      result.debug_end_frame = (GameDebugEndFrame*)GetProcAddress(result.dll, "game_debug_end_frame");
      result.is_valid = result.update_and_render != NULL;
    }
  }

  if(!result.is_valid)
  {
    result.update_and_render = NULL;
  }

  return result;
}

#endif

#ifdef RENDERER_OPENGL

// TODO(doug): use BGRA?
GLint get_internal_format(i32 channels) {
  switch (channels) {
    case 1: return GL_R8;
    case 2: return GL_RG8;
    case 4: return GL_RGBA8;
    default:
      LOG("Attempted to load image with %d channels\n", channels);
      return GL_RGBA8;
  }
}

GLenum get_format(i32 channels) {
  switch (channels) {
    case 1: return GL_RED;
    case 2: return GL_RG;
    case 4: return GL_RGBA;
    default:
      LOG("Attempted to load image with %d channels\n", channels);
      return GL_RGBA8;
  }
}

GLenum get_type(i32 channels) {
  switch (channels) {
    case 1: return GL_UNSIGNED_BYTE;
    case 2: return GL_UNSIGNED_BYTE;
    case 4: return GL_UNSIGNED_BYTE;
    default:
      LOG("Attempted to load image with %d channels\n", channels);
      return GL_RGBA8;
  }
}

PLATFORM_UNREGISTER_TEXTURE(platform_unregister_texture) {
  GLuint gl_texture = (GLuint)(intptr_t)(texture_handle);
  glDeleteTextures(1, &gl_texture);
}

PLATFORM_REGISTER_TEXTURE(platform_register_texture) {
  PlatformTexture result;
  GLuint texture_id;
  glGenTextures(1, &texture_id);
  glBindTexture(GL_TEXTURE_2D, texture_id);

  GLint internal_format = get_internal_format(channels);
  GLenum format = get_format(channels);
  GLenum type = get_type(channels);

  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
  glTexImage2D(GL_TEXTURE_2D,
               0,
               internal_format,
               width,
               height,
               0,
               format,
               type,
               (void*)data);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  result.handle = (void*)(intptr_t)texture_id;
  result.width = width;
  result.height = height;
  result.channels = channels;
  return result;
}

#endif


f32 get_seconds_elapsed(u64 old, u64 current) {
  return ((f32)(current - old) / (f32)(SDL_GetPerformanceFrequency()));
}

void check_sdl_error(int line = -1) {
  const char *error = SDL_GetError();
  if (*error != '\0')
  {
    LOG("SDL Error: %s\n", error);
    if (line != -1)
      LOG(" + line: %i\n", line);
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

  g_debug_print_ring_buffer = (char*)calloc(DEBUG_PRINT_RING_BUFFER_SIZE + 1, 1);
  i32 debug_print_ring_buffer_write_head = 0;
  g_debug_print_ring_buffer_write_head = &debug_print_ring_buffer_write_head;

  PlatformContext context = {0};
  g_platform.DEBUG_read_entire_file = DEBUG_platform_read_entire_file;
  g_platform.DEBUG_write_entire_file = DEBUG_platform_write_entire_file;
  g_platform.DEBUG_free_file_memory = DEBUG_platform_free_file_memory;
  g_platform.begin_read_entire_file = platform_begin_read_entire_file;
  g_platform.file_io_complete = platform_file_io_complete;
  g_platform.entire_file_result = platform_entire_file_result;
  g_platform.free_file_memory = platform_free_file_memory;
  g_platform.register_texture = platform_register_texture;
  g_platform.unregister_texture = platform_unregister_texture;
  g_platform.file_has_been_touched = platform_file_has_been_touched;
  g_platform.get_last_write_time = platform_get_last_write_time;

  jt_init();

  if (SDL_Init(SDL_INIT_EVERYTHING)) {
    LOG("Unable to init SDL: %s\n", SDL_GetError());
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

#ifdef PLATFORM_WINDOWS
  get_exe_filepath(context.exe_filepath, sizeof(context.exe_filepath));
  i32 exe_directory_len = (i32)(strrchr(context.exe_filepath, '\\') - context.exe_filepath + 1);

  char source_dll_name[FILEPATH_SIZE] = {};
  memcpy(source_dll_name, context.exe_filepath, exe_directory_len);
  strcpy(source_dll_name + exe_directory_len, "fall_game.dll");

  char temp_dll_name[FILEPATH_SIZE] = {};
  memcpy(temp_dll_name, context.exe_filepath, exe_directory_len);
  strcpy(temp_dll_name + exe_directory_len, "fall_game_temp.dll");

  char lock_file_name[FILEPATH_SIZE] = {};
  memcpy(lock_file_name, context.exe_filepath, exe_directory_len);
  strcpy(lock_file_name + exe_directory_len, "lock.tmp");

  g_game_code = load_game_code(source_dll_name, temp_dll_name, lock_file_name);
  FILETIME game_code_last_touched = get_file_last_write_time(source_dll_name);
#endif

  ImGui_ImplSdlGL3_Init(context.window);

  if(SDL_GL_SetSwapInterval(1) < 0){
    check_sdl_error(__LINE__);
  }

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glLineWidth(2.0f);

  const u64 megabytes = 1024LL * 1024LL;
  g_game_state = (GameState*)calloc(GAME_MEMORY_SIZE, sizeof(u8));
  g_transient_state = (TransientState*)calloc(TRANSIENT_MEMORY_SIZE, sizeof(u8));

  void** texture_handles = (void**)calloc(megabytes * 8, sizeof(u8));
  auto textured_quad_vertices_memory = megabytes * 16;
  auto circle_vertices_memory = megabytes * 1;
  TexturedQuadVertex* quad_vertices = (TexturedQuadVertex*)calloc(textured_quad_vertices_memory, sizeof(u8));
  LineVertex* circle_vertices = (LineVertex*)calloc(circle_vertices_memory, sizeof(u8));
  i32* line_loop_lengths = (i32*)calloc(circle_vertices_memory / 4, sizeof(u8));
  g_render_commands = (RenderCommands*)calloc(1, sizeof(RenderCommands));
  g_render_commands->textured_quad_vertices = quad_vertices;
  g_render_commands->line_vertices = circle_vertices;
  g_render_commands->line_loop_lengths = line_loop_lengths;
  g_render_commands->quad_textures = texture_handles;
  g_render_commands->textured_quad_vertices_max_count = textured_quad_vertices_memory / sizeof(TexturedQuadVertex);
  g_render_commands->line_vertices_max_count = circle_vertices_memory / sizeof(LineVertex);
  g_render_commands->screen_width = START_WIDTH;
  g_render_commands->screen_height = START_HEIGHT;

  GameMemory game_memory = {};
  game_memory.game_state = g_game_state;
  game_memory.transient_state = g_transient_state;
  game_memory.input = g_input;
  game_memory.platform = g_platform;
  game_memory.render_commands = g_render_commands;
  game_memory.debug_print_ring_buffer = g_debug_print_ring_buffer;
  game_memory.debug_print_ring_buffer_write_head = g_debug_print_ring_buffer_write_head;

#ifdef RENDERER_OPENGL
  opengl_init(START_WIDTH, START_HEIGHT);
#endif

  context.controller_handle = find_controller_handle();

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

    for (int i = 0; i < ARRAY_LENGTH(next_input.buttons); ++i) {
      next_input.buttons[i].ended_down = prev_input.buttons[i].ended_down;
    }

    for (int i = 0; i < ARRAY_LENGTH(next_input.keyboard.buttons); ++i) {
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
      ImGui_ImplSdlGL3_ProcessEvent(&event);
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

    next_input.dt = 1.0f / FRAME_RATE;

    // Allow g_input to be mutable, in order for code to have the option of
    // eating input events.
    PlatformInput tmp_input = next_input;
    game_memory.input = g_input = &tmp_input;

    ImGui_ImplSdlGL3_NewFrame(context.window);
    g_game_code.update_and_render(&game_memory);

    prev_input = next_input;

#ifdef RENDERER_OPENGL
    opengl_render_commands(g_render_commands, START_WIDTH, START_HEIGHT);
#endif
    g_render_commands->textured_quad_vertices_count = 0;
    g_render_commands->line_vertices_count = 0;
    g_render_commands->line_loop_lengths_count = 0;
    g_game_code.debug_end_frame();
    g_game_code.imgui_render();

    SDL_GL_SwapWindow(context.window);

#ifdef PLATFORM_WINDOWS
    b32 reload_game_code = file_has_been_touched(source_dll_name, &game_code_last_touched);
    if (reload_game_code) {
      LOG("Reloading game code\n");
      ImGui_ImplSdlGL3_Shutdown();
      unload_game_code(&g_game_code);
      for(i32 i = 0; !g_game_code.is_valid && (i < 100); ++i) {
          g_game_code = load_game_code(source_dll_name, temp_dll_name, lock_file_name);
          SDL_Delay(100);
      }
      game_code_last_touched = get_file_last_write_time(source_dll_name);
      ImGui_ImplSdlGL3_Init(context.window);
    }
#endif
  }

  exit_gracefully(0);
}
