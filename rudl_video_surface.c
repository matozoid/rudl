/* RUDL - a C library wrapping SDL for use in Ruby. Copyright (C) 2001, 2002  Danny van Bruggen */
#include "rudl_events.h"
#include "rudl_video.h"

#ifdef HAVE_SDL_IMAGE_H
#include "SDL_image.h"
#endif

///////////////////////////////// SURFACE
/*
=begin
<<< docs/head
= Surface
A (({Surface})) is a two dimensional array of pixels with some information about those pixels.
This might not seem like much, but it is just about the most important class in RUDL.
=end */

// SUPPORT:

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

__inline__ void setMasksFromBPP(Uint32 bpp, Uint32* Rmask, Uint32* Gmask, Uint32* Bmask, Uint32* Amask)
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
containing 32 bit values with bits set where the colorcomponent should be stored.
For example: [0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF] describes a 32 bit color
with red in the highest values.
=end */
static VALUE surface_new(int argc, VALUE* argv, VALUE self)
{
	Uint32 flags = 0;
	Uint16 width, height;
	short bpp=0;
	Uint32 Rmask, Gmask, Bmask, Amask;
	VALUE tmp;
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
					bpp=NUM2Sint16(surfaceOrDepthObject);
					if(argc==4){ // got masks
						Check_Type(masksObject, T_ARRAY);
						if(RARRAY(masksObject)->len==4){
							tmp=rb_ary_entry(masksObject, 0);	Rmask=NUM2UINT(tmp);
							tmp=rb_ary_entry(masksObject, 1);	Gmask=NUM2UINT(tmp);
							tmp=rb_ary_entry(masksObject, 2);	Bmask=NUM2UINT(tmp);
							tmp=rb_ary_entry(masksObject, 3);	Amask=NUM2UINT(tmp);
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
If the SDL_image library was not found, only simple BMP loading is supported.
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
--- Surface#destroy
Frees memory used by this surface.
The surface is no longer useable after this call.

Returns nil.
=end */
static VALUE surface_destroy(VALUE self)
{
	GET_SURFACE;
	SDL_FreeSurface(surface);
	DATA_PTR(self)=NULL;
	return Qnil;
}

/*
=begin
--- Surface#blit( source, coordinate )
--- Surface#blit( source, coordinate, sourceRect )
This method blits (copies, pastes, draws) ((|source|)) onto the (({Surface})) it is called on.
((|coordinate|)) is the position [x, y] where ((|source|)) will end up in the destination (({Surface})).
((|sourcerect|)) is the area in the ((|source|)) bitmap that you want blitted.
Not supplying it will blit the whole ((|source|)).

Returns the rectangle array ([x,y,w,h]) in (({Surface})) that was changed.
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
	return new_rect_from_SDL_Rect(&dest_rect);
}

/*
=begin
--- Surface#convert
--- Surface#convert_alpha
Creates a new version of the surface in the current display's format, 
making it faster to blit.
The alpha version optimizes for fast alpha blitting.

Returns the converted surface.
--- Surface#convert!
--- Surface#convert_alpha!
Like convert and convert_alpha, but these change the surface itself.

Returns self.
=end */
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

/*
=begin
--- Surface#lock
--- Surface#must_lock
--- Surface#unlock
--- Surface#locked?
These methods control the locking of surfaces.
If you ever encounter a locking error,
you might try these out.
Locking errors are expected when trying to access video hardware.
Keep (({Surface}))s locked for as short a time as possible.
* must_lock returns true when a surface needs locking for pixel access.
* lock locks the surface and returns self.
* unlock unlocks it again and returns self.
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

Returns self;
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
w returns width, 
h returns height, 
size returns [w, h] and rect returns an array of [0, 0, w, h].
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
	return new_rect_from_SDL_Rect(&rect);
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

((|colorkey|)) returns the current colorkey color.
The others return self;
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

Returns self.
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
	GET_SURFACE;
	return UINT2NUM(surface->pitch);
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
Returns the bitshifts [redshift, greenshift, blueshift, alphashift] used for each color plane.
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
	int amount;
	int i;
	SDL_Color newPal[256];
	VALUE color;

	VALUE tmp;

	RUDL_ASSERT(rb_obj_is_kind_of(colors, rb_cArray), "Need array of colors");

	amount=RARRAY(colors)->len;

	if(!pal) return Qfalse;

	if(first+amount>256) amount=256-first;

	for(i=0; i<amount; i++){
		color=rb_ary_entry(colors, i);
		tmp=rb_ary_entry(color, 0);		newPal[i].r=NUM2Uint8(tmp);
		tmp=rb_ary_entry(color, 1);		newPal[i].g=NUM2Uint8(tmp);
		tmp=rb_ary_entry(color, 2);		newPal[i].b=NUM2Uint8(tmp);
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
Gets or sets the overall transparency for the (({Surface})).
An ((|alpha|)) of 0 is fully transparent, an ((|alpha|)) of 255 is fully opaque.
If your surface has a pixel alpha channel, it will override the overall surface transparency.
You'll need to change the actual pixel transparency to make changes.
If your image also has pixel alpha values and will be used repeatedly, you
will probably want to pass the ((|RLEACCEL|)) flag to the call.
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

	alpha=(Uint8)NUM2UINT(alphaObject);

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
	return new_rect_from_SDL_Rect(&(retrieveSurfacePointer(self)->clip_rect));
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
An example is in the samples directory, in crapola.rbw.
This is not part of SDL, it is RUDL-specific.
=end */
static VALUE surface_contained_images(VALUE self)
{
	Sint16 x=0;
	Sint16 y=0;
	Sint16 w=1;
	Sint16 h=1;
	Sint16 nextRowY=0;
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
		while(internal_get(surface, (Sint16)(x+w), y)!=cornerColor){
			w++;
			if(x+w>=surface->w){
				SDL_RAISE_S("No terminating white pixel: aborting");
			}
		}

		// Find height
		while(internal_get(surface, x, (Sint16)(y+h))!=cornerColor){
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

/*
=begin
--- Surface#get( coordinate )
--- Surface#[ x, y ]
These methods read single pixels on a surface.
((|get|)) or ((|[]|)) get the color of a pixel.
These methods require the surface to be locked if neccesary.
((|[]=|)) and ((|[]|)) are the only methods in RUDL that take a seperate ((|x|)) and ((|y|)) coordinate.
=end */
__inline__ Uint32 internal_get(SDL_Surface* surface, Sint16 x, Sint16 y)
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
	Uint8* pixels = (Uint8*)surface->pixels;
	Uint32 color;
	Uint8* pix;

	SDL_LockSurface(surface);
	pixels = (Uint8*)surface->pixels;

	if(x < 0 || x >= surface->w || y < 0 || y >= surface->h){
		return 0;
	}

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

static VALUE surface_get(VALUE self, VALUE coordinate)
{
	SDL_Surface* surface=retrieveSurfacePointer(self);
	Sint16 x,y;
	Uint8 r, g, b, a;

	PARAMETER2COORD(coordinate, &x, &y);

	SDL_GetRGBA(internal_get(surface, x, y), surface->format, &r, &g, &b, &a);

	return rb_ary_new3(4, UINT2NUM(r), UINT2NUM(g), UINT2NUM(b), UINT2NUM(a));
}

static VALUE surface_array_get(VALUE self, VALUE x, VALUE y)
{
	SDL_Surface* surface=retrieveSurfacePointer(self);
	Uint8 r, g, b, a;

	SDL_GetRGBA(internal_get(surface, NUM2Sint16(x), NUM2Sint16(y)), surface->format, &r, &g, &b, &a);

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


/*
=begin
--- Surface#rows
--- Surface#get_row( y )
--- Surface#set_row( y, pixels )
--- Surface#each_row { |row_of_pixels| ... }
--- Surface#each_row! { |row_of_pixels| ... }
These methods manipulate rows of pixels.
(({Surface}))#((|rows|)) returns an array of strings with one row of imagedata each.
((|get_row|)) and ((|set_row|)) get and set a single such row.
((|each_row|)) and ((|each_row!|)) iterate through the rows, 
passing each of them to the supplied codeblock.
For more info, see (({Surface}))#((|pixels|)).
=end */
static VALUE surface_get_row(VALUE self, VALUE y)
{
	GET_SURFACE;

	RUDL_ASSERT(NUM2INT(y)<surface->h, "y>surface.h");
	RUDL_ASSERT(NUM2INT(y)>=0, "y<0");
	
	return rb_str_new(get_line_pointer(surface, NUM2INT(y)), surface->w*surface->format->BytesPerPixel);
}

static VALUE surface_set_row(VALUE self, VALUE y, VALUE pixels)
{
	GET_SURFACE;

	RUDL_ASSERT(NUM2INT(y)<surface->h, "y>surface.h");
	RUDL_ASSERT(NUM2INT(y)>=0, "y<0");

	RUDL_ASSERT(RSTRING(pixels)->len >= surface->w*surface->format->BytesPerPixel, "Not enough data for a complete row");

	copy_line_to_surface(surface, NUM2INT(y), RSTRING(pixels)->ptr);
	return self;
}

void define_ruby_row_methods()
{
	rb_eval_string(
		"module RUDL class Surface				\n"
		"	def each_row						\n"
		"		(0...h).each {|y|				\n"
		"			yield(get_row(y))			\n"
		"		}								\n"
		"		self							\n"
		"	end									\n"
		"	def each_row!						\n"
		"		(0...h).each {|y|					\n"
		"			set_row(y, yield(get_row(y)))	\n"
		"		}									\n"
		"		self							\n"
		"	end									\n"
		"	def rows							\n"
		"		retval=[]						\n"
		"		each_row {|r| retval.push(r)}	\n"
		"		retval							\n"
		"	end									\n"
		"	def rows=(rows)						\n"
		"		(0...h).each {|row|				\n"
		"			set_row(row, rows[row])		\n"
		"		}								\n"
		"		self							\n"
		"	end									\n"
		"end end								\n"
	);
}

/*
=begin
--- Surface#columns
--- Surface#get_column( x )
--- Surface#set_column( x, pixels )
--- Surface#each_column { |column_of_pixels| ... }
--- Surface#each_column! { |column_of_pixels| ... }
These methods manipulate columns of pixels.
(({Surface}))#((|columns|)) returns an array of strings with one column of imagedata each.
((|get_column|)) and ((|set_column|)) get and set a single such column.
((|each_column|)) and ((|each_column!|)) iterate through the columns, 
passing each of them to the supplied codeblock.
For more info, see (({Surface}))#((|pixels|)).
=end */
static VALUE surface_get_column(VALUE self, VALUE x)
{
	int y;
	int pixelsize;
	int h;
	Uint8* src, *dest, *column;

	GET_SURFACE;

	RUDL_ASSERT(NUM2INT(x)<surface->w, "x>surface.w");
	RUDL_ASSERT(NUM2INT(x)>=0, "x<0");
	
	h=surface->h;
	pixelsize=surface->format->BytesPerPixel;
	
	column=malloc(h*pixelsize);
	src=((Uint8*)surface->pixels)+(NUM2INT(x))*pixelsize;
	dest=column;
	for(y=0; y<h; y++){
		memcpy(dest, src, pixelsize);
		dest+=pixelsize;
		src+=surface->pitch;
	}

	return rb_str_new(column, h*pixelsize);
}

static VALUE surface_set_column(VALUE self, VALUE x, VALUE pixels)
{
	int y;
	int pixelsize;
	int h;
	Uint8* src, *dest;

	GET_SURFACE;

	RUDL_ASSERT(NUM2INT(x)<surface->w, "x>surface.w");
	RUDL_ASSERT(NUM2INT(x)>=0, "x<0");
	
	h=surface->h;
	pixelsize=surface->format->BytesPerPixel;
	
	dest=((Uint8*)surface->pixels)+(NUM2INT(x))*pixelsize;
	src=RSTRING(pixels)->ptr;
	for(y=0; y<h; y++){
		memcpy(dest, src, pixelsize);
		dest+=surface->pitch;
		src+=pixelsize;
	}

	return self;
}

void define_ruby_column_methods()
{
	rb_eval_string(
		"module RUDL class Surface				\n"
		"	def each_column						\n"
		"		(0...w).each {|x|				\n"
		"			yield(get_column(x))		\n"
		"		}								\n"
		"		self							\n"
		"	end									\n"
		"	def each_column!					\n"
		"		(0...w).each {|x|						\n"
		"			set_column(x, yield(get_column(x)))	\n"
		"		}										\n"
		"		self									\n"
		"	end									\n"
		"	def columns							\n"
		"		retval=[]							\n"
		"		each_column {|c| retval.push(c)}	\n"
		"		retval								\n"
		"	end									\n"
		"	def columns=(cols)					\n"
		"		(0...w).each {|col|				\n"
		"			set_column(col, cols[col])	\n"
		"		}								\n"
		"		self							\n"
		"	end									\n"
		"end end								\n"
	);
}

/*
=begin
--- Surface#pixels
--- Surface#pixels=( pixeldata )
These methods get and set all image data at once.
The transport medium is a string with binary data in it.
The data is raw, no fancy color arrays here.
If bytesize (the amount of bytes used to describe the color of a pixel) is
four, for example, a (({Surface})) of 5x5 pixels will return a string of length (5x5x4).
If the colorformat is specified as BGRA, then character zero will be the
B component, character one the G component etc.
Eight bit color surfaces store one byte indexes into the palette.
These methods perform best when the surface's pitch is equal to its width.
There is not much errorchecking so beware of crashes.
=end */
static VALUE surface_pixels(VALUE self)
{
	Uint32 image_size;
	GET_SURFACE;
	
	image_size=surface->w*surface->h*surface->format->BytesPerPixel;

	if(surface->pitch==surface->w){
		return rb_str_new(surface->pixels, image_size);
	}else{
		int y;
		Uint8* tmp_pixels=malloc(image_size);
		VALUE retval;
		Uint16 bytewidth=surface->w*surface->format->BytesPerPixel;

		for(y=0; y<surface->h; y++){
			copy_surface_to_line(surface, y, tmp_pixels+y*bytewidth);
		}
		retval=rb_str_new(tmp_pixels, image_size);
		free(tmp_pixels);
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

///////////////////////////////// INIT
void initVideoSurfaceClasses()
{
	DEBUG_S("initVideoSurfaceClasses()");

	classSurface=rb_define_class_under(moduleRUDL, "Surface", rb_cObject);
	rb_define_singleton_method(classSurface, "new", surface_new, -1);
	rb_define_singleton_method(classSurface, "load_new", surface_load_new, 1);
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

	rb_define_method(classSurface, "get", surface_get, 1);
	rb_define_method(classSurface, "[]", surface_array_get, 2);
	rb_define_method(classSurface, "fill", surface_fill, -1);

	rb_define_method(classSurface, "pixels", surface_pixels, 0);
	rb_define_method(classSurface, "pixels=", surface_set_pixels, 1);

	rb_define_method(classSurface, "get_row", surface_get_row, 1);
	rb_define_method(classSurface, "set_row", surface_set_row, 2);
	define_ruby_row_methods();

	rb_define_method(classSurface, "get_column", surface_get_column, 1);
	rb_define_method(classSurface, "set_column", surface_set_column, 2);
	define_ruby_column_methods();
}
