/* RUDL - a C library wrapping SDL for use in Ruby. Copyright (C) 2001, 2002  Danny van Bruggen */
#include "rudl_events.h"
#include "rudl_video.h"

ID id_rect, id_atx, id_aty, id_atw, id_ath;

void initVideo()
{
	if(!SDL_WasInit(SDL_INIT_VIDEO)){
		RUDL_ASSERT(!SDL_WasInit(SDL_INIT_AUDIO), "Always use video classes before starting with audio");
		RUDL_ASSERT(!SDL_WasInit(SDL_INIT_TIMER), "Always use video classes before using the timer");

		DEBUG_S("Starting video subsystem");
		SDL_VERIFY(!SDL_InitSubSystem(SDL_INIT_VIDEO));
		SDL_EnableUNICODE(1);
	}
}

void quitVideo()
{
	rb_eval_string(
		"ObjectSpace.each_object(RUDL::Surface) {|x|	\n"
		"	x.destroy if(x.class==RUDL::Surface)		\n"
		"}												\n");
	if(SDL_WasInit(SDL_INIT_VIDEO)){
		DEBUG_S("Stopping video subsystem");
		SDL_QuitSubSystem(SDL_INIT_VIDEO);
	}
}

__inline__ Uint32 VALUE2COLOR_NOMAP(VALUE colorObject)
{
	VALUE r,g,b,a;
	if(rb_obj_is_kind_of(colorObject, rb_cArray)){
		switch(RARRAY(colorObject)->len){
			case 3:
				r=rb_ary_entry(colorObject, 0);
				g=rb_ary_entry(colorObject, 1);
				b=rb_ary_entry(colorObject, 2);
				return (((Uint32)(NUM2UINT(r)))<<24)+
					(((Uint32)(NUM2UINT(g)))<<16)+
					(((Uint32)(NUM2UINT(b)))<<8)+
					((Uint32)0x000000ff);
				break;
			case 4:
				r=rb_ary_entry(colorObject, 0);
				g=rb_ary_entry(colorObject, 1);
				b=rb_ary_entry(colorObject, 2);
				a=rb_ary_entry(colorObject, 3);
				return (((Uint32)(NUM2UINT(r)))<<24)+
					(((Uint32)(NUM2UINT(g)))<<16)+
					(((Uint32)(NUM2UINT(b)))<<8)+
					((Uint32)(NUM2UINT(a)));
				break;
			default:
				rb_raise(rb_eTypeError, "Need colorarray with 3 or 4 elements");
				return Qnil;
		}
	}else{
		return NUM2UINT(colorObject);
	}
}

__inline__ Uint32 VALUE2COLOR(VALUE colorObject, SDL_PixelFormat* format)
{
	if(rb_obj_is_kind_of(colorObject, rb_cArray)){
		VALUE r,g,b,a;
		switch(RARRAY(colorObject)->len){
			case 3:
				r=rb_ary_entry(colorObject, 0);
				g=rb_ary_entry(colorObject, 1);
				b=rb_ary_entry(colorObject, 2);
				return SDL_MapRGB(format,
						(Uint8)NUM2UINT(r),
						(Uint8)NUM2UINT(g),
						(Uint8)NUM2UINT(b));
				break;
			case 4:
				r=rb_ary_entry(colorObject, 0);
				g=rb_ary_entry(colorObject, 1);
				b=rb_ary_entry(colorObject, 2);
				a=rb_ary_entry(colorObject, 3);
				return SDL_MapRGBA(format,
						(Uint8)NUM2UINT(r),
						(Uint8)NUM2UINT(g),
						(Uint8)NUM2UINT(b),
						(Uint8)NUM2UINT(a));
				break;
			default:
				rb_raise(rb_eTypeError, "Need colorarray with 3 or 4 elements");
				return Qnil;
		}
	}else{
		Uint32 color=NUM2UINT(colorObject);
		return SDL_MapRGBA(format, (Uint8)(color>>24), (Uint8)(color>>16&0xff), (Uint8)(color>>8&0xff), (Uint8)(color&0xff));
	}
}

__inline__ VALUE COLOR2VALUE(Uint32 color, SDL_Surface* surface)
{
	Uint8 r,g,b,a;

	// Is this construction usefull? Should I always use GetRGBA?
	if(surface->flags&SDL_SRCALPHA){
		SDL_GetRGBA(color, surface->format, &r, &g, &b, &a);
		return rb_ary_new3(4, UINT2NUM(r), UINT2NUM(g), UINT2NUM(b), UINT2NUM(a));
	}else{
		SDL_GetRGB(color, surface->format, &r, &g, &b);
		return rb_ary_new3(3, UINT2NUM(r), UINT2NUM(g), UINT2NUM(b));
	}
}

__inline__ void PARAMETER2COORD(VALUE parameter, Sint16* x, Sint16* y)
{
	VALUE tmp;
	if(rb_obj_is_kind_of(parameter, rb_cArray)){
		tmp=rb_ary_entry(parameter, 0);
		*x=NUM2Sint16(tmp);
		tmp=rb_ary_entry(parameter, 1);
		*y=NUM2Sint16(tmp);
	}else{
		rb_raise(rb_eTypeError, "Expected coordinate array with at least 2 elements");
	}
}

///////////////////////////////// INIT
void initVideoClasses()
{
	initVideoSurfaceClasses();
	initVideoDisplaySurfaceClasses();
	initVideoRectClasses();
	initVideoSDLGFXClasses();
	initVideoSGEClasses();
	
/*
=begin
<<< docs/head
= Video
General stuff used by the rest of the video classes.
== Exceptions
=== SurfacesLostException
This gruesome thing is thrown bij Surface#blit when Windows manages to destroy all your
surfaces.
This might happen when switching to another application, for example.
The only thing to rescue your application is by waiting for blit to stop throwing exceptions,
then reloading all your surfaces.
=end */

	classSurfacesLostException=rb_define_class_under(moduleRUDL, "SurfacesLostException", rb_cObject);
/*
=begin
== Events
=== ResizeEvent
--- ResizeEvent#size
This is [w, h] the new size of the window.
=end */
	classResizeEvent=rb_define_class_under(moduleRUDL, "ResizeEvent", classEvent);
	rb_define_attr(classResizeEvent, "size", 1, 1);
/*
=begin
=== ActiveEvent
--- ActiveEvent#gain
--- ActiveEvent#state
=end */
	classActiveEvent=rb_define_class_under(moduleRUDL, "ActiveEvent", classEvent);
	rb_define_attr(classActiveEvent, "gain", 1, 1);
	rb_define_attr(classActiveEvent, "state", 1, 1);
/*
=begin
=== QuitEvent
This event signals that the user or the program itself has requested to be terminated.
=end */
	classQuitEvent=rb_define_class_under(moduleRUDL, "QuitEvent", classEvent);

/*
=begin
== Constants
=== SDL constants:
SWSURFACE, HWSURFACE, RESIZABLE, ASYNCBLIT, OPENGL, ANYFORMAT, HWPALETTE, DOUBLEBUF, 
FULLSCREEN, HWACCEL, SRCCOLORKEY, RLEACCELOK, RLEACCEL, SRCALPHA, PREALLOC, NOFRAME

=== OpenGL constants:
GL_RED_SIZE, GL_GREEN_SIZE, GL_BLUE_SIZE, GL_ALPHA_SIZE, GL_BUFFER_SIZE, 
GL_DOUBLEBUFFER, GL_DEPTH_SIZE, GL_STENCIL_SIZE, GL_ACCUM_RED_SIZE, GL_ACCUM_GREEN_SIZE, 
GL_ACCUM_BLUE_SIZE, GL_ACCUM_ALPHA_SIZE
=end */

	DEC_CONST(SWSURFACE);
	DEC_CONST(HWSURFACE);
	DEC_CONST(RESIZABLE);
	DEC_CONST(ASYNCBLIT);
	DEC_CONST(OPENGL);
	DEC_CONST(ANYFORMAT);
	DEC_CONST(HWPALETTE);
	DEC_CONST(DOUBLEBUF);
	DEC_CONST(FULLSCREEN);
	DEC_CONST(HWACCEL);
	DEC_CONST(SRCCOLORKEY);
	DEC_CONST(RLEACCELOK);
	DEC_CONST(RLEACCEL);
	DEC_CONST(SRCALPHA);
	DEC_CONST(PREALLOC);
	DEC_CONST(NOFRAME);

	DEC_CONST(GL_RED_SIZE);
	DEC_CONST(GL_GREEN_SIZE);
	DEC_CONST(GL_BLUE_SIZE);
	DEC_CONST(GL_ALPHA_SIZE);
	DEC_CONST(GL_BUFFER_SIZE);
	DEC_CONST(GL_DOUBLEBUFFER);
	DEC_CONST(GL_DEPTH_SIZE);
	DEC_CONST(GL_STENCIL_SIZE);
	DEC_CONST(GL_ACCUM_RED_SIZE);
	DEC_CONST(GL_ACCUM_GREEN_SIZE);
	DEC_CONST(GL_ACCUM_BLUE_SIZE);
	DEC_CONST(GL_ACCUM_ALPHA_SIZE);

	id_rect=rb_intern("rect");
	id_atx=rb_intern("@x");
	id_aty=rb_intern("@y");
	id_atw=rb_intern("@w");
	id_ath=rb_intern("@h");
}
