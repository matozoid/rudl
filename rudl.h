/* RUDL - a C library wrapping SDL for use in Ruby. Copyright (C) 2001  Danny van Bruggen */
#ifndef _RUDL_H
#define _RUDL_H

#define RUDLVERSION_MAJOR 0
#define RUDLVERSION_MINOR 7
#define RUDLVERSION_PATCH 0
#define DEBUG_RUDL

#include "ruby.h"
#include "rubyio.h"

#include <SDL.h>

#ifdef __cplusplus
extern "C" {
#endif

#define bool int
#define false (0)
#define true (1)

#ifdef DEBUG_RUDL
	#define DEBUG_I(____i) {char koe[500];sprintf(koe, "puts '(%i)'", ____i); rb_eval_string(koe);}
	#define DEBUG_S(____s) {char koe[500];sprintf(koe, "puts '(%s)'", ____s); rb_eval_string(koe);}
	#define RUDL_ASSERT(____x, ____msg) {if(!(____x)){SDL_RAISE_S(____msg);}}
	#define RUDL_VERIFY(____x, ____msg) {if(!(____x)){SDL_RAISE_S(____msg);}}
	#define SDL_ASSERT(____x) {if(!(____x)){SDL_RAISE;}}
	#define SDL_VERIFY(____x) {if(!(____x)){SDL_RAISE;}}
#else
	#define DEBUG_I(____i) {}
	#define DEBUG_S(____s) {}
	#define RUDL_ASSERT(____x, ____msg) {}
	#define RUDL_VERIFY(____x, ____msg) {if(!(____x)){SDL_RAISE_S(____msg);}}
	#define SDL_ASSERT(____x) {}
	#define SDL_VERIFY(____x) {if(!(____x)){SDL_RAISE;}}
#endif

#ifdef WIN32
	#define DECKLSPECKL __declspec(dllexport)
#else
	#define DECKLSPECKL
#endif

#ifdef __cplusplus
	#define VALUEFUNC(f) ((VALUE (*)(void))f)
	#define VOIDFUNC(f) ((void (*)(...))f)
#else
	#define VALUEFUNC(f) (f)
	#define VOIDFUNC(f) (f)
#endif

extern Uint32 PARAMETER2FLAGS(VALUE flagsArg);

extern void initSDL();

extern VALUE moduleRUDL;
extern VALUE classSDLError;
extern VALUE moduleConstant;

extern VALUE rb_range_first(VALUE obj); // Why no predefined range-access functions?
extern VALUE rb_range_last(VALUE obj);

#define SDL_RAISE		{rb_raise(classSDLError, SDL_GetError());}
#define SDL_RAISE_S(____s)	{rb_raise(classSDLError, ____s);}

#define NUM2BOOL(____b)	((____b)==Qtrue? true:false)
#define INT2BOOL(____b)	((____b) ? Qtrue : Qfalse)
#define DBL2NUM(x)	rb_float_new(x)
#define FLT2NUM(x)	rb_float_new(x)
#define CSTR2STR(x)	rb_str_new2(x)
#define NUM2Sint32(x)	((Sint32)NUM2INT(x))
#define NUM2Uint32(x)	((Uint32)NUM2UINT(x))
#define NUM2Sint16(x)	((Sint16)NUM2INT(x))
#define NUM2Uint16(x)	((Uint16)NUM2UINT(x))
#define NUM2Sint8(x)	((Sint8)NUM2INT(x))
#define NUM2Uint8(x)	((Uint8)NUM2UINT(x))
#define NUM2FLT(x)	((float)NUM2DBL(x))

#define RUDL_MIN(a, b) ((a)<(b)?(a):(b))
#define RUDL_MAX(a, b) ((a)>(b)?(a):(b))

#define DEC_CONST(x)	rb_define_const(moduleConstant, #x, UINT2NUM(SDL_##x));
#define DEC_CONSTK(x)	rb_define_const(moduleConstant, #x, UINT2NUM(SDL##x));
#define DEC_CONSTN(x)	rb_define_const(moduleConstant, #x, UINT2NUM(x));

#ifdef __cplusplus
}
#endif
#endif
