/* RUDL - a C library wrapping SDL for use in Ruby. Copyright (C) 2001, 2002  Danny van Bruggen */
#include "rudl_events.h"
#include "rudl_video.h"
#include "sge.h"

static VALUE collision_map_new(VALUE self, VALUE surface)
{
	sge_cdata* map=sge_make_cmap(retrieveSurfacePointer(surface));
	if(map){
		return Data_Wrap_Struct(classCollisionMap, 0, sge_destroy_cmap, map);
	}else{
		SDL_RAISE_S("Collision map could not be created");
		return Qnil;
	}

}

static VALUE collision_map_collides_with(VALUE self, VALUE coord1, VALUE other_map, VALUE coord2)
{
	Sint16 x1,y1,x2,y2;
	sge_cdata* map1, *map2;

	Data_Get_Struct(self, sge_cdata, map1);
	Data_Get_Struct(other_map, sge_cdata, map2);

	PARAMETER2COORD(coord1, &x1, &y1);
	PARAMETER2COORD(coord2, &x2, &y2);

	if(sge_cmcheck(map1, x1, y1, map2, x2, y2)){
		return rb_ary_new3(2, INT2NUM(sge_get_cx()), INT2NUM(sge_get_cy()));
	}else{
		return Qnil;
	}
}

static VALUE collision_map_destroy(VALUE self)
{
	sge_cdata* map;
	Data_Get_Struct(self, sge_cdata, map);
	sge_destroy_cmap(map);
	return Qnil;
}

static VALUE collision_map_set(VALUE self, VALUE area)
{
	sge_cdata* map;
	SDL_Rect rect;
	Data_Get_Struct(self, sge_cdata, map);
	PARAMETER2CRECT(area, &rect);
	sge_set_cdata(map, rect.x, rect.y, rect.w, rect.h);
	return self;
}

static VALUE collision_map_unset(VALUE self, VALUE area)
{
	sge_cdata* map;
	SDL_Rect rect;
	Data_Get_Struct(self, sge_cdata, map);
	PARAMETER2CRECT(area, &rect);
	sge_unset_cdata(map, rect.x, rect.y, rect.w, rect.h);
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

