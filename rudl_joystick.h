/* RUDL - a C library wrapping SDL for use in Ruby. Copyright (C) 2001  Danny van Bruggen */
#include "rudl.h"

VALUE classJoystick;
VALUE classJoyAxisEvent;
VALUE classJoyBallEvent;
VALUE classJoyHatEvent;
VALUE classJoyButtonUpEvent;
VALUE classJoyButtonDownEvent;

extern void initJoystickClasses();
extern void quitJoystick();
