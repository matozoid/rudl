/* RUDL - a C library wrapping SDL for use in Ruby. Copyright (C) 2001, 2002  Danny van Bruggen */
#include "rudl_events.h"
#include "rudl_video.h"
#include "SDL_bitmask.h"

/*
=begin
<<< docs/head
= CollisionMap
This code is "BitMask" from ((<URL:http://www.ifm.liu.se/~ulfek/bitmask/Bitmask.html>))

This class contains a map of all the visible pixels in a surface.
(That's the ones that don't have the color key's color)
With it, you can detect whether any pixels in a surface and another
surface overlap when they are at two specific positions.

== Class Methods
--- CollisionMap#new( surface )
Creates a new collision map with the information from surface.
The map will not be automatically updated when the surface contents
change, so a new (({CollisionMap})) will have to be made each time.

Surface has an attribute, ((|collision_map|)), that you can use to attach
a (({CollisionMap})) to.
RUDL doesn't use that attribute for itself.
The syntax would be:

some_surface.collision_map=CollisionMap.new( some_surface )
=end */
static VALUE collision_map_new(VALUE self, VALUE surface)
{
	SDL_Surface* surface_ptr=retrieveSurfacePointer(surface);
	Uint32 colorkey=surface_ptr->format->colorkey;
	SDL_Rect whole_surface;
	BitMask* map;

	whole_surface.x=0;
	whole_surface.y=0;
	whole_surface.w=surface_ptr->w;
	whole_surface.h=surface_ptr->h;
	
	map=BitMask_from_image_SDL(surface_ptr, &whole_surface, colorkey);

	if(map){
		return Data_Wrap_Struct(classCollisionMap, 0, BitMask_destroy, map);
	}else{
		SDL_RAISE_S("Collision map could not be created");
		return Qnil;
	}

}

/*
=begin
--- CollisionMap#collides_with( own_coord, other_map, other_coord )
This returns the first found overlapping (colliding) pixel for two collision maps,
or nil if no collision occurred.
The coordinates specify where the two maps are, 
which will probably mean that the two surfaces are blitted to the screen at those coordinates.

If using the Surface#collision_map attribute, you would get for one surface at [10,10] and
another at [20,20]:
onesurface.collision_map( [10,10], other_surface.collision_map, [20,20] )
=end */
static VALUE collision_map_collides_with(VALUE self, VALUE coord1, VALUE other_map, VALUE coord2)
{
	Sint16 x1,y1,x2,y2;
	int hit_x,hit_y;
	BitMask* map1, *map2;

	Data_Get_Struct(self, BitMask, map1);
	Data_Get_Struct(other_map, BitMask, map2);

	PARAMETER2COORD(coord1, &x1, &y1);
	PARAMETER2COORD(coord2, &x2, &y2);

	if(BitMask_overlap_pos(map1, map2, x2-x1, y2-y1, &hit_x, &hit_y)){
		return rb_ary_new3(2, INT2NUM(hit_x), INT2NUM(hit_y));
	}else{
		return Qnil;
	}
}

/*
=begin
--- CollisionMap.destroy
Removes the map from memory.
This instance of CollisionMap will be useless from this call on.
=end */
static VALUE collision_map_destroy(VALUE self)
{
	BitMask* map;
	Data_Get_Struct(self, BitMask, map);
	BitMask_destroy(map);
	DATA_PTR(self)=NULL;
	return Qnil;
}

/*
=begin
--- CollisionMap.set( coord )
--- CollisionMap.unset( coord )
This fills and erases one point in the collision map,
in case you want to have collision with parts of a surface that
weren't color keyed, or you want parts of the surface to appear
"untouchable"
=end */
static VALUE collision_map_set(VALUE self, VALUE coord)
{
	BitMask* map;
	Sint16 x, y;
	Data_Get_Struct(self, BitMask, map);
	PARAMETER2COORD(coord, &x, &y);
	BitMask_setbit(map, x, y, 1);
	return self;
}

static VALUE collision_map_unset(VALUE self, VALUE coord)
{
	BitMask* map;
	Sint16 x, y;
	Data_Get_Struct(self, BitMask, map);
	PARAMETER2COORD(coord, &x, &y);
	BitMask_setbit(map, x, y, 0);
	return self;
}

///////////////////////////////// INIT
void initVideoSGEClasses()
{
	rb_eval_string(
		"module RUDL class Surface		\n"
		"	attr_accessor :collision_map	\n"
		"end end"
	);

	classCollisionMap=rb_define_class_under(moduleRUDL, "CollisionMap", rb_cObject);
	rb_define_singleton_method(classCollisionMap, "new", VALUEFUNC(collision_map_new), 1);
	rb_define_method(classCollisionMap, "collides_with", VALUEFUNC(collision_map_collides_with), 3);
	rb_define_method(classCollisionMap, "set", VALUEFUNC(collision_map_set), 1);
	rb_define_method(classCollisionMap, "unset", VALUEFUNC(collision_map_unset), 1);
	rb_define_method(classCollisionMap, "destroy", VALUEFUNC(collision_map_destroy), 0);
}

