/* RUDL - a C library wrapping SDL for use in Ruby. Copyright (C) 2001  Danny van Bruggen */
#include "rudl_sprite.h"
#include "sge_collision.h"

typedef struct {
	#ifdef HAVE_SGE_H
		sge_cdata* collision_map;
	#else
		char placeholder; // So Sprite won't be empty when SGE not installed
	#endif
}RUDLSprite;

static VALUE sprite_new(VALUE self)
{
	VALUE newSprite;
	RUDLSprite* newSpritePtr=(RUDLSprite*)malloc(sizeof(RUDLSprite));
	#ifdef HAVE_SGE_H
		newSpritePtr->collision_map=NULL;
	#else
	#endif
	return newSprite;
}

static void add_sprite_properties()
{
	rb_eval_string(
		"module RUDL class Sprite	\n"
		"	attr_accessor :x, :y	\n"
		"	attr_reader :surface	\n"
		"end end"
	);
}

static void add_sprite_methods()
{
	rb_eval_string(
		"module RUDL class Sprite	\n"
		"	def move(dx, dy)	\n"
		"		@x+=dx		\n"
		"		@y+=dy		\n"
		"	end			\n"
		"	def w			\n"
		"		return @surface.w if @surface	\n"
		"		nil		\n"
		"	end			\n"
		"	def h			\n"
		"		return @surface.h if @surface	\n"
		"		nil		\n"
		"	end			\n"
		"	def rect		\n"
		"		[@x,@y,@w,@h]	\n"
		"	end			\n"
		"	def rect=(r)		\n"
		"		@x=r.x		\n"
		"		@y=r.y		\n"
		"		@w=r.w		\n"
		"		@h=r.h		\n"
		"	end			\n"
		"	def pos			\n"
		"		[@x,@y]		\n"
		"	end			\n"
		"	def pos=(p)		\n"
		"	end			\n"
		"	def area_collision_check(other_sprite)	\n"
		"	end			\n"
		"end end"
	);
}

void initSpriteClasses()
{
	classSprite=rb_define_class_under(moduleRUDL, "Sprite", rb_cObject);
	add_sprite_properties();
	rb_define_singleton_method(classSprite, "new", sprite_new, 2);
	//rb_define_method(classTTF, "ascent", truetypefont_ascent, 0);
	add_sprite_methods();
}
