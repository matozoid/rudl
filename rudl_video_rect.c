/* RUDL - a C library wrapping SDL for use in Ruby. Copyright (C) 2001, 2002  Danny van Bruggen */
#include "rudl_events.h"
#include "rudl_video.h"

extern void add_ruby_to_rect();

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
<<< docs/head
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
=end */

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
A more advanced collision detection method can be found in CollisionMap.
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

void add_ruby_code_to_rect()
{
	rb_eval_string(
		"	module RUDL class Rect	\n"
/*
=begin
--- Rect#x
--- Rect#y
--- Rect#w
--- Rect#h
These can be set and read at will.
=end */
		"		attr_accessor :x, :y, :w, :h\n"
/*
=begin
--- Rect#to_ary
Returns [x, y, w, h]
=end */
		"		def to_ary			\n"
		"			[@x, @y, @w, @h]	\n"
		"		end				\n"
/*
=begin
--- Rect#move( delta )
--- Rect#move!( delta )
Returns a new rectangle which is the base rectangle moved by the given amount.
=end */

		"		def move!( delta )		\n"
		"			@x+=delta[0]		\n"
		"			@y+=delta[1]		\n"
		"			self			\n"
		"		end\n"

		"		def move( delta )		\n"
		"			clone.move!(delta)	\n"
		"		end				\n"
/*
=begin
--- Rect#normalize
--- Rect#normalize!
If w and h aren't positive,
this will change them to positive and keep the rect covering the same space.
=end */
		"		def normalize!			\n"
		"			if @w<0 then		\n"
		"				@x+=@w		\n"
		"				@w=-@w		\n"
		"			end			\n"
		"			if @h<0 then		\n"
		"				@y+=@h		\n"
		"				@h=-@h		\n"
		"			end			\n"
		"			self			\n"
		"		end				\n"

		"		def normalize			\n"
		"			clone.normalize!	\n"
		"		end				\n"
/*
=begin
--- Rect#inflate( sizes )
--- Rect#inflate!( sizes )
Returns a rectangle which has the sizes changed by the given amounts.
((|sizes|)) is an array of [dx, dy].
The rectangle shrinks and expands around the rectangle's center.
Negative values will shrink the rectangle.
=end */
		"		def inflate!( size )		\n"
		"			@x-=size[0]/2		\n"
		"			@y-=size[1]/2		\n"
		"			@w+=size[0]		\n"
		"			@h+=size[1]		\n"
		"			self			\n"
		"		end				\n"

		"		def inflate( size )		\n"
		"			clone.inflate!(size)	\n"
		"		end				\n"
/*
=begin
--- Rect#union( rect )
--- Rect#union!( rect )
Returns a new rectangle that completely covers the given inputs.
There may be area inside the newrectangle that is not covered by the inputs.
=end */
		"		def union!( other_rect )		\n"
		"			if other_rect.type==Array	\n"
		"				other_rect=RUDL::Rect.new(other_rect)	\n"
		"			end				\n"
		"			newx = [@x, other_rect.x].min	\n"
		"			newy = [@y, other_rect.y].min	\n"
		"			@w = [@x+@w, other_rect.x+other_rect.w].max-newx	\n"
		"			@h = [@y+@h, other_rect.y+other_rect.h].max-newy	\n"
		"			@x=newx				\n"
		"			@y=newy				\n"
		"			self				\n"
		"		end					\n"

		"		def union(other_rect)			\n"
		"			clone.union!(other_rect)	\n"
		"		end					\n"
/*
=begin
--- Rect#contains( thing )
Returns whether thing (a Rect, [x, y, w, h] or [x, y]) fits completely within the rectangle.
=end */
		"		def contains(thing)\n"
		"			if thing.type==RUDL::Rect	\n"
		"				thing=thing.to_ary	\n"
		"			end				\n"
		"			if thing.length==2		\n"
		"				return(thing[0]>=@x && thing[0]<=@x+@w && thing[1]>=@y && thing[1]<=@y+@h)\n"
		"			elsif thing.length==4		\n"
		"				return((@x <= thing[0]) && (@y <= thing[1]) && (@x + @w >= thing[0] + thing[2]) && (@y + @h >= thing[1] + thing[3]))\n"
		"			end				\n"
		"			raise ArgumentError.new		\n"
		"		end					\n"
/*
=begin
--- Rect#find_overlapping_rect( rects )
Returns the first rectangle in the list to overlap the base rectangle.
Once an overlap is found, this will stop checking the remaining list.
If no overlap is found, it will return nil.
=end */
		"		def find_overlapping_rect(rects)	\n"
		"			rects.each {|r|			\n"
		"				if overlap(r) then	\n"
		"					return r	\n"
		"				end			\n"
		"			}				\n"
		"			nil				\n"
		"		end					\n"
/*
=begin
--- Rect#find_overlapping_rects( rects )
Returns an array with the rectangles in the list to overlap the base rectangle.
If no overlap is found, it will return [].
=end */
		"		def find_overlapping_rects(rects)\n"
		"			overlaps=[]\n"
		"			rects.each {|r| \n"
		"				if overlap(r) then \n"
		"					overlaps.push(r)\n"
		"				end \n"
		"			}\n"
		"			overlaps\n"
		"		end\n"
/*
=begin
--- Rect#clip( rect )
--- Rect#clip!( rect )
Returns a new rectangle that is the given rectangle cropped to the inside of the base rectangle.
If the two rectangles do not overlap to begin with, you will get a rectangle with 0 size.
=end */
		"		def clip!(b)				\n"
		"			if b.type==Array		\n"
		"				b=RUDL::Rect.new(b)	\n"
		"			end				\n"
		"			# Left\n"
		"			if((@x >= b.x) && (@x < (b.x+b.w))) then \n"
		"				x = @x\n"
		"			elsif((b.x >= @x) && (b.x < (@x+@w))) then\n"
		"				x = b.x\n"
		"			else\n"
		"				@w=0\n"
		"				@h=0\n"
		"				return self\n"
		"			end\n"
		"\n"
		"			# Right\n"
		"			if(((@x+@w) > b.x) && ((@x+@w) <= (b.x+b.w)))\n"
		"				w = (@x+@w) - x;\n"
		"			elsif(((b.x+b.w) > @x) && ((b.x+b.w) <= (@x+@w)))\n"
		"				w = (b.x+b.w) - x;\n"
		"			else\n"
		"				@w=0\n"
		"				@h=0\n"
		"				return self\n"
		"			end\n"
		"\n"
		"			# Top\n"
		"			if (@y >= b.y) && (@y < (b.y+b.h)) then\n"
		"				y = @y;\n"
		"			elsif (b.y >= @y) && (b.y < (@y+@h)) then\n"
		"				y = b.y;\n"
		"			else\n"
		"				@w=0\n"
		"				@h=0\n"
		"				return self\n"
		"			end\n"
		"\n"
		"			# Bottom\n"
		"			if ((@y+@h) > b.y) && ((@y+@h) <= (b.y+b.h)) then\n"
		"				h = (@y+@h) - y;\n"
		"			elsif ((b.y+b.h) > @y) && ((b.y+b.h) <= (@y+@h)) then\n"
		"				h = (b.y+b.h) - y;\n"
		"			else\n"
		"				@w=0\n"
		"				@h=0\n"
		"				return self\n"
		"			end\n"
		"			@x=x\n"
		"			@y=y\n"
		"			@w=w\n"
		"			@h=h\n"
		"			self\n"
		"		end\n"
		"\n"
		"		def clip(b)\n"
		"			clone.clip!(b)\n"
		"		end\n"
/*
=begin
--- Rect#clamp( rect )
--- Rect#clamp!( rect )
Returns a new rectangle that is moved to be completely inside the base rectangle.
If the given rectangle is too large for the base rectangle in an axis, 
it will be centered on that axis.
=end */
		"		def clamp!(argrect)\n"
		"			if argrect.type==Array then argrect=RUDL::Rect.new(argrect); end\n"
		"\n"
		"			if @w >= argrect.w then\n"
		"				x = (argrect.x+argrect.w) / 2 - @w / 2\n"
		"			elsif @x < argrect.x then\n"
		"				x = argrect.x\n"
		"			elsif @x + @w > argrect.x + argrect.w then\n"
		"				x = argrect.x + argrect.w - @w\n"
		"			else\n"
		"				x = @x\n"
		"			end\n"
		"\n"
		"			if @h >= argrect.h then\n"
		"				y = (argrect.y+argrect.h) / 2 - @h / 2\n"
		"			elsif @y < argrect.y then\n"
		"				y = argrect.y\n"
		"			elsif @y + @h > argrect.y + argrect.h then\n"
		"				y = argrect.y + argrect.h - @h\n"
		"			else\n"
		"				y = @y\n"
		"			end\n"
		"\n"
		"			@x=x\n"
		"			@y=y\n"
		"			self\n"
		"		end\n"
		"\n"
		"		def clamp(argrect)\n"
		"			clone.clamp!(argrect)\n"
		"		end\n"
		"	end\n"
		"end\n"
	);
}
/*
=begin
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
///////////////////////////////// INIT
void initVideoRectClasses()
{
	classRect=rb_define_class_under(moduleRUDL, "Rect", rb_cObject);
	rb_define_singleton_method(classRect, "new", rect_new, -1);
	rb_define_method(classRect, "overlap", rect_overlap, 1);
	rb_define_singleton_method(classRect, "collide_lists", rect_collide_lists, 2);
	add_ruby_code_to_rect();
}
