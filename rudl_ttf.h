/* RUDL - a C library wrapping SDL for use in Ruby. Copyright (C) 2001  Danny van Bruggen */
#include "rudl.h"

#ifdef HAVE_SDL_TTF_H
#include <SDL_ttf.h>
#endif

VALUE classTTF;

extern void initTrueTypeFontClasses();
extern void initTTF();
extern void quitTTF();
