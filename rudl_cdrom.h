/* RUDL - a C library wrapping SDL for use in Ruby. Copyright (C) 2001  Danny van Bruggen */
#include "rudl.h"

VALUE classCD;
VALUE classCDROM;

extern void initCDClasses();
extern void initCD();
extern void quitCD();
