/* RUDL - a C library wrapping SDL for use in Ruby. Copyright (C) 2001  Danny van Bruggen */
#include "rudl.h"
#include "rudl_audio.h"
#include "rudl_cdrom.h"
#include "rudl_events.h"
#include "rudl_joystick.h"
#include "rudl_keyboard.h"
#include "rudl_mappy.h"
#include "rudl_mouse.h"
#include "rudl_sfont.h"
#include "rudl_timer.h"
#include "rudl_ttf.h"
#include "rudl_video.h"

// For getting versions:
#ifdef HAVE_SDL_MIXER_H
	#include "SDL_mixer.h"
	#ifndef MIX_MAJOR_VERSION
		#define MIX_MAJOR_VERSION 0
		#define MIX_MINOR_VERSION 0
	#endif
#endif
//

bool sDLInitWasCalled=false;

ID id_begin; // For ranges
ID id_end;
VALUE moduleRUDL;
VALUE classSDLError;
VALUE moduleConstant;

#if defined(macintosh)
#if !defined(__MWERKS__) && !TARGET_API_MAC_CARBON
QDGlobals qd;
#endif
#endif

///////////////////////////////// "CORE"

void initSDL()
{
	if(!sDLInitWasCalled){
#ifdef MS_WIN32
		SDL_RegisterApp("SDL window", 0, GetModuleHandle(NULL));
#endif
#if defined(macintosh)
#if !TARGET_API_MAC_CARBON
		SDL_InitQuickDraw(&qd);
#endif
#endif		
		//if(SDL_SetTimerThreaded(false)) SDL_RAISE;

		rb_eval_string("Kernel.at_exit {RUDL.at_exit}");
		sDLInitWasCalled=true;
	}
}

static VALUE RUDL_version(VALUE obj)
{
	return rb_float_new(RUDLVERSION);
}

static VALUE RUDL_at_exit(VALUE obj)
{
	quitJoystick();
#ifdef HAVE_SDL_MIXER_H
	quitAudio();
#endif
	quitTTF();
	SDL_Quit();
	return Qnil;
}

Uint32 PARAMETER2FLAGS(VALUE flagsArg)
{
	Uint32 flags=0;
	int i;

	if(rb_obj_is_kind_of(flagsArg, rb_cArray)){
		for(i=0; i<RARRAY(flagsArg)->len; i++){
			flags|=NUM2UINT(rb_ary_entry(flagsArg, i));
		}
	}else{
		flags=NUM2UINT(flagsArg);
	}
	return flags;
}


VALUE rb_range_first(VALUE obj)
{
    return rb_ivar_get(obj, id_begin);
}

VALUE rb_range_last(VALUE obj)
{
    return rb_ivar_get(obj, id_end);
}

VALUE RUDL_init(VALUE obj, VALUE flags)
{
	initSDL();
	return UINT2NUM(SDL_Init(PARAMETER2FLAGS(flags)));
}

VALUE RUDL_quit(VALUE obj, VALUE flags)
{
	SDL_QuitSubSystem(PARAMETER2FLAGS(flags));
	return obj;
}

VALUE RUDL_is_init(VALUE obj, VALUE flags)
{
	initSDL();
	return UINT2NUM(SDL_WasInit(PARAMETER2FLAGS(flags)));
}

VALUE RUDL_used_libraries(VALUE self)
{
	VALUE retval=rb_hash_new();
#ifdef HAVE_SDL_IMAGE_H
	rb_hash_aset(retval, CSTR2STR("SDL_image"), DBL2NUM(0.0));
#endif
#ifdef HAVE_SDL_MIXER_H
	rb_hash_aset(retval, CSTR2STR("SDL_mixer"), DBL2NUM(MIX_MAJOR_VERSION+(MIX_MINOR_VERSION/10.)));
#endif
#ifdef HAVE_SDL_TTF_H
	rb_hash_aset(retval, CSTR2STR("SDL_ttf"), DBL2NUM(0.0));
#endif
	return retval;
}

//////////////////////////////////

/*
=begin
= RUDL
Module (({RUDL})) contains all RUDL classes as inner classes.
It has some class methods of its own too.
== Class Methods
--- RUDL.version
    Returns a Float containing RUDL's version number.
--- RUDL.used_libraries
    Returns hash of librarynames with their versionnumbers that are supported by RUDL.
    This list was determined when RUDL was compiled for a certain system,
    and might change when other libraries have been installed or removed
    and RUDL is recompiled.
= SDLError
SDLError is the class that is thrown when SDL or RUDL find an SDL-specific
problem.
= Constants
All for the hacked ((|init|)):

INIT_TIMER, INIT_AUDIO, INIT_VIDEO, INIT_CDROM, INIT_JOYSTICK, INIT_NOPARACHUTE, INIT_EVERYTHING
=end */

void Init_RUDL()
{
	moduleRUDL=rb_define_module("RUDL");

	rb_define_singleton_method(moduleRUDL, "version", RUDL_version, 0);
	rb_define_singleton_method(moduleRUDL, "at_exit", RUDL_at_exit, 0);
	rb_define_singleton_method(moduleRUDL, "init_subsystem", RUDL_init, 1);
	rb_define_singleton_method(moduleRUDL, "quit_subsystem", RUDL_quit, 1);
	rb_define_singleton_method(moduleRUDL, "is_subsystem_init", RUDL_is_init, 1);
	rb_define_singleton_method(moduleRUDL, "used_libraries", RUDL_used_libraries, 0);

	classSDLError=rb_define_class("SDLError", rb_eStandardError);
	moduleConstant=rb_define_module_under(moduleRUDL, "Constant");

	id_begin=rb_intern("begin");
	id_end=rb_intern("end");

	DEC_CONST(INIT_TIMER);
	DEC_CONST(INIT_AUDIO);
	DEC_CONST(INIT_VIDEO);
	DEC_CONST(INIT_CDROM);
	DEC_CONST(INIT_JOYSTICK);
	DEC_CONST(INIT_NOPARACHUTE);
	DEC_CONST(INIT_EVERYTHING);

	initAudioClasses();
	initCDClasses();
	initEventsClasses();
	initTrueTypeFontClasses();
	initJoystickClasses();
	initKeyClasses();
	initMouseClasses();
	initTimerClasses();
	initVideoClasses();
	initSFontClasses();
	//initMappyClasses();
	initSDL();
}
