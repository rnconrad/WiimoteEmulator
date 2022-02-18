#include "input_sdl.h"
#include "SDL/SDL.h"

void input_sdl_init(void)
{
  if (SDL_Init(0) < 0)
  {
    printf("Could not initialize SDL: %s\n", SDL_GetError());
    exit(1);
  }
  SDL_SetVideoMode(128, 128, 0, 0);
  printf("Commands overview\n"
         "-----------------\n"
         "                 __ arrows use keypad numbers!\n"
         "     ........  L'\n"
         "    |   .    |\\           ,.._\n"
         "    |   8    | |        ,'    \\\\\n"
         "    | -4 6-  | |        |   ^  |\\\n"
         "    |   2    | |        | <-|->/ |q\n"
         "    |   '    | |\\       `   v '  |\n"
         "    |  /-\\   | |d|      |     | _/e\n"
         "    |  |a|   |  \\|      |     //\n"
         "    |  \\-/   |   |      |    |/\n"
         "    |        |   |      |    |\n"
         "    | 3 h 4  |   |       \\   /\n"
         "    |        |   |        --'\n"
         "    |        |   |        _...______________,...\n"
         "    |   1    |   |      ,'                      `-\n"
         "    |        |   |     /   ^      3 h 4       q   `.\n"
         "    |   2    |  /      | <-|->             e    a  |\n"
         "    |        |-/       \\   v                 d     /\n"
         "    '`'''''''           \\                        ,'\n"
         "     _     _             `-..-'------------`...-'\n"
         "   ,'       `.\n"
         "  ,' t     y '.     0: toggles arrow keys between\n"
         "  V           V        IR/nunchuk/classic/motion plus\n"
         "                    ESC: quit\n\n");
}

static void input_sdl_unload(void)
{
  SDL_Quit();
}

int up, down, left, right;
bool steerright, steerleft;
double steerang = (PI / 2);
int arrow_function = 0;
bool togglekey0 = 0;
bool togglekey9 = 0;
bool shift;

static bool input_sdl_poll_event(struct input_event *out_event)
{
  SDL_Event event;
  if (!SDL_PollEvent(&event))
  {
    return false;
  }

  switch (event.type)
  {
  case SDL_KEYDOWN:
  case SDL_KEYUP:
    out_event->type = INPUT_EVENT_TYPE_BUTTON;
    out_event->button_event.pressed = (event.type == SDL_KEYDOWN);

    switch (event.key.keysym.sym)
    {
    case SDLK_ESCAPE:
      if (event.type != SDL_KEYDOWN)
      {
        return false;
      }

      out_event->type = INPUT_EVENT_TYPE_EMULATOR_CONTROL;
      if (shift)
      {
        out_event->emulator_control_event.control = INPUT_EMULATOR_CONTROL_POWER_OFF;
      }
      else
      {
        out_event->emulator_control_event.control = INPUT_EMULATOR_CONTROL_QUIT;
      }
      break;
    case SDLK_0:
      if (event.type == SDL_KEYUP)
      {
        togglekey0 = 0;
        return false;
      }
      else if (togglekey0 == 0)
      {
        togglekey0 = 1;
        arrow_function = (arrow_function + 1) % 4;
        printf("arrows (IR, nunchuk, classic, wmp): %d \n", arrow_function);

        out_event->type = INPUT_EVENT_TYPE_HOTPLUG;
        if (arrow_function == 1)
        {
          out_event->hotplug_event.extension = Nunchuk;
        }
        else if (arrow_function == 2)
        {
          out_event->hotplug_event.extension = Classic;
        }
        else if (arrow_function == 3)
        {
          out_event->hotplug_event.extension = Nunchuk;
        }
        else
        {
          out_event->hotplug_event.extension = NoExtension;
        }
      }
      break;
    case SDLK_9:
      if (event.type == SDL_KEYUP)
      {
        togglekey9 = 0;
        return false;
      }
      else if (togglekey9 == 0)
      {
        togglekey9 = 1;

        out_event->type = INPUT_EVENT_TYPE_EMULATOR_CONTROL;
        out_event->emulator_control_event.control = INPUT_EMULATOR_CONTROL_TOGGLE_REPORTS;
      }
      break;
    case SDLK_LSHIFT:
      shift = (event.type == SDL_KEYDOWN);

      out_event->type = INPUT_EVENT_TYPE_ANALOG_MOTION;
      out_event->analog_motion_event.moving = !shift;
      out_event->analog_motion_event.motion = INPUT_ANALOG_MOTION_MOTIONPLUS_SLOW;
      break;
    case SDLK_a:
      if (arrow_function == 2)
      {
        out_event->button_event.button = INPUT_BUTTON_CLASSIC_A;
      }
      else
      {
        out_event->button_event.button = INPUT_BUTTON_WIIMOTE_A;
      }
      break;
    case SDLK_d:
      if (arrow_function == 2)
      {
        out_event->button_event.button = INPUT_BUTTON_CLASSIC_B;
      }
      else
      {
        out_event->button_event.button = INPUT_BUTTON_WIIMOTE_B;
      }
      break;
    case SDLK_q:
      if (arrow_function == 2)
      {
        out_event->button_event.button = INPUT_BUTTON_CLASSIC_X;
      }
      else
      {
        out_event->button_event.button = INPUT_BUTTON_NUNCHUK_C;
      }
      break;
    case SDLK_e:
      if (arrow_function == 2)
      {
        out_event->button_event.button = INPUT_BUTTON_CLASSIC_Y;
      }
      else
      {
        out_event->button_event.button = INPUT_BUTTON_NUNCHUK_Z;
      }
      break;
    case SDLK_1:
      out_event->button_event.button = INPUT_BUTTON_WIIMOTE_1;
      break;
    case SDLK_2:
      out_event->button_event.button = INPUT_BUTTON_WIIMOTE_2;
      break;
    case SDLK_3:
      if (arrow_function == 2)
      {
        out_event->button_event.button = INPUT_BUTTON_CLASSIC_MINUS;
      }
      else
      {
        out_event->button_event.button = INPUT_BUTTON_WIIMOTE_MINUS;
      }
      break;
    case SDLK_4:
      if (arrow_function == 2)
      {
        out_event->button_event.button = INPUT_BUTTON_CLASSIC_PLUS;
      }
      else
      {
        out_event->button_event.button = INPUT_BUTTON_WIIMOTE_PLUS;
      }
      break;
    case SDLK_h:
      out_event->button_event.button = INPUT_BUTTON_HOME;
      break;
    case SDLK_KP8:
      if (arrow_function == 2)
      {
        out_event->button_event.button = INPUT_BUTTON_CLASSIC_UP;
      }
      else
      {
        out_event->button_event.button = INPUT_BUTTON_WIIMOTE_UP;
      }
      break;
    case SDLK_KP2:
      if (arrow_function == 2)
      {
        out_event->button_event.button = INPUT_BUTTON_CLASSIC_DOWN;
      }
      else
      {
        out_event->button_event.button = INPUT_BUTTON_WIIMOTE_DOWN;
      }
      break;
    case SDLK_KP4:
      if (arrow_function == 2)
      {
        out_event->button_event.button = INPUT_BUTTON_CLASSIC_LEFT;
      }
      else
      {
        out_event->button_event.button = INPUT_BUTTON_WIIMOTE_LEFT;
      }
      break;
    case SDLK_KP6:
      if (arrow_function == 2)
      {
        out_event->button_event.button = INPUT_BUTTON_CLASSIC_RIGHT;
      }
      else
      {
        out_event->button_event.button = INPUT_BUTTON_WIIMOTE_RIGHT;
      }
      break;

    case SDLK_UP:
      out_event->type = INPUT_EVENT_TYPE_ANALOG_MOTION;
      out_event->analog_motion_event.moving = (event.type == SDL_KEYDOWN);
      if (arrow_function == 0)
      {
        out_event->analog_motion_event.motion = INPUT_ANALOG_MOTION_IR_UP;
      }
      else if (arrow_function == 1)
      {
        out_event->analog_motion_event.motion = INPUT_ANALOG_MOTION_NUNCHUK_UP;
      }
      else if (arrow_function == 2)
      {
        out_event->analog_motion_event.motion = INPUT_ANALOG_MOTION_CLASSIC_LEFT_STICK_UP;
      }
      else
      {
        out_event->analog_motion_event.motion = INPUT_ANALOG_MOTION_MOTIONPLUS_UP;
      }
      break;
    case SDLK_DOWN:
      out_event->type = INPUT_EVENT_TYPE_ANALOG_MOTION;
      out_event->analog_motion_event.moving = (event.type == SDL_KEYDOWN);
      if (arrow_function == 0)
      {
        out_event->analog_motion_event.motion = INPUT_ANALOG_MOTION_IR_DOWN;
      }
      else if (arrow_function == 1)
      {
        out_event->analog_motion_event.motion = INPUT_ANALOG_MOTION_NUNCHUK_DOWN;
      }
      else if (arrow_function == 2)
      {
        out_event->analog_motion_event.motion = INPUT_ANALOG_MOTION_CLASSIC_LEFT_STICK_DOWN;
      }
      else
      {
        out_event->analog_motion_event.motion = INPUT_ANALOG_MOTION_MOTIONPLUS_DOWN;
      }
      break;
    case SDLK_LEFT:
      out_event->type = INPUT_EVENT_TYPE_ANALOG_MOTION;
      out_event->analog_motion_event.moving = (event.type == SDL_KEYDOWN);
      if (arrow_function == 0)
      {
        out_event->analog_motion_event.motion = INPUT_ANALOG_MOTION_IR_LEFT;
      }
      else if (arrow_function == 1)
      {
        out_event->analog_motion_event.motion = INPUT_ANALOG_MOTION_NUNCHUK_LEFT;
      }
      else if (arrow_function == 2)
      {
        out_event->analog_motion_event.motion = INPUT_ANALOG_MOTION_CLASSIC_LEFT_STICK_LEFT;
      }
      else
      {
        out_event->analog_motion_event.motion = INPUT_ANALOG_MOTION_MOTIONPLUS_LEFT;
      }
      break;
    case SDLK_RIGHT:
      out_event->type = INPUT_EVENT_TYPE_ANALOG_MOTION;
      out_event->analog_motion_event.moving = (event.type == SDL_KEYDOWN);
      if (arrow_function == 0)
      {
        out_event->analog_motion_event.motion = INPUT_ANALOG_MOTION_IR_RIGHT;
      }
      else if (arrow_function == 1)
      {
        out_event->analog_motion_event.motion = INPUT_ANALOG_MOTION_NUNCHUK_RIGHT;
      }
      else if (arrow_function == 2)
      {
        out_event->analog_motion_event.motion = INPUT_ANALOG_MOTION_CLASSIC_LEFT_STICK_RIGHT;
      }
      else
      {
        out_event->analog_motion_event.motion = INPUT_ANALOG_MOTION_MOTIONPLUS_RIGHT;
      }
      break;

    case SDLK_t:
      out_event->type = INPUT_EVENT_TYPE_ANALOG_MOTION;
      out_event->analog_motion_event.moving = (event.type == SDL_KEYDOWN);
      out_event->analog_motion_event.motion = INPUT_ANALOG_MOTION_STEER_LEFT;
      break;
    case SDLK_y:
      out_event->type = INPUT_EVENT_TYPE_ANALOG_MOTION;
      out_event->analog_motion_event.moving = (event.type == SDL_KEYDOWN);
      out_event->analog_motion_event.motion = INPUT_ANALOG_MOTION_STEER_RIGHT;
      break;

    default:
      return false;
    }
    return true;
  default:
    return false;
  }
}

struct input_source input_source_sdl = {
  .unload = input_sdl_unload,
  .poll_event = input_sdl_poll_event
};