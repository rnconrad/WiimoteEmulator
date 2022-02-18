#ifndef INPUT_H
#define INPUT_H

#include <stdbool.h>
#include "wiimote.h"

#define PI 3.141592654

enum input_event_type
{
    INPUT_EVENT_TYPE_EMULATOR_CONTROL,
    INPUT_EVENT_TYPE_HOTPLUG,
    INPUT_EVENT_TYPE_BUTTON,
    INPUT_EVENT_TYPE_ANALOG_MOTION,
};

enum input_emulator_control
{
    INPUT_EMULATOR_CONTROL_QUIT, // Quits the emulator
    INPUT_EMULATOR_CONTROL_POWER_OFF, // Powers off host

    INPUT_EMULATOR_CONTROL_TOGGLE_REPORTS,
};

struct input_emulator_control_event
{
    enum input_emulator_control control;
};

struct input_hotplug_event
{
    enum wiimote_connected_extension_type extension;
};

enum input_button
{
    INPUT_BUTTON_HOME,

    INPUT_BUTTON_WIIMOTE_UP,
    INPUT_BUTTON_WIIMOTE_DOWN,
    INPUT_BUTTON_WIIMOTE_LEFT,
    INPUT_BUTTON_WIIMOTE_RIGHT,
    INPUT_BUTTON_WIIMOTE_A,
    INPUT_BUTTON_WIIMOTE_B,
    INPUT_BUTTON_WIIMOTE_1,
    INPUT_BUTTON_WIIMOTE_2,
    INPUT_BUTTON_WIIMOTE_PLUS,
    INPUT_BUTTON_WIIMOTE_MINUS,

    INPUT_BUTTON_NUNCHUK_C,
    INPUT_BUTTON_NUNCHUK_Z,

    INPUT_BUTTON_CLASSIC_UP,
    INPUT_BUTTON_CLASSIC_DOWN,
    INPUT_BUTTON_CLASSIC_LEFT,
    INPUT_BUTTON_CLASSIC_RIGHT,
    INPUT_BUTTON_CLASSIC_A,
    INPUT_BUTTON_CLASSIC_B,
    INPUT_BUTTON_CLASSIC_X,
    INPUT_BUTTON_CLASSIC_Y,
    INPUT_BUTTON_CLASSIC_L,
    INPUT_BUTTON_CLASSIC_R,
    INPUT_BUTTON_CLASSIC_ZL,
    INPUT_BUTTON_CLASSIC_ZR,
    INPUT_BUTTON_CLASSIC_PLUS,
    INPUT_BUTTON_CLASSIC_MINUS,
};

struct input_button_event
{
    bool pressed;
    enum input_button button;
};

enum input_analog_motion
{
    INPUT_ANALOG_MOTION_IR_UP,
    INPUT_ANALOG_MOTION_IR_DOWN,
    INPUT_ANALOG_MOTION_IR_LEFT,
    INPUT_ANALOG_MOTION_IR_RIGHT,

    INPUT_ANALOG_MOTION_POINTER,

    INPUT_ANALOG_MOTION_STEER_LEFT,
    INPUT_ANALOG_MOTION_STEER_RIGHT,

    INPUT_ANALOG_MOTION_NUNCHUK_UP,
    INPUT_ANALOG_MOTION_NUNCHUK_DOWN,
    INPUT_ANALOG_MOTION_NUNCHUK_LEFT,
    INPUT_ANALOG_MOTION_NUNCHUK_RIGHT,

    INPUT_ANALOG_MOTION_CLASSIC_LEFT_STICK_UP,
    INPUT_ANALOG_MOTION_CLASSIC_LEFT_STICK_DOWN,
    INPUT_ANALOG_MOTION_CLASSIC_LEFT_STICK_LEFT,
    INPUT_ANALOG_MOTION_CLASSIC_LEFT_STICK_RIGHT,

    INPUT_ANALOG_MOTION_MOTIONPLUS_UP,
    INPUT_ANALOG_MOTION_MOTIONPLUS_DOWN,
    INPUT_ANALOG_MOTION_MOTIONPLUS_LEFT,
    INPUT_ANALOG_MOTION_MOTIONPLUS_RIGHT,
    INPUT_ANALOG_MOTION_MOTIONPLUS_SLOW,
};

struct input_analog_motion_event
{
    bool moving;
    float delta_x;
    float delta_y;
    float delta_z;
    enum input_analog_motion motion;
};

struct input_event
{
    enum input_event_type type;
    union {
        struct input_emulator_control_event emulator_control_event;
        struct input_hotplug_event hotplug_event;
        struct input_button_event button_event;
        struct input_analog_motion_event analog_motion_event;
    };
};

struct input_source
{
    void (*unload)(void);
    bool (*poll_event)(struct input_event *event);
};

int input_update(struct wiimote_state * state, struct input_source const * source);

#endif
