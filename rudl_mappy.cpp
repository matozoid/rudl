/* RUDL - a C library wrapping SDL for use in Ruby. Copyright (C) 2001  Danny van Bruggen */
#include "rudl_mappy.h"
#include "SDLMappy.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "rudl_video.h"

void MAPPY_RAISE(SDLMappy*mappy)
{
	switch(mappy->GetError()){
		case MER_NONE: SDL_RAISE_S("MER_NONE"); break;
		case MER_OUTOFMEM: SDL_RAISE_S("MER_OUTOFMEM"); break;
		case MER_MAPLOADERROR: SDL_RAISE_S("MER_MAPLOADERROR"); break;
		case MER_NOOPEN: SDL_RAISE_S("MER_NOOPEN"); break;
		default: SDL_RAISE_S("Mystifying Mappy error"); break;
	}
}

void delete_mappy(SDLMappy* map)
{
	delete map;
}

VALUE createMappyObject(SDLMappy* map)
{
	return Data_Wrap_Struct(classMappy, 0, delete_mappy, map);
}

SDLMappy* retrieveMappyPointer(VALUE self)
{
	SDLMappy* mappy;
	Data_Get_Struct(self, SDLMappy, mappy);
	return mappy;
}

VALUE rudl_mappy_block_getter(VALUE self)
{
	BLKSTR* block;
	Data_Get_Struct(self, BLKSTR, block);
	rb_iv_get(

/*
=begin
= Mappy
This wraps the SDLMappy library made by Regis Quercioli.
Read about it on ((<http://membres.tripod.fr/edorul/SDLMappye.htm>)).
The state of SDLMappy seems to be "abandoned in the middle of development" but
more work will be done on it.
This doc is also rather silly because of that.
== Class Methods
--- Mappy.new( filename, destination_rect )
((|filename|)) is the name of an FMP file to load (maybe MAP too?).
It seems like only v0.5 normal grid-like maps can be loaded.
((|destination_rect|)) is supposed to be the rect where the map will be displayed,
but it doesn't seem to use it at all.
=end */

static VALUE mappy_new(VALUE self, VALUE filename, VALUE destination_rect)
{
	SDL_Rect rect;
	PARAMETER2CRECT(destination_rect, &rect);
	SDLMappy* map=new SDLMappy;
	if(map->LoadMap(STR2CSTR(filename), rect.x, rect.y, rect.w, rect.h)==-1) MAPPY_RAISE(map);
	return createMappyObject(map);
}

/*
=begin
--- Mappy#draw_background( surface )
--- Mappy#draw_background_transparent( surface )
Draws the current layer on the surface.
The transparent version draws with colorkeying.
=end */
static VALUE mappy_draw_background(VALUE self, VALUE destination)
{
	retrieveMappyPointer(self)->MapDrawBG(retrieveSurfacePointer(destination));
	return self;
}

static VALUE mappy_draw_background_transparent(VALUE self, VALUE destination)
{
	retrieveMappyPointer(self)->MapDrawBGT(retrieveSurfacePointer(destination));
	return self;
}

/*
=begin
--- Mappy#reset_animations
Sets all animations in their starting positions.
--- Mappy#update_animations
Advances the animations.
=end */
static VALUE mappy_reset_animations(VALUE self)
{
	retrieveMappyPointer(self)->MapInitAnims();
	return self;
}

static VALUE mappy_update_animations(VALUE self)
{
	retrieveMappyPointer(self)->MapUpdateAnims();
	return self;
}

/*
=begin
--- Mappy#create_parallax( filename )
Creates a parallax layer or so from an image file, whatever that may mean.
--- Mappy#draw_parallax( surface )
Draws the parallax layer on the surface.
--- Mappy#restore_parallax
Reloads the parallax layer, where it is used, I don't know.
=end */
static VALUE mappy_create_parallax(VALUE self, VALUE filename)
{
	if(!retrieveMappyPointer(self)->CreateParallax(STR2CSTR(filename))) MAPPY_RAISE(retrieveMappyPointer(self));
	return self;
}

static VALUE mappy_draw_parallax(VALUE self, VALUE surface)
{
	retrieveMappyPointer(self)->DrawParallax(retrieveSurfacePointer(surface));
	return self;
}

static VALUE mappy_restore_parallax(VALUE self)
{
	if(!retrieveMappyPointer(self)->RestoreParallax()) SDL_RAISE_S("failed");
	return self;
}

/*
=begin
--- Mappy#layer=( number )
((|number|)) is the layer you want to work with.
If that layer doesn't exist, it raises SDL_Error.
=end */
static VALUE mappy_layer_(VALUE self, VALUE number)
{
	if((retrieveMappyPointer(self)->MapChangeLayer(NUM2INT(number)))==-1){
		SDL_RAISE_S("failed");
	}
	return self;
}

/*
=begin
--- Mappy#tile( coord )
--- Mappy#set_block( coord )
These methods confuse me.
Please refer to the original documentation until I find out what each of these
does exactly.
--- Mappy#block( coord )
This returns a MappyBlock from the map.
Changing its members should change the block on the map
(or maybe all identical blocks on the map?)
=end */
static VALUE mappy_tile(VALUE self, VALUE coord)
{
	Sint16 x,y;
	PARAMETER2COORD(coord, &x, &y);
	return INT2NUM(retrieveMappyPointer(self)->MapGetTile(x, y));
}

static VALUE mappy_block(VALUE self, VALUE coord)
{
	Sint16 x,y;
	PARAMETER2COORD(coord, &x, &y);
	return Data_Wrap_Struct(classMappyBlock, 0, 0, retrieveMappyPointer(self)->MapGetBlock(x, y));
}

static VALUE mappy_set_block(VALUE self, VALUE coord, VALUE newval)
{
	Sint16 x,y;
	PARAMETER2COORD(coord, &x, &y);
	retrieveMappyPointer(self)->MapSetBlock(x, y, NUM2INT(newval));
	return self;
}

/*
=begin
--- Mappy#bpp
Returns the bits per pixel value.
It would be nice if the surface that the map is going to be blitted on has the same value,
since that will save a lot of conversions.
There is no way to convert the map to another bitdepth.
=end */
static VALUE mappy_bpp(VALUE self)
{
	return UINT2NUM(retrieveMappyPointer(self)->GetMapBPP());
}

/*
=begin
--- Mappy#size_in_pixels
--- Mappy#size_in_blocks
These return the size of the map in pixels and in blocks.
=end */
static VALUE mappy_size_in_pixels(VALUE self)
{
	SDLMappy* mappy=retrieveMappyPointer(self);
	return rb_ary_new3(2, UINT2NUM(mappy->GetMapWidth()), UINT2NUM(mappy->GetMapHeight()));
}

static VALUE mappy_size_in_blocks(VALUE self)
{
	SDLMappy* mappy=retrieveMappyPointer(self);
	return rb_ary_new3(2, UINT2NUM(mappy->GetMapWidthInBlocks()), UINT2NUM(mappy->GetMapHeightInBlocks()));
}
/*
=begin
--- Mappy#blocksize
Returns the size of the blocks that the map is made of.
=end */
static VALUE mappy_blocksize(VALUE self)
{
	SDLMappy* mappy=retrieveMappyPointer(self);
	return rb_ary_new3(2, INT2NUM(mappy->GetMapBlockWidth()), INT2NUM(mappy->GetMapBlockHeight()));
}

/*
=begin
--- Mappy#pos=coord
--- Mappy#pos
With these you can move the viewport to another part of the map.
((|coord|)) is [x, y].
=end */
static VALUE mappy_pos_(VALUE self, VALUE pos)
{
	Sint16 x,y;
	PARAMETER2COORD(pos, &x, &y);
	retrieveMappyPointer(self)->MapMoveTo(x, y);
	return self;
}

static VALUE mappy_pos(VALUE self)
{
	SDLMappy* mappy=retrieveMappyPointer(self);
	return rb_ary_new3(2, INT2NUM(mappy->MapGetXPosition()), INT2NUM(mappy->MapGetYPosition()));
}

	
typedef VALUE (*fptr)(void);

void initMappyClasses()
{
	
	classMappy=rb_define_class_under(moduleRUDL, "Mappy", rb_cObject);
	rb_define_singleton_method(classMappy, "new", (fptr)mappy_new, 2);
	rb_define_method(classMappy, "draw_background", (fptr)mappy_draw_background, 1);
	rb_define_method(classMappy, "draw_background_transparent", (fptr)mappy_draw_background, 1);
	rb_define_method(classMappy, "reset_animations", (fptr)mappy_reset_animations, 0);
	rb_define_method(classMappy, "update_animations", (fptr)mappy_update_animations, 0);
	rb_define_method(classMappy, "create_parallax", (fptr)mappy_create_parallax, 1);
	rb_define_method(classMappy, "draw_parallax", (fptr)mappy_draw_parallax, 1);
	rb_define_method(classMappy, "restore_parallax", (fptr)mappy_restore_parallax, 0);
	rb_define_method(classMappy, "layer=", (fptr)mappy_layer_, 1);
	rb_define_method(classMappy, "tile", (fptr)mappy_tile, 1);
	rb_define_method(classMappy, "block", (fptr)mappy_block, 1);
	rb_define_method(classMappy, "set_block", (fptr)mappy_set_block, 2);
	rb_define_method(classMappy, "bpp", (fptr)mappy_bpp, 0);
	rb_define_method(classMappy, "size_in_pixels", (fptr)mappy_size_in_pixels, 0);
	rb_define_method(classMappy, "size_in_blocks", (fptr)mappy_size_in_blocks, 0);
	rb_define_method(classMappy, "blocksize", (fptr)mappy_blocksize, 0);
	rb_define_method(classMappy, "pos", (fptr)mappy_pos, 0);
	rb_define_method(classMappy, "pos=", (fptr)mappy_pos_, 1);

	classMappyBlock=rb_define_class_under(moduleRUDL, "MappyBlock", rb_cObject);
}

#ifdef __cplusplus
}
#endif
