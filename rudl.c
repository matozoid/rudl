/* RUDL - a C library wrapping SDL for use in Ruby. Copyright (C) 2001  Danny van Bruggen */
#include "rudl.h"
#include "rudl_audio.h"
#include "rudl_cdrom.h"
#include "rudl_events.h"
#include "rudl_joystick.h"
#include "rudl_keyboard.h"
//#include "rudl_mappy.h"
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
#ifdef HAVE_SMPEG_SMPEG_H
	#include "smpeg/smpeg.h"
#endif
#ifdef HAVE_SDL_GFXPRIMITIVES_H
	#include "SDL_gfxPrimitives.h"
#endif
#ifdef HAVE_SDL_FRAMERATE_H
	#include "SDL_framerate.h"
#endif
#ifdef HAVE_SDL_IMAGEFILTER_H
	#include "SDL_imageFilter.h"
#endif
#ifdef HAVE_SDL_ROTOZOOM_H
	#include "SDL_rotozoom.h"
#endif
//

bool sDLInitWasCalled=false;

VALUE id_new;
VALUE id_clone;
ID id_begin; // For ranges
ID id_end;
VALUE moduleRUDL;
VALUE classPit;
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
		SDL_RegisterApp("RUDL window", 0, GetModuleHandle(NULL));
#endif
#if defined(macintosh)
#if !TARGET_API_MAC_CARBON
		SDL_InitQuickDraw(&qd);
#endif
#endif		
		rb_eval_string("Kernel.at_exit {RUDL.at_exit}");
		sDLInitWasCalled=true;
	}
}

static VALUE RUDL_at_exit(VALUE obj)
{
	DEBUG_S("Reached RUDL_at_exit");
	quitJoystick();
#ifdef HAVE_SDL_MIXER_H
	quitAudio();
#endif
	quitVideo();
	quitTTF();
	DEBUG_S("Quitting the rest of it");
	SDL_Quit();
	return Qnil;
}

__inline__ Uint32 PARAMETER2FLAGS(VALUE flagsArg)
{
	Uint32 flags=0;
	int i;
	VALUE tmp;

	if(rb_obj_is_kind_of(flagsArg, rb_cArray)){
		for(i=0; i<RARRAY(flagsArg)->len; i++){
			tmp=rb_ary_entry(flagsArg, i);
			flags|=NUM2UINT(tmp);
		}
	}else{
		flags=NUM2UINT(flagsArg);
	}
	return flags;
}


__inline__ VALUE rb_range_first(VALUE obj)
{
    return rb_ivar_get(obj, id_begin);
}

__inline__ VALUE rb_range_last(VALUE obj)
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

static VALUE RUDL_versions(VALUE self)
{
	char versions[8192]; // Ugh
	sprintf(versions, "{"
		"'RUDL'=>RUDL::Version.new(%i,%i,%i),\n"
		"'SDL'=>RUDL::Version.new(%i,%i,%i),\n"
		"'BitMask'=>RUDL::Version.new,\n"
#ifdef HAVE_SDL_IMAGE_H
		"'SDL_image'=>RUDL::Version.new,\n"
#endif
#ifdef HAVE_SDL_MIXER_H
		"'SDL_mixer'=>RUDL::Version.new(%i, %i),\n"
#endif
#ifdef HAVE_SDL_TTF_H
		"'SDL_ttf'=>RUDL::Version.new,\n"
#endif
#ifdef HAVE_SDL_GFXPRIMITIVES_H
		"'SDL_gfxPrimitives'=>RUDL::Version.new(%i, %i),\n"
#endif
#ifdef HAVE_SDL_FRAMERATE_H
// not used	"'SDL_framerate'=>RUDL::Version.new,\n"
#endif
#ifdef HAVE_SDL_IMAGEFILTER_H
//		"'SDL_imagefilter'=>RUDL::Version.new,\n"
#endif
#ifdef HAVE_SDL_ROTOZOOM_H
		"'SDL_rotozoom'=>RUDL::Version.new,\n"
#endif
#ifdef HAVE_SMPEG_SMPEG_H
		"'smpeg'=>RUDL::Version.new(%i, %i, %i),\n"
#endif
	"}"

	,RUDLVERSION_MAJOR, RUDLVERSION_MINOR, RUDLVERSION_PATCH
	,SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_PATCHLEVEL
	// no version info (BitMask)
#ifdef HAVE_SDL_IMAGE_H
	// no version info
#endif
#ifdef HAVE_SDL_MIXER_H
	,MIX_MAJOR_VERSION ,MIX_MINOR_VERSION
#endif
#ifdef HAVE_SDL_TTF_H
	// no version info
#endif
#ifdef HAVE_SDL_GFXPRIMITIVES_H
	,SDL_GFXPRIMITIVES_MAJOR, SDL_GFXPRIMITIVES_MINOR
#endif
#ifdef HAVE_SDL_FRAMERATE_H
	// no version info
#endif
#ifdef HAVE_SDL_IMAGEFILTER_H
	// no version info
#endif
#ifdef HAVE_SDL_ROTOZOOM_H
	// no version info
#endif
#ifdef HAVE_SMPEG_SMPEG_H
	,SMPEG_MAJOR_VERSION, SMPEG_MINOR_VERSION, SMPEG_PATCHLEVEL
#endif
	);
	
	return rb_eval_string(versions);
}

//////////////////////////////////

/*
=begin
<<< docs/head
= RUDL
Module (({RUDL})) contains all RUDL classes as inner classes.
It has some class methods of its own too.
== Class Methods
--- RUDL.version and RUDL.used_libraries
No longer implemented, use versions instead.
--- RUDL.versions
Returns hash of librarynames with their versions that are supported by RUDL.
This list was determined when RUDL was compiled for a certain system,
and might change when other libraries have been installed or removed
and RUDL is recompiled.
Versions are RUDL::Version objects.
This includes "RUDL" itself.
= Pit
The (({Pit})) is where the things end up that don't fit anywhere else.
The methods are defined where they fit best.
= Version
(({Version})) is the class used for version comparisons.
It defines four version levels: major, minor, patch and deepest.
--- Version#initialize( major=0, minor=0, patch=0, deepest=0 )
Initializes a new Version object.
--- Version#<( v )
Compares this version number with the one in ((|v|)).
Returns ((|true|)) if older.
--- Version#to_s
Returns the version as a string: "major.minor.patch.deepest"
= SDLError
SDLError is the class that is thrown when SDL or RUDL find an SDL-specific
problem.
= Constants
All for the hacked ((|init_subsystem|)) and ((|quit_subsystem|)),
which should officially never be needed:

INIT_TIMER, INIT_AUDIO, INIT_VIDEO, INIT_CDROM, INIT_JOYSTICK, INIT_NOPARACHUTE, INIT_EVERYTHING
=end */

DECKLSPECKL void Init_RUDL()
{
	DEBUG_S("Init_RUDL()");
	moduleRUDL=rb_define_module("RUDL");

	rb_define_singleton_method(moduleRUDL, "at_exit", RUDL_at_exit, 0);
	rb_define_singleton_method(moduleRUDL, "init_subsystem", RUDL_init, 1);
	rb_define_singleton_method(moduleRUDL, "quit_subsystem", RUDL_quit, 1);
	rb_define_singleton_method(moduleRUDL, "is_subsystem_init", RUDL_is_init, 1);
	rb_define_singleton_method(moduleRUDL, "versions", RUDL_versions, 0);

	classSDLError=rb_define_class("SDLError", rb_eStandardError);
	moduleConstant=rb_define_module_under(moduleRUDL, "Constant");

	classPit=rb_define_class_under(moduleRUDL, "Pit", rb_cObject);

	rb_eval_string(
		"module RUDL\n"
		"	class Version\n"
		"		attr_accessor :major, :minor, :patch, :deepest\n"
		"		def <(v)\n"
		"			(@major<v.major) or\n"
		"			(@major==v.major and @minor<v.minor) or\n"
		"			(@major==v.major and @minor==v.minor and @patch<v.patch) or\n"
		"			(@major==v.major and @minor==v.minor and @patch==v.patch and @deepest<v.deepest)\n"
		"		end\n"
		"		def initialize(major=0, minor=0, patch=0, deepest=0)\n"
		"			@major=major\n"
		"			@minor=minor\n"
		"			@patch=patch\n"
		"			@deepest=deepest\n"
		"		end\n"
		"		def to_s\n"
		"			\"#{major}.#{minor}.#{patch}.#{deepest}\"\n"
		"		end\n"
		"	end\n"
		"end\n"
	);


	id_begin=rb_intern("begin");
	id_end=rb_intern("end");
	id_new=rb_intern("new");
	id_clone=rb_intern("clone");

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
	initOverlay();
	initSDL();
}

/*
Note to self: RDOC tags
= H1
== H2
=== H3
==== H4
+ H5
  (indented) SOURCE
* UL LI
(1) OL LI
--- Class#|.method
((|var|))
(({class}))
((*emphasis*))
((< >))
((<URL:http://enz>))
*/