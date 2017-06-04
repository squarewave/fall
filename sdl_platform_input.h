#ifndef SDL_PLATFORM_INPUT_H__
#define SDL_PLATFORM_INPUT_H__

#include "SDL2/SDL.h"

#include "platform.h"
#include "sdl_platform.h"

SDL_GameController* find_controller_handle() {
  i32 num_joysticks = SDL_NumJoysticks();

  for (int i = 0; i < num_joysticks; ++i) {
    printf("%s\n", SDL_GameControllerName(SDL_GameControllerOpen(i)));
    return SDL_GameControllerOpen(i);
  }

  printf("No controller found. Joysticks attached: %d\n", num_joysticks);
  return 0;
}

b32 handle_sdl_event(SDL_Event* event, PlatformContext* context) {
  PlatformInput* input = context->next_input;
  b32 should_quit = false;
  switch (event->type) {
    case SDL_QUIT: {
      printf("SDL_QUIT\n");
      should_quit = true;
    } break;
    case SDL_WINDOWEVENT: {
      switch (event->window.event) {
        case SDL_WINDOWEVENT_RESIZED: {
        } break;
        case SDL_WINDOWEVENT_EXPOSED: {
        } break;
      } break;
    } break;
    case SDL_KEYDOWN:
    case SDL_KEYUP: {
      SDL_Keycode key_code = event->key.keysym.sym;
      b32 ended_down = event->key.state != SDL_RELEASED;

      if (event->type == SDL_KEYUP || event->key.repeat == 0) {
        switch (key_code) {
          #define KEY_MAPPING(sdl, platform) case sdl: {\
            input->platform.ended_down = ended_down;\
            input->platform.transition_count++;\
          } break;
          #define MODIFIER_MAPPING(sdl, platform) case sdl: {\
            input->platform = ended_down;\
          } break;
          #include "key_mappings.h"
          #undef SIMPLE_KEY_MAPPING
        }
        switch (key_code) {
          case SDLK_UP: {
            input->button_a.ended_down = ended_down;
            input->button_a.transition_count++;
          } break;
          case SDLK_DOWN: {
            input->button_x.ended_down = ended_down;
            input->button_x.transition_count++;
          } break;
          case SDLK_LEFT: {
            input->joystick_l.x = ended_down ?
              -1.0f :
              (input->keyboard.right.ended_down ? 1.0f : 0.0f);
          } break;
          case SDLK_RIGHT: {
            input->joystick_l.x = ended_down ?
              1.0f :
              (input->keyboard.left.ended_down ? -1.0f : 0.0f);
          } break;
          case SDLK_x: {
            input->analog_r_trigger.value = ended_down ? 1.0f : 0.0f;
          } break;
          case SDLK_z: {
            input->analog_l_trigger.value = ended_down ? 1.0f : 0.0f;
          } break;
        }
      }
    } break;
    case SDL_CONTROLLERDEVICEADDED: {
      if (context->controller_handle) {
        SDL_GameControllerClose(context->controller_handle);
      }
      context->controller_handle = SDL_GameControllerOpen(event->cdevice.which);
    } break;
    case SDL_CONTROLLERDEVICEREMOVED: {
      if (context->controller_handle) {
        SDL_GameControllerClose(context->controller_handle);
      }
    } break;
    case SDL_MOUSEMOTION: {
      if (!g_game_code.imgui_get_io().WantCaptureMouse) {
        i32 window_width = 0;
        i32 window_height = 0;
        SDL_GetWindowSize(context->window,
                          &window_width,
                          &window_height);

        input->mouse.x = (f32)event->motion.x;
        input->mouse.y = window_height - (f32)event->motion.y - 1;
        input->mouse.dx = (f32)event->motion.xrel;
        input->mouse.dy = -(f32)event->motion.yrel;
      }
    } break;
    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP: {
      if (!g_game_code.imgui_get_io().WantCaptureMouse) {
        b32 ended_down = event->button.state != SDL_RELEASED;

        switch (event->button.button) {
        case SDL_BUTTON_LEFT: {
          input->mouse.button_l.ended_down = ended_down;
          input->mouse.button_l.transition_count++;
        } break;
        case SDL_BUTTON_RIGHT: {
          input->mouse.button_r.ended_down = ended_down;
          input->mouse.button_r.transition_count++;
        } break;
        case SDL_BUTTON_MIDDLE: {
          input->mouse.button_middle.ended_down = ended_down;
          input->mouse.button_middle.transition_count++;
        } break;
        }
      }
    } break;
    case SDL_MOUSEWHEEL: {
      input->mouse.dwheel = event->wheel.y;
    } break;
    case SDL_CONTROLLERBUTTONDOWN:
    case SDL_CONTROLLERBUTTONUP: {
      u8 button = event->cbutton.button;
      b32 ended_down = event->cbutton.state != SDL_RELEASED;

      switch (button) {
        case SDL_CONTROLLER_BUTTON_A: {
          input->button_a.ended_down = ended_down;
          input->button_a.transition_count++;
        } break;
        case SDL_CONTROLLER_BUTTON_B: {
          input->button_b.ended_down = ended_down;
          input->button_b.transition_count++;
        } break;
        case SDL_CONTROLLER_BUTTON_X: {
          input->button_x.ended_down = ended_down;
          input->button_x.transition_count++;
        } break;
        case SDL_CONTROLLER_BUTTON_Y: {
          input->button_y.ended_down = ended_down;
          input->button_y.transition_count++;
        } break;
        case SDL_CONTROLLER_BUTTON_DPAD_LEFT: {
          input->joystick_l.x = ended_down ? -1.0f : 0.0f;
        } break;
        case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: {
          input->joystick_l.x = ended_down ? 1.0f : 0.0f;
        } break;
        case SDL_CONTROLLER_BUTTON_BACK: {
        } break;
        case SDL_CONTROLLER_BUTTON_GUIDE: {
        } break;
        case SDL_CONTROLLER_BUTTON_START: {
        } break;
        case SDL_CONTROLLER_BUTTON_LEFTSTICK: {
        } break;
        case SDL_CONTROLLER_BUTTON_RIGHTSTICK: {
        } break;
        case SDL_CONTROLLER_BUTTON_LEFTSHOULDER: {
        } break;
        case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: {
        } break;
        case SDL_CONTROLLER_BUTTON_DPAD_UP: {
        } break;
        case SDL_CONTROLLER_BUTTON_DPAD_DOWN: {
        } break;
      }
    } break;
    case SDL_CONTROLLERAXISMOTION: {
      u8 axis = event->caxis.axis;
      i32 value = event->caxis.value;

      switch (axis) {
        case SDL_CONTROLLER_AXIS_LEFTX: {
          input->joystick_l.x = ((f32)value) / 32768.0f;
        } break;
        case SDL_CONTROLLER_AXIS_LEFTY: {
          input->joystick_l.y = ((f32)value) / 32768.0f;
        } break;
        case SDL_CONTROLLER_AXIS_RIGHTX: {
          input->joystick_r.x = ((f32)value) / 32768.0f;
        } break;
        case SDL_CONTROLLER_AXIS_RIGHTY: {
          input->joystick_r.y = ((f32)value) / 32768.0f;
        } break;
        case SDL_CONTROLLER_AXIS_TRIGGERLEFT: {
          input->analog_l_trigger.value = ((f32)value) / 32768.0f;
        } break;
        case SDL_CONTROLLER_AXIS_TRIGGERRIGHT: {
          input->analog_r_trigger.value = ((f32)value) / 32768.0f;
        } break;
      }
    } break;
  }

  return should_quit;
}

#endif /* end of include guard: SDL_PLATFORM_INPUT_H__ */
