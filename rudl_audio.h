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
extern int rudl_convert_audio(
		Uint8* source, int source_length,
		Uint8** destination, int* destination_length,
		Uint16 src_format, Uint8 src_channels, int src_rate,
		Uint16 dst_format, Uint8 dst_channels, int dst_rate);