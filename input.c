#include "input.h"

#include "SDL/SDL.h"
#include <math.h>

int ir_up, ir_down, ir_left, ir_right,
    steer_left, steer_right,
    nunchuk_up, nunchuk_down, nunchuk_left, nunchuk_right,
    classic_left_stick_up, classic_left_stick_down, classic_left_stick_left, classic_left_stick_right,
    motionplus_up, motionplus_down, motionplus_left, motionplus_right, motionplus_slow; 
double steer_angle = (PI / 2);
extern int show_reports;

int input_update(struct wiimote_state *state, struct input_source const * source)
{
  struct input_event event;

  /* Loop through waiting messages and process them */

  while (source->poll_event(&event))
  {
    switch (event.type)
    {
    case INPUT_EVENT_TYPE_EMULATOR_CONTROL:
      switch (event.emulator_control_event.control)
      {
      case INPUT_EMULATOR_CONTROL_QUIT:
        return -1;
      case INPUT_EMULATOR_CONTROL_POWER_OFF:
        return -2;
      case INPUT_EMULATOR_CONTROL_TOGGLE_REPORTS:
        show_reports = (show_reports + 1) % 2;
        break;
      }
      break;
    case INPUT_EVENT_TYPE_HOTPLUG:
      switch (event.hotplug_event.extension)
      {
      case Nunchuk:
      case Classic:
      case BalanceBoard:
        ir_object_clear(state, 0);
        ir_object_clear(state, 1);
        ir_object_clear(state, 2);
        ir_object_clear(state, 3);
        break;
      case NoExtension:
        state->usr.ir_object[0].x = 400;
        state->usr.ir_object[0].y = 400;
        state->usr.ir_object[0].size = 8;
        state->usr.ir_object[1].x = 600;
        state->usr.ir_object[1].y = 400;
        state->usr.ir_object[1].size = 8;
        break;
      default:
        goto invalid;
      }

      state->usr.connected_extension_type = event.hotplug_event.extension;
    invalid:
      break;
    case INPUT_EVENT_TYPE_BUTTON: {
      bool pressed = event.button_event.pressed;
      switch (event.button_event.button)
      {
      case INPUT_BUTTON_HOME:
        state->usr.home = pressed;
        break;

      case INPUT_BUTTON_WIIMOTE_UP:
        state->usr.up = pressed;
        break;
      case INPUT_BUTTON_WIIMOTE_DOWN:
        state->usr.down = pressed;
        break;
      case INPUT_BUTTON_WIIMOTE_LEFT:
        state->usr.left = pressed;
        break;
      case INPUT_BUTTON_WIIMOTE_RIGHT:
        state->usr.right = pressed;
        break;
      case INPUT_BUTTON_WIIMOTE_A:
        state->usr.a = pressed;
        break;
      case INPUT_BUTTON_WIIMOTE_B:
        state->usr.b = pressed;
        break;
      case INPUT_BUTTON_WIIMOTE_1:
        state->usr.one = pressed;
        break;
      case INPUT_BUTTON_WIIMOTE_2:
        state->usr.two = pressed;
        break;
      case INPUT_BUTTON_WIIMOTE_PLUS:
        state->usr.plus = pressed;
        break;
      case INPUT_BUTTON_WIIMOTE_MINUS:
        state->usr.minus = pressed;
        break;

      case INPUT_BUTTON_NUNCHUK_C:
        state->usr.nunchuk.c = pressed;
        break;
      case INPUT_BUTTON_NUNCHUK_Z:
        state->usr.nunchuk.z = pressed;
        break;

      case INPUT_BUTTON_CLASSIC_UP:
        state->usr.classic.up = pressed;
        break;
      case INPUT_BUTTON_CLASSIC_DOWN:
        state->usr.classic.down = pressed;
        break;
      case INPUT_BUTTON_CLASSIC_LEFT:
        state->usr.classic.left = pressed;
        break;
      case INPUT_BUTTON_CLASSIC_RIGHT:
        state->usr.classic.right = pressed;
        break;
      case INPUT_BUTTON_CLASSIC_A:
        state->usr.classic.a = pressed;
        break;
      case INPUT_BUTTON_CLASSIC_B:
        state->usr.classic.b = pressed;
        break;
      case INPUT_BUTTON_CLASSIC_X:
        state->usr.classic.x = pressed;
        break;
      case INPUT_BUTTON_CLASSIC_Y:
        state->usr.classic.y = pressed;
        break;
      case INPUT_BUTTON_CLASSIC_L:
        state->usr.classic.ltrigger = pressed;
        break;
      case INPUT_BUTTON_CLASSIC_R:
        state->usr.classic.rtrigger = pressed;
        break;
      case INPUT_BUTTON_CLASSIC_ZL:
        state->usr.classic.lz = pressed;
        break;
      case INPUT_BUTTON_CLASSIC_ZR:
        state->usr.classic.rz = pressed;
        break;
      case INPUT_BUTTON_CLASSIC_PLUS:
        state->usr.classic.plus = pressed;
        break;
      case INPUT_BUTTON_CLASSIC_MINUS:
        state->usr.classic.minus = pressed;
        break;
      default:
        printf("warning: button %d not handled by input_update\n", event.button_event.button);
        break;
      }
      break;
    }
    case INPUT_EVENT_TYPE_ANALOG_MOTION: {
      bool moving = event.analog_motion_event.moving;
      switch (event.analog_motion_event.motion)
      {
        case INPUT_ANALOG_MOTION_IR_UP:
          ir_up = moving;
          break;
        case INPUT_ANALOG_MOTION_IR_DOWN:
          ir_down = moving;
          break;
        case INPUT_ANALOG_MOTION_IR_LEFT:
          ir_left = moving;
          break;
        case INPUT_ANALOG_MOTION_IR_RIGHT:
          ir_right = moving;
          break;

        case INPUT_ANALOG_MOTION_STEER_LEFT:
          steer_left = moving;
          break;
        case INPUT_ANALOG_MOTION_STEER_RIGHT:
          steer_right = moving;
          break;

        case INPUT_ANALOG_MOTION_NUNCHUK_UP:
          nunchuk_up = moving;
          break;
        case INPUT_ANALOG_MOTION_NUNCHUK_DOWN:
          nunchuk_down = moving;
          break;
        case INPUT_ANALOG_MOTION_NUNCHUK_LEFT:
          nunchuk_left = moving;
          break;
        case INPUT_ANALOG_MOTION_NUNCHUK_RIGHT:
          nunchuk_right = moving;
          break;

        case INPUT_ANALOG_MOTION_CLASSIC_LEFT_STICK_UP:
          classic_left_stick_up = moving;
          break;
        case INPUT_ANALOG_MOTION_CLASSIC_LEFT_STICK_DOWN:
          classic_left_stick_down = moving;
          break;
        case INPUT_ANALOG_MOTION_CLASSIC_LEFT_STICK_LEFT:
          classic_left_stick_left = moving;
          break;
        case INPUT_ANALOG_MOTION_CLASSIC_LEFT_STICK_RIGHT:
          classic_left_stick_right = moving;
          break;

        case INPUT_ANALOG_MOTION_MOTIONPLUS_UP:
          motionplus_up = moving;
          break;
        case INPUT_ANALOG_MOTION_MOTIONPLUS_DOWN:
          motionplus_down = moving;
          break;
        case INPUT_ANALOG_MOTION_MOTIONPLUS_LEFT:
          motionplus_left = moving;
          break;
        case INPUT_ANALOG_MOTION_MOTIONPLUS_RIGHT:
          motionplus_right = moving;
          break;
        case INPUT_ANALOG_MOTION_MOTIONPLUS_SLOW:
          motionplus_slow = moving;
          break;
      }
      break;
    }
    default:
      break;
    }
  }

  if ((steer_left && steer_right) || (!steer_left && !steer_right))
  {
    steer_angle = (PI / 2);
  }
  else if (steer_left)
  {
    steer_angle = (6 * PI / 8);
  }
  else if (steer_right)
  {
    steer_angle = (2 * PI / 8);
  }

  /*

       if (steer_left)
       {
       if (steer_angle < (7 * PI / 8))
       steer_angle += 0.02;
       state->usr.accel_y = -cos(steer_angle) * (0x19 << 2) + 0x200;
       state->usr.accel_x = -sin(steer_angle) * (0x19 << 2) + 0x200;
       }

       if (steer_right)
       {
       if (steer_angle > (1 * PI / 8))
       steer_angle -= 0.02;
       state->usr.accel_y = -cos(steer_angle) * (0x19 << 2) + 0x200;
       state->usr.accel_x = -sin(steer_angle) * (0x19 << 2) + 0x200;
       }

*/

  //state->usr.accel_y = -cos(steer_angle) * (0x19 << 2) + 0x200;
  //state->usr.accel_x = -sin(steer_angle) * (0x19 << 2) + 0x200;

  if (ir_down)
  {
    if (state->usr.ir_object[0].y < 764)
    {
      state->usr.ir_object[0].y += 4;
      state->usr.ir_object[1].y += 4;
    }
  }
  if (ir_up)
  {
    if (state->usr.ir_object[0].x > 3)
    {
      state->usr.ir_object[0].y -= 4;
      state->usr.ir_object[1].y -= 4;
    }
  }
  if (ir_left)
  {
    if (state->usr.ir_object[0].x < 1020)
    {
      state->usr.ir_object[0].x += 4;
      state->usr.ir_object[1].x += 4;
    }
  }
  if (ir_right)
  {
    if (state->usr.ir_object[0].x > 3)
    {
      state->usr.ir_object[0].x -= 4;
      state->usr.ir_object[1].x -= 4;
    }
  }

  state->usr.nunchuk.x = 128 + nunchuk_right * 100 - nunchuk_left * 100;
  state->usr.nunchuk.y = 128 + nunchuk_up * 100 - nunchuk_down * 100;

  state->usr.classic.ls_x = 32 + classic_left_stick_right * 30 - classic_left_stick_left * 30;
  state->usr.classic.ls_y = 32 + classic_left_stick_up * 30 - classic_left_stick_down * 30;

  state->usr.motionplus.pitch_left = 0x1F7F + motionplus_down * 800 * (1 + !motionplus_slow) - motionplus_up * 800 * (1 + !motionplus_slow);
  state->usr.motionplus.yaw_down = 0x1F7F + motionplus_left * 800 * (1 + !motionplus_slow) - motionplus_right * 800 * (1 + !motionplus_slow);
  state->usr.motionplus.pitch_slow = motionplus_slow;
  state->usr.motionplus.yaw_slow = motionplus_slow;

  return 0;
}
