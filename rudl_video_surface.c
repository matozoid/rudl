/*
RUDL - a C library wrapping SDL for use in Ruby.
Copyright (C) 2001, 2002, 2003  Danny van Bruggen

$Log: rudl_video_surface.c,v $
Revision 1.38  2004/10/23 19:54:22  rennex
Fixed docs

Revision 1.37  2004/08/21 23:25:31  tsuihark
Fixed little documentation typo

Revision 1.36  2004/08/04 23:03:47  tsuihark
Updated all documentation to Dokumentat format.

Revision 1.35  2004/01/21 22:02:14  tsuihark
Converted to Dokumentat format.

Revision 1.34  2004/01/06 18:01:39  tsuihark
Removed a few warnings

Revision 1.33  2003/12/30 01:02:41  rennex
Added mirror_x.
Fixed segfault when calling set_palette(0, nil).
Added (set_)palette prototypes before Surface.new

Revision 1.32  2003/12/29 20:46:36  rennex
Added mirror_y for now.
Changed previous ALLOC_N to ALLOCA_N.

Revision 1.31  2003/12/29 20:15:04  tsuihark
Changed malloc to ALLOC_N to prevent memory leaks

Revision 1.30  2003/12/26 22:41:14  rennex
Combined Surface#get and Surface#[] into one method

Revision 1.29  2003/12/16 01:04:15  rennex
Added target coordinate for scale2x and reorganized that code a little

Revision 1.28  2003/12/09 15:09:01  rennex
Added Scale2x support for 24-bit surfaces

Revision 1.27  2003/12/09 14:22:17  rennex
Added Scale2x support for 8-bit and 16-bit surfaces.
Made set_colorkey(nil) and set_alpha(nil) work.
Made Surface.new copy the palette if a 8-bit surface was given.

Revision 1.26  2003/12/09 01:38:54  rennex
Added the scale2x method (currently only for 32bit surfaces)

Revision 1.25  2003/11/28 22:24:58  rennex
Fixed bugs that caused errors on Linux.

Revision 1.24  2003/10/26 15:29:32  tsuihark
Did stuff

Revision 1.23  2003/10/19 11:26:13  tsuihark
Added VideoExposeEvent and removed an odd bug in rudl_video_surface.c

Revision 1.22  2003/10/01 21:26:01  tsuihark
Some 16 bit + alpha stuff

Revision 1.21  2003/09/29 12:43:21  rennex
Moved SDL_LockSurface after the coordinate check in internal_(nonlocking_)get

*/
#include "rudl_events.h"
#include "rudl_video.h"

#ifdef HAVE_SDL_IMAGE_H
#include "SDL_image.h"
#endif

///////////////////////////////// SURFACE
/**
@file Surface
@class Surface
A @Surface is a two dimensional array of pixels with some information about those pixels.
This might not seem like much, but it is just about the most important class in RUDL.
*/

// SUPPORT:

ID id_shared_surface_reference;

__inline__ VALUE createSurfaceObject(SDL_Surface* surface)
{
    return Data_Wrap_Struct(classSurface, 0, SDL_FreeSurface, surface);
}

__inline__ SDL_Surface* retrieveSurfacePointer(VALUE self)
{
    SDL_Surface* surface;
    Data_Get_Struct(self, SDL_Surface, surface);
    return surface;
}

__inline__ void setMasksFromBPP(Uint32 bpp, bool alphaWanted, Uint32* Rmask, Uint32* Gmask, Uint32* Bmask, Uint32* Amask)
{
    *Amask = 0;
    if(alphaWanted && (bpp==32||bpp==16)){
        switch(bpp){
            case 16: *Rmask=0xF<<12; *Gmask=0xF<<8; *Bmask=0xF<<4; *Amask=0xF;break;
            case 32: *Rmask = 0xFF << 24; *Gmask = 0xFF << 16; *Bmask = 0xFF << 8; *Amask=0xFF; break;
        }
    }else{
        switch(bpp){
            case 8:  *Rmask = 0xFF >> 6 << 5; *Gmask = 0xFF >> 5 << 2; *Bmask = 0xFF >> 6; break;
            case 12: *Rmask = 0xFF >> 4 << 8; *Gmask = 0xFF >> 4 << 4; *Bmask = 0xFF >> 4; break;
            case 15: *Rmask = 0xFF >> 3 << 10; *Gmask = 0xFF >> 3 << 5; *Bmask = 0xFF >> 3; break;
            case 16: *Rmask = 0xFF >> 3 << 11; *Gmask = 0xFF >> 2 << 5; *Bmask = 0xFF >> 3; break;
            case 24:
            case 32: *Rmask = 0xFF << 16; *Gmask = 0xFF << 8; *Bmask = 0xFF; break;
            default: SDL_RAISE_S("no standard masks exist for given bitdepth");
        }
    }
}

/* declare the functions here, so we can use them in Surface.new */
static VALUE surface_palette(VALUE self);
static VALUE surface_set_palette(VALUE self, VALUE firstValue, VALUE colors);

// METHODS:

/**
@section Initializers
@method new( size ) => Surface
@method new( size, surface ) => Surface
@method new( size, flags ) => Surface
@method new( size, flags, surface ) => Surface
@method new( size, flags, depth ) => Surface
@method new( size, flags, depth, masks ) => Surface
All these methods create a new @Surface with @size = [w, h].
If only @size is supplied, the rest of the arguments will be set to reasonable values.
If a surface is supplied, it is used to copy the values from that aren't given.

@flags is, according to SDL's documentation:
<ul>
<li>SWSURFACE:
	SDL will create the surface in system memory.
	This improves the performance of pixel level access,
	however you may not be able to take advantage of some types of hardware blitting.
<li>HWSURFACE:
	SDL will attempt to create the surface in video memory.
	This will allow SDL to take advantage of Video->Video blits (which are often accelerated).
<li>SRCCOLORKEY:
	This flag turns on colourkeying for blits from this surface.
	If HWSURFACE is also specified and colourkeyed blits are hardware-accelerated,
	then SDL will attempt to place the surface in video memory.
	Use @Surface#set_colorkey to set or clear this flag after surface creation.
<li>SRCALPHA:
	This flag turns on alpha-blending for blits from this surface.
	If HWSURFACE is also specified and alpha-blending blits are hardware-accelerated,
	then the surface will be placed in video memory if possible.
	Use @Surface#set_alpha to set or clear this flag after surface creation.
	For a 32 bitdepth surface, an alpha mask will automatically be added,
	in other cases, you will have to specify a mask.
</ul>

@depth is bitdepth, like 8, 15, 16, 24 or 32.

@masks describes the format for the pixels and is an array of [R, G, B, A]
containing 32 bit values with bits set where the colorcomponent should be stored.
For example: [0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF] describes a 32 bit color
with red in the highest values and an alpha channel. If it is not specified, the following
defaults are used:

<pre>
........ ........ ........ .RRGGGBB (8bpp)
........ ........ ....RRRR GGGGBBBB (12bpp)
........ ........ .RRRRRGG GGGBBBBB (15bpp)
........ ........ RRRRRGGG GGGBBBBB (16bpp)
........ RRRRRRRR GGGGGGGG BBBBBBBB (24 bpp)
........ RRRRRRRR GGGGGGGG BBBBBBBB (32 bpp)
RRRRRRRR GGGGGGGG BBBBBBBB AAAAAAAA (32 bpp, SRCALPHA set)
</pre>

Normally this shouldn't have to be of interest.
*/
VALUE surface_new(int argc, VALUE* argv, VALUE self)
{
    Uint32 flags = 0;
    Uint16 width, height;
    short bpp = 0;
    Uint32 Rmask, Gmask, Bmask, Amask;
    VALUE tmp;
    bool wildGuess = false;
    VALUE sizeObject, surfaceOrFlagsObject, surfaceOrDepthObject, masksObject;
    SDL_PixelFormat* pix = NULL;
    SDL_Surface* oldsurface = NULL;
    VALUE oldsurfaceobj = 0, newsurfaceobj;

    initVideo();

    rb_scan_args(argc, argv, "13", &sizeObject, &surfaceOrFlagsObject, &surfaceOrDepthObject, &masksObject);

    PARAMETER2COORD(sizeObject, &width, &height);

    if(argc>1){
        if(rb_obj_is_kind_of(surfaceOrFlagsObject, classSurface)){ // got surface on pos 1
            oldsurfaceobj = surfaceOrFlagsObject;
            oldsurface = retrieveSurfacePointer(oldsurfaceobj);
            pix = oldsurface->format;
            flags = oldsurface->flags;
        }else{
            flags = PARAMETER2FLAGS(surfaceOrFlagsObject);

            if(argc>2){     // got surface on pos 2, or depth

                if(rb_obj_is_kind_of(surfaceOrDepthObject, classSurface)){ // got Surface on pos 2
                    RUDL_VERIFY(argc == 3, "masks are taken from surface");
                    oldsurfaceobj = surfaceOrDepthObject;
                    oldsurface = retrieveSurfacePointer(oldsurfaceobj);
                    pix = oldsurface->format;
                }else{      // got depth
                    bpp = NUM2Sint16(surfaceOrDepthObject);
                    if(argc == 4){  // got masks
                        Check_Type(masksObject, T_ARRAY);
                        RUDL_VERIFY(RARRAY(masksObject)->len==4, "Need 4 elements in masks array");
                        tmp = rb_ary_entry(masksObject, 0);   Rmask = NUM2UINT(tmp);
                        tmp = rb_ary_entry(masksObject, 1);   Gmask = NUM2UINT(tmp);
                        tmp = rb_ary_entry(masksObject, 2);   Bmask = NUM2UINT(tmp);
                        tmp = rb_ary_entry(masksObject, 3);   Amask = NUM2UINT(tmp);
                    }else{  // no masks
                        setMasksFromBPP(bpp, (flags&SDL_SRCALPHA)>0, &Rmask, &Gmask, &Bmask, &Amask);
                    }
                }
            }else{
                wildGuess = true;   // only size and flags given
            }
        }
    }else{  // only size given... Guess a bit:
        wildGuess = true;
    }

    if(wildGuess){
        if(SDL_GetVideoSurface()){
            pix = SDL_GetVideoSurface()->format;
        }else{
            pix = SDL_GetVideoInfo()->vfmt;
        }
    }

    if(pix){
        bpp = pix->BitsPerPixel;
        Rmask = pix->Rmask;
        Gmask = pix->Gmask;
        Bmask = pix->Bmask;
        Amask = pix->Amask;
    }

    newsurfaceobj = createSurfaceObject(SDL_CreateRGBSurface(flags, width, height, bpp, Rmask, Gmask, Bmask, Amask));

    /* paletted (8-bit) surface given to copy values from? then copy the palette */
    if(oldsurface && oldsurface->format->BytesPerPixel == 1){
        surface_set_palette(newsurfaceobj, INT2FIX(0), surface_palette(oldsurfaceobj));
    }

    return newsurfaceobj;
}

/**
@method load_new( filename ) => Surface
This creates a @Surface with an image in it,
loaded from disk from @filename by using load_new.
The file should be in a supported file format.
If the SDL_image library was found during RUDL's installation,
it will load the following formats:
BMP, PNM, XPM, XCF, PCX, GIF, JPEG, TIFF, PNG, TGA and LBM.
If the SDL_image library was not found, only simple BMP loading is supported.
Simple means: not all BMP files can be loaded.
*/
static VALUE surface_load_new(VALUE self, VALUE filename)
{
    SDL_Surface* surface=NULL;
    initVideo();
#ifdef HAVE_SDL_IMAGE_H
    surface=IMG_Load(STR2CSTR(filename));
#else
    surface=SDL_LoadBMP(STR2CSTR(filename));
#endif
    if(!surface) SDL_RAISE;
    return createSurfaceObject(surface);
}

/**
@method String.to_surface => Surface
This creates a @Surface with an image in it,
loaded by treating @String as the image data when using @to_surface.
The @string should be in a supported file format,
just like the file for @load_new should be.
*/
static VALUE string_to_surface(VALUE self)
{
    SDL_RWops* rwops=NULL;
    SDL_Surface* surface=NULL;

    initVideo();

    rwops=SDL_RWFromMem(RSTRING(self)->ptr, RSTRING(self)->len);

#ifdef HAVE_SDL_IMAGE_H
    surface=IMG_Load_RW(rwops, 0);
#else
    surface=SDL_LoadBMP_RW(rwops, 0);
#endif

    SDL_FreeRW(rwops);
    if(!surface) SDL_RAISE;
    return createSurfaceObject(surface);
}

static VALUE surface_destroy(VALUE self);

void dont_free(void*_)
{
    DEBUG_S("dont_free");
}

/**
@method shared_new( surface ) => Surface
This method is two things:

<ol>
<li>a way to share the same bunch of data (width, height, bpp, pixeldata) between two Surface objects.
	Please don't use it this way if there isn't a very good reason for it.
<li>a way to import foreign objects that wrap an SDL_Surface*.
	If that doesn't mean anything to you, please ignore this point.
	It takes the pointer from the foreign object and creates a new Surface that wraps it.
</ol>

Garbage collection problems should be prevented by giving the new surface
a reference to @surface

Note that if the original surface is destroyed by a call to @Surface.destroy,
the shared ones will be invalidated too!
*/
static VALUE surface_shared_new(VALUE self, VALUE other)
{
    VALUE new_surface=createSurfaceObject(DATA_PTR(other));
    rb_ivar_set(new_surface, id_shared_surface_reference, other);
    RDATA(new_surface)->dfree=dont_free;
    return new_surface;
}

/**
@section Methods
@method share( other_surface ) => self
Like @Surface.shared_new, but this works on an existing surface.
It will destroy this surface, then make it share @other_surface 's data.
*/
static VALUE surface_share(VALUE self, VALUE other)
{
    if(DATA_PTR(self)!=DATA_PTR(other)){
        surface_destroy(self);
        DATA_PTR(self)=DATA_PTR(other);
        RDATA(self)->dfree=dont_free;
        rb_ivar_set(self, id_shared_surface_reference, other);
    }
    return self;
}

/**
@method immodest_export( other_surface ) => self
Like Surface.share, but this works the other way around.
It will destroy the @other_surface then make it share the data in itself
and setting a reference on @other_surface back to @self.
It's called immodest because it interferes with another object, bluntly
assuming it contains a SDL_Surface pointer and changing it to something
else...
*/
static VALUE surface_immodest_export(VALUE self, VALUE other)
{
    if(DATA_PTR(self)!=DATA_PTR(other)){
        surface_destroy(other);
        DATA_PTR(other)=DATA_PTR(self);
        RDATA(other)->dfree=dont_free;
        rb_ivar_set(other, id_shared_surface_reference, self);
    }
    return self;
}

/**
@method destroy => nil
Frees memory used by this surface.
The surface is no longer useable after this call.
*/
static VALUE surface_destroy(VALUE self)
{
    if(RDATA(self)->dfree!=dont_free){
        GET_SURFACE;
        SDL_FreeSurface(surface);
        DATA_PTR(self)=NULL;
    }else{
        rb_ivar_set(self, id_shared_surface_reference, Qnil);
        DATA_PTR(self)=NULL;
    }
    return Qnil;
}

/**
@method blit( source, coordinate ) => Array
@method blit( source, coordinate, sourceRect ) => Array
This method blits (copies, pastes, draws) @source onto the surface it is called on.
@coordinate is the position [x, y] where @source will end up in the destination surface.
@sourcerect is the area in the @source bitmap that you want blitted.
Not supplying it will blit the whole @source.

Returns the rectangle array ([x,y,w,h]) in the surface that was changed.
*/
static VALUE surface_blit(int argc, VALUE* argv, VALUE self)
{
    SDL_Surface* src;
    SDL_Surface* dest=retrieveSurfacePointer(self);

    int result;
    SDL_Rect src_rect, dest_rect;

    VALUE sourceSurfaceObject, coordinateObject, sourceRectObject;

    rb_scan_args(argc, argv, "21", &sourceSurfaceObject, &coordinateObject, &sourceRectObject);

    src=retrieveSurfacePointer(sourceSurfaceObject);
    PARAMETER2COORD(coordinateObject, &dest_rect.x, &dest_rect.y);

    if(argc==3){
        PARAMETER2CRECT(sourceRectObject, &src_rect);
        result=SDL_BlitSurface(src, &src_rect, dest, &dest_rect);
    }else{
        result=SDL_BlitSurface(src, NULL, dest, &dest_rect);
    }

    switch(result){
        case -1:SDL_RAISE;return Qnil;break;
        case -2:rb_raise(classSurfacesLostException, "all surfaces lost their contents - reload graphics");return Qnil;break;
    }

    return new_rect_from_SDL_Rect(&dest_rect);
}

/**
@method convert => Surface
@method convert! => self
Creates a new version of the surface in the current display's format,
making it faster to blit.
*/
/**
@method convert_alpha => Surface
@method convert_alpha! => self
Like @convert, creates a new version of the surface in the current display's format,
making it faster to blit.
The alpha version optimizes for fast alpha blitting.
*/
static VALUE surface_convert(VALUE self)
{
    GET_SURFACE;
    surface=SDL_DisplayFormat(surface);
    if(surface){
        return createSurfaceObject(surface);
    }else{
        SDL_RAISE;
        return Qnil;
    }
}

static VALUE surface_convert_alpha(VALUE self)
{
    GET_SURFACE;
    surface=SDL_DisplayFormatAlpha(surface);
    if(surface){
        return createSurfaceObject(surface);
    }else{
        SDL_RAISE;
        return Qnil;
    }
}

static VALUE surface_convert_(VALUE self)
{
    SDL_Surface* new_surface;
    GET_SURFACE;
    new_surface=SDL_DisplayFormat(surface);
    if(new_surface){
        SDL_FreeSurface(surface);
        DATA_PTR(self)=new_surface;
        return self;
    }else{
        SDL_RAISE;
        return Qnil;
    }
}

static VALUE surface_convert_alpha_(VALUE self)
{
    SDL_Surface* new_surface;
    GET_SURFACE;
    new_surface=SDL_DisplayFormatAlpha(surface);
    if(new_surface){
        SDL_FreeSurface(surface);
        DATA_PTR(self)=new_surface;
        return self;
    }else{
        SDL_RAISE;
        return Qnil;
    }
}

/**
@section Locking
These methods control the locking of surfaces.
If you ever encounter a locking error,
you might try these out.
Locking errors are expected when trying to access video hardware.
Keep a @Surface locked for as short a time as possible.
*/
/**
@method lock => self
Locks the surface.
*/
static VALUE surface_lock(VALUE self)
{
    if(SDL_LockSurface(retrieveSurfacePointer(self)) == -1) SDL_RAISE;
    return self;
}

/**
@method must_lock => boolean
Returns true when a surface needs locking for pixel access.
*/
static VALUE surface_must_lock(VALUE self)
{
    return INT2BOOL(SDL_MUSTLOCK(retrieveSurfacePointer(self)));
}

/**
@method unlock => self
Unlocks a surface that was locked with @lock and returns self.
*/
static VALUE surface_unlock(VALUE self)
{
    SDL_UnlockSurface(retrieveSurfacePointer(self));
    return self;
}

/**
@method locked? => boolean
Returns true when the surface is locked.
*/
static VALUE surface_locked_(VALUE self)
{
    return INT2BOOL(retrieveSurfacePointer(self)->locked);
}

/**
@section Methods
@method save_bmp( filename ) => self
This is the only method in RUDL which stores surface data.
Pass @save_bmp the @filename and the surface data will be saved to that file.
*/
static VALUE surface_save_bmp(VALUE self, VALUE filename)
{
    if(SDL_SaveBMP(retrieveSurfacePointer(self), STR2CSTR(filename))==-1) SDL_RAISE;
    return self;
}

/**
@section Size methods
These methods return the size of the surface.
*/
/**
@method size => Array[w,h]
*/
static VALUE surface_size(VALUE self)
{
    SDL_Surface* surface=retrieveSurfacePointer(self);

    return rb_ary_new3(2, UINT2NUM(surface->w), UINT2NUM(surface->h));
}
/**
@method rect => Array[0, 0, w, h]
*/
static VALUE surface_rect(VALUE self)
{
    SDL_Rect rect;
    SDL_Surface* surface=retrieveSurfacePointer(self);

    rect.x=0;
    rect.y=0;
    rect.w=surface->w;
    rect.h=surface->h;

    return new_rect_from_SDL_Rect(&rect);
}

/**
@method w => Number
Returns width in pixels.
*/
static VALUE surface_w(VALUE self)
{
    return UINT2NUM(retrieveSurfacePointer(self)->w);
}

/**
@method h => Number
Returns height in pixels.
*/
static VALUE surface_h(VALUE self)
{
    return UINT2NUM(retrieveSurfacePointer(self)->h);
}

/**
@section Colorkey methods
These methods control the color that will be completely transparent (it will not be copied
to the destination surface.)
*/
/**
@method colorkey => Array[R,G,B]
@method unset_colorkey => self
@method set_colorkey( color ) => self
@method set_colorkey( color, flags ) => self
The only flag for @flags is <code>RLEACCEL</code> which will encode the bitmap in a more efficient way for blitting,
by skipping the transparent pixels.
set_colorkey(nil) removes the color key, same as @unset_colorkey .
*/
static VALUE surface_set_colorkey(int argc, VALUE* argv, VALUE self)
{
    SDL_Surface* surface = retrieveSurfacePointer(self);
    Uint32 flags = 0, color = 0;
    VALUE colorObject, flagsObject;

    switch (rb_scan_args(argc, argv, "11", &colorObject, &flagsObject)) {
        case 2:
            flags = PARAMETER2FLAGS(flagsObject);
        case 1:
            if (colorObject == Qnil) {
                flags = 0;
            } else {
                flags |= SDL_SRCCOLORKEY;
                color = VALUE2COLOR(colorObject, surface->format);
            }
    }

    if (SDL_SetColorKey(surface, flags, color) == -1) SDL_RAISE;

    return self;
}

static VALUE surface_unset_colorkey(VALUE self)
{
    if (SDL_SetColorKey(retrieveSurfacePointer(self), 0, 0) == -1) SDL_RAISE;
    return self;
}

static VALUE surface_colorkey(VALUE self)
{
    SDL_Surface* surface=retrieveSurfacePointer(self);

    if(!(surface->flags&SDL_SRCCOLORKEY)){
        return Qnil;
    }

    return COLOR2VALUE(surface->format->colorkey, surface);
}

/**
@section Drawing
@method fill( color ) => self
@method fill( color, rect ) => self
Fills rectangle @rect in the surface with @color.
*/
static VALUE surface_fill(int argc, VALUE* argv, VALUE self)
{
    SDL_Rect rectangle;
    SDL_Surface* surface=retrieveSurfacePointer(self);

    VALUE rect, color;

    switch(rb_scan_args(argc, argv, "11", &color, &rect)){
        case 1:
            SDL_FillRect(surface, NULL, VALUE2COLOR(color, surface->format));
            break;
        case 2:
            PARAMETER2CRECT(rect, &rectangle);
            SDL_FillRect(surface, &rectangle, VALUE2COLOR(color, surface->format));
            break;
    }
    return self;
}

/**
@section Information
@method pitch
The surface pitch is the number of bytes used in each scanline.
This function should rarely needed, mainly for any special-case debugging.
*/
static VALUE surface_pitch(VALUE self)
{
    GET_SURFACE;
    return UINT2NUM(surface->pitch);
}

/**
@method bitsize
Returns the number of bits used to represent each pixel.
This value may not exactly fill the number of bytes used per pixel.
For example a 15 bit Surface still requires a full 2 bytes.
*/
static VALUE surface_bitsize(VALUE self)
{
    return UINT2NUM(retrieveSurfacePointer(self)->format->BitsPerPixel);
}

/**
@method bytesize
Returns the number of bytes used to store each pixel.
*/
static VALUE surface_bytesize(VALUE self)
{
    return UINT2NUM(retrieveSurfacePointer(self)->format->BytesPerPixel);
}

/**
@method flags
Returns the current state flags for the surface.
*/
static VALUE surface_flags(VALUE self)
{
    return UINT2NUM(retrieveSurfacePointer(self)->flags);
}

/**
@method losses => Array[redloss, greenloss, blueloss, alphaloss]
Returns the bitloss for each color plane.
The loss is the number of bits removed for each colorplane from a full 8 bits of
resolution. A value of 8 usually indicates that colorplane is not used
(like the alpha plane)
*/
static VALUE surface_losses(VALUE self)
{
    SDL_Surface* surface=retrieveSurfacePointer(self);
    return rb_ary_new3(4,
            UINT2NUM(surface->format->Rloss),
            UINT2NUM(surface->format->Gloss),
            UINT2NUM(surface->format->Bloss),
            UINT2NUM(surface->format->Aloss));
}

/**
@method shifts => Array[redshift, greenshift, blueshift, alphashift]
Returns the bitshifts used for each color plane.
The shift is determine how many bits left-shifted a colorplane value is in a
mapped color value.
*/
static VALUE surface_shifts(VALUE self)
{
    SDL_Surface* surface=retrieveSurfacePointer(self);
    return rb_ary_new3(4,
            UINT2NUM(surface->format->Rshift),
            UINT2NUM(surface->format->Gshift),
            UINT2NUM(surface->format->Bshift),
            UINT2NUM(surface->format->Ashift));
}

/**
@method masks => Array[redmask, greenmask, bluemask, alphamask]
Returns the bitmasks for each color plane.
The bitmask is used to isolate each colorplane value from a mapped color value.
A value of zero means that colorplane is not used (like alpha)
*/
static VALUE surface_masks(VALUE self)
{
    SDL_Surface* surface=retrieveSurfacePointer(self);
    return rb_ary_new3(4,
            UINT2NUM(surface->format->Rmask),
            UINT2NUM(surface->format->Gmask),
            UINT2NUM(surface->format->Bmask),
            UINT2NUM(surface->format->Amask));
}

/**
@section Palette manipulation
@method palette => Array[[R,G,B], [R,G,B],....]
@method set_palette( first, colors ) => self
These methods return or set the 256 color palette that is part of 8 bit @Surface s.
@first is the first color to change.
@colors and the return value of @palette are arrays of colors like
[[50,80,120], [255,255,0]]
=end */
static VALUE surface_palette(VALUE self)
{
    SDL_Surface* surface = retrieveSurfacePointer(self);
    SDL_Palette* pal = surface->format->palette;

    int i;

    VALUE retval;
    VALUE color;

    if(!pal) return Qnil;

    retval=rb_ary_new2(256);

    for(i=0; i<256; i++){
        color=rb_ary_new3(3,
            UINT2NUM(pal->colors[i].r),
            UINT2NUM(pal->colors[i].g),
            UINT2NUM(pal->colors[i].b));
        rb_ary_push(retval, color);
    }

    return retval;
}

static VALUE surface_set_palette(VALUE self, VALUE firstValue, VALUE colors)
{
    SDL_Surface* surface = retrieveSurfacePointer(self);
    SDL_Palette* pal = surface->format->palette;

    int first=NUM2INT(firstValue);
    int amount;
    int i;
    SDL_Color newPal[256];
    VALUE color;

    VALUE tmp;

    RUDL_VERIFY(rb_obj_is_kind_of(colors, rb_cArray), "Need array of colors");

    amount=RARRAY(colors)->len;

    if (!pal) return Qfalse;

    if (first+amount>256) amount=256-first;

    for (i=0; i<amount; i++) {
        color=rb_ary_entry(colors, i);
        tmp=rb_ary_entry(color, 0);     newPal[i].r=NUM2Uint8(tmp);
        tmp=rb_ary_entry(color, 1);     newPal[i].g=NUM2Uint8(tmp);
        tmp=rb_ary_entry(color, 2);     newPal[i].b=NUM2Uint8(tmp);
    }

    if (SDL_SetColors(surface, newPal, first, amount)==0) SDL_RAISE;

    return self;
}

/**
@section Alpha methods
Gets or sets the overall transparency for the @Surface.
An alpha of 0 is fully transparent, an alpha of 255 is fully opaque.
If your surface has a pixel alpha channel, it will override the overall surface transparency.
You'll need to change the actual pixel transparency to make changes.
If your image also has pixel alpha values and will be used repeatedly, you
will probably want to pass the <code>RLEACCEL</code> flag to the call.
This will take a short time to compile your surface, and increase the blitting speed.
Note that the per-surface alpha value of 128 is considered a special case and is
optimised, so it's much faster than other per-surface values.
*/
/**
@method alpha
@method unset_alpha
@method set_alpha( alpha )
@method set_alpha( alpha, flags )
@set_alpha(nil) removes the per-surface alpha, same as @unset_alpha.
*/
static VALUE surface_set_alpha(int argc, VALUE* argv, VALUE self)
{
    SDL_Surface* surface = retrieveSurfacePointer(self);
    Uint32 flags = SDL_SRCALPHA;
    Uint8 alpha = 0;

    VALUE alphaObject, flagsObject;

    switch (rb_scan_args(argc, argv, "11", &alphaObject, &flagsObject)) {
        case 2:
            flags = PARAMETER2FLAGS(flagsObject);
    }

    if (alphaObject == Qnil) {
        flags = alpha = 0;
    } else {
        alpha = (Uint8) NUM2UINT(alphaObject);
    }

    if (SDL_SetAlpha(surface, flags, alpha) == -1) SDL_RAISE;

    return self;
}

static VALUE surface_unset_alpha(VALUE self)
{
    if (SDL_SetAlpha(retrieveSurfacePointer(self), 0, 0) == -1) SDL_RAISE;
    return self;
}

static VALUE surface_alpha(VALUE self)
{
    SDL_Surface* surface = retrieveSurfacePointer(self);
    if(surface->flags & SDL_SRCALPHA){
        return UINT2NUM(surface->format->alpha);
    }
    return Qnil;
}


/**
@section Clipping
@method clip
@method unset_clip
@method clip=( rect )
Retrieves, removes or sets the clipping rectangle for surfaces that are
blitted to this surface.
*/
static VALUE surface_unset_clip(VALUE self)
{
    SDL_SetClipRect(retrieveSurfacePointer(self), NULL);
    return self;
}

static VALUE surface_clip_(VALUE self, VALUE rectObject)
{
    SDL_Rect rect;
    PARAMETER2CRECT(rectObject, &rect);
    SDL_SetClipRect(retrieveSurfacePointer(self), &rect);
    return self;
}

static VALUE surface_clip(VALUE self)
{
    return new_rect_from_SDL_Rect(&(retrieveSurfacePointer(self)->clip_rect));
}

/**
@section Drawing
@method get( x, y )
@method get( coordinate )
These methods read single pixels on a surface.
@get or @[] get the color of a pixel.
The coordinate can be given as an [x,y] array or two separate numbers.
@get is an alias for @[].
These methods require the surface to be locked if necessary.
@[]= and @[] are the only methods in RUDL that take separate x and y coordinates.
See also: @Surface.plot, @Surface.[]=
*/
/**
@method [ x, y ]
@method [ coordinate ]
See @get
*/
__inline__ Uint32 internal_get(SDL_Surface* surface, Sint16 x, Sint16 y)
{
    SDL_PixelFormat* format = surface->format;
    Uint8* pixels;
    Uint32 color;
    Uint8* pix;

    if(x < 0 || x >= surface->w || y < 0 || y >= surface->h){
        return 0;
    }

    SDL_LockSurface(surface);
    pixels = (Uint8*)surface->pixels;

    RUDL_ASSERT(format->BytesPerPixel>=1, "Color depth too small for surface (<1)");
    RUDL_ASSERT(format->BytesPerPixel<=4, "Color depth too large for surface (>4)");

    switch(format->BytesPerPixel){
        case 1:
            color = (Uint32)*((Uint8*)pixels + y * surface->pitch + x);
            break;
        case 2:
            color = (Uint32)*((Uint16*)(pixels + y * surface->pitch) + x);
            break;
        case 3:
            pix = ((Uint8*)(pixels + y * surface->pitch) + x * 3);
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
            color = (pix[0]) + (pix[1]<<8) + (pix[2]<<16);
#else
            color = (pix[2]) + (pix[1]<<8) + (pix[0]<<16);
#endif
            break;
        default: /*case 4:*/
            color = *((Uint32*)(pixels + y * surface->pitch) + x);
            break;
    }
    SDL_UnlockSurface(surface);
    return color;
}

__inline__ Uint32 internal_nonlocking_get(SDL_Surface* surface, Sint16 x, Sint16 y)
{
    SDL_PixelFormat* format = surface->format;
    Uint8* pixels;
    Uint32 color;
    Uint8* pix;

    if(x < 0 || x >= surface->w || y < 0 || y >= surface->h){
        return 0;
    }

    SDL_LockSurface(surface);
    pixels = (Uint8*)surface->pixels;

    RUDL_ASSERT(format->BytesPerPixel>=1, "Color depth too small for surface (<1)");
    RUDL_ASSERT(format->BytesPerPixel<=4, "Color depth too large for surface (>4)");

    switch(format->BytesPerPixel){
        case 1:
            color = (Uint32)*((Uint8*)pixels + y * surface->pitch + x);
            break;
        case 2:
            color = (Uint32)*((Uint16*)(pixels + y * surface->pitch) + x);
            break;
        case 3:
            pix = ((Uint8*)(pixels + y * surface->pitch) + x * 3);
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
            color = (pix[0]) + (pix[1]<<8) + (pix[2]<<16);
#else
            color = (pix[2]) + (pix[1]<<8) + (pix[0]<<16);
#endif
            break;
        default: /*case 4:*/
            color = *((Uint32*)(pixels + y * surface->pitch) + x);
            break;
    }
    SDL_UnlockSurface(surface);
    return color;
}


static VALUE surface_get(int argc, VALUE* argv, VALUE self)
{
    SDL_Surface* surface = retrieveSurfacePointer(self);
    Uint8 r, g, b, a;
    Sint16 x, y;

    /* did we get an [x,y] array or x,y separately? */
    if (argc == 1) {
        PARAMETER2COORD(argv[0], &x, &y);
    } else if (argc == 2) {
        x = NUM2Sint16(argv[0]);
        y = NUM2Sint16(argv[1]);
    } else {
        rb_raise(rb_eArgError, "wrong number of arguments");
    }

    SDL_GetRGBA(internal_get(surface, x, y), surface->format, &r, &g, &b, &a);

    return rb_ary_new3(4, UINT2NUM(r), UINT2NUM(g), UINT2NUM(b), UINT2NUM(a));
}

__inline__ static void* get_line_pointer(SDL_Surface* surface, int y)
{
    return (((Uint8*)surface->pixels)+surface->pitch*y);
}

__inline__ static void copy_line_to_surface(SDL_Surface* surface, int y, Uint8* data)
{
    memcpy(get_line_pointer(surface, y), data, surface->w*surface->format->BytesPerPixel);
}

__inline__ static void copy_surface_to_line(SDL_Surface* surface, int y, Uint8* data)
{
    memcpy(data, get_line_pointer(surface, y), surface->w*surface->format->BytesPerPixel);
}

/**@section Batch pixel access
These methods manipulate the pixels in the Surface.
The transport medium is a string with binary data in it.
The data is raw, no fancy color arrays here.
If bytesize (the amount of bytes used to describe the color of a pixel) is
four, for example, a (({Surface})) of 5x5 pixels will return a string of length (5x5x4).
If the colorformat is specified as BGRA, then character zero will be the
B component, character one the G component etc.
Eight bit color surfaces store one byte indexes into the palette.
These methods perform best when the surface's pitch is equal to its width.
There is not much errorchecking so beware of crashes.
*/
/**
@method rows
Returns an array of strings with one row of imagedata each.
*/
/**
@method get_row( y )
Get a single row.
*/
/**
@method set_row( y, pixels )
Set a single row.
*/
/**
@method each_row { |row_of_pixels| ... }
@method each_row! { |row_of_pixels| ... }
@each_row and @each_row! iterate through the rows,
passing each of them to the supplied codeblock.
*/
static VALUE surface_get_row(VALUE self, VALUE yval)
{
    int y = NUM2INT(yval);
    VALUE ret;
    GET_SURFACE;

    RUDL_VERIFY(y >= 0 && y < surface->h, "coordinate out of bounds");

    SDL_LockSurface(surface);
    ret = rb_str_new(get_line_pointer(surface, y), surface->w*surface->format->BytesPerPixel);
    SDL_UnlockSurface(surface);

    return ret;
}

static VALUE surface_set_row(VALUE self, VALUE yval, VALUE pixels)
{
    int y = NUM2INT(yval);
    GET_SURFACE;

    RUDL_VERIFY(y >= 0 && y < surface->h, "coordinate out of bounds");

    RUDL_ASSERT(RSTRING(pixels)->len >= surface->w*surface->format->BytesPerPixel, "Not enough data for a complete row");

    SDL_LockSurface(surface);
    copy_line_to_surface(surface, y, RSTRING(pixels)->ptr);
    SDL_UnlockSurface(surface);
    return self;
}

void define_ruby_row_methods()
{
    rb_eval_string(
        "module RUDL class Surface              \n"
        "   def each_row                        \n"
        "       (0...h).each {|y|               \n"
        "           yield(get_row(y))           \n"
        "       }                               \n"
        "       self                            \n"
        "   end                                 \n"
        "   def each_row!                       \n"
        "       (0...h).each {|y|                   \n"
        "           set_row(y, yield(get_row(y)))   \n"
        "       }                                   \n"
        "       self                            \n"
        "   end                                 \n"
        "   def rows                            \n"
        "       retval=[]                       \n"
        "       each_row {|r| retval.push(r)}   \n"
        "       retval                          \n"
        "   end                                 \n"
        "   def rows=(rows)                     \n"
        "       (0...h).each {|row|             \n"
        "           set_row(row, rows[row])     \n"
        "       }                               \n"
        "       self                            \n"
        "   end                                 \n"
        "end end                                \n"
    );
}

/**
@method columns
Returns an array of strings with one column of imagedata each.
*/
/**
@method get_column( x )
Get a column.
*/
/**
@method set_column( x, pixels )
Set a column
*/
/**
@method each_column { |column_of_pixels| ... }
@method each_column! { |column_of_pixels| ... }
@each_column and @each_column! iterate through the columns,
passing each of them to the supplied codeblock.
*/
static VALUE surface_get_column(VALUE self, VALUE xval)
{
    int x = NUM2INT(xval), y, h, pixelsize, pitch;
    Uint8 *src, *dest, *column;

    GET_SURFACE;

    RUDL_VERIFY(x >= 0 && x < surface->w, "coordinate out of bounds");

    h = surface->h;
    pixelsize = surface->format->BytesPerPixel;
    pitch = surface->pitch;
    column = ALLOCA_N(Uint8, h*pixelsize);

    SDL_LockSurface(surface);

    src = ((Uint8*) surface->pixels) + x*pixelsize;
    dest = column;

    for (y=0; y<h; y++) {
        memcpy(dest, src, pixelsize);
        dest += pixelsize;
        src += pitch;
    }

    SDL_UnlockSurface(surface);

    return rb_str_new(column, h*pixelsize);
}

static VALUE surface_set_column(VALUE self, VALUE xval, VALUE pixels)
{
    int x = NUM2INT(xval), y, h, pixelsize, pitch;
    Uint8 *src, *dest;

    GET_SURFACE;

    RUDL_VERIFY(x >= 0 && x < surface->w, "coordinate out of bounds");

    h = surface->h;
    pixelsize = surface->format->BytesPerPixel;
    pitch = surface->pitch;

    SDL_LockSurface(surface);

    dest = ((Uint8*) surface->pixels) + x*pixelsize;
    src = RSTRING(pixels)->ptr;

    for (y=0; y<h; y++) {
        memcpy(dest, src, pixelsize);
        dest += pitch;
        src += pixelsize;
    }

    SDL_UnlockSurface(surface);

    return self;
}

void define_ruby_column_methods()
{
    rb_eval_string(
        "module RUDL class Surface              \n"
        "   def each_column                     \n"
        "       (0...w).each {|x|               \n"
        "           yield(get_column(x))        \n"
        "       }                               \n"
        "       self                            \n"
        "   end                                 \n"
        "   def each_column!                    \n"
        "       (0...w).each {|x|                       \n"
        "           set_column(x, yield(get_column(x))) \n"
        "       }                                       \n"
        "       self                                    \n"
        "   end                                 \n"
        "   def columns                         \n"
        "       retval=[]                           \n"
        "       each_column {|c| retval.push(c)}    \n"
        "       retval                              \n"
        "   end                                 \n"

        "   def columns=(cols)                  \n"
        "       (0...w).each {|col|             \n"
        "           set_column(col, cols[col])  \n"
        "       }                               \n"
        "       self                            \n"
        "   end                                 \n"
        "end end                                \n"
    );
}

/**
@method pixels
@method pixels=( pixeldata )
These methods get and set all image data at once.
*/
static VALUE surface_pixels(VALUE self)
{
    Uint32 image_size;
    GET_SURFACE;

    image_size=surface->w*surface->h*surface->format->BytesPerPixel;

    if(surface->pitch==surface->w){
        return rb_str_new(surface->pixels, image_size);
    }else{
        int y;
        Uint8* tmp_pixels=ALLOCA_N(Uint8, image_size);
        VALUE retval;
        Uint16 bytewidth=surface->w*surface->format->BytesPerPixel;

        for(y=0; y<surface->h; y++){
            copy_surface_to_line(surface, y, tmp_pixels+y*bytewidth);
        }
        retval=rb_str_new(tmp_pixels, image_size);
        return retval;
    }
}

static VALUE surface_set_pixels(VALUE self, VALUE pixels)
{
    int size;
    Uint8* pixelpointer;

    GET_SURFACE;

    Check_Type(pixels, T_STRING);
    size=surface->w*surface->h*surface->format->BytesPerPixel;
    pixelpointer=RSTRING(pixels)->ptr;

    RUDL_ASSERT(RSTRING(pixels)->len>=size, "Not enough data in string");

    if(surface->pitch==surface->w){
        memcpy(surface->pixels, pixelpointer, size);
    }else{
        int y;
        Uint16 bytewidth=surface->w*surface->format->BytesPerPixel;
        for(y=0; y<surface->h; y++){
            copy_line_to_surface(surface, y, pixelpointer+y*bytewidth);
        }
    }
    return self;
}

/**@section Scaling, Flipping and Rotating
@method scale2x => Surface
@method scale2x( dest_surface ) => Surface
@method scale2x( dest_surface, coordinate ) => Surface
Scales the surface to double size with the Scale2x algorithm developed
by Andrea Mazzoleni.
See <a href='http://scale2x.sourceforge.net/'>the project page</a>.

Creates a new surface to hold the result, or reuses @dest_surface,
which must be at least twice as wide and twice as high as this surface,
and have the same depth.

@coordinate is the [x, y] coordinate where you want the scaled image
positioned. This way you can draw it directly on screen at the wanted
position, without having to use a temporary Surface.
*/

#include "scale2x.h"

static VALUE surface_scale2x(int argc, VALUE* argv, VALUE self)
{
    VALUE dest, coord;
    SDL_Surface* srcsurface = retrieveSurfacePointer(self);
    SDL_Surface* destsurface;
    int bipp = srcsurface->format->BitsPerPixel;
    int w = srcsurface->w, h = srcsurface->h;
    Sint16 x = 0, y = 0;

    rb_scan_args(argc, argv, "02", &dest, &coord);

    RUDL_VERIFY(w>=2 && h>=2, "Source surface not large enough");

    /* were we given a target coordinate? */
    if (argc == 2) {
        PARAMETER2COORD(coord, &x, &y);
        RUDL_VERIFY(x>=0 && y>=0, "Destination coordinate cannot be negative");
    }

    /* were we given a destination surface? */
    if (argc > 0) {
        destsurface = retrieveSurfacePointer(dest);
        RUDL_VERIFY(destsurface->format->BitsPerPixel == bipp, "Destination surface has wrong depth");
        RUDL_VERIFY(destsurface->w >= x+2*w && destsurface->h >= y+2*h, "Destination surface is too small");
    } else {
        /* nope, create a new one */
        VALUE newargv[] = {rb_ary_new3(2, INT2FIX(2*w), INT2FIX(2*h)), self};
        dest = surface_new(2, newargv, classSurface);
        destsurface = retrieveSurfacePointer(dest);
    }

    scale2x(srcsurface, destsurface, x, y);

    return dest;
}

/**
@method mirror_y => Surface
Mirrors the surface vertically into a new Surface.
*/
static VALUE surface_mirror_y(VALUE self)
{
    SDL_Surface* src = retrieveSurfacePointer(self);
    SDL_Surface* dest;
    VALUE destsurf;
    int bpp = src->format->BytesPerPixel;
    int w = src->w, h = src->h;
    int y, srcpitch, destpitch;
    Uint8 *srcline, *destline;

    /* create a new surface for the result */
    VALUE newargv[] = {rb_ary_new3(2, INT2FIX(w), INT2FIX(h)), self};
    destsurf = surface_new(2, newargv, classSurface);
    dest = retrieveSurfacePointer(destsurf);

    SDL_LockSurface(src);
    SDL_LockSurface(dest);

    /* just in case the surfaces' pitches can be different?? */
    srcpitch = src->pitch;
    destpitch = dest->pitch;

    srcline = src->pixels;
    destline = ((Uint8*) dest->pixels) + (h-1)*destpitch;
    for (y = 0; y < h; y++) {
        memcpy(destline, srcline, w*bpp);
        srcline += srcpitch;
        destline -= destpitch;
    }

    SDL_UnlockSurface(src);
    SDL_UnlockSurface(dest);

    return destsurf;
}

/**
@method mirror_x => Surface
Mirrors the surface horizontally into a new @Surface.
*/
static VALUE surface_mirror_x(VALUE self)
{
    SDL_Surface* src = retrieveSurfacePointer(self);
    SDL_Surface* dest;
    VALUE destsurf;
    int bpp = src->format->BytesPerPixel;
    int w = src->w, h = src->h;
    int x, y, b, srcextra, destpitch2;
    Uint8 *srcline, *destline;

    /* create a new surface for the result */
    VALUE newargv[] = {rb_ary_new3(2, INT2FIX(w), INT2FIX(h)), self};
    destsurf = surface_new(2, newargv, classSurface);
    dest = retrieveSurfacePointer(destsurf);

    SDL_LockSurface(src);
    SDL_LockSurface(dest);

    /* just in case the source surface's pitch != w*bpp */
    srcextra = src->pitch - w*bpp;
    destpitch2 = 2*dest->pitch;

    srcline = src->pixels;
    destline = ((Uint8*) dest->pixels) + (w-1)*bpp;
    for (y = 0; y < h; y++) {
        for (x = 0; x < w; x++) {
            for (b = 0; b < bpp; b++) {
                destline[b] = srcline[b];
            }
            srcline += bpp;
            destline -= bpp;
        }
        srcline += srcextra;
        destline += destpitch2;
    }

    SDL_UnlockSurface(src);
    SDL_UnlockSurface(dest);

    return destsurf;
}


///////////////////////////////// INIT
void initVideoSurfaceClasses()
{
    classSurface=rb_define_class_under(moduleRUDL, "Surface", rb_cObject);
    rb_define_singleton_method(classSurface, "new", surface_new, -1);
    rb_define_singleton_method(classSurface, "load_new", surface_load_new, 1);
    rb_define_singleton_method(classSurface, "shared_new", surface_shared_new, 1);
    rb_define_method(rb_cString, "to_surface", string_to_surface, 0);
    rb_define_method(classSurface, "destroy", surface_destroy, 0);
    rb_define_method(classSurface, "save_bmp", surface_save_bmp, 1);

    rb_define_method(classSurface, "w", surface_w, 0);
    rb_define_method(classSurface, "h", surface_h, 0);
    rb_define_method(classSurface, "size", surface_size, 0);
    rb_define_method(classSurface, "rect", surface_rect, 0);

    rb_define_method(classSurface, "blit", surface_blit, -1);

    rb_define_method(classSurface, "convert", surface_convert, 0);
    rb_define_method(classSurface, "convert_alpha", surface_convert_alpha, 0);
    rb_define_method(classSurface, "convert!", surface_convert_, 0);
    rb_define_method(classSurface, "convert_alpha!", surface_convert_alpha_, 0);

//  rb_define_method(classSurface, "contained_images", surface_contained_images, 0);

    rb_define_method(classSurface, "lock", surface_lock, 0);
    rb_define_method(classSurface, "must_lock", surface_must_lock, 0);
    rb_define_method(classSurface, "unlock", surface_unlock, 0);
    rb_define_method(classSurface, "locked?", surface_locked_, 0);

    rb_define_method(classSurface, "alpha", surface_alpha, 0);
    rb_define_method(classSurface, "set_alpha", surface_set_alpha, -1);
    rb_define_method(classSurface, "unset_alpha", surface_unset_alpha, 0);

    rb_define_method(classSurface, "flags", surface_flags, 0);
    rb_define_method(classSurface, "pitch", surface_pitch, 0);
    rb_define_method(classSurface, "bitsize", surface_bitsize, 0);
    rb_define_method(classSurface, "bytesize", surface_bytesize, 0);
    rb_define_method(classSurface, "shifts", surface_shifts, 0);
    rb_define_method(classSurface, "losses", surface_losses, 0);
    rb_define_method(classSurface, "masks", surface_masks, 0);

    rb_define_method(classSurface, "clip", surface_clip, 0);
    rb_define_method(classSurface, "clip=", surface_clip_, 1);
    rb_define_method(classSurface, "unset_clip", surface_unset_clip, 0);

    rb_define_method(classSurface, "palette", surface_palette, 0);
    rb_define_method(classSurface, "set_palette", surface_set_palette, 2);

    rb_define_method(classSurface, "set_colorkey", surface_set_colorkey, -1);
    rb_define_method(classSurface, "unset_colorkey", surface_unset_colorkey, 0);
    rb_define_method(classSurface, "colorkey", surface_colorkey, 0);

    rb_define_method(classSurface, "share", surface_share, 1);
    rb_define_method(classSurface, "immodest_export", surface_immodest_export, 1);

    rb_define_method(classSurface, "[]", surface_get, -1);
    rb_alias(classSurface, rb_intern("get"), rb_intern("[]"));
    rb_define_method(classSurface, "fill", surface_fill, -1);

    rb_define_method(classSurface, "pixels", surface_pixels, 0);
    rb_define_method(classSurface, "pixels=", surface_set_pixels, 1);

    rb_define_method(classSurface, "get_row", surface_get_row, 1);
    rb_define_method(classSurface, "set_row", surface_set_row, 2);
    define_ruby_row_methods();

    rb_define_method(classSurface, "get_column", surface_get_column, 1);
    rb_define_method(classSurface, "set_column", surface_set_column, 2);
    define_ruby_column_methods();

    rb_define_method(classSurface, "scale2x", surface_scale2x, -1);
    rb_define_method(classSurface, "mirror_y", surface_mirror_y, 0);
    rb_define_method(classSurface, "mirror_x", surface_mirror_x, 0);

    id_shared_surface_reference=rb_intern("__shared_surface_reference");

    rb_eval_string(
            "module RUDL class Surface                  \n"
            "   def inspect                             \n"
            "       \"<Surface: #{w}x#{h},#{bitsize}>\" \n"
            "   end                                     \n"
            "end end                                    \n"
    );
}
