/* RUDL - a C library wrapping SDL for use in Ruby. Copyright (C) 2001, 2002  Danny van Bruggen */
#include "rudl_events.h"
#include "rudl_video.h"

static VALUE rb_array_overlaps(VALUE self, VALUE otherRect);

///////////////////////////////// ARRAY
__inline__ static bool intersect(SDL_Rect *a, SDL_Rect *b)
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

#define GET_X() double x=array_get_x(self) 
#define GET_Y() double y=array_get_y(self) 
#define GET_W() double w=array_get_w(self) 
#define GET_H() double h=array_get_h(self)
#define SET_X(x) array_set_x(self, (x))
#define SET_Y(x) array_set_y(self, (x))
#define SET_W(x) array_set_w(self, (x))
#define SET_H(x) array_set_h(self, (x))

__inline__ static VALUE array_get_x_value(VALUE array)	{return rb_ary_entry(array, 0);}
__inline__ static VALUE array_get_y_value(VALUE array)	{return rb_ary_entry(array, 1);}
__inline__ static VALUE array_get_w_value(VALUE array)	{return rb_ary_entry(array, 2);}
__inline__ static VALUE array_get_h_value(VALUE array)	{return rb_ary_entry(array, 3);}

__inline__ static double array_get_x(VALUE array)	{return NUM2DBL(rb_ary_entry(array, 0));}
__inline__ static double array_get_y(VALUE array)	{return NUM2DBL(rb_ary_entry(array, 1));}
__inline__ static double array_get_w(VALUE array)	{return NUM2DBL(rb_ary_entry(array, 2));}
__inline__ static double array_get_h(VALUE array)	{return NUM2DBL(rb_ary_entry(array, 3));}

__inline__ static void array_set_x_value(VALUE array, VALUE x)	{rb_ary_store(array, 0, x);}
__inline__ static void array_set_y_value(VALUE array, VALUE y)	{rb_ary_store(array, 1, y);}
__inline__ static void array_set_w_value(VALUE array, VALUE w)	{rb_ary_store(array, 2, w);}
__inline__ static void array_set_h_value(VALUE array, VALUE h)	{rb_ary_store(array, 3, h);}

__inline__ static void array_set_x(VALUE array, double x)	{rb_ary_store(array, 0, DBL2NUM(x));}
__inline__ static void array_set_y(VALUE array, double y)	{rb_ary_store(array, 1, DBL2NUM(y));}
__inline__ static void array_set_w(VALUE array, double w)	{rb_ary_store(array, 2, DBL2NUM(w));}
__inline__ static void array_set_h(VALUE array, double h)	{rb_ary_store(array, 3, DBL2NUM(h));}

__inline__ void PARAMETER2CRECT(VALUE arg1, SDL_Rect* rect)
{
	VALUE tmp;
	if(rb_obj_is_kind_of(arg1, rb_cArray)){
		tmp=rb_ary_entry(arg1, 0);			rect->x=NUM2Sint16(tmp);
		tmp=rb_ary_entry(arg1, 1);			rect->y=NUM2Sint16(tmp);
		tmp=rb_ary_entry(arg1, 2);			rect->w=NUM2Uint16(tmp);
		tmp=rb_ary_entry(arg1, 3);			rect->h=NUM2Uint16(tmp);
	}else{
		rb_raise(rb_eTypeError, "Wanted RUDL::Rect or array");
	}
}

__inline__ void RECT2CRECT(VALUE source, SDL_Rect* destination)
{
	destination->x=array_get_x(source);
	destination->y=array_get_y(source);
	destination->w=array_get_w(source);
	destination->h=array_get_h(source);
}

__inline__ VALUE new_rect(double x, double y, double w, double h)
{
	return rb_ary_new3(4, DBL2NUM(x), DBL2NUM(y), DBL2NUM(w), DBL2NUM(h));
}

__inline__ VALUE new_rect_from_SDL_Rect(SDL_Rect* source)
{
	return new_rect(source->x, source->y, source->w, source->h);
}

__inline__ static void normalize(VALUE self)
{
	GET_W();
	GET_H();
	if(w<0){
		GET_X();
		x=x+w;
		w=-w;
		SET_X(x);
		SET_W(w);
	}
	if(h<0){
		GET_Y();
		y=y+h;
		h=-h;
		SET_Y(y);
		SET_H(h);
	}
}

/*
=begin
<<< docs/head
= Rect
Rect has been discarded.
It's methods have moved to the standard Array.
Rect is now a subclass of Array for backward compatability.
= Array
The standard Ruby array class has been extended to provide 
methods for using it as a rectangle.
Arrays like these have entries 0, 1, 2 and 3 set to integer values for
x, y, width and height.
== Class Methods
=end */

struct ExtRect{
	SDL_Rect crect;
	VALUE rect;
	VALUE sprite;
};

/*
=begin
--- Array.collide_lists( l1, l2 ) BOGUS
This method looks through list ((|l1|)),
checking collisions with every object in list ((|l2|)).
It does this by calling "rect" on all objects, expecting an array of [x,y,w,h] back,
defining the area this object is in.
It yields (object_from_l1, object_from_l2) for every collision it detects.
A more advanced collision detection method can be found in CollisionMap.
=end */

static VALUE rb_array_collide_lists(VALUE self, VALUE list1Value, VALUE list2Value)
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

	if(list1len==0 || list2len==0) return self;

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

/*
=begin
--- Array.union_list( array_of_rects )
Returns a new array representing a rectangle covering all rectangles
in ((|array_of_rects|)).
=end */
static VALUE rb_array_union_list(VALUE self, VALUE other_rects)
{
	int i;
	double left;
	double right;
	double top;
	double bottom;
	double l,r,t,b;
	VALUE rect;
	
	if(RARRAY(other_rects)->len==0){
		return Qnil;
	}

	rect=rb_ary_entry(other_rects, 0);
	left=array_get_x(rect);
	right=array_get_w(rect)+left;
	top=array_get_y(rect);
	bottom=array_get_h(rect)+top;

	for(i=1; i<RARRAY(other_rects)->len; i++){
		rect=rb_ary_entry(other_rects, i);
		l=array_get_x(rect);
		r=array_get_w(rect)+l;
		t=array_get_y(rect);
		b=array_get_h(rect)+t;
		left=RUDL_MIN(left, l);
		right=RUDL_MAX(right, r);
		top=RUDL_MIN(top, t);
		bottom=RUDL_MAX(bottom, b);
	}

	return new_rect(left, top, right-left, bottom-top);
}

/* accessors */
/*
=begin
--- Array#x
--- Array#x=( x )
--- Array#y
--- Array#y=( y )
--- Array#w
--- Array#w=( w )
--- Array#h
--- Array#h=( h )
These can be set and read at will.
((|w|)) and ((|h|)) have ((|width|)) and ((|height|)) as aliases.
=end */
static VALUE rb_array_get_x(VALUE self) {return array_get_x_value(self);}
static VALUE rb_array_get_y(VALUE self) {return array_get_y_value(self);}
static VALUE rb_array_get_w(VALUE self) {return array_get_w_value(self);}
static VALUE rb_array_get_h(VALUE self) {return array_get_h_value(self);}

static VALUE rb_array_set_x(VALUE self, VALUE new_x) {array_set_x_value(self, new_x); return self;}
static VALUE rb_array_set_y(VALUE self, VALUE new_y) {array_set_y_value(self, new_y); return self;}
static VALUE rb_array_set_w(VALUE self, VALUE new_w) {array_set_w_value(self, new_w); return self;}
static VALUE rb_array_set_h(VALUE self, VALUE new_h) {array_set_h_value(self, new_h); return self;}

/*
=begin
== Instance methods
--- Array#left
--- Array#left=( left )
--- Array#right
--- Array#right=( right )
--- Array#top
--- Array#top=( top )
--- Array#bottom
--- Array#bottom=( bottom )
These can be set and read at will.
Differences with x, y, w and h are:
* ((|right|)) and ((|bottom|)) are screen coordinates.
	((|right|)) is ((|x|))+((|w|)) and ((|bottom|)) is ((|y|))+((|h|)).
=end */
static VALUE rb_array_set_right(VALUE self, VALUE new_right)
{
	SET_X(NUM2DBL(new_right)-array_get_w(self));
	return self;
}

static VALUE rb_array_set_bottom(VALUE self, VALUE new_bottom)
{
	SET_Y(NUM2DBL(new_bottom)-array_get_h(self));
	return self;
}

static VALUE rb_array_get_right(VALUE self)
{
	return DBL2NUM(array_get_x(self)+array_get_w(self));
}

static VALUE rb_array_get_bottom(VALUE self)
{
	return DBL2NUM(array_get_y(self)+array_get_h(self));
}

/* functions */
/*
=begin
--- Array#normalize
--- Array#normalize!
If w and h aren't positive,
this will change them to positive and keep the rectangle covering the same space.
=end */

static VALUE rb_array_normalize_bang(VALUE self)
{
	normalize(self);
	return self;
}

static VALUE rb_array_normalize(VALUE self)
{
	VALUE retval=rb_obj_dup(self);
	normalize(retval);
	return retval;
}

/*
=begin
--- Array#move( delta )
--- Array#move!( delta )
Returns a new rectangle which is the base rectangle moved by the given amount.
=end */

static VALUE rb_array_move_bang(VALUE self, VALUE delta)
{
	array_set_x(self, array_get_x(self)+array_get_x(delta));
	array_set_y(self, array_get_y(self)+array_get_y(delta));
	return self;
}

static VALUE rb_array_move(VALUE self, VALUE delta)
{
	return rb_array_move_bang(rb_obj_dup(self), delta);
}

/*
=begin
--- Array#inflate( sizes )
--- Array#inflate!( sizes )
Returns a rectangle which has the sizes changed by the given amounts.
((|sizes|)) is an array of [dx, dy].
The rectangle shrinks and expands around the rectangle's center.
Negative values will shrink the rectangle.
=end */

static VALUE rb_array_inflate_bang(VALUE self, VALUE size)
{
	normalize(self);
	{
		GET_X();
		GET_Y();
		GET_W();
		GET_H();
		double dx=array_get_x(size);
		double dy=array_get_y(size);
		SET_X(x-dx/2);
		SET_Y(y-dy/2);
		SET_W(w+dx);
		SET_H(h+dy);
	}
	return self;
}

static VALUE rb_array_inflate(VALUE self, VALUE size)
{
	return rb_array_inflate_bang(rb_obj_dup(self), size);
}

/*
=begin
--- Array#union( rect )
--- Array#union!( rect )
Returns a new rectangle that completely covers the given rectangle(s).
There may be an area inside the new rectangle that is not covered by the inputs.
Rectangles are pre-normalized.
=end */
static VALUE rb_array_union_bang(VALUE self, VALUE other_rect)
{
	normalize(self);
	normalize(other_rect);
	{
		GET_X();
		GET_Y();
		GET_W();
		GET_H();
		double other_x=array_get_x(other_rect);
		double other_y=array_get_y(other_rect);
		double other_w=array_get_w(other_rect);
		double other_h=array_get_h(other_rect);

		double new_x=RUDL_MIN(x, other_x);
		double new_y=RUDL_MIN(y, other_y);
		
		SET_X( new_x );
		SET_Y( new_y );
		SET_W( RUDL_MAX(x+w, other_x+other_w)-new_x );
		SET_H( RUDL_MAX(y+h, other_y+other_h)-new_y );
	}
	return self;
}

static VALUE rb_array_union(VALUE self, VALUE other_rect)
{
	return rb_array_union_bang(rb_obj_dup(self), other_rect);
}

/*
=begin
--- Array#contains( thing )
Returns whether thing ([x, y, w, h] or [x, y]) fits completely within the rectangle.
=end */
static VALUE rb_array_contains(VALUE self, VALUE thing)
{
	GET_X();
	GET_Y();
	GET_W();
	GET_H();
	double x2,y2,w2,h2;
	
	Check_Type(thing, T_ARRAY);
	
	x2=array_get_x(thing);
	y2=array_get_y(thing);
	
	if(RARRAY(thing)->len>3){ // It's a rectangle
		w2=array_get_w(thing);
		h2=array_get_h(thing);
		return INT2BOOL((x <= x2) && (y <= y2) &&
				(x + w >= x2 + w2) && (y + h >= y2 + h2) &&
				(x + w > x2) && (y + h > y2));
	}else{ // It's a point
		return INT2BOOL((x2>=x && x2<x+w && y2>=y && y2<y+h));
	}

	return Qfalse;
}
/*
=begin
--- Array#find_overlapping_rect( rects )
Returns the first rectangle in the array to overlap the base rectangle.
Once an overlap is found, this will stop checking the remaining array.
If no overlap is found, it will return false.
=end */
static VALUE rb_array_find_overlapping_rect(VALUE self, VALUE rects)
{
	int i;
	Check_Type(rects, T_ARRAY);
	for(i=0; i<RARRAY(rects)->len; i++){
		if(rb_array_overlaps(self, rb_ary_entry(rects, i))==Qtrue){
			return rb_ary_entry(rects, i);
		}
	}
	return Qfalse;
}
/*
=begin
--- Array#find_overlapping_rects( rects )
Returns an array with the rectangles in the list to overlaps the base rectangle.
If no overlaps is found, it will return [].
=end */
static VALUE rb_array_find_overlapping_rects(VALUE self, VALUE rects)
{
	int i;
	VALUE retval=rb_ary_new();
	Check_Type(rects, T_ARRAY);
	for(i=0; i<RARRAY(rects)->len; i++){
		if(rb_array_overlaps(self, rb_ary_entry(rects, i))==Qtrue){
			rb_ary_push(retval, rb_ary_entry(rects, i));
		}
	}
	return retval;
}
/*
=begin
--- Array#clip( rect )
--- Array#clip!( rect )
Returns a new rectangle that is the given rectangle cropped to the inside of the base rectangle.
If the two rectangles do not overlaps to begin with, you will get a rectangle with 0 size.
=end */
static VALUE rb_array_clip_bang(VALUE self, VALUE rect)
{
	SDL_Rect a, b;
	double x,y,w,h;
	RECT2CRECT(self, &a);
	RECT2CRECT(rect, &b);
	/* Left */
	if((a.x >= b.x) && (a.x < (b.x+b.w)))
		x = a.x;
	else if((b.x >= a.x) && (b.x < (a.x+a.w)))
		x = b.x;
	else
		goto nointersect;
	/* Right */
	if(((a.x+a.w) > b.x) && ((a.x+a.w) <= (b.x+b.w)))
		w = (a.x+a.w) - x;
	else if(((b.x+b.w) > a.x) && ((b.x+b.w) <= (a.x+a.w)))
		w = (b.x+b.w) - x;
	else
		goto nointersect;
	
	/* Top */
	if((a.y >= b.y) && (a.y < (b.y+b.h)))
		y = a.y;
	else if((b.y >= a.y) && (b.y < (a.y+a.h)))
		y = b.y;
	else
		goto nointersect;
	/* Bottom */
	if (((a.y+a.h) > b.y) && ((a.y+a.h) <= (b.y+b.h)))
		h = (a.y+a.h) - y;
	else if(((b.y+b.h) > a.y) && ((b.y+b.h) <= (a.y+a.h)))
		h = (b.y+b.h) - y;
	else
		goto nointersect;

	SET_X(x);
	SET_Y(y);
	SET_W(w);
	SET_H(h);
	return self;
nointersect:
	SET_X(a.x);
	SET_Y(a.y);
	SET_W(0);
	SET_H(0);
	return self;
}

static VALUE rb_array_clip(VALUE self, VALUE rect)
{
	return rb_array_clip_bang(rb_obj_dup(self), rect);
}
/*
=begin
--- Array#clamp( rect )
--- Array#clamp!( rect )
Returns a new rectangle that is moved to be completely inside the base rectangle.
If the given rectangle is too large for the base rectangle in an axis, 
it will be centered on that axis.
=end */
static VALUE rb_array_clamp_bang(VALUE self, VALUE rect)
{
	SDL_Rect a, b;
	double x, y;
	RECT2CRECT(self, &a);
	RECT2CRECT(rect, &b);

	if(a.w >= b.w)
		x = b.x + b.w / 2 - a.w / 2;
	else if(a.x < b.x)
		x = b.x;
	else if(a.x + a.w > b.x + b.w)
		x = b.x + b.w - a.w;
	else
		x = a.x;

	if(a.h >= b.h)
		y = b.y + b.h / 2 - a.h / 2;
	else if(a.y < b.y)
		y = b.y;
	else if(a.y + a.h > b.y + b.h)
		y = b.y + b.h - a.h;
	else
		y = a.y;

	SET_X(x);
	SET_Y(y);
	return self;
}

static VALUE rb_array_clamp(VALUE self, VALUE rect)
{
	return rb_array_clamp_bang(rb_obj_dup(self), rect);
}
/*
=begin
--- Array#overlaps( rect )
Returns true if any area of the two rectangles overlapss.
=end */
static VALUE rb_array_overlaps(VALUE self, VALUE otherRect)
{
	SDL_Rect a, b;
	RECT2CRECT(self, &a);
	RECT2CRECT(otherRect, &b);
	return INT2BOOL(intersect(&a, &b));
}

///////////////////////////////// INIT
void initVideoRectClasses()
{
	rb_define_singleton_method(rb_cArray, "collide_lists", rb_array_collide_lists, 2);
	rb_define_singleton_method(rb_cArray, "union_list", rb_array_union_list, 1);

	rb_define_method(rb_cArray, "x", rb_array_get_x, 0);
	rb_define_method(rb_cArray, "y", rb_array_get_y, 0);
	rb_define_method(rb_cArray, "w", rb_array_get_w, 0);
	rb_define_method(rb_cArray, "h", rb_array_get_h, 0);

	rb_define_method(rb_cArray, "x=", rb_array_set_x, 1);
	rb_define_method(rb_cArray, "y=", rb_array_set_y, 1);
	rb_define_method(rb_cArray, "w=", rb_array_set_w, 1);
	rb_define_method(rb_cArray, "h=", rb_array_set_h, 1);

	rb_alias(rb_cArray, rb_intern("left"), rb_intern("x"));
	rb_alias(rb_cArray, rb_intern("top"), rb_intern("y"));
	rb_define_method(rb_cArray, "right", rb_array_get_right, 0);
	rb_define_method(rb_cArray, "bottom", rb_array_get_bottom, 0);

	rb_alias(rb_cArray, rb_intern("left="), rb_intern("x="));
	rb_alias(rb_cArray, rb_intern("top="), rb_intern("y="));
	rb_define_method(rb_cArray, "right=", rb_array_set_right, 1);
	rb_define_method(rb_cArray, "bottom=", rb_array_set_bottom, 1);

	rb_define_method(rb_cArray, "normalize", rb_array_normalize, 0);
	rb_define_method(rb_cArray, "normalize!", rb_array_normalize_bang, 0);

	rb_define_method(rb_cArray, "move", rb_array_move, 1);
	rb_define_method(rb_cArray, "move!", rb_array_move_bang, 1);

	rb_define_method(rb_cArray, "inflate", rb_array_inflate, 1);
	rb_define_method(rb_cArray, "inflate!", rb_array_inflate_bang, 1);

	rb_define_method(rb_cArray, "union", rb_array_union, 1);
	rb_define_method(rb_cArray, "union!", rb_array_union_bang, 1);

	rb_define_method(rb_cArray, "contains", rb_array_contains, 1);

	rb_define_method(rb_cArray, "overlaps", rb_array_overlaps, 1);

	rb_define_method(rb_cArray, "find_overlapping_rect", rb_array_find_overlapping_rect, 1);
	rb_define_method(rb_cArray, "find_overlapping_rects", rb_array_find_overlapping_rects, 1);
	
	rb_define_method(rb_cArray, "clip", rb_array_clip, 1);
	rb_define_method(rb_cArray, "clip!", rb_array_clip_bang, 1);

	rb_define_method(rb_cArray, "clamp", rb_array_clamp, 1);
	rb_define_method(rb_cArray, "clamp!", rb_array_clamp_bang, 1);

	// Backward compatability:
	
	rb_define_class_under(moduleRUDL, "Rect", rb_cArray);
}
