#ifndef PLATFORM_H__
#define PLATFORM_H__

#include <stdint.h>

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

b32 was_pressed(ButtonInput button) {
  return button.ended_down && button.transition_count;
}

b32 was_down(ButtonInput button) {
  // NOTE(doug): this is just for some visual consistency with the other two functions
  return button.ended_down;
}

b32 was_released(ButtonInput button) {
  return (!button.ended_down) && button.transition_count;
}

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

struct PlatformTexture {
  i32 width, height, channels;
  void* handle;
};

struct PlatformFileLastWriteTime {
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

extern GameState* g_game_state;
extern TransientState* g_transient_state;
extern PlatformInput* g_input;
extern PlatformServices g_platform;

void game_update_and_render();

#define ARRAY_LENGTH(array) (sizeof(array) / sizeof(array[0]))
#define PUSH_STRUCT(arena, type) (type *)_push_size(arena, sizeof(type))
#define PUSH_ARRAY(arena, count, type) (type *)_push_size(arena, ((size_t)count) * sizeof(type))
#define ZERO_STRUCT(instance) memset(&(instance), 0, sizeof(instance))
#define ZERO_ARRAY(instance, count) memset(instance, 0, ((size_t)count) * sizeof(*instance))
#define OFFSET_OF(type, member) (void*)&(((type *)0)->member)

#ifdef PLATFORM_WINDOWS
#include "Windows.h"
#include "Strsafe.h"
#include "malloc.h"
#include "string.h"

#define DEBUG_PRINT_RING_BUFFER_SIZE 0x00000800
#define DEBUG_PRINT_RING_BUFFER_MASK 0x000007ff
extern char g_debug_print_ring_buffer[DEBUG_PRINT_RING_BUFFER_SIZE + 1]; // add 1 to ensure null-termination
extern i32 g_debug_print_ring_buffer_write_head;

// http://stackoverflow.com/questions/29049686/is-there-a-better-way-to-pass-formatted-output-to-outputdebugstring
#define LOG(kszDebugFormatString, ...) _LOG(__FUNCTION__, __LINE__, kszDebugFormatString, __VA_ARGS__)

VOID _LOG( LPCSTR kszFunction, INT iLineNumber, LPCSTR kszDebugFormatString, ... ) \
{
  INT cbFormatString = 0;
  va_list args;
  PCHAR szDebugString = NULL;
  size_t st_Offset = 0;

  va_start( args, kszDebugFormatString );

  cbFormatString = _scprintf("[%s:%d] ", kszFunction, iLineNumber ) * sizeof( CHAR );
  cbFormatString += _vscprintf( kszDebugFormatString, args ) * sizeof( CHAR ) + 2;

  /* Depending on the size of the format string, allocate space on the stack or the heap. */
  szDebugString = (PCHAR)_malloca( cbFormatString );

  /* Populate the buffer with the contents of the format string. */
  StringCbPrintf( szDebugString, cbFormatString, "[%s:%d] ", kszFunction, iLineNumber );
  StringCbLength( szDebugString, cbFormatString, &st_Offset );
  StringCbVPrintf( &szDebugString[st_Offset / sizeof(CHAR)], cbFormatString - st_Offset, kszDebugFormatString, args );

  OutputDebugString( szDebugString );

  i32 write_head = g_debug_print_ring_buffer_write_head & DEBUG_PRINT_RING_BUFFER_MASK;
  i32 remaining_write_space = DEBUG_PRINT_RING_BUFFER_SIZE - write_head;
  i32 debug_str_len = strlen(szDebugString);
  if (debug_str_len < remaining_write_space) {
    strcpy(g_debug_print_ring_buffer + write_head, szDebugString);
  } else {
    strncpy(g_debug_print_ring_buffer + write_head, szDebugString, remaining_write_space);
    strcpy(g_debug_print_ring_buffer, szDebugString + remaining_write_space);
  }
  g_debug_print_ring_buffer_write_head += debug_str_len;

  _freea( szDebugString );
  va_end( args );
}
#else
#define LOG( kszDebugFormatString, ... ) ;;
#endif

#endif /* end of include guard: PLATFORM_H__ */

