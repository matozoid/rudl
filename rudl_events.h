/* RUDL - a C library wrapping SDL for use in Ruby. Copyright (C) 2001  Danny van Bruggen */
#include "rudl.h"

#ifdef __cplusplus
extern "C" {
#endif

VALUE classEvent;
//VALUE classUserEvent;
//VALUE classSysWMEvent;

VALUE classEventQueue;

extern void initEventsClasses();

#ifdef __cplusplus
}
#endif
