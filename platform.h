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

#define DEBUG_PLATFORM_FREE_FILE_MEMORY(name) void name(void* memory)
typedef DEBUG_PLATFORM_FREE_FILE_MEMORY(DEBUG_PlatformFreeFileMemory);

#define DEBUG_PLATFORM_READ_ENTIRE_FILE(name) PlatformEntireFile name(char* filename)
typedef DEBUG_PLATFORM_READ_ENTIRE_FILE(DEBUG_PlatformReadEntireFile);

#define DEBUG_PLATFORM_WRITE_ENTIRE_FILE(name) b32 name(char* filename, i32 size, void* memory)
typedef DEBUG_PLATFORM_WRITE_ENTIRE_FILE(DEBUG_PlatformWriteEntireFile);

struct PlatformServices {
    DEBUG_PlatformFreeFileMemory* DEBUG_platform_free_file_memory;
    DEBUG_PlatformWriteEntireFile* DEBUG_platform_write_entire_file;
    DEBUG_PlatformReadEntireFile* DEBUG_platform_read_entire_file;
};

struct PlatformRenderSettings {
    i32 width;
    i32 height;
};

struct GameState;
struct TransientState;

extern GameState* g_game_state;
extern TransientState* g_transient_state;
extern PlatformInput* g_platform_input;
extern PlatformServices g_platform_services;
extern PlatformRenderSettings g_platform_render_settings;

void game_update_and_render();

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))
#define PUSH_STRUCT(arena, type) (type *)_push_size(arena, sizeof(type))
#define PUSH_ARRAY(arena, count, type) (type *)_push_size(arena, ((size_t)count) * sizeof(type))
#define ZERO_STRUCT(instance) memset(&(instance), 0, sizeof(instance))
#define ZERO_ARRAY(instance, count) memset(instance, 0, ((size_t)count) * sizeof(*instance))

#endif /* end of include guard: PLATFORM_H__ */

