/* RUDL - a C library wrapping SDL for use in Ruby. Copyright (C) 2001  Danny van Bruggen */
#include "rudl.h"

#define RUDL_ENDMUSICEVENT (SDL_USEREVENT+1)

VALUE classSound;
VALUE classChannel;
VALUE classMixer;
VALUE classMusic;
VALUE classEndOfMusicEvent;

extern void initAudioClasses();
extern void quitAudio();
