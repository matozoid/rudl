/*
RUDL - a C library wrapping SDL for use in Ruby.
Copyright (C) 2001, 2002, 2003  Danny van Bruggen

$Log: rudl_video_display_surface.c,v $
Revision 1.24  2004/08/04 23:03:46  tsuihark
Updated all documentation to Dokumentat format.

Revision 1.23  2004/01/25 00:21:34  rennex
Added DisplaySurface.get
Made DisplaySurface#destroy also a class method
Tweaked docs

Revision 1.22  2004/01/21 22:55:10  tsuihark
Converted to Dokumentat format.

Revision 1.21  2004/01/04 19:48:45  rennex
Added automatic fixing of window clipping after a resize.
Changed default window title to "RUDL window"

Revision 1.20  2003/12/23 01:29:30  rennex
Tweaked docs

Revision 1.19  2003/12/03 16:04:11  rennex
Tweaked the docs a bit

Revision 1.18  2003/12/01 23:30:27  rennex
Added mask_string size check in set_icon

Revision 1.17  2003/12/01 22:36:49  rennex
Fixed some typos in docs

Revision 1.16  2003/12/01 19:14:24  rennex
Added transparency support to set_icon

Revision 1.15  2003/11/28 23:58:44  rennex
Added set_icon, but without transparency support

Revision 1.14  2003/10/26 15:29:32  tsuihark
Did stuff

Revision 1.13  2003/10/05 16:59:53  tsuihark
Fixed a lot of documentation

*/
#include "rudl_events.h"
#include "rudl_video.h"

#ifdef HAVE_SDL_IMAGE_H
#include "SDL_image.h"
#endif

///////////////////////////////// DISPLAYSURFACE
/**
@file Video
@class DisplaySurface < Surface
The DisplaySurface is the surface that represents the window or the full screen that you
will be drawing and blitting on.
Since it is inherited from Surface, it can be used just like an ordinary surface.
You will need to create a DisplaySurface to show anything on your screen.
*/
/**
@section Initializers
@method new( size ) => DisplaySurface
@method new( size, flags ) => DisplaySurface
@method new( size, flags, depth ) => DisplaySurface
<ul>
<li>@size is the requested size for the new display in [w,h] format.
<li>@flags is a combination of the following:
	<ul>
	<li><code>SWSURFACE</code>, to create the display in normal memory,
	<li><code>HWSURFACE</code>, to create the display in the memory of your video hardware, if possible,
	<li><code>ASYNCBLIT</code>, to (i quote) "enable the use of asynchronous to the display surface.
		This will usually slow down blitting on single CPU machines, but may provide a speed increase on SMP systems."
	<li><code>RESIZABLE</code>, to make a resizable display (see events to find the event that it sends),
	<li><code>HWPALETTE</code>, to grab the system palette,
	<li><code>DOUBLEBUF</code>, to enable double buffered hardware pageflipping. Use @flip with this.
	<li><code>FULLSCREEN</code>, attempts to grab all of the screen,
	<li><code>NOFRAME</code>, leaves the frame off the window.
	<li><code>OPENGL</code>, to create an OpenGL window.
	</ul>
<li>@depth selects the bits per pixel value (8 is 256 colors, 16 is thousands of colors,
	24 and 32 are millions of colors). If it is not supplied, a good one will be selected for you.
</ul>
*/
static VALUE displaySurface_new(int argc, VALUE* argv, VALUE self)
{
    SDL_Surface* surf;
    Uint32 flags = SDL_SWSURFACE;
    int depth = 0, hasbuf, numargs;
    Sint16 w=0, h=0;
    char* title, *icontitle;

    VALUE vsize, vflags, vdepth;

    initVideo();

    switch (numargs = rb_scan_args(argc, argv, "12", &vsize, &vflags, &vdepth)) {
        case 3: depth = NUM2INT(vdepth);
        case 2: flags = PARAMETER2FLAGS(vflags);
    }

    PARAMETER2COORD(vsize, &w, &h);

    /* store parameters for re-setting video mode after a resize event */
    currDSnumargs = numargs;
    currDSflags = vflags;
    currDSdepth = vdepth;

    if (flags & SDL_OPENGL) {
        if (flags & SDL_DOUBLEBUF) {
            flags &= ~SDL_DOUBLEBUF;
            SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        } else {
            SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 0);
        }
        if (argc > 2) {
            SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, depth);
        }
        surf = SDL_SetVideoMode(w, h, depth, flags);

        if (!surf) SDL_RAISE;

        SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &hasbuf);
        if (hasbuf) surf->flags |= SDL_DOUBLEBUF;
    } else {
        if (argc < 3) flags |= SDL_ANYFORMAT;

        surf = SDL_SetVideoMode(w, h, depth, flags);

        if (!surf) SDL_RAISE;
    }

    SDL_WM_GetCaption(&title, &icontitle);
    SDL_PumpEvents();
    /* don't change the title if this was a call to resize the window! */
    if (!title || !*title) {
        SDL_WM_SetCaption("RUDL window", "RUDL");
    }

    currentDisplaySurface = Data_Wrap_Struct(classDisplaySurface, 0, 0, surf);
    return currentDisplaySurface;
}

/**
@section Methods
@method destroy -> nil
Destroys the display, removing the window or returning from fullscreen mode.
Do not call methods on a destroyed DisplaySurface!
*/
static VALUE displaySurface_destroy(VALUE self)
{
    currentDisplaySurface = NULL;   /* just in case */
    quitVideo();
    return Qnil;
}

/**
@method DisplaySurface.get => DisplaySurface or nil
Gets the current DisplaySurface, or nil if there isn't one.
*/

static VALUE displaySurface_get(VALUE self)
{
    if (SDL_GetVideoSurface()) {
        return currentDisplaySurface;
    } else {
        return Qnil;
    }
}

/**
@section Video modes
@method DisplaySurface.modes => [w,h] or nil
@method DisplaySurface.modes( bitdepth ) => [w,h] or nil
@method DisplaySurface.modes( bitdepth, flags ) => [w,h] or nil
Lists available modes for a certain @bitdepth and optionally only those modes that
can do @flags.
Flags are like those in @DisplaySurface.new.
No flags is the same as passing <code>FULLSCREEN</code>.
Return value nil means any mode is ok, an empty array means <em>no</em> mode is supported.
*/
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


/**
@method DisplaySurface.mode_ok? => boolean
Like @new, but doesn't set the mode, only returns true if the mode can be set,
and false if it can't.
*/
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

/**
@section Methods
@method info
See @best_mode_info
*/
/**
@section Video modes
@method best_mode_info
This method returns a hash filled with information about the video hardware.

These entries are true or false:
<ul>
<li>hardware surfaces available
<li>window manager available
<li>hardware to hardware blits accelerated
<li>hardware to hardware colorkey blits accelerated
<li>hardware to hardware alpha blits accelerated
<li>software to hardware blits accelerated
<li>software to hardware colorkey blits accelerated
<li>software to hardware alpha blits accelerated
<li>color fills accelerated
<li>video memory (in kilobytes)
</ul>

There is currently no difference between @best_mode_info and @info,
except that one is a class method and the other an instance method,
but there may be differences in the future.
*/
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

/**
@section OpenGL
RUDL supports setting up an OpenGL window in a cross platform way.
Several other methods apply. See @flip and @new.
*/
/**
@method VideoSurface.gl_set_attribute( name, value ) => self
Set an attribute of the OpenGL subsystem before intialization.
*/
static VALUE displaySurface_gl_set_attribute(VALUE self, VALUE attribute, VALUE value)
{
    SDL_VERIFY(SDL_GL_SetAttribute(NUM2INT(attribute), NUM2INT(value))==0);
    return self;
}

/**
@method VideoSurface.gl_get_attribute( name ) => Number
From the SDL documentation:

Get an attribute of the OpenGL subsystem from the windowing
interface, such as glX. This is of course different from getting
the values from SDL's internal OpenGL subsystem, which only
stores the values you request before initialization.

Developers should track the values they pass into SDL_GL_SetAttribute
themselves if they want to retrieve these values.
*/
static VALUE displaySurface_gl_get_attribute(VALUE self, VALUE attribute)
{
    int buf;

    SDL_VERIFY(SDL_GL_GetAttribute(NUM2INT(attribute), &buf)==0);
    return INT2NUM(buf);
}

/**@section Methods
@method driver => String
Returns the name of the videodriver that is being used.
*/
static VALUE displaySurface_driver(VALUE self)
{
    char buf[256];

    if(!SDL_VideoDriverName(buf, sizeof(buf))){
        return rb_str_new2("");
    }
    return rb_str_new2(buf);
}

/**
@method update => self
@method update( rect ) => self
This call will update a section (or sections) of the display screen.
You must update an area of your display when you change its contents.
@rect is an array of rectangles.
If passed with no arguments, this will update the entire display surface.

This call cannot be used on OpenGL displays, and will generate an exception.
*/
static VALUE displaySurface_update(int argc, VALUE* argv, VALUE self)
{
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
            rects=malloc(sizeof(SDL_Rect)*total_rects);
            for(i=0; i<total_rects; i++){
                PARAMETER2CRECT(rb_ary_entry(rectObject, i), rects+i);
                RUDL_ASSERT((rects+i)->w != 0, "rectangle width is zero");
                RUDL_ASSERT((rects+i)->h != 0, "rectangle height is zero");
                RUDL_ASSERT((rects+i)->x >= 0, "rectangle out of bounds at left");
                RUDL_ASSERT((rects+i)->y >= 0, "rectangle out of bounds at top");
                RUDL_ASSERT((rects+i)->x+(rects+i)->w <= surface->w, "rectangle out of bounds at right");
                RUDL_ASSERT((rects+i)->y+(rects+i)->h <= surface->h, "rectangle out of bounds at bottom");
            }
            SDL_UpdateRects(surface, total_rects, rects);
            free(rects);
    }
    return self;
}

/**
@method flip => self
This will update the contents of the entire display.
If your display mode is using the flags <code>HWSURFACE</code> and <code>DOUBLEBUF</code>, this
will wait for a vertical retrace (if the video driver supports it)
and swap the surfaces.
If you are using a different type of display mode, it will simply update
the entire contents of the surface.

When using an OpenGL display mode this will perform a gl buffer swap.
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

/**
@section Windowing system
@method active? -> boolean
Returns true if the application is active (i.e. not minimized).
*/
static VALUE displaySurface_active_(VALUE self)
{
    return INT2BOOL((SDL_GetAppState()&SDL_APPACTIVE) != 0);
}

/**
@method caption => [String, String]
Returns the title and icontitle of the window.
*/
/**
@method set_caption( title ) -> self
@method set_caption( title, icontitle ) -> self
Sets the title of the window (if the application runs in a window) to @title.
Supplying @icontitle sets the title of the icon that shows when the application is iconified to @icontitle,
or @title if @icontitle is not supplied.
*/
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

/**
@method set_icon( icon_surface ) -> nil
@method set_icon( icon_surface, mask_string ) -> nil
Sets the icon for the display window.

The SDL docs say this must be called before calling DisplaySurface.new, but
at least on Windows this is not true. This method exists also as a class method
of DisplaySurface.

Win32 icons must be 32x32 to work properly.

If @icon_surface has a colorkey set, that color will be transparent. (Since
SDL currently handles that wrong, RUDL generates the mask instead, unless
you supply nil for mask_string.)
Alternatively, you can supply @mask_string where each byte represents
the visibility of 8 pixels (MSB is the leftmost pixel, 0 means transparent).
*/
static VALUE displaySurface_set_icon(int argc, VALUE* argv, VALUE self)
{
    VALUE icon, mask;
    SDL_Surface* surface;
    Uint8* maskstr = NULL;
    Uint8* maskptr = NULL;

    rb_scan_args(argc, argv, "11", &icon, &mask);

    surface = retrieveSurfacePointer(icon);

    /* custom mask supplied? */
    if (argc == 2) {
        if (mask != Qnil) {
            RUDL_VERIFY(RSTRING(mask)->len >= surface->h*((surface->w + 7)/8), "Not enough data in mask_string");
            maskstr = STR2CSTR(mask);
        }
    /* colorkey set? generate a working mask, unlike SDL currently :) */
    } else if (surface->flags & SDL_SRCCOLORKEY) {
        Sint16 x, y, xb;
        Uint8 bit, b;
        int wb = (surface->w + 7) / 8;  /* width in bytes */
        Uint32 key = surface->format->colorkey;

        /* get memory for the mask. It gets freed when we exit this function */
        maskptr = maskstr = ALLOCA_N(Uint8, wb * surface->h);

        for (y=0; y < surface->h; y++) {
            x = 0;
            for (xb=0; xb < wb; xb++) {
                b = 0;
                for (bit=0x80; bit; bit >>= 1) {
                    if (internal_get(surface, x, y) != key) {
                        b |= bit;
                    }
                    x++;
                }
                *maskptr++ = b;
            }
        }
    }

    SDL_WM_SetIcon(surface, maskstr);

    return Qnil;
}



/**
@method iconify => boolean
Iconifies (minimizes) the application.
Returns true if successful.
*/
static VALUE displaySurface_iconify(VALUE self)
{
    return INT2BOOL(SDL_WM_IconifyWindow()!=0);
}

/**
@section Methods
@method gamma=( [r,g,b] ) -> boolean
@method gamma=( intensity ) -> boolean
Sets the gamma value for the display when this is supported.
@intensity is a shortcut for values where r=g=b.
*/
static VALUE displaySurface_gamma_(VALUE self, VALUE color)
{
    float r, g, b;
    VALUE tmp;

    if(rb_obj_is_kind_of(color, rb_cArray)){
        if(RARRAY(color)->len==3){
            tmp=rb_ary_entry(color, 0);     r=NUM2FLT(tmp);
            tmp=rb_ary_entry(color, 1);     g=NUM2FLT(tmp);
            tmp=rb_ary_entry(color, 2);     b=NUM2FLT(tmp);
        }else{
            SDL_RAISE_S("Want [r,g,b] array");
            return Qnil;
        }
    }else{
        r=g=b=NUM2FLT(color);
    }

    return INT2BOOL(SDL_SetGamma(r, g, b)==0);
}

/**
@method toggle_fullscreen
Toggles between fullscreen and windowed mode.
The code is experimental and is known to crash in some cases,
please report problems if found.
It might be better to not use this at all and use DisplaySurface.destroy to dispose
of the current DisplaySurface and create a new one with DisplaySurface.new with the
FULLSCREEN flag toggled.
*/
/*
 * (This code may be considered public domain, and under no licensing
 *  restrictions. --rcg.)
 */

/*
 * Attempt to flip the video surface to fullscreen or windowed mode.
 *  Attempts to maintain the surface's state, but makes no guarantee
 *  that pointers (i.e., the surface's pixels field) will be the same
 *  after this call.
 *
 * Caveats: Your surface pointers will be changing; if you have any other
 *           copies laying about, they are invalidated.
 *
 *          Do NOT call this from an SDL event filter on Windows. You can
 *           call it based on the return values from SDL_PollEvent, etc, just
 *           not during the function you passed to SDL_SetEventFilter().
 *
 *          Thread safe? Likely not.
 *
 *          This has been tested briefly under Linux/X11 and Win/DirectX. YMMV.
 *
 *          Palette setting is possibly/probably broken. Please fix.
 *
 *   @param surface pointer to surface ptr to toggle. May be different
 *                  pointer on return. MAY BE NULL ON RETURN IF FAILURE!
 *   @param flags   pointer to flags to set on surface. The value pointed
 *                  to will be XOR'd with SDL_FULLSCREEN before use. Actual
 *                  flags set will be filled into pointer. Contents are
 *                  undefined on failure. Can be NULL, in which case the
 *                  surface's current flags are used.
 *  @return non-zero on success, zero on failure.
 */
static int attempt_fullscreen_toggle(SDL_Surface **surface, Uint32 *flags)
{
    long framesize = 0;
    void *pixels = NULL;
    SDL_Rect clip;
    Uint32 tmpflags = 0;
    int w = 0;
    int h = 0;
    int bpp = 0;
    int grabmouse = (SDL_WM_GrabInput(SDL_GRAB_QUERY) == SDL_GRAB_ON);
    int showmouse = SDL_ShowCursor(-1);

#ifdef BROKEN
    SDL_Color *palette = NULL;
    int ncolors = 0;
#endif

    DEBUG_S("attempting to toggle fullscreen flag...");

    if ( (!surface) || (!(*surface)) )  /* don't try if there's no surface. */
    {
        DEBUG_S("Null surface (?!). Not toggling fullscreen flag.");
        return(0);
    } /* if */

    if (SDL_WM_ToggleFullScreen(*surface))
    {
        DEBUG_S("SDL_WM_ToggleFullScreen() seems to work on this system.");
        if (flags)
            *flags ^= SDL_FULLSCREEN;
        return(1);
    } /* if */

    if ( !(SDL_GetVideoInfo()->wm_available) )
    {
        DEBUG_S("No window manager. Not toggling fullscreen flag.");
        return(0);
    } /* if */

    DEBUG_S("toggling fullscreen flag The Hard Way...");
    tmpflags = (*surface)->flags;
    w = (*surface)->w;
    h = (*surface)->h;
    bpp = (*surface)->format->BitsPerPixel;

    if (flags == NULL)  /* use the surface's flags. */
        flags = &tmpflags;

    SDL_GetClipRect(*surface, &clip);

        /* save the contents of the screen. */
    if ( (!(tmpflags & SDL_OPENGL)) && (!(tmpflags & SDL_OPENGLBLIT)) )
    {
        framesize = (w * h) * ((*surface)->format->BytesPerPixel);
        pixels = malloc(framesize);
        if (pixels == NULL)
            return(0);
        memcpy(pixels, (*surface)->pixels, framesize);
    } /* if */

#ifdef BROKEN
    if ((*surface)->format->palette != NULL)
    {
        ncolors = (*surface)->format->palette->ncolors;
        palette = malloc(ncolors * sizeof (SDL_Color));
        if (palette == NULL)
        {
            free(pixels);
            return(0);
        } /* if */
        memcpy(palette, (*surface)->format->palette->colors,
               ncolors * sizeof (SDL_Color));
    } /* if */
#endif

    if (grabmouse)
        SDL_WM_GrabInput(SDL_GRAB_OFF);

    SDL_ShowCursor(1);

    *surface = SDL_SetVideoMode(w, h, bpp, (*flags) ^ SDL_FULLSCREEN);

    if (*surface != NULL)
        *flags ^= SDL_FULLSCREEN;

    else  /* yikes! Try to put it back as it was... */
    {
        *surface = SDL_SetVideoMode(w, h, bpp, tmpflags);
        if (*surface == NULL)  /* completely screwed. */
        {
            if (pixels != NULL)
                free(pixels);
#ifdef BROKEN
            if (palette != NULL)
                free(palette);
#endif
            return(0);
        } /* if */
    } /* if */

    /* Unfortunately, you lose your OpenGL image until the next frame... */

    if (pixels != NULL)
    {
        memcpy((*surface)->pixels, pixels, framesize);
        free(pixels);
    } /* if */

#ifdef BROKEN
    if (palette != NULL)
    {
            /* !!! FIXME : No idea if that flags param is right. */
        SDL_SetPalette(*surface, SDL_LOGPAL, palette, 0, ncolors);
        free(palette);
    } /* if */
#endif

    SDL_SetClipRect(*surface, &clip);

    if (grabmouse)
        SDL_WM_GrabInput(SDL_GRAB_ON);

    SDL_ShowCursor(showmouse);

    return(1);
} /* attempt_fullscreen_toggle */

static VALUE displaySurface_toggle_fullscreen(VALUE self)
{
    VALUE result;
    GET_SURFACE;
    result=INT2BOOL(attempt_fullscreen_toggle(&surface, NULL)!=0);
    DATA_PTR(self)=surface;
    return result;
    //return INT2BOOL(SDL_WM_ToggleFullScreen(retrieveSurfacePointer(self))!=0); <- old code
}
///////////////////////////////// INIT
void initVideoDisplaySurfaceClasses()
{
    classDisplaySurface=rb_define_class_under(moduleRUDL, "DisplaySurface", classSurface);
    rb_define_singleton_method(classDisplaySurface, "new", displaySurface_new, -1);
    rb_define_singleton_method(classDisplaySurface, "modes", displaySurface_modes, -1);
    rb_define_singleton_method(classDisplaySurface, "mode_ok?", displaySurface_mode_ok_, -1);
    rb_define_singleton_method(classDisplaySurface, "best_mode_info", displaySurface_best_mode_info, 0);
    rb_define_singleton_method(classDisplaySurface, "gl_set_attribute", displaySurface_gl_set_attribute, 2);
    rb_define_singleton_method(classDisplaySurface, "gl_get_attribute", displaySurface_gl_get_attribute, 1);
    rb_define_singleton_method(classDisplaySurface, "get", displaySurface_get, 0);
    rb_define_singleton_and_instance_method(classDisplaySurface, "destroy", displaySurface_destroy, 0);
    rb_define_singleton_and_instance_method(classDisplaySurface, "set_icon", displaySurface_set_icon, -1);

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

    rb_eval_string(
            "module RUDL class DisplaySurface                   \n"
            "   def inspect                                     \n"
            "       \"<DisplaySurface: #{w}x#{h},#{bitsize}>\"  \n"
            "   end                                             \n"
            "end end                                            \n"
    );
}
