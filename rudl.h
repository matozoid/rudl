/* RUDL - a C library wrapping SDL for use in Ruby. Copyright (C) 2001  Danny van Bruggen */
#ifndef _RUBY_SDL_H
#define _RUBY_SDL_H

#define RUDLVERSION 0.51
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
#else
	#define DEBUG_I(____i) {}
	#define DEBUG_S(____s) {}
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

#define NUM2BOOL(____b) ((____b)==Qtrue? true:false)
#define INT2BOOL(____b) ((____b) ? Qtrue : Qfalse)
#define DBL2NUM(x) rb_float_new(x)
#define CSTR2STR(x) rb_str_new2(x)

#define DEC_CONST(x)  rb_define_const(moduleConstant, #x, UINT2NUM(SDL_##x));
#define DEC_CONSTK(x) rb_define_const(moduleConstant, #x, UINT2NUM(SDL##x));
#define DEC_CONSTN(x) rb_define_const(moduleConstant, #x, UINT2NUM(x));

#ifdef __cplusplus
}
#endif
#endif
