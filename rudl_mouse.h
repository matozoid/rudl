/* RUDL - a C library wrapping SDL for use in Ruby. Copyright (C) 2001  Danny van Bruggen */
#include "rudl.h"

VALUE classMouse;
VALUE classMouseMotionEvent;
VALUE classMouseButtonUpEvent;
VALUE classMouseButtonDownEvent;

extern void initMouseClasses();