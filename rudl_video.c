/* RUDL - a C library wrapping SDL for use in Ruby. Copyright (C) 2001, 2002  Danny van Bruggen */
#include "rudl_events.h"
#include "rudl_video.h"

#ifdef HAVE_SDL_ROTOZOOM_H
#include "SDL_rotozoom.h"
#endif

#ifdef HAVE_SDL_GFXPRIMITIVES_H
#include "SDL_gfxPrimitives.h"
#endif

#ifdef HAVE_SDL_IMAGE_H
#include "SDL_image.h"
#endif

extern void add_ruby_to_rect();

ID id_rect, id_atx, id_aty, id_atw, id_ath;

void initVideo()
{
	initSDL();

	if(!SDL_WasInit(SDL_INIT_VIDEO)){
		if(SDL_WasInit(SDL_INIT_AUDIO)){
			SDL_RAISE_S("Always start video before audio");
		}
		
		if(SDL_InitSubSystem(SDL_INIT_VIDEO)){
			SDL_RAISE;
		}
		//SDL_EnableUNICODE(1);
	}
}
Uint32 VALUE2COLOR_NOMAP(VALUE colorObject)
{
	if(rb_obj_is_kind_of(colorObject, rb_cArray)){
		switch(RARRAY(colorObject)->len){
			case 3:
				return (((Uint32)(NUM2UINT(rb_ary_entry(colorObject, 0))))<<24)+
					(((Uint32)(NUM2UINT(rb_ary_entry(colorObject, 1))))<<16)+
					(((Uint32)(NUM2UINT(rb_ary_entry(colorObject, 2))))<<8)+
					((Uint32)0x000000ff);
				break;
			case 4:
				return (((Uint32)(NUM2UINT(rb_ary_entry(colorObject, 0))))<<24)+
					(((Uint32)(NUM2UINT(rb_ary_entry(colorObject, 1))))<<16)+
					(((Uint32)(NUM2UINT(rb_ary_entry(colorObject, 2))))<<8)+
					((Uint32)(NUM2UINT(rb_ary_entry(colorObject, 3))));
				break;
			default:
				rb_raise(rb_eTypeError, "Need colorarray with 3 or 4 elements");
				return Qnil;
		}
	}else{
		return NUM2UINT(colorObject);
	}
}

Uint32 VALUE2COLOR(VALUE colorObject, SDL_PixelFormat* format)
{
	if(rb_obj_is_kind_of(colorObject, rb_cArray)){
		switch(RARRAY(colorObject)->len){
			case 3:
				return SDL_MapRGB(format,
						NUM2UINT(rb_ary_entry(colorObject, 0)),
						NUM2UINT(rb_ary_entry(colorObject, 1)),
						NUM2UINT(rb_ary_entry(colorObject, 2)));
				break;
			case 4:
				return SDL_MapRGBA(format,
						NUM2UINT(rb_ary_entry(colorObject, 0)),
						NUM2UINT(rb_ary_entry(colorObject, 1)),
						NUM2UINT(rb_ary_entry(colorObject, 2)),
						NUM2UINT(rb_ary_entry(colorObject, 3)));
				break;
			default:
				rb_raise(rb_eTypeError, "Need colorarray with 3 or 4 elements");
				return Qnil;
		}
	}else{
		return NUM2UINT(colorObject);
	}
}

VALUE COLOR2VALUE(Uint32 color, SDL_Surface* surface)
{
	Uint8 r,g,b,a;

	// Is this construction usefull? Should I always user GetRGBA?
	if(surface->flags&SDL_SRCALPHA){
		SDL_GetRGBA(color, surface->format, &r, &g, &b, &a);
		return rb_ary_new3(4, UINT2NUM(r), UINT2NUM(g), UINT2NUM(b), UINT2NUM(a));
	}else{
		SDL_GetRGB(color, surface->format, &r, &g, &b);
		return rb_ary_new3(3, UINT2NUM(r), UINT2NUM(g), UINT2NUM(b));
	}
}

void RECT2CRECT(VALUE source, SDL_Rect* destination)
{
	destination->x=NUM2INT(rb_ivar_get(source, id_atx));
	destination->y=NUM2INT(rb_ivar_get(source, id_aty));
	destination->w=NUM2UINT(rb_ivar_get(source, id_atw));
	destination->h=NUM2UINT(rb_ivar_get(source, id_ath));
}

void CRECT2RECT(SDL_Rect* source, VALUE destination)
{
	rb_ivar_set(destination, id_atx, INT2NUM(source->x));
	rb_ivar_set(destination, id_aty, INT2NUM(source->y));
	rb_ivar_set(destination, id_atw, UINT2NUM(source->w));
	rb_ivar_set(destination, id_ath, UINT2NUM(source->h));
}

void PARAMETER2COORD(VALUE parameter, Sint16* x, Sint16* y)
{
	if(rb_obj_is_kind_of(parameter, rb_cArray)){
		if(RARRAY(parameter)->len==2){
			*x=NUM2INT(rb_ary_entry(parameter, 0));
			*y=NUM2INT(rb_ary_entry(parameter, 1));
		}else{
			rb_raise(rb_eTypeError, "Need coordinate array with 2 elements");
		}
	}else{
		rb_raise(rb_eTypeError, "Expected coordinate array with 2 elements");
	}
}

void PARAMETER2CRECT(VALUE arg1, SDL_Rect* rect)
{
	if(rb_obj_is_kind_of(arg1, classRect)){
		RECT2CRECT(arg1, rect);
	}else{
		if(rb_obj_is_kind_of(arg1, rb_cArray)){
			if(RARRAY(arg1)->len==4){
				rect->x=NUM2INT(rb_ary_entry(arg1, 0));
				rect->y=NUM2INT(rb_ary_entry(arg1, 1));
				rect->w=NUM2UINT(rb_ary_entry(arg1, 2));
				rect->h=NUM2UINT(rb_ary_entry(arg1, 3));
			}else{
				rb_raise(rb_eTypeError, "Need rectangle array with 4 elements");
			}
		}else{
			rb_raise(rb_eTypeError, "Wanted RUDL::Rect or array");
		}
	}
}

///////////////////////////////// SURFACE
/*
=begin
= Surface
A surface is a two dimensional array of pixels.
This might not seem like much, but it is just about the most important class in RUDL.
=end */

// SUPPORT:

VALUE createSurfaceObject(SDL_Surface* surface)
{
	return Data_Wrap_Struct(classSurface, 0, SDL_FreeSurface, surface);
}

SDL_Surface* retrieveSurfacePointer(VALUE self)
{
	SDL_Surface* surface;
	Data_Get_Struct(self, SDL_Surface, surface);
	return surface;
}

void setMasksFromBPP(Uint32 bpp, Uint32* Rmask, Uint32* Gmask, Uint32* Bmask, Uint32* Amask)
{
	*Amask = 0;
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


// METHODS:

/*
=begin
== Class Methods
--- Surface.new( size )
--- Surface.new( size, surface )
--- Surface.new( size, flags )
--- Surface.new( size, flags, surface )
--- Surface.new( size, flags, depth )
--- Surface.new( size, flags, depth, masks )
All these methods create a new (({Surface})) with ((|size|)) = [w, h].
If only ((|size|)) is supplied, the rest of the arguments will be set to reasonable values.
If a surface is supplied, it is used to copy the values from that aren't given.

((|flags|)) is, quoted from SDL's documentation:
* SWSURFACE: SDL will create the surface in system memory. This improves the performance 
  of pixel level access, however you may not be able to take advantage of some types of
  hardware blitting.
* HWSURFACE: SDL will attempt to create the surface in video memory. This will allow SDL 
  to take advantage of Video->Video blits (which are often accelerated).
* SRCCOLORKEY: With this flag SDL will attempt to find the best location for this surface, 
  either in system memory or video memory, to obtain hardware colorkey blitting support.
* SRCALPHA: With this flag SDL will attempt to find the best location for this surface, 
  either in system memory or video memory, to obtain hardware alpha support.

((|depth|)) is bitdepth, like 8, 15, 16, 24 or 32.

((|masks|)) describes the format for the pixels and is an array of [R, G, B, A]
=end */
static VALUE surface_new(int argc, VALUE* argv, VALUE self)
{
	Uint32 flags = 0;
	Uint16 width, height;
	short bpp=0;
	Uint32 Rmask, Gmask, Bmask, Amask;
	bool wildGuess=false;

	VALUE sizeObject, surfaceOrFlagsObject, surfaceOrDepthObject, masksObject;

	SDL_PixelFormat* pix=NULL;

	initVideo();
	
	rb_scan_args(argc, argv, "13", &sizeObject, &surfaceOrFlagsObject, &surfaceOrDepthObject, &masksObject);

	PARAMETER2COORD(sizeObject, &width, &height);

	if(argc>1){
		if(rb_obj_is_kind_of(surfaceOrFlagsObject, classSurface)){ // got surface on pos 1
			pix=retrieveSurfacePointer(surfaceOrFlagsObject)->format;
			flags=retrieveSurfacePointer(surfaceOrFlagsObject)->flags;
		}else{
			flags=PARAMETER2FLAGS(surfaceOrFlagsObject);

			if(argc>2){ // got surface on pos 2, or depth
			
				if(rb_obj_is_kind_of(surfaceOrDepthObject, classSurface)){ // got Surface on pos 2
					if(argc==3){
						pix=retrieveSurfacePointer(surfaceOrDepthObject)->format;
					}else{
						SDL_RAISE_S("masks are taken from surface");
						return Qnil;
					}
				}else{ // got depth
					bpp=NUM2INT(surfaceOrDepthObject);
					if(argc==4){ // got masks
						Check_Type(masksObject, T_ARRAY);
						if(RARRAY(masksObject)->len==4){
							Rmask=NUM2UINT(rb_ary_entry(masksObject, 0));
							Gmask=NUM2UINT(rb_ary_entry(masksObject, 1));
							Bmask=NUM2UINT(rb_ary_entry(masksObject, 2));
							Amask=NUM2UINT(rb_ary_entry(masksObject, 3));
						}else{
							SDL_RAISE_S("Need 4 elements in masks array");
						}
					}else{ // no masks
						setMasksFromBPP(bpp, &Rmask, &Gmask, &Bmask, &Amask);
					}
				}
			}else{
				wildGuess=true; // only size and flags given
			}
		}
	}else{ // only size given... Guess a bit:
		wildGuess=true;
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

	return createSurfaceObject(SDL_CreateRGBSurface(flags, width, height, bpp, Rmask, Gmask, Bmask, Amask));
}

/*
=begin
--- Surface.load_new( filename )
This creates a (({Surface})) with an image in it, loaded from ((|filename|)).
If the SDL_image library was found during RUDL's installation, it will load many formats, like
BMP, PNM, XPM, PCX, GIF, JPEG, PNG and TGA.
If the SDL_image library was not found, only BMP loading is supported.
=end */
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

/*
=begin
== Instance Methods
--- Surface#blit( source, coordinate )
--- Surface#blit( source, coordinate, sourceRect )
This method blits ((|source|)) onto the (({Surface})).
((|coordinate|)) is the position [x, y] where ((|source|)) will end up in the destination (({Surface})).
((|sourcerect|)) can be used to blit only a portion of the ((|source|)).
=end */
static VALUE surface_blit(int argc, VALUE* argv, VALUE self)
{
	SDL_Surface* src;
	SDL_Surface* dest=retrieveSurfacePointer(self);

	int result;
	SDL_Rect src_rect, dest_rect;
	VALUE retval;
	
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

	retval=rb_obj_alloc(classRect);
	CRECT2RECT(&dest_rect, retval);

	return retval;
}

/*
=begin
--- Surface#convert
--- Surface#convert_alpha
Converts this surface to the current display's format, making it faster to blit.
The alpha version optimizes for fast alpha blitting.
=end */
static VALUE surface_convert(VALUE self)
{
	return createSurfaceObject(SDL_DisplayFormat(retrieveSurfacePointer(self)));
}

static VALUE surface_convert_alpha(VALUE self)
{
	return createSurfaceObject(SDL_DisplayFormatAlpha(retrieveSurfacePointer(self)));
}

/*
=begin
--- Surface#lock
--- Surface#must_lock
--- Surface#unlock
--- Surface#locked?
These methods control the locking of surfaces.
Locking is needed when the pixels in the surface need to be accessed.
* must_lock returns true when a surface needs locking for pixel access.
* lock locks the surface.
* unlock unlocks it again.
* locked? returns true when the surface is locked.
=end */
static VALUE surface_lock(VALUE self)
{
	if(SDL_LockSurface(retrieveSurfacePointer(self)) == -1) SDL_RAISE;
	return self;
}

static VALUE surface_must_lock(VALUE self)
{
	return INT2BOOL(SDL_MUSTLOCK(retrieveSurfacePointer(self)));
}

static VALUE surface_unlock(VALUE self)
{
	SDL_UnlockSurface(retrieveSurfacePointer(self));
	return self;
}

static VALUE surface_locked_(VALUE self)
{
	return INT2BOOL(retrieveSurfacePointer(self)->locked);
}

/*
=begin
--- Surface#save_bmp( filename ) => self
This is the only method in RUDL which stores surface data.
Pass it the filename and the surface data will be saved to that file.
=end */
static VALUE surface_save_bmp(VALUE self, VALUE filename)
{
	if(SDL_SaveBMP(retrieveSurfacePointer(self), STR2CSTR(filename))==-1) SDL_RAISE;
	return self;
}

/*
=begin
--- Surface#w
--- Surface#h
--- Surface#size
--- Surface#rect
These methods return the size of the surface.
w returns width, h returns height, size returns [w, h] and rect returns an array of [0, 0, w, h].
=end */
static VALUE surface_size(VALUE self)
{
	SDL_Surface* surface=retrieveSurfacePointer(self);

	return rb_ary_new3(2, UINT2NUM(surface->w), UINT2NUM(surface->h));
}

static VALUE surface_rect(VALUE self)
{
	SDL_Rect rect;
	SDL_Surface* surface=retrieveSurfacePointer(self);
	VALUE retval;
	
	rect.x=0;
	rect.y=0;
	rect.w=surface->w;
	rect.h=surface->h;

	retval=rb_obj_alloc(classRect);
	CRECT2RECT(&rect, retval);
	return retval;
}

static VALUE surface_w(VALUE self)
{
	return UINT2NUM(retrieveSurfacePointer(self)->w);
}

static VALUE surface_h(VALUE self)
{
	return UINT2NUM(retrieveSurfacePointer(self)->h);
}

/*
=begin
--- Surface#colorkey
--- Surface#unset_colorkey
--- Surface#set_colorkey( color )
--- Surface#set_colorkey( color, flags )
These methods control the color that will be completely transparent (it will not be copied 
to the destination surface.)
The only flag is "RLEACCEL" which will encode the bitmap in a more efficient way for blitting,
by skipping the transparent pixels.
=end */
static VALUE surface_set_colorkey(int argc, VALUE* argv, VALUE self)
{
	SDL_Surface* surface=retrieveSurfacePointer(self);
	Uint32 flags = 0, color = 0;
	VALUE colorObject, flagsObject;

	switch(rb_scan_args(argc, argv, "11", &colorObject, &flagsObject)){
		case 2: flags=PARAMETER2FLAGS(flagsObject);
		case 1: flags|=SDL_SRCCOLORKEY;
			color=VALUE2COLOR(colorObject, surface->format);
	}

	if(SDL_SetColorKey(surface, flags, color)==-1) SDL_RAISE;

	return self;
}

static VALUE surface_unset_colorkey(VALUE self)
{
	if(SDL_SetColorKey(retrieveSurfacePointer(self), 0, 0)==-1) SDL_RAISE;
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

/*
=begin
--- Surface#fill( color )
--- Surface#fill( color, rect )
Fills rectangle ((|rect|)) in the surface with ((|color|)).
=end */
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

/*
=begin
--- Surface#pitch
The surface pitch is the number of bytes used in each scanline.
This function should rarely needed, mainly for any special-case debugging.
=end */
static VALUE surface_pitch(VALUE self)
{
	return retrieveSurfacePointer(self)->pitch;
}

/*
=begin
--- Surface#bitsize
Returns the number of bits used to represent each pixel.
This value may not exactly fill the number of bytes used per pixel.
For example a 15 bit Surface still requires a full 2 bytes.
=end */
static VALUE surface_bitsize(VALUE self)
{
	return UINT2NUM(retrieveSurfacePointer(self)->format->BitsPerPixel);
}

/*
=begin
--- Surface#bytesize
Returns the number of bytes used to store each pixel.
=end */
static VALUE surface_bytesize(VALUE self)
{
	return UINT2NUM(retrieveSurfacePointer(self)->format->BytesPerPixel);
}

/*
=begin
--- Surface#flags
Returns the current state flags for the surface.
=end */
static VALUE surface_flags(VALUE self)
{
	return UINT2NUM(retrieveSurfacePointer(self)->flags);
}

/*
=begin
--- Surface#losses
Returns the bitloss for each color plane.
The loss is the number of bits removed for each colorplane from a full 8 bits of
resolution. A value of 8 usually indicates that colorplane is not used
(like the alpha)

Returns an array of [redloss, greenloss, blueloss, alphaloss]
=end */
static VALUE surface_losses(VALUE self)
{
	SDL_Surface* surface=retrieveSurfacePointer(self);
	return rb_ary_new3(4, 
			UINT2NUM(surface->format->Rloss), 
			UINT2NUM(surface->format->Gloss),
			UINT2NUM(surface->format->Bloss), 
			UINT2NUM(surface->format->Aloss));
}

/*
=begin
--- Surface#shifts
Returns the bitshifts [redshift, greenshift, blueshift] used for each color plane.
The shift is determine how many bits left-shifted a colorplane value is in a
mapped color value.
=end */
static VALUE surface_shifts(VALUE self)
{
	SDL_Surface* surface=retrieveSurfacePointer(self);
	return rb_ary_new3(4, 
			UINT2NUM(surface->format->Rshift), 
			UINT2NUM(surface->format->Gshift),
			UINT2NUM(surface->format->Bshift), 
			UINT2NUM(surface->format->Ashift));
}

/*
=begin
--- Surface#masks
Returns the bitmasks [redmask, greenmask, bluemask, alphamask] for each color plane.
The bitmask is used to isolate each colorplane value from a mapped color value.
A value of zero means that colorplane is not used (like alpha)
=end */
static VALUE surface_masks(VALUE self)
{
	SDL_Surface* surface=retrieveSurfacePointer(self);
	return rb_ary_new3(4, 
			UINT2NUM(surface->format->Rmask), 
			UINT2NUM(surface->format->Gmask),
			UINT2NUM(surface->format->Bmask), 
			UINT2NUM(surface->format->Amask));
}

/*
=begin
--- Surface#palette
--- Surface#set_palette( first, colors )
These methods return or set the 256 color palette that is part of 8 bit (({Surface}))s.
((|first|)) is the first color to change.
((|colors|)) and the return value of ((|palette|)) are arrays of colors like
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
	int amount=RARRAY(colors)->len;
	int i;

	SDL_Color newPal[256];

	VALUE color;

	if(!pal) return Qfalse;

	if(first+amount>256) amount=256-first;

	for(i=0; i<amount; i++){
		color=rb_ary_entry(colors, i);
		newPal[i].r=NUM2INT(rb_ary_entry(color, 0));
		newPal[i].g=NUM2INT(rb_ary_entry(color, 1));
		newPal[i].b=NUM2INT(rb_ary_entry(color, 2));
	}

	if(SDL_SetColors(surface, newPal, first, amount)==0) SDL_RAISE;

	return self;
}

/*
=begin
--- Surface#alpha
--- Surface#unset_alpha
--- Surface#set_alpha( alpha )
--- Surface#set_alpha( alpha, flags )
Gets or sets the overall transparency for the surface.
An alpha of 0 is fully transparent, an alpha of 255 is fully opaque.
If your surface has a pixel alpha channel, it will override the overall surface transparency.
You'll need to change the actual pixel transparency to make changes.
If your image also has pixel alpha values, will be used repeatedly, you
will probably want to pass the RLEACCEL flag to the call.
This will take a short time to compile your surface, and increase the blitting speed.
=end */
static VALUE surface_set_alpha(int argc, VALUE* argv, VALUE self)
{
	SDL_Surface* surface = retrieveSurfacePointer(self);
	Uint32 flags = SDL_SRCALPHA;
	Uint8 alpha = 0;

	VALUE alphaObject, flagsObject;
	
	switch(rb_scan_args(argc, argv, "11", &alphaObject, &flagsObject)){
		case 2:
			flags=PARAMETER2FLAGS(flagsObject);
	}

	alpha=NUM2UINT(alphaObject);

	if(SDL_SetAlpha(surface, flags, alpha) == -1) SDL_RAISE;
	
	return self;
}

static VALUE surface_unset_alpha(VALUE self)
{
	if(SDL_SetAlpha(retrieveSurfacePointer(self), 0, 0) == -1) SDL_RAISE;
	return self;
}

static VALUE surface_alpha(VALUE self)
{
	SDL_Surface* surface=retrieveSurfacePointer(self);
	if(surface->flags&SDL_SRCALPHA){
		return UINT2NUM(surface->format->alpha);
	}
	return Qnil;
}


/*
=begin
--- Surface#clip
--- Surface#unset_clip
--- Surface#clip=( rect )
Retrieves, removes or sets the clipping rectangle for surfaces that are
blitted to this surface.
=end */
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
	VALUE rect=rb_obj_alloc(classRect);
	CRECT2RECT(&(retrieveSurfacePointer(self)->clip_rect), rect);
	return rect;
}


/*
=begin
--- Surface#subsurface
Not implemented
=end */
static VALUE surface_subsurface(VALUE self)
{
	rb_notimplement();
	return Qnil;
}

/*
=begin
--- Surface#contained_images
Returns an array of surfaces that are found by parsing this surface in a certain way.
An example is in the samples directory.
This is not part of SDL, it is RUDL-specific.
=end */
static VALUE surface_contained_images(VALUE self)
{
	int x=0;
	int y=0;
	int w=1;
	int h=1;
	int nextRowY=0;
	bool rowDone=false;
	bool linesDone=false;
	bool nextXFound=false;
	bool done;
	SDL_Surface* surface=retrieveSurfacePointer(self);
	SDL_Surface* tmp;
	SDL_Rect srcrect, dstrect;
	Uint32 cornerColor=internal_get(surface, 0, 0);
	VALUE images=rb_ary_new();
	VALUE imageLine=rb_ary_new();

	while(!(rowDone&&linesDone)){
		w=1;
		h=1;

		rowDone=false;

		if(internal_get(surface, x, y)!=cornerColor){
			SDL_RAISE_S("Upper left pixel not white: aborting");
		}

		// Find width
		while(internal_get(surface, x+w, y)!=cornerColor){
			w++;
			if(x+w>=surface->w){
				SDL_RAISE_S("No terminating white pixel: aborting");
			}
		}

		// Find height
		while(internal_get(surface, x, y+h)!=cornerColor){
			h++;
			if(y+h>=surface->h){
				SDL_RAISE_S("No terminating white pixel: aborting");
			}
		}

		// Found image
		//_log("Found an image\n");
		w--;
		h--;

		tmp=SDL_CreateRGBSurface(
				surface->flags, w, h, surface->format->BitsPerPixel,
				surface->format->Rmask, surface->format->Gmask,
				surface->format->Bmask, surface->format->Amask);
		srcrect.x=x+1;
		srcrect.y=y+1;
		srcrect.w=w;
		srcrect.h=h;
		dstrect.x=0;
		dstrect.y=0;

		if(SDL_BlitSurface(surface, &srcrect, tmp, &dstrect)!=0) SDL_RAISE;
		rb_ary_push(imageLine, createSurfaceObject(tmp));

		// Find next line
		if(x==0){
			//_log("Already looking for next line\n");
			nextRowY=y+h+2;
			done=false;
			while(!done){
				if(nextRowY>=surface->h){
					done=true;
					linesDone=true;
					//_log("It's the last line\n");
				}else{
					if(internal_get(surface, 0, nextRowY)==cornerColor){
						done=true;
					}else{
						nextRowY++;
					}
				}
			}
		}

		// Setup for next image
		x+=w+2;

		// Find next X
		nextXFound=false;
		while(!nextXFound){
			if(x>=surface->w){
				//_log("Line done\n");
				x=0;
				y=nextRowY;
				rowDone=true;
				nextXFound=true;
				rb_ary_push(images, imageLine);
				imageLine=rb_ary_new();
			}else{
				if(internal_get(surface, x, y)==cornerColor){
					nextXFound=true;
				}else{
					x++;
				}
			}
		}
	}
	if(RARRAY(images)->len==0){
		return Qnil;
	}
	if(RARRAY(images)->len==1){
		return rb_ary_entry(images, 0);
	}
	return images;
}

///////////////////////////////// DISPLAY
/*
=begin
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
* ((|depth|)) selects the bits per pixel value (8 is 256 colors, 16 is thousands of colors, 
  24 and 32 are millions of colors). If it is not supplied, a good one will be selected for you.
=end */
static VALUE displaySurface_new(int argc, VALUE* argv, VALUE self)
{
	SDL_Surface* surf;
	Uint32 flags = SDL_SWSURFACE;
	int depth = 0;
	Sint16 w=0, h=0;
	char* title, *icontitle;

	VALUE vsize, vflags, vdepth;

	initVideo();

	switch(rb_scan_args(argc, argv, "12", &vsize, &vflags, &vdepth)){
		case 3: depth=NUM2INT(vdepth);
		case 2: flags=PARAMETER2FLAGS(vflags);
	}

	PARAMETER2COORD(vsize, &w, &h);

	if(argc<3) flags |= SDL_ANYFORMAT;

	surf = SDL_SetVideoMode(w, h, depth, flags);

	if(!surf) SDL_RAISE;

	SDL_WM_GetCaption(&title, &icontitle);
	if(!title || !*title){
		SDL_WM_SetCaption("SDL window", "SDL");
	}

	return Data_Wrap_Struct(classDisplaySurface, 0, 0, surf);
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
			format.BitsPerPixel=NUM2INT(bppObject);
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
--- DisplaySurface.info
Not implemented because I think it's a gross hack in Pygame.
=end */
static VALUE displaySurface_info(VALUE self)
{
	rb_notimplement();
	return Qnil;
}

/*
=begin
== Instance Methods
--- DisplaySurface#update
--- DisplaySurface#update( rect )
This call will update a section (or sections) of the display screen.
You must update an area of your display when you change its contents.
((|rect|)) is, starting from v0.4, an array of rectangles.
If passed with no arguments, this will update the entire display surface.
=end
This call cannot be used on OPENGL displays, and will generate an exception.
If you have many rects that need updating, it is best to combine them into a sequence and pass
them all at once. This call will accept a sequence of rectstyle
arguments. Any None's in the list will be ignored.
*/
static VALUE displaySurface_update(int argc, VALUE* argv, VALUE self)
{
	SDL_Rect rect;
	SDL_Surface* surface=retrieveSurfacePointer(self);
	VALUE rectObject;
	int i;

	switch(rb_scan_args(argc, argv, "01", &rectObject)){
		case 0:
			SDL_UpdateRect(surface, 0, 0, 0, 0);
			return self;
		case 1:
			Check_Type(rectObject, T_ARRAY);
			for(i=0; i<RARRAY(rectObject)->len; i++){
				PARAMETER2CRECT(rb_ary_entry(rectObject, i), &rect);
				SDL_UpdateRect(surface, rect.x, rect.y, rect.w, rect.h);
			}
	}
	return self;
}

/*
=begin
--- DisplaySurface#flip
This will update the contents of the entire display.
If your display mode is using the flags HWSURFACE and DOUBLEBUF, this
will wait for a vertical retrace and swap the surfaces.
If you are using a different type of display mode, it will simply update
the entire contents of the surface.
=end
When using an OPENGL display mode this will perform a gl buffer swap.
*/
static VALUE displaySurface_flip(VALUE self)
{
	if(SDL_Flip(retrieveSurfacePointer(self))==-1) SDL_RAISE;
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

	if(rb_obj_is_kind_of(color, rb_cArray)){
		if(RARRAY(color)->len==3){
			r=NUM2DBL(rb_ary_entry(color, 0));
			g=NUM2DBL(rb_ary_entry(color, 1));
			b=NUM2DBL(rb_ary_entry(color, 2));
		}else{
			SDL_RAISE_S("Want [r,g,b] array");
			return Qnil;
		}
	}else{
		r=g=b=NUM2DBL(color);
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

///////////////////////////////// RECT
static bool intersect(SDL_Rect *a, SDL_Rect *b)
{
	if(!((a->x >= b->x) && (a->x < (b->x+b->w))) &&
	   !((b->x >= a->x) && (b->x < (a->x+a->w))) &&
	   !(((a->x+a->w) > b->x) && ((a->x+a->w) <= (b->x+b->w))) &&
	   !(((b->x+b->w) > a->x) && ((b->x+b->w) <= (a->x+a->w))))
		return false;
	if(!((a->y >= b->y) && (a->y < (b->y+b->h))) &&
	   !((b->y >= a->y) && (b->y < (a->y+a->h))) &&
	   !(((a->y+a->h) > b->y) && ((a->y+a->h) <= (b->y+b->h))) &&
	   !(((b->y+b->h) > a->y) && ((b->y+b->h) <= (a->y+a->h))))
		return false;

	return true;
}
/*
=begin
= Rect
== Class Methods
--- Rect.new( x, y, w, h )
--- Rect.new( rectangle )
Creates a new Rect object that can be used as a parameter to methods instead of
an [x, y, w, h] array.
=end */
static VALUE rect_new(int argc, VALUE* argv, VALUE self)
{
	VALUE new_rect=rb_obj_alloc(classRect);
	VALUE arg1, arg2, arg3, arg4;
	SDL_Rect rect;
	switch(rb_scan_args(argc, argv, "13", &arg1, &arg2, &arg3, &arg4)){
		case 4:{
			rb_ivar_set(new_rect, id_atx, arg1);
			rb_ivar_set(new_rect, id_aty, arg2);
			rb_ivar_set(new_rect, id_atw, arg3);
			rb_ivar_set(new_rect, id_ath, arg4);
			break;
		}
		case 3: case 2:{ // should be argumenterror or so
			SDL_RAISE_S("Rect.new wants 1 or 4 arguments");
			break;
		}
		case 1:{
			PARAMETER2CRECT(arg1, &rect);
			rb_ivar_set(new_rect, id_atx, INT2NUM(rect.x));
			rb_ivar_set(new_rect, id_aty, INT2NUM(rect.y));
			rb_ivar_set(new_rect, id_atw, UINT2NUM(rect.w));
			rb_ivar_set(new_rect, id_ath, UINT2NUM(rect.h));
			break;
		}
	}
	return new_rect;
}

/*
=begin
== Instance Methods
--- Rect#overlap( rect )
Returns true if any area of the two rectangles overlaps.
=end */
static VALUE rect_overlap(VALUE self, VALUE otherRect)
{
	SDL_Rect a, b;
	RECT2CRECT(self, &a);
	RECT2CRECT(otherRect, &b);
	return INT2BOOL(intersect(&a, &b));
}

struct ExtRect{
	SDL_Rect crect;
	VALUE rect;
	VALUE sprite;
};

/*
=begin
--- Rect.collide_lists( l1, l2 ) BOGUS
This method looks through list ((|l1|)),
checking collisions with every object in list ((|l2|)).
It does this by calling "rect" on all objects, expecting an array of [x,y,w,h] back,
defining the area this object is in.
It yields (object_from_l1, object_from_l2) for every collision it detects.
More advanced collision detection methods will follow.
=end */

static VALUE rect_collide_lists(VALUE self, VALUE list1Value, VALUE list2Value)
{
	struct ExtRect* list2=NULL;
	SDL_Rect rect1;
	int i1, i2;
	int list1len, list2len;
	VALUE yieldValue=rb_ary_new2(2);
	VALUE sprite;

	Check_Type(list1Value, T_ARRAY);
	Check_Type(list2Value, T_ARRAY);

	list1len=RARRAY(list1Value)->len;
	list2len=RARRAY(list2Value)->len;

	list2=(struct ExtRect*)malloc(list2len*sizeof(struct ExtRect));

	for(i2=0; i2<list2len; i2++){
		sprite=rb_ary_entry(list2Value,i2);
		if(sprite!=Qnil){
			list2[i2].rect=rb_funcall3(sprite, id_rect, 0, NULL);
			RECT2CRECT(list2[i2].rect, &list2[i2].crect);
			list2[i2].sprite=sprite;
		}else{
			list2[i2].sprite=Qnil;
		}
	}

	for(i1=0; i1<list1len; i1++){
		sprite=rb_ary_entry(list1Value,i1);
		if(sprite!=Qnil){
			RECT2CRECT(rb_funcall3(sprite, id_rect, 0, NULL), &rect1);
			for(i2=0; i2<list2len; i2++){
				if(list2[i2].sprite!=Qnil){
					if(intersect(&rect1, &list2[i2].crect)){
						rb_ary_store(yieldValue, 0, sprite);
						rb_ary_store(yieldValue, 1, list2[i2].sprite);
						rb_yield(yieldValue);
					}
				}
			}
		}
	}

	free(list2);
	return self;
}

///////////////////////////////// SDL_GFX: ROTOZOOM
#ifdef HAVE_SDL_ROTOZOOM_H
/*
=begin
== SDL_gfx functions
SDL_gfx was written by Andreas Schiffler.
See ((<URL:http://de.ferzkopp.net/>))
=== SDL_gfx: SDL_rotozoom
--- Surface#rotozoom( angle, zoom, smooth )
Returns a new surface that is rotated ((|angle|)) degrees and zoomed
((|zoom|)) times (fractions are OK).
This method returns a 32 bit surface.
Exception: for now it returns an 8 bit surface when fed an 8 bit surface.
If ((|smooth|)) is true and the surface is not 8 bits,
bilinear interpolation will be applied, resulting in a smoother image.
=end */
static VALUE surface_rotozoom(VALUE self, VALUE angle, VALUE zoom, VALUE smooth)
{
	return createSurfaceObject(rotozoomSurface(retrieveSurfacePointer(self), NUM2DBL(angle), NUM2DBL(zoom), NUM2BOOL(smooth)));
}

/*
=begin
--- Surface#zoom( zoom_horizontal, zoom_vertical, smooth )
Returns a new surface that is zoomed.
1.0 doesn't zoom, bigger than 1.0 zooms in, smaller than 1.0 zooms out.
This method returns a 32 bit surface.
Exception: for now it returns an 8 bit surface when fed an 8 bit surface.
If ((|smooth|)) is true and the surface is not 8 bits,
bilinear interpolation will be applied, resulting in a smoother image.
(The last two methods are from Andreas Schiffler's SDL_rotozoom, aschiffler@home.com)
=end */
static VALUE surface_zoom(VALUE self, VALUE zoom_x, VALUE zoom_y, VALUE smooth)
{
	return createSurfaceObject(zoomSurface(retrieveSurfacePointer(self), NUM2DBL(zoom_x), NUM2DBL(zoom_y), NUM2BOOL(smooth)));
}
#endif

///////////////////////////////// SDL_GFX: GFXPRIMITIVES
#ifdef HAVE_SDL_GFXPRIMITIVES_H
/*
=begin
=== SDL_gfx: SDL_gfxPrimitives
--- Surface#plot( coordinate, color )
--- Surface#get( coordinate )
--- Surface#[ x, y ]= color
--- Surface#[ x, y ]
These methods access single pixels on a surface.
((|plot|)) or ((|[]=|)) set a pixel to ((|color|)) at ((|coordinate|)).
((|get|)) or ((|[]|)) get the color of a pixel.
These methods require the surface to be locked if neccesary.
((|[]=|)) and ((|[]|)) are the only methods in RUDL that take a seperate x and y coordinate.
=end */

Uint32 internal_get(SDL_Surface* surface, Sint16 x, Sint16 y)
{
	SDL_PixelFormat* format = surface->format;
	Uint8* pixels = (Uint8*)surface->pixels;
	Uint32 color;
	Uint8* pix;

	SDL_LockSurface(surface);
	pixels = (Uint8*)surface->pixels;

	if(x < 0 || x >= surface->w || y < 0 || y >= surface->h){
		return 0;
	}

	if(format->BytesPerPixel < 1 || format->BytesPerPixel > 4){
		SDL_RAISE_S("invalid color depth for surface");
	}

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

// not in SDL_gfxprimitives, but it fits here, next to plot.
static VALUE surface_get(VALUE self, VALUE coordinate)
{
	SDL_Surface* surface=retrieveSurfacePointer(self);
	Sint16 x,y;
	Uint8 r, g, b, a;

	PARAMETER2COORD(coordinate, &x, &y);

	SDL_GetRGBA(internal_get(surface, x, y), surface->format, &r, &g, &b, &a);

	return rb_ary_new3(4, UINT2NUM(r), UINT2NUM(g), UINT2NUM(b), UINT2NUM(a));
}


static VALUE surface_plot(VALUE self, VALUE coordinate, VALUE color)
{
	Sint16 x,y;
	PARAMETER2COORD(coordinate, &x, &y);
	if(pixelColor(retrieveSurfacePointer(self), x, y, VALUE2COLOR_NOMAP(color))) SDL_RAISE_S("failed");
	return self;
}

static VALUE surface_array_get(VALUE self, VALUE x, VALUE y)
{
	SDL_Surface* surface=retrieveSurfacePointer(self);
	Uint8 r, g, b, a;

	SDL_GetRGBA(internal_get(surface, NUM2INT(x), NUM2INT(y)), surface->format, &r, &g, &b, &a);

	return rb_ary_new3(4, UINT2NUM(r), UINT2NUM(g), UINT2NUM(b), UINT2NUM(a));
}


static VALUE surface_array_plot(VALUE self, VALUE x, VALUE y, VALUE color)
{
	if(pixelColor(retrieveSurfacePointer(self), NUM2INT(x), NUM2INT(y), VALUE2COLOR_NOMAP(color))) SDL_RAISE_S("failed");
	return self;
}

/*
=begin
--- Surface#horizontal_line( coord, endx, color )
--- Surface#vertical_line( coord, endy, color )
--- Surface#rectangle( rect, color )
--- Surface#filled_rectangle( rect, color )
--- Surface#line( coord1, coord2, color )
--- Surface#antialiased_line( coord1, coord2, color )
--- Surface#circle( coord, radius, color )
--- Surface#filled_circle( coord, radius, color )
--- Surface#filled_pie( coord, radius, start, end, color )
--- Surface#ellipse( coord, radius_x, radius_y, color )
--- Surface#antialiased_ellipse( coord, radius_x, radius_y, color )
--- Surface#filled_ellipse( coord, radius_x, radius_y, color )
These methods are thought to be self-explanatory.
Filled_rectangle is a lot like fill.
Fill comes from SDL, filled_rectangle from SDL_gfxPrimitives,
choose whichever you like best.
--- Surface#polygon( coord_list, color )
--- Surface#filled_polygon( coord_list, color )
--- Surface#antialiased_polygon( coord_list, color)
The polygon methods take an array of [x,y], like [[10,10],[40,60]].
--- Surface#print( coord, text, color )
Puts ((|text|)) on the surface in a monospaced standard old ASCII font.
=end */
static VALUE surface_horizontal_line(VALUE self, VALUE coord, VALUE endx, VALUE color)
{
	Sint16 x,y;
	PARAMETER2COORD(coord, &x, &y);
	if(hlineColor(retrieveSurfacePointer(self), x, NUM2INT(endx), y, VALUE2COLOR_NOMAP(color))) SDL_RAISE_S("failed");
	return self;
}

static VALUE surface_vertical_line(VALUE self, VALUE coord, VALUE endy, VALUE color)
{
	Sint16 x,y;
	PARAMETER2COORD(coord, &x, &y);
	if(vlineColor(retrieveSurfacePointer(self), x, y, NUM2INT(endy), VALUE2COLOR_NOMAP(color))) SDL_RAISE_S("failed");
	return self;
}

static VALUE surface_rectangle(VALUE self, VALUE rectObject, VALUE color)
{
	SDL_Rect rect;
	PARAMETER2CRECT(rectObject, &rect);
	if(rectangleColor(retrieveSurfacePointer(self), rect.x, rect.y, rect.x+rect.w-1, rect.y+rect.h-1, VALUE2COLOR_NOMAP(color))) SDL_RAISE_S("failed");
	return self;
}

static VALUE surface_filled_rectangle(VALUE self, VALUE rectObject, VALUE color)
{
	SDL_Rect rect;
	PARAMETER2CRECT(rectObject, &rect);
	if(boxColor(retrieveSurfacePointer(self), rect.x, rect.y, rect.x+rect.w-1, rect.y+rect.h-1, VALUE2COLOR_NOMAP(color))) SDL_RAISE_S("failed");
	return self;
}

static VALUE surface_line(VALUE self, VALUE coord1, VALUE coord2, VALUE color)
{
	Sint16 x1,y1,x2,y2;
	PARAMETER2COORD(coord1, &x1, &y1);
	PARAMETER2COORD(coord2, &x2, &y2);
	if(lineColor(retrieveSurfacePointer(self), x1, y1, x2, y2, VALUE2COLOR_NOMAP(color))) SDL_RAISE_S("failed");
	return self;
}

static VALUE surface_antialiased_line(VALUE self, VALUE coord1, VALUE coord2, VALUE color)
{
	Sint16 x1,y1,x2,y2;
	PARAMETER2COORD(coord1, &x1, &y1);
	PARAMETER2COORD(coord2, &x2, &y2);
	if(aalineColor(retrieveSurfacePointer(self), x1, y1, x2, y2, VALUE2COLOR_NOMAP(color))) SDL_RAISE_S("failed");
	return self;
}

static VALUE surface_circle(VALUE self, VALUE coord, VALUE r, VALUE color)
{
	Sint16 x,y;
	PARAMETER2COORD(coord, &x, &y);
	if(circleColor(retrieveSurfacePointer(self), x, y, NUM2INT(r), VALUE2COLOR_NOMAP(color))) SDL_RAISE_S("failed");
	return self;
}

static VALUE surface_filled_circle(VALUE self, VALUE coord, VALUE r, VALUE color)
{
	Sint16 x,y;
	PARAMETER2COORD(coord, &x, &y);
	if(filledCircleColor(retrieveSurfacePointer(self), x, y, NUM2INT(r), VALUE2COLOR_NOMAP(color))) SDL_RAISE_S("failed");
	return self;
}

static VALUE surface_filled_pie(VALUE self, VALUE coord, VALUE r, VALUE start, VALUE end, VALUE color)
{
	Sint16 x,y;
	PARAMETER2COORD(coord, &x, &y);
	if(filledpieColor(retrieveSurfacePointer(self), x, y, NUM2INT(r), NUM2INT(start), NUM2INT(end), VALUE2COLOR_NOMAP(color))) SDL_RAISE_S("failed");
	return self;
}



static VALUE surface_ellipse(VALUE self, VALUE coord, VALUE rx, VALUE ry, VALUE color)
{
	Sint16 x,y;
	PARAMETER2COORD(coord, &x, &y);
	if(ellipseColor(retrieveSurfacePointer(self), x, y, NUM2INT(rx), NUM2INT(ry), VALUE2COLOR_NOMAP(color))) SDL_RAISE_S("failed");
	return self;
}

static VALUE surface_antialiased_ellipse(VALUE self, VALUE coord, VALUE rx, VALUE ry, VALUE color)
{
	Sint16 x,y;
	PARAMETER2COORD(coord, &x, &y);
	if(aaellipseColor(retrieveSurfacePointer(self), x, y, NUM2INT(rx), NUM2INT(ry), VALUE2COLOR_NOMAP(color))) SDL_RAISE_S("failed");
	return self;
}

static VALUE surface_filled_ellipse(VALUE self, VALUE coord, VALUE rx, VALUE ry, VALUE color)
{
	Sint16 x,y;
	PARAMETER2COORD(coord, &x, &y);
	if(filledEllipseColor(retrieveSurfacePointer(self), x, y, NUM2INT(rx), NUM2INT(ry), VALUE2COLOR_NOMAP(color))) SDL_RAISE_S("failed");
	return self;
}

static VALUE surface_polygon(VALUE self, VALUE coordlist, VALUE color)
{
	int numpoints=RARRAY(coordlist)->len;
	Sint16 *x=malloc(sizeof(Sint16)*numpoints);
	Sint16 *y=malloc(sizeof(Sint16)*numpoints);
	int i;
	
	for(i=0; i<numpoints; i++){
		x[i]=NUM2INT(rb_ary_entry(rb_ary_entry(coordlist, i), 0));
		y[i]=NUM2INT(rb_ary_entry(rb_ary_entry(coordlist, i), 1));
	}

	if(polygonColor(retrieveSurfacePointer(self), x, y, numpoints, VALUE2COLOR_NOMAP(color)))  SDL_RAISE_S("failed");
	
	free(x);
	free(y);
	return self;
}

static VALUE surface_filled_polygon(VALUE self, VALUE coordlist, VALUE color)
{
	int numpoints=RARRAY(coordlist)->len;
	Sint16 *x=malloc(sizeof(Sint16)*numpoints);
	Sint16 *y=malloc(sizeof(Sint16)*numpoints);
	int i;
	
	for(i=0; i<numpoints; i++){
		x[i]=NUM2INT(rb_ary_entry(rb_ary_entry(coordlist, i), 0));
		y[i]=NUM2INT(rb_ary_entry(rb_ary_entry(coordlist, i), 1));
	}

	if(filledPolygonColor(retrieveSurfacePointer(self), x, y, numpoints, VALUE2COLOR_NOMAP(color)))  SDL_RAISE_S("failed");
	
	free(x);
	free(y);
	return self;
}

static VALUE surface_antialiased_polygon(VALUE self, VALUE coordlist, VALUE color)
{
	int numpoints=RARRAY(coordlist)->len;
	Sint16 *x=malloc(sizeof(Sint16)*numpoints);
	Sint16 *y=malloc(sizeof(Sint16)*numpoints);
	int i;
	
	for(i=0; i<numpoints; i++){
		x[i]=NUM2INT(rb_ary_entry(rb_ary_entry(coordlist, i), 0));
		y[i]=NUM2INT(rb_ary_entry(rb_ary_entry(coordlist, i), 1));
	}

	if(aapolygonColor(retrieveSurfacePointer(self), x, y, numpoints, VALUE2COLOR_NOMAP(color)))  SDL_RAISE_S("failed");
	
	free(x);
	free(y);
	return self;
}

static VALUE surface_print(VALUE self, VALUE coord, VALUE text, VALUE color)
{
	Sint16 x,y;
	PARAMETER2COORD(coord, &x, &y);
	if(stringColor(retrieveSurfacePointer(self), x, y, STR2CSTR(text), VALUE2COLOR_NOMAP(color))) SDL_RAISE_S("failed");
	return self;
}
#endif

/*

Unsupported for now (don't know how they work yet)
=== SDL_gfx: SDL_imageFilter
--- Surface#filterMMXdetect
--- Surface#filterEnableMMX( true_or_false )
--- Surface#filterAdd: D = saturation255(S1 + S2)
--- Surface#filterMean: D = S1/2 + S2/2
SDL_imageFilterSub: D = saturation0(S1 - S2)
SDL_imageFilterAbsDiff: D = | S1 - S2 |
SDL_imageFilterMult: D = saturation(S1 * S2)
SDL_imageFilterMultNor: D = S1 * S2   (non-MMX)
SDL_imageFilterMultDivby2: D = saturation255(S1/2 * S2)
SDL_imageFilterMultDivby4: D = saturation255(S1/2 * S2/2)
SDL_imageFilterBitAnd: D = S1 & S2
SDL_imageFilterBitOr: D = S1 | S2
SDL_imageFilterDiv: D = S1 / S2   (non-MMX)
SDL_imageFilterBitNegation: D = !S
SDL_imageFilterAddByte: D = saturation255(S + C)
SDL_imageFilterAddByteToHalf: D = saturation255(S/2 + C)
SDL_imageFilterSubByte: D = saturation0(S - C)
SDL_imageFilterShiftRight: D = saturation0(S >> N)
SDL_imageFilterMultByByte: D = saturation255(S * C)
SDL_imageFilterShiftRightAndMultByByte: D = saturation255((S >> N) * C)
SDL_imageFilterShiftLeftByte: D = (S << N)
SDL_imageFilterShiftLeft: D = saturation255(S << N)
SDL_imageFilterBinarizeUsingThreshold: D = S >= T ? 255:0
SDL_imageFilterClipToRange: D = (S >= Tmin) & (S <= Tmax) 255:0
SDL_imageFilterNormalizeLinear: D = saturation255((Nmax - Nmin)/(Cmax - Cmin)*(S - Cmin) + Nmin)
 !!! NO C-ROUTINE FOR THESE FUNCTIONS YET !!!

SDL_imageFilterConvolveKernel3x3Divide: Dij = saturation0and255( ... )
SDL_imageFilterConvolveKernel5x5Divide: Dij = saturation0and255( ... )
SDL_imageFilterConvolveKernel7x7Divide: Dij = saturation0and255( ... )
SDL_imageFilterConvolveKernel9x9Divide: Dij = saturation0and255( ... )
SDL_imageFilterConvolveKernel3x3ShiftRight: Dij = saturation0and255( ... )
SDL_imageFilterConvolveKernel5x5ShiftRight: Dij = saturation0and255( ... )
SDL_imageFilterConvolveKernel7x7ShiftRight: Dij = saturation0and255( ... )
SDL_imageFilterConvolveKernel9x9ShiftRight: Dij = saturation0and255( ... )
SDL_imageFilterSobelX: Dij = saturation255( ... )

int SDL_imageFilterAdd (unsigned char *Src1, unsigned char *Src2, unsigned char *Dest, int length);
int SDL_imageFilterMean(unsigned char *Src1, unsigned char *Src2, unsigned char *Dest, int length);
int SDL_imageFilterSub(unsigned char *Src1, unsigned char *Src2, unsigned char *Dest, int length);
int SDL_imageFilterAbsDiff(unsigned char *Src1, unsigned char *Src2, unsigned char *Dest, int length);
int SDL_imageFilterMult(unsigned char *Src1, unsigned char *Src2, unsigned char *Dest, int length);
int SDL_imageFilterMultNor(unsigned char *Src1, unsigned char *Src2, unsigned char *Dest, int length);
int SDL_imageFilterMultDivby2(unsigned char *Src1, unsigned char *Src2, unsigned char *Dest, int length);
int SDL_imageFilterMultDivby4(unsigned char *Src1, unsigned char *Src2, unsigned char *Dest, int length);
int SDL_imageFilterBitAnd(unsigned char *Src1, unsigned char *Src2, unsigned char *Dest, int length);
int SDL_imageFilterBitOr(unsigned char *Src1, unsigned char *Src2, unsigned char *Dest, int length);
int SDL_imageFilterDiv(unsigned char *Src1, unsigned char *Src2, unsigned char *Dest, int length);
int SDL_imageFilterBitNegation(unsigned char *Src1, unsigned char *Dest, int length);
int SDL_imageFilterAddByte(unsigned char *Src1, unsigned char *Dest, int length, unsigned char C);
int SDL_imageFilterAddByteToHalf(unsigned char *Src1, unsigned char *Dest, int length, unsigned char C);
int SDL_imageFilterSubByte(unsigned char *Src1, unsigned char *Dest, int length, unsigned char C);
int SDL_imageFilterShiftRight(unsigned char *Src1, unsigned char *Dest, int length, unsigned char N);
int SDL_imageFilterMultByByte(unsigned char *Src1, unsigned char *Dest, int length, unsigned char C);
int SDL_imageFilterShiftRightAndMultByByte(unsigned char *Src1, unsigned char *Dest, int length, unsigned char N, unsigned char C);
int SDL_imageFilterShiftLeftByte(unsigned char *Src1, unsigned char *Dest, int length, unsigned char N);
int SDL_imageFilterShiftLeft(unsigned char *Src1, unsigned char *Dest, int length, unsigned char N);
int SDL_imageFilterBinarizeUsingThreshold(unsigned char *Src1, unsigned char *Dest, int length, unsigned char T);
int SDL_imageFilterClipToRange(unsigned char *Src1, unsigned char *Dest, int length, unsigned char Tmin, unsigned char Tmax);
int SDL_imageFilterNormalizeLinear(unsigned char *Src1, unsigned char *Dest, int length, int Cmin, int Cmax, int Nmin, int Nmax);
int SDL_imageFilterConvolveKernel3x3Divide(unsigned char *Src, unsigned char *Dest, int rows, int columns, signed short *Kernel, unsigned char Divisor);
int SDL_imageFilterConvolveKernel5x5Divide(unsigned char *Src, unsigned char *Dest, int rows, int columns, signed short *Kernel, unsigned char Divisor);
int SDL_imageFilterConvolveKernel7x7Divide(unsigned char *Src, unsigned char *Dest, int rows, int columns, signed short *Kernel, unsigned char Divisor);
int SDL_imageFilterConvolveKernel9x9Divide(unsigned char *Src, unsigned char *Dest, int rows, int columns, signed short *Kernel, unsigned char Divisor);
int SDL_imageFilterConvolveKernel3x3ShiftRight(unsigned char *Src, unsigned char *Dest, int rows, int columns, signed short *Kernel, unsigned char NRightShift);
int SDL_imageFilterConvolveKernel5x5ShiftRight(unsigned char *Src, unsigned char *Dest, int rows, int columns, signed short *Kernel, unsigned char NRightShift);
int SDL_imageFilterConvolveKernel7x7ShiftRight(unsigned char *Src, unsigned char *Dest, int rows, int columns, signed short *Kernel, unsigned char NRightShift);
int SDL_imageFilterConvolveKernel9x9ShiftRight(unsigned char *Src, unsigned char *Dest, int rows, int columns, signed short *Kernel, unsigned char NRightShift);
int SDL_imageFilterSobelX(unsigned char *Src, unsigned char *Dest, int rows, int columns);
*/
///////////////////////////////// INIT
void initVideoClasses()
{
	classSurface=rb_define_class_under(moduleRUDL, "Surface", rb_cObject);
	rb_define_singleton_method(classSurface, "new", surface_new, -1);
	rb_define_singleton_method(classSurface, "load_new", surface_load_new, 1);
	rb_define_method(classSurface, "save_bmp", surface_save_bmp, 1);

	rb_define_method(classSurface, "w", surface_w, 0);
	rb_define_method(classSurface, "h", surface_h, 0);
	rb_define_method(classSurface, "size", surface_size, 0);
	rb_define_method(classSurface, "rect", surface_rect, 0);
	
	rb_define_method(classSurface, "blit", surface_blit, -1);
	rb_define_method(classSurface, "convert", surface_convert, 0);
	rb_define_method(classSurface, "convert_alpha", surface_convert_alpha, 0);

	rb_define_method(classSurface, "contained_images", surface_contained_images, 0);
	
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

	rb_define_method(classSurface, "subsurface", surface_subsurface, 0);

	classDisplaySurface=rb_define_class_under(moduleRUDL, "DisplaySurface", classSurface);
	rb_define_singleton_method(classDisplaySurface, "new", displaySurface_new, -1);
	rb_define_singleton_method(classDisplaySurface, "modes", displaySurface_modes, -1);
	rb_define_singleton_method(classDisplaySurface, "mode_ok?", displaySurface_mode_ok_, -1);
	rb_define_singleton_method(classDisplaySurface, "info", displaySurface_info, 0);
	rb_define_method(classDisplaySurface, "update", displaySurface_update, -1);
	rb_define_method(classDisplaySurface, "flip", displaySurface_flip, 0);
	rb_define_method(classDisplaySurface, "active?", displaySurface_active_, 0);
	rb_define_method(classDisplaySurface, "caption", displaySurface_caption, 0);
	rb_define_method(classDisplaySurface, "driver", displaySurface_driver, 0);
	rb_define_method(classDisplaySurface, "iconify", displaySurface_iconify, 0);
	rb_define_method(classDisplaySurface, "set_caption", displaySurface_set_caption, -1);
	rb_define_method(classDisplaySurface, "gamma=", displaySurface_gamma_, 1);
	rb_define_method(classDisplaySurface, "toggle_fullscreen", displaySurface_toggle_fullscreen, 0);

	//classSurfaceArray=rb_define_class_under(moduleRUDL, "SurfaceArray", rb_cObject);
	#ifdef HAVE_SDL_GFXPRIMITIVES_H
	rb_define_method(classSurface, "get", surface_get, 1);
	rb_define_method(classSurface, "[]", surface_array_get, 2);
	rb_define_method(classSurface, "plot", surface_plot, 2);
	rb_define_method(classSurface, "[]=", surface_array_plot, 3);
	rb_define_method(classSurface, "fill", surface_fill, -1);
	rb_define_method(classSurface, "horizontal_line", surface_horizontal_line, 3);
	rb_define_method(classSurface, "vertical_line", surface_vertical_line, 3);
	rb_define_method(classSurface, "rectangle", surface_rectangle, 2);
	rb_define_method(classSurface, "filled_rectangle", surface_filled_rectangle, 2);
	rb_define_method(classSurface, "line", surface_line, 3);
	rb_define_method(classSurface, "antialiased_line", surface_antialiased_line, 3);
	rb_define_method(classSurface, "circle", surface_circle, 3);
	rb_define_method(classSurface, "filled_circle", surface_filled_circle, 3);
	rb_define_method(classSurface, "filled_pie", surface_filled_pie, 5);
	rb_define_method(classSurface, "ellipse", surface_ellipse, 4);
	rb_define_method(classSurface, "filled_ellipse", surface_filled_ellipse, 4);
	rb_define_method(classSurface, "antialiased_ellipse", surface_antialiased_ellipse, 4);
	rb_define_method(classSurface, "polygon", surface_polygon, 2);
	rb_define_method(classSurface, "filled_polygon", surface_filled_polygon, 2);
	rb_define_method(classSurface, "antialiased_polygon", surface_antialiased_polygon, 2);
	rb_define_method(classSurface, "print", surface_print, 3);
	#endif
	#ifdef HAVE_SDL_ROTOZOOM_H
	rb_define_method(classSurface, "rotozoom", surface_rotozoom, 3);
	rb_define_method(classSurface, "zoom", surface_zoom, 3);
	#endif
	#ifdef HAVE_SDL_IMAGEFILTER_H
	#endif

	classRect=rb_define_class_under(moduleRUDL, "Rect", rb_cObject);
	rb_define_singleton_method(classRect, "new", rect_new, -1);
	rb_define_method(classRect, "overlap", rect_overlap, 1);
	rb_define_singleton_method(classRect, "collide_lists", rect_collide_lists, 2);
/*
=begin
--- Rect#x
--- Rect#y
--- Rect#w
--- Rect#h
These can be set and read at will.
--- Rect#to_ary
Returns [x, y, w, h]
--- Rect#move( delta )
--- Rect#move!( delta )
Returns a new rectangle which is the base rectangle moved by the given amount.
--- Rect#inflate( sizes )
--- Rect#inflate!( sizes )
Returns a rectangle which has the sizes changed by the given amounts.
((|sizes|)) is an array of [dx, dy].
The rectangle shrinks and expands around the rectangle's center.
Negative values will shrink the rectangle.
--- Rect#normalize
--- Rect#normalize!
If w and h aren't positive, this will change them to positive.
--- Rect#union( rect )
--- Rect#union!( rect )
Returns a new rectangle that completely covers the given inputs.
There may be area inside the newrectangle that is not covered by the inputs.
--- Rect#contains( thing )
Returns whether thing (a Rect, [x, y, w, h] or [x, y]) fits completely within the rectangle.
--- Rect#find_overlapping_rect( rects )
Returns the first rectangle in the list to overlap the base rectangle.
Once an overlap is found, this will stop checking the remaining list.
If no overlap is found, it will return nil.
--- Rect#find_overlapping_rects( rects )
Returns an array with the rectangles in the list to overlap the base rectangle.
If no overlap is found, it will return [].
--- Rect#clip( rect )
--- Rect#clip!( rect )
Returns a new rectangle that is the given rectangle cropped to the inside of the base rectangle.
If the two rectangles do not overlap to begin with, you will get a rectangle with 0 size.
--- Rect#clamp( rect )
--- Rect#clamp!( rect )
Returns a new rectangle that is moved to be completely inside the base rectangle.
If the given rectangle is too large for the base rectangle in an axis, 
it will be centered on that axis.
=end */
	add_ruby_to_rect();
	
	//classCursors=rb_define_class_under(moduleRUDL, "Cursors", rb_cObject);
/*
=begin
= SurfacesLostException
This gruesome thing is thrown bij Surface#blit when Windows manages to destroy all your
surfaces.
This might happen when switching to another application, for example.
The only thing to rescue your application is by waiting for blit to stop throwing exceptions,
then reloading all your surfaces.
=end */

	classSurfacesLostException=rb_define_class_under(moduleRUDL, "SurfacesLostException", rb_cObject);
/*
=begin
= ResizeEvent
--- ResizeEvent#size
This is [w, h] the new size of the window.
=end */
	classResizeEvent=rb_define_class_under(moduleRUDL, "ResizeEvent", classEvent);
	rb_define_attr(classResizeEvent, "size", 1, 1);
/*
=begin
= ActiveEvent
--- ActiveEvent#gain
--- ActiveEvent#state
=end */
	classActiveEvent=rb_define_class_under(moduleRUDL, "ActiveEvent", classEvent);
	rb_define_attr(classActiveEvent, "gain", 1, 1);
	rb_define_attr(classActiveEvent, "state", 1, 1);
/*
=begin
= QuitEvent
This event signals that the user or the program itself has requested to be terminated.
=end */
	classQuitEvent=rb_define_class_under(moduleRUDL, "QuitEvent", classEvent);

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

/*	DEC_CONST(GL_RED_SIZE);
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
*/
	id_rect=rb_intern("rect");
	id_atx=rb_intern("@x");
	id_aty=rb_intern("@y");
	id_atw=rb_intern("@w");
	id_ath=rb_intern("@h");

}
