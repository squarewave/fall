#ifndef PLATFORM_H__
#define PLATFORM_H__

#include <stdint.h>

#ifdef FALL_INTERNAL
#include "imgui/imgui.h"
#endif

#define reflectable
#define reflect_member(options)
#define ignore

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;
typedef int64_t i64;
typedef int32_t i32;
typedef int16_t i16;
typedef int8_t i8;
typedef uint32_t b32;
typedef float f32;
typedef double f64;

enum GamepadButtons {
  BUTTON_A = 0,
  BUTTON_B = 1,
  BUTTON_X = 2,
  BUTTON_Y = 3,
  BUTTON_L_BUMPER = 4,
  BUTTON_R_BUMPER = 5,

  BUTTON_L_STICK = 9,
  BUTTON_R_STICK = 10,
};

enum GamepadAxes {
  L_STICK_X = 0,
  L_STICK_Y = 1,
  L_TRIGGER = 2,
  R_STICK_X = 3,
  R_STICK_Y = 4,
  R_TRIGGER = 5
};

struct OffscreenBuffer
{
  void *memory;
  int width;
  int height;
  int pitch;
  int bytes_per_pixel;
};

struct ButtonInput {
  u32 transition_count;
  b32 ended_down;
};

struct AnalogInput {
  f32 delta, value;
};

struct JoystickInput {
  f32 x, y;
  f32 dx, dy;
};

struct MouseInput {
  f32 x, y;
  f32 dx, dy;
  i32 dwheel;
  ButtonInput button_l;
  ButtonInput button_r;
  ButtonInput button_middle;
};

struct KeyboardInput {
  union {
    struct {
      ButtonInput k0,k1,k2,k3,k4,k5,k6,k7,k8,k9;
      ButtonInput a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z;

      ButtonInput up,down,left,right;

    };
    ButtonInput buttons[40];
  };
  b32 shift_down, alt_down, ctrl_down;
};

#define collection_add(collection, item) collection[collection##_count++] = (item)
#define collection_reserve(collection) &(collection[collection##_count++])
#define collection_reserve_i(collection) collection##_count++

inline void eat_button_input(ButtonInput* button) {
  button->ended_down = false;
  button->transition_count = 0;
}

inline b32 was_pressed(ButtonInput button) {
  return button.ended_down && button.transition_count;
}

inline b32 was_down(ButtonInput button) {
  return button.ended_down;
}

inline b32 was_released(ButtonInput button) {
  return (!button.ended_down) && button.transition_count;
}

#define has_flag(value, flag) ((value) & (flag))
#define set_flag(value, flag) ((value) |= (flag))
#define clear_flag(value, flag) ((value) &= ~(flag))

struct PlatformInput {
  union {
    struct {
      ButtonInput button_a;
      ButtonInput button_b;
      ButtonInput button_x;
      ButtonInput button_y;
      ButtonInput button_l_bumper;
      ButtonInput button_r_bumper;
      ButtonInput button_l_stick;
      ButtonInput button_r_stick;
    };
    ButtonInput buttons[12];
  };
  AnalogInput analog_l_trigger, analog_r_trigger;
  JoystickInput joystick_l, joystick_r;

  MouseInput mouse;
  KeyboardInput keyboard;

  f32 dt;
};

struct PlatformEntireFile {
  unsigned char* contents;
  i32 content_size;
};

reflectable struct PlatformTexture {
  i32 width, height, channels;
  void* handle;
};

reflectable struct PlatformFileLastWriteTime {
	char platform_data[8];
};

#define PLATFORM_BEGIN_READ_ENTIRE_FILE(name) void* name(char* filename)
typedef PLATFORM_BEGIN_READ_ENTIRE_FILE(PlatformBeginReadEntireFile);

#define PLATFORM_FILE_IO_COMPLETE(name) b32 name(void* handle)
typedef PLATFORM_FILE_IO_COMPLETE(PlatformFileIOComplete);

#define PLATFORM_ENTIRE_FILE_RESULT(name) PlatformEntireFile name(void* handle)
typedef PLATFORM_ENTIRE_FILE_RESULT(PlatformEntireFileResult);

#define PLATFORM_FREE_FILE_MEMORY(name) void name(void* handle)
typedef PLATFORM_FREE_FILE_MEMORY(PlatformFreeFileMemory);

#define PLATFORM_GET_LAST_WRITE_TIME(name) PlatformFileLastWriteTime name(char* filename)
typedef PLATFORM_GET_LAST_WRITE_TIME(PlatformGetLastWriteTime);

#define PLATFORM_FILE_HAS_BEEN_TOUCHED(name) b32 name(char* filename, PlatformFileLastWriteTime* last_write_time)
typedef PLATFORM_FILE_HAS_BEEN_TOUCHED(PlatformFileHasBeenTouched);

#define PLATFORM_REGISTER_TEXTURE(name) PlatformTexture name(char* data, i32 width, i32 height, i32 channels)
typedef PLATFORM_REGISTER_TEXTURE(PlatformRegisterTexture);

#define PLATFORM_UNREGISTER_TEXTURE(name) void name(void* texture_handle)
typedef PLATFORM_UNREGISTER_TEXTURE(PlatformUnregisterTexture);

#define DEBUG_PLATFORM_FREE_FILE_MEMORY(name) void name(void* memory)
typedef DEBUG_PLATFORM_FREE_FILE_MEMORY(DEBUG_PlatformFreeFileMemory);

#define DEBUG_PLATFORM_READ_ENTIRE_FILE(name) PlatformEntireFile name(char* filename)
typedef DEBUG_PLATFORM_READ_ENTIRE_FILE(DEBUG_PlatformReadEntireFile);

#define DEBUG_PLATFORM_WRITE_ENTIRE_FILE(name) b32 name(char* filename, i32 size, void* memory)
typedef DEBUG_PLATFORM_WRITE_ENTIRE_FILE(DEBUG_PlatformWriteEntireFile);

struct PlatformServices {
  DEBUG_PlatformFreeFileMemory* DEBUG_free_file_memory;
  DEBUG_PlatformWriteEntireFile* DEBUG_write_entire_file;
  DEBUG_PlatformReadEntireFile* DEBUG_read_entire_file;

  PlatformFileIOComplete* file_io_complete;
  PlatformEntireFileResult* entire_file_result;
  PlatformBeginReadEntireFile* begin_read_entire_file;
  PlatformFreeFileMemory* free_file_memory;
  PlatformGetLastWriteTime* get_last_write_time;
  PlatformFileHasBeenTouched* file_has_been_touched;

  PlatformRegisterTexture* register_texture;
  PlatformUnregisterTexture* unregister_texture;
};

struct PlatformRenderSettings {
  i32 width;
  i32 height;
};

struct GameState;
struct TransientState;
struct RenderCommands;

struct GameMemory {
  GameState* game_state;
  TransientState* transient_state;
  PlatformInput* input;
  RenderCommands* render_commands;
  PlatformServices platform;

  char* debug_print_ring_buffer;
  i32* debug_print_ring_buffer_write_head;
};

extern GameState* g_game_state;
extern TransientState* g_transient_state;
extern PlatformInput* g_input;
extern RenderCommands* g_render_commands;
extern PlatformServices g_platform;

#define DEBUG_PRINT_RING_BUFFER_SIZE 0x00000800
#define DEBUG_PRINT_RING_BUFFER_MASK 0x000007ff
extern char* g_debug_print_ring_buffer;
extern i32* g_debug_print_ring_buffer_write_head;

#define GAME_UPDATE_AND_RENDER(name) void name(GameMemory* memory)
typedef GAME_UPDATE_AND_RENDER(GameUpdateAndRender);

#ifdef FALL_INTERNAL
#define GAME_DEBUG_END_FRAME(name) void name()
typedef GAME_DEBUG_END_FRAME(GameDebugEndFrame);

#define GAME_IMGUI_GET_IO(name) ImGuiIO& name()
typedef GAME_IMGUI_GET_IO(GameImguiGetIO);

#define GAME_IMGUI_NEW_FRAME(name) void name()
typedef GAME_IMGUI_NEW_FRAME(GameImguiNewFrame);

#define GAME_IMGUI_SHUTDOWN(name) void name()
typedef GAME_IMGUI_SHUTDOWN(GameImguiShutdown);

#define GAME_IMGUI_RENDER(name) void name()
typedef GAME_IMGUI_RENDER(GameImguiRender);

#define GAME_IMGUI_GET_TEX_DATA_AS_RGBA32(name) void name(unsigned char** pixels, int* width, int* height)
typedef GAME_IMGUI_GET_TEX_DATA_AS_RGBA32(GameImguiGetTexDataAsRGBA32);
#endif

#define ARRAY_LENGTH(array) (sizeof(array) / sizeof(array[0]))
#define ZERO_STRUCT(instance) memset(&(instance), 0, sizeof(instance))
#define ZERO_ARRAY(instance, count) memset(instance, 0, ((size_t)count) * sizeof(*instance))
#define OFFSET_OF(type, member) (void*)&(((type *)0)->member)

#define MEGABYTES 1024LL * 1024LL
#define GAME_MEMORY_SIZE (MEGABYTES * 24)
#define TRANSIENT_MEMORY_SIZE (MEGABYTES * 1)


#ifdef PLATFORM_WINDOWS
#include "Windows.h"
#include "Strsafe.h"
#include "malloc.h"
#include "string.h"

// http://stackoverflow.com/questions/29049686/is-there-a-better-way-to-pass-formatted-output-to-outputdebugstring
#define LOG(debug_format_string, ...) _LOG(__FUNCTION__, __LINE__, debug_format_string, __VA_ARGS__)

VOID _LOG( LPCSTR kszFunction, INT iLineNumber, LPCSTR debug_format_string, ... ) \
{
  INT result_string_size = 0;
  va_list args;
  PCHAR result_string = NULL;
  size_t string_print_offset = 0;

  va_start( args, debug_format_string );

  result_string_size = _scprintf("[%s:%d] ", kszFunction, iLineNumber ) * sizeof( CHAR );
  result_string_size += _vscprintf( debug_format_string, args ) * sizeof( CHAR ) + 2;

  /* Depending on the size of the format string, allocate space on the stack or the heap. */
  result_string = (PCHAR)_malloca(result_string_size);

  /* Populate the buffer with the contents of the format string. */
  StringCbPrintf(result_string, result_string_size, "[%s:%d] ", kszFunction, iLineNumber);
  StringCbLength(result_string, result_string_size, &string_print_offset);
  StringCbVPrintf(&result_string[string_print_offset], result_string_size - string_print_offset, debug_format_string, args);
  StringCbLength(result_string, result_string_size, &string_print_offset);
  result_string[string_print_offset] = '\n';
  result_string[string_print_offset + 1] = NULL;

  OutputDebugString( result_string );

  i32 write_head = *g_debug_print_ring_buffer_write_head & DEBUG_PRINT_RING_BUFFER_MASK;
  i32 remaining_write_space = DEBUG_PRINT_RING_BUFFER_SIZE - write_head;
  i32 debug_str_len = strlen(result_string);
  if (debug_str_len < remaining_write_space) {
    strcpy(g_debug_print_ring_buffer + write_head, result_string);
  } else {
    strncpy(g_debug_print_ring_buffer + write_head, result_string, remaining_write_space);
    strcpy(g_debug_print_ring_buffer, result_string + remaining_write_space);
  }
  *g_debug_print_ring_buffer_write_head += debug_str_len;

  _freea(result_string);
  va_end(args);
}
#else
#define LOG(debug_format_string, ... ) ;;
#endif

#endif /* end of include guard: PLATFORM_H__ */

