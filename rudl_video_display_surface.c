/* RUDL - a C library wrapping SDL for use in Ruby. Copyright (C) 2001, 2002  Danny van Bruggen */
#include "rudl_events.h"
#include "rudl_video.h"

#ifdef HAVE_SDL_IMAGE_H
#include "SDL_image.h"
#endif

///////////////////////////////// DISPLAYSURFACE
/*
=begin
<<< docs/head
= DisplaySurface < Surface
The DisplaySurface is the surface that represents the window or the full screen that you
will be drawing and blitting on.
Since it is inherited from Surface, it can be used just like an ordinary surface.
You will need to create a DisplaySurface to show anything on your screen.
== Class Methods
--- DisplaySurface.new( [w,h] )
--- DisplaySurface.new( [w,h], flags )
--- DisplaySurface.new( [w,h], flags, depth )
Return value: the new DisplaySurface object
* ((|[w,h]|)) is the requested size for the new display.
* ((|flags|)) is a combination of the following:
  * SWSURFACE, to create the display in normal memory,
  * HWSURFACE, to create the display in the memory of your video hardware, if possible,
  * ASYNCBLIT, to (i quote) "enable the use of asynchronous to the display surface.
    This will usually slow down blitting on single CPU machines, but may provide a speed increase on SMP systems."
  * RESIZABLE, to make a resizable display (see events to find the event that it sends),
  * HWPALETTE, to grab the system palette,
  * DOUBLEBUF, to enable double buffered hardware pageflipping. Use ((|flip|)) with this.
  * FULLSCREEN, attempts to grab all of the screen,
  * NOFRAME, in RUDL 0.3 and up, leaves the frame off the window.
  * OPENGL, to create an OpenGL window.
* ((|depth|)) selects the bits per pixel value (8 is 256 colors, 16 is thousands of colors, 
  24 and 32 are millions of colors). If it is not supplied, a good one will be selected for you.
=end */
static VALUE displaySurface_new(int argc, VALUE* argv, VALUE self)
{
	SDL_Surface* surf;
	Uint32 flags = SDL_SWSURFACE;
	int depth = 0, hasbuf;
	Sint16 w=0, h=0;
	char* title, *icontitle;

	VALUE vsize, vflags, vdepth;

	initVideo();

	switch(rb_scan_args(argc, argv, "12", &vsize, &vflags, &vdepth)){
		case 3: depth=NUM2INT(vdepth);
		case 2: flags=PARAMETER2FLAGS(vflags);
	}

	PARAMETER2COORD(vsize, &w, &h);

	if(flags&SDL_OPENGL){
		if(flags&SDL_DOUBLEBUF){
			flags&=~SDL_DOUBLEBUF;
			SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		}else{
			SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 0);
		}
		if(argc>2){
			SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, depth);
		}
		surf = SDL_SetVideoMode(w, h, depth, flags);

		if(!surf) SDL_RAISE;

		SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &hasbuf);
		if(hasbuf) surf->flags|=SDL_DOUBLEBUF;
	}else{
		if(argc<3) flags |= SDL_ANYFORMAT;

		surf = SDL_SetVideoMode(w, h, depth, flags);

		if(!surf) SDL_RAISE;
	}

	SDL_WM_GetCaption(&title, &icontitle);
	SDL_PumpEvents();
	if(!title || !*title){
		SDL_WM_SetCaption("SDL window", "SDL");
	}

	return Data_Wrap_Struct(classDisplaySurface, 0, 0, surf);
}

/*
=begin
--- DisplaySurface.destroy
Destroys the display, removing the window or returning from fullscreen mode.
Do not call methods on a destroyed DisplaySurface
=end
*/
static VALUE displaySurface_destroy(VALUE self)
{
	quitVideo();
	return Qnil;
}

/*
=begin
--- DisplaySurface.modes
--- DisplaySurface.modes( bitdepth )
--- DisplaySurface.modes( bitdepth, flags )
Lists available modes for a certain ((|bitdepth|)) and optionally only those modes that 
can do ((|flags|)).
Flags are like those in DisplaySurface.new.
No flags is the same as passing FULLSCREEN.
Returns an array with arrays of [w, h], or nil.
nil means any mode is ok, an empty array means ((|no|)) mode is supported.
=end */
static VALUE displaySurface_modes(int argc, VALUE* argv, VALUE self)
{
	SDL_PixelFormat format;
	SDL_Rect** rects;
	int flags=SDL_FULLSCREEN;
	VALUE list, size;

	VALUE bppObject, flagsObject;

	format.BitsPerPixel = 0;
	initVideo();

	switch(rb_scan_args(argc, argv, "02", &bppObject, &flagsObject)){
		case 2:
			flags=NUM2UINT(flagsObject);
		case 1:
			format.BitsPerPixel=(Uint8)NUM2UINT(bppObject);
			break;
	}
	if(format.BitsPerPixel==0){
		format.BitsPerPixel = SDL_GetVideoInfo()->vfmt->BitsPerPixel;
	}

	rects = SDL_ListModes(&format, flags);
	
	if(rects == (SDL_Rect**)-1){
		return Qnil;
	}

	list=rb_ary_new();

	if(!rects) return list;

	for(; *rects; ++rects){
		size=rb_ary_new3(2, INT2NUM((*rects)->w), INT2NUM((*rects)->h));
		rb_ary_push(list, size);
	}

	return list;
}


/*
=begin
--- DisplaySurface.mode_ok?
Like DisplaySurface.new, but doesn't set the mode, only returns true if the mode can be set,
and false if it can't.
=end */
static VALUE displaySurface_mode_ok_(int argc, VALUE* argv, VALUE self)
{
	int flags=SDL_SWSURFACE, depth=0;
	Sint16 w, h;

	VALUE vsize, vflags, vdepth;

	initVideo();

	rb_scan_args(argc, argv, "12", &vsize, &vflags, &vdepth);

	PARAMETER2COORD(vsize, &w, &h);

	if(argc>2){
		flags=PARAMETER2FLAGS(vflags);
		if(argc>3){
			depth=NUM2INT(vdepth);
		}else{
			depth=SDL_GetVideoInfo()->vfmt->BitsPerPixel;
		}
	}

	return UINT2NUM(SDL_VideoModeOK(w, h, depth, flags));
}

/*
=begin
--- DisplaySurface.best_mode_info
This method return a hash filled with information about the video hardware.

These entries are true or false:
 * hardware surfaces available
 * window manager available
 * hardware to hardware blits accelerated
 * hardware to hardware colorkey blits accelerated
 * hardware to hardware alpha blits accelerated
 * software to hardware blits accelerated
 * software to hardware colorkey blits accelerated
 * software to hardware alpha blits accelerated
 * color fills accelerated
This is in kilobytes:
 * video memory

There is currently no difference between best_mode_info and info, 
except that one is a class method and the other an instance method, 
but there may be differences in the future.
=end */

static VALUE get_video_info()
{
	const SDL_VideoInfo* info=SDL_GetVideoInfo();
	VALUE retval=rb_hash_new();
	rb_hash_aset(retval, CSTR2STR("hardware surfaces available"), INT2BOOL(info->hw_available));
	rb_hash_aset(retval, CSTR2STR("window manager available"), INT2BOOL(info->wm_available));
	rb_hash_aset(retval, CSTR2STR("hardware to hardware blits accelerated"), INT2BOOL(info->blit_hw));
	rb_hash_aset(retval, CSTR2STR("hardware to hardware colorkey blits accelerated"), INT2BOOL(info->blit_hw_CC));
	rb_hash_aset(retval, CSTR2STR("hardware to hardware alpha blits accelerated"), INT2BOOL(info->blit_hw_A));
	rb_hash_aset(retval, CSTR2STR("software to hardware blits accelerated"), INT2BOOL(info->blit_sw));
	rb_hash_aset(retval, CSTR2STR("software to hardware colorkey blits accelerated"), INT2BOOL(info->blit_sw_CC));
	rb_hash_aset(retval, CSTR2STR("software to hardware alpha blits accelerated"), INT2BOOL(info->blit_sw_A));
	rb_hash_aset(retval, CSTR2STR("color fills accelerated"), INT2BOOL(info->blit_fill));
	rb_hash_aset(retval, CSTR2STR("video memory"), UINT2NUM(info->video_mem));
	return retval;
}

static VALUE displaySurface_best_mode_info(VALUE self)
{
	initVideo();
	if(SDL_GetVideoSurface()==NULL){
		return get_video_info();
	}else{
		SDL_RAISE_S("Cannot be called after creating a DisplaySurface");
		return Qnil;
	}
}

static VALUE displaySurface_info(VALUE self)
{
	initVideo();
	return get_video_info();
}

/*
=begin
== Instance Methods
--- DisplaySurface#info
See DisplaySurface.best_mode_info
--- DisplaySurface#driver
Returns the name of the videodriver that is being used.
=end */
static VALUE displaySurface_driver(VALUE self)
{
	char buf[256];
	
	if(!SDL_VideoDriverName(buf, sizeof(buf))){
		return rb_str_new2("");
	}
	return rb_str_new2(buf);
}


/*
=begin
--- DisplaySurface#update
--- DisplaySurface#update( rect )
This call will update a section (or sections) of the display screen.
You must update an area of your display when you change its contents.
((|rect|)) is, starting from v0.4, an array of rectangles.
If passed with no arguments, this will update the entire display surface.
=end
This call cannot be used on OPENGL displays, and will generate an exception.
If you have many rects that need updating,
it is best to combine them into an array and pass them all at once.
*/
static VALUE displaySurface_update(int argc, VALUE* argv, VALUE self)
{
	SDL_Rect rect;
	VALUE rectObject;
	int i, total_rects;
	SDL_Rect* rects;


	GET_SURFACE;

	if(surface->flags&SDL_OPENGL) SDL_RAISE_S("Cannot update an OPENGL display");

	switch(rb_scan_args(argc, argv, "01", &rectObject)){
		case 0:
			SDL_UpdateRect(surface, 0, 0, 0, 0);
			return self;
		case 1:
			Check_Type(rectObject, T_ARRAY);
			total_rects=RARRAY(rectObject)->len;
			rects=malloc(sizeof(rects)*total_rects);
			for(i=0; i<total_rects; i++){
				PARAMETER2CRECT(rb_ary_entry(rectObject, i), rects+i);
			}
			SDL_UpdateRects(surface, total_rects, rects);
			free(rects);
	}
	return self;
}

/*
=begin
--- DisplaySurface#flip
This will update the contents of the entire display.
If your display mode is using the flags HWSURFACE and DOUBLEBUF, this
will wait for a vertical retrace (if the video driver supports it) 
and swap the surfaces.
If you are using a different type of display mode, it will simply update
the entire contents of the surface.
=end
When using an OPENGL display mode this will perform a gl buffer swap.
*/
static VALUE displaySurface_flip(VALUE self)
{
	SDL_Surface* surface=retrieveSurfacePointer(self);
	if(surface->flags&SDL_OPENGL){
		SDL_GL_SwapBuffers();
	}else{
		if(SDL_Flip(retrieveSurfacePointer(self))==-1) SDL_RAISE;
	}
	return self;
}

/*
=begin
--- DisplaySurface#active?
Returns true if the application is active.
=end */
static VALUE displaySurface_active_(VALUE self)
{
	return INT2BOOL((SDL_GetAppState()&SDL_APPACTIVE) != 0);
}

/*
=begin
--- DisplaySurface#caption
--- DisplaySurface#set_caption( title )
--- DisplaySurface#set_caption( title, icontitle )
caption= sets the title of the window (if the application runs in a window) to title.
Sets the title of the icon that shows when the application is iconified to icontitle,
or title if icontitle is not supplied.
caption returns the title and icontitle of the window.
=end */
static VALUE displaySurface_set_caption(int argc, VALUE* argv, VALUE self)
{
	VALUE titleObject, iconTitleObject;
	char* title;
	char* iconTitle="RUDL application";

	rb_scan_args(argc, argv, "11", &titleObject, &iconTitleObject);
	
	if(argc==2){
		iconTitle=STR2CSTR(iconTitleObject);
	}
	
	title=STR2CSTR(titleObject);

	SDL_WM_SetCaption(title, iconTitle);

	return self;
}

static VALUE displaySurface_caption(VALUE self)
{
	char* title, *icontitle;

	SDL_WM_GetCaption(&title, &icontitle);
	
	if(title && *title){
		return rb_ary_new3(2, rb_str_new2(title), rb_str_new2(icontitle));
	}

	return rb_ary_new3(2, rb_str_new2(""), rb_str_new2(""));
}

/*
=begin
--- DisplaySurface#iconify
Iconifies the application.
Returns true if it is succesfull.
=end */
static VALUE displaySurface_iconify(VALUE self)
{
	return INT2BOOL(SDL_WM_IconifyWindow()!=0);
}

/*
=begin
--- DisplaySurface#gamma=([r,g,b])
--- DisplaySurface#gamma=(intensity)
Sets the gamma value for the display when this is supported.
Intensity is a shortcut for values where r=g=b.
=end */
static VALUE displaySurface_gamma_(VALUE self, VALUE color)
{
	float r, g, b;
	VALUE tmp;

	if(rb_obj_is_kind_of(color, rb_cArray)){
		if(RARRAY(color)->len==3){
			tmp=rb_ary_entry(color, 0);		r=NUM2FLT(tmp);
			tmp=rb_ary_entry(color, 1);		g=NUM2FLT(tmp);
			tmp=rb_ary_entry(color, 2);		b=NUM2FLT(tmp);
		}else{
			SDL_RAISE_S("Want [r,g,b] array");
			return Qnil;
		}
	}else{
		r=g=b=NUM2FLT(color);
	}

	return INT2BOOL(SDL_SetGamma(r, g, b)==0);
}

/*
=begin
--- DisplaySurface#toggle_fullscreen
Toggles between fullscreen and windowed mode.
Only works on a few platforms.
This will someday be replaced by a method that will work on all platforms.
=end */
static VALUE displaySurface_toggle_fullscreen(VALUE self)
{
	return INT2BOOL(SDL_WM_ToggleFullScreen(retrieveSurfacePointer(self))!=0);
}

static VALUE displaySurface_gl_get_attribute(VALUE self, VALUE attribute, VALUE value)
{
}

/* I'll do this another day
static VALUE displaySurface_gl_set_attribute(VALUE self, VALUE attribute, VALUE value)
int SDL_GL_SetAttribute(SDL_GLattr attr, int value);
int SDL_GL_GetAttribute(SDL_GLattr attr, int* value);
*/

///////////////////////////////// INIT
void initVideoDisplaySurfaceClasses()
{
	classDisplaySurface=rb_define_class_under(moduleRUDL, "DisplaySurface", classSurface);
	rb_define_singleton_method(classDisplaySurface, "new", displaySurface_new, -1);
	rb_define_singleton_method(classDisplaySurface, "modes", displaySurface_modes, -1);
	rb_define_singleton_method(classDisplaySurface, "mode_ok?", displaySurface_mode_ok_, -1);
	rb_define_singleton_method(classDisplaySurface, "best_mode_info", displaySurface_best_mode_info, 0);
	rb_define_method(classDisplaySurface, "driver", displaySurface_driver, 0);
	rb_define_method(classDisplaySurface, "info", displaySurface_info, 0);
	rb_define_method(classDisplaySurface, "update", displaySurface_update, -1);
	rb_define_method(classDisplaySurface, "flip", displaySurface_flip, 0);
	rb_define_method(classDisplaySurface, "active?", displaySurface_active_, 0);
	rb_define_method(classDisplaySurface, "caption", displaySurface_caption, 0);
	rb_define_method(classDisplaySurface, "iconify", displaySurface_iconify, 0);
	rb_define_method(classDisplaySurface, "set_caption", displaySurface_set_caption, -1);
	rb_define_method(classDisplaySurface, "gamma=", displaySurface_gamma_, 1);
	rb_define_method(classDisplaySurface, "toggle_fullscreen", displaySurface_toggle_fullscreen, 0);
	rb_define_method(classDisplaySurface, "destroy", displaySurface_destroy, 0);
}
