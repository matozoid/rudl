/* RUDL - a C library wrapping SDL for use in Ruby. Copyright (C) 2001, 2002  Danny van Bruggen */
#include "rudl_events.h"
#include "rudl_video.h"
#include "bitmask.h"


bitmask *SCAM_mask_from_image_SDL(SDL_Surface *surface, Uint32 colorkey)
{
	bitmask *b = bitmask_create(surface->w, surface->h);

	if(b){
		Sint16 x, y;

		SDL_LockSurface(surface);

		for (y=0; y<surface->h; y++){
			for (x=0; x<surface->w; x++){
				if(internal_nonlocking_get(surface, x, y)!=colorkey){

					bitmask_setbit(b, x, y);
				}
			}
		}

		SDL_UnlockSurface(surface);
	}

	return b;
}

/**
@file Misc
@class CollisionMap
This code is "bitmask" from <a href='http://www.ifm.liu.se/~ulfek/projects/2d_Collision_Detection.html'>Ulf Ekstrom</a>

<p>This class contains a map of all the visible pixels in a surface.
(That's the ones that don't have the color key's color)
With it, you can detect whether any pixels in a surface and another
surface overlap when they are at two specific positions.
*/
/**
@section Class Methods
@method new( surface ) -> CollisionMap
@method new( size ) -> CollisionMap
Creates a new collision map.

<p>Supplying a Surface will create the map with the information from surface.
The map will not be automatically updated when the surface contents
change, so a new @CollisionMap will have to be made each time.
The Surface's colorkey will be used to identify "uncollidable" area's.

<p>Supplying a size array: [w, h] will create an empty bitmask of that size.

<p>Surface has an attribute, @collision_map, that you can use to attach
a CollisionMap to.
RUDL doesn't use that attribute for itself.
The syntax would be:

<code>some_surface.collision_map=CollisionMap.new( some_surface )</code>
*/
static VALUE collision_map_new(VALUE self, VALUE par1)
{
	bitmask* map;

	if(rb_obj_is_kind_of(par1, rb_cArray)){ // empty bitmask
		Sint16 w,h;
		PARAMETER2COORD(par1, &w, &h);
		map=bitmask_create(w, h);
	}else{ // bitmask from surface
		Uint32 colorkey;
		SDL_Surface* surface=retrieveSurfacePointer(par1);
		SDL_Rect whole_surface;

		RUDL_ASSERT(surface, "got null surface");
		colorkey=surface->format->colorkey;
		whole_surface.x=0;
		whole_surface.y=0;
		whole_surface.w=surface->w;
		whole_surface.h=surface->h;

		map=SCAM_mask_from_image_SDL(surface, colorkey);
	}

	if(map){
		return Data_Wrap_Struct(classCollisionMap, 0, bitmask_free, map);
	}else{
		SDL_RAISE_S("Collision map could not be created");
		return Qnil;
	}
}

/**
@method collides_with( own_coord, other_map, other_coord ) -> [hit_x, hit_y] or nil
This returns the first found overlapping (colliding) pixel for two collision maps,
or nil if no collision occurred.
The coordinates specify where the two maps are,
which will probably mean that the two surfaces are blitted to the screen at those coordinates.

<p>If using the @collision_map attribute, you would get for one surface at [10,10] and
another at [20,20]:
<code>onesurface.collision_map( [10,10], other_surface.collision_map, [20,20] )</code>
*/
static VALUE collision_map_collides_with(VALUE self, VALUE coord1, VALUE other_map, VALUE coord2)
{
	Sint16 x1,y1,x2,y2;
	int hit_x,hit_y;
	bitmask* map1, *map2;

	Data_Get_Struct(self, bitmask, map1);
	Data_Get_Struct(other_map, bitmask, map2);

	PARAMETER2COORD(coord1, &x1, &y1);
	PARAMETER2COORD(coord2, &x2, &y2);

	if(bitmask_overlap_pos(map1, map2, x2-x1, y2-y1, &hit_x, &hit_y)){
		return rb_ary_new3(2, INT2NUM(hit_x), INT2NUM(hit_y));
	}else{
		return Qnil;
	}
}

/**
@method destroy -> nil
Removes the map from memory.
This instance of CollisionMap will be useless from this call on.
*/
static VALUE collision_map_destroy(VALUE self)
{
	bitmask* map;
	Data_Get_Struct(self, bitmask, map);
	bitmask_free(map);
	DATA_PTR(self)=NULL;
	return Qnil;
}

/**
@method set( coord ) -> self
@method unset( coord ) -> self
This fills and erases one point in the collision map,
in case you want to have collision with parts of a surface that
weren't color keyed, or you want parts of the surface to appear
"untouchable"
*/
static VALUE collision_map_set(VALUE self, VALUE coord)
{
	bitmask* map;
	Sint16 x, y;
	Data_Get_Struct(self, bitmask, map);
	PARAMETER2COORD(coord, &x, &y);
	bitmask_setbit(map, x, y);
	return self;
}

static VALUE collision_map_unset(VALUE self, VALUE coord)
{
	bitmask* map;
	Sint16 x, y;
	Data_Get_Struct(self, bitmask, map);
	PARAMETER2COORD(coord, &x, &y);
	bitmask_clearbit(map, x, y);
	return self;
}

/**
@method [ x, y ] -> Number
@method [ x, y ]= collidebit -> self
The array operator accesses single points in the collision map,
in case you want to have collision with parts of a surface that
weren't color keyed, or you want parts of the surface to appear
"untouchable".

If collidebit is set to 0, no collision will be detected for that point.
If it is set to anything else, it will be set to 1 and collisions will
be checked at that point.
*/
static VALUE collision_map_array_set(VALUE self, VALUE x, VALUE y, VALUE bit)
{
	bitmask* map;
	Data_Get_Struct(self, bitmask, map);
	if(NUM2BOOL(bit)){
		bitmask_setbit(map, NUM2Sint16(x), NUM2Sint16(y));
	}else{
		bitmask_clearbit(map, NUM2Sint16(x), NUM2Sint16(y));
	}
	return self;
}

static VALUE collision_map_array_get(VALUE self, VALUE x, VALUE y)
{
	bitmask* map;
	Data_Get_Struct(self, bitmask, map);
	return INT2NUM(bitmask_getbit(map, NUM2Sint16(x), NUM2Sint16(y)));
}

/**
@method size -> [w, h]
Returns the size of the collision map.
*/
static VALUE collision_map_size(VALUE self)
{
	bitmask* map;
	Data_Get_Struct(self, bitmask, map);
	return rb_ary_new3(2, INT2NUM(map->w), INT2NUM(map->h));
}

///////////////////////////////// INIT
void initVideoBitmaskClasses()
{
	classCollisionMap=rb_define_class_under(moduleRUDL, "CollisionMap", rb_cObject);
	rb_define_singleton_method(classCollisionMap, "new", collision_map_new, 1);
	rb_define_method(classCollisionMap, "collides_with", collision_map_collides_with, 3);
	rb_define_method(classCollisionMap, "set", collision_map_set, 1);
	rb_define_method(classCollisionMap, "unset", collision_map_unset, 1);
	rb_define_method(classCollisionMap, "[]", collision_map_array_get, 2);
	rb_define_method(classCollisionMap, "[]=", collision_map_array_set, 3);
	rb_define_method(classCollisionMap, "destroy", collision_map_destroy, 0);
	rb_define_method(classCollisionMap, "size", collision_map_size, 0);

	rb_eval_string(
		"module RUDL class Surface			\n"
		"	attr_accessor :collision_map	\n"
		"end end								\n"
		"module RUDL class CollisionMap				\n"
		"	def inspect									\n"
		"		\"<CollisionMap: #{size.x}x#{size.y}>\"	\n"
		"	end											\n"
		"end end"
	);

}

