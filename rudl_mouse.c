/* RUDL - a C library wrapping SDL for use in Ruby. Copyright (C) 2001  Danny van Bruggen */
#include "rudl_mouse.h"
#include "rudl_video.h"
#include "rudl_events.h"

/**
@file mouse
@class Mouse
The mouse class methods can also be used as instance methods once you
instantiate the class. However, there is no need to do that, it's just
for convenience.
*/
/**
@section Class and instance Methods
@method focused? -> boolean
Returns true when the application is receiving the mouse input focus.
*/
static VALUE mouse_focused_(VALUE self)
{
	initVideo();
	return INT2BOOL((SDL_GetAppState()&SDL_APPMOUSEFOCUS) != 0);
}
/**
@method pos -> [x, y]
Returns the current position of the mouse cursor.
This is the absolute mouse position inside your game window.
*/
static VALUE mouse_pos(VALUE self)
{
	int x, y;

	initVideo();

	SDL_GetMouseState(&x, &y);
	return rb_ary_new3(2, INT2NUM(x), INT2NUM(y));
}
/**
@method pressed? -> [boolean, boolean, boolean]
This will return an array containing the pressed state of each mouse button.
*/
static VALUE mouse_pressed_(VALUE self)
{
	Uint32 state = SDL_GetMouseState(NULL, NULL);

	return rb_ary_new3(3,
		INT2BOOL((state&SDL_BUTTON(1)) != 0),
		INT2BOOL((state&SDL_BUTTON(2)) != 0),
		INT2BOOL((state&SDL_BUTTON(3)) != 0));
}
/**
@method rel -> [dx, dy]
Returns the total distance the mouse has moved since your last call to @rel.
On the first call to @rel the movement will always be 0,0.
When the mouse is at the edges of the screen, the relative movement will be stopped.
See @visible for a way to resolve this.
*/
static VALUE mouse_rel(VALUE self)
{
	int x, y;

	initVideo();

	SDL_GetRelativeMouseState(&x, &y);
	return rb_ary_new3(2, INT2NUM(x), INT2NUM(y));
}
/**
@method pos=( pos ) -> self
Moves the mouse cursor to the specified position.
This will generate a @MouseMotionEvent on the input queue.
*/
static VALUE mouse_set_pos(VALUE self, VALUE pos)
{
	Sint16 x,y;
	initVideo();
	PARAMETER2COORD(pos, &x, &y);
	SDL_WarpMouse(x, y);
	return self;
}
/**
@method visible=( onOrOff ) -> boolean
Shows or hides the mouse cursor.
This will return the previous visible state of the mouse cursor.

Note that when the cursor is hidden and the application has grabbed the input.
SDL will force the mouse to stay in the center of the screen.
Since the mouse is hidden it won't matter that it's not moving,
but it will keep the mouse from the edges of the screen so the relative mouse 
position will always be true.
*/
static VALUE mouse_set_visible(VALUE self, VALUE yes)
{
	initVideo();
	return INT2BOOL(SDL_ShowCursor(NUM2BOOL(yes)));
}

/**
@method set_cursor( hotspot, xormasks, andmasks ) -> self
When the mouse cursor is visible, it will be displayed as a black and white bitmap 
using the given bitmask arrays.
Hotspot is an array containing the cursor hotspot position.
xormasks is an array of arrays of bytes containing the cursor xor data masks.
Lastly is andmasks, an array of arrays of bytes containting the cursor bitmask data.

The example "mousecursor.rb" explains this much better.
*/
static VALUE mouse_set_cursor(VALUE self, VALUE hotspot, VALUE xormasks, VALUE andmasks)
{
	int sx, sy;
	int x, y;
	SDL_Cursor *lastcursor, *cursor = NULL;
	Uint8 *xordata=NULL, *anddata=NULL;
	Sint16 spotx, spoty;

	VALUE tmp;

	initVideo();

	PARAMETER2COORD(hotspot, &spotx, &spoty);

	Check_Type(xormasks, T_ARRAY);
	Check_Type(andmasks, T_ARRAY);
	sx=RARRAY(rb_ary_entry(xormasks,0))->len;
	sy=RARRAY(xormasks)->len;
	if((sx!=RARRAY(rb_ary_entry(andmasks,0))->len)||(sy!=RARRAY(xormasks)->len)){
		SDL_RAISE_S("andmasks and xormasks should be the same size");
	}

	xordata = (Uint8*)malloc(sx*sy);
	anddata = (Uint8*)malloc(sx*sy);

	for(y=0; y<sy; y++){
		for(x=0; x<sx; x++){

			tmp=rb_ary_entry(rb_ary_entry(xormasks, y), x);
			xordata[y*sx+x]=(Uint8)NUM2UINT(tmp);

			tmp=rb_ary_entry(rb_ary_entry(andmasks, y), x);
			anddata[y*sx+x]=(Uint8)NUM2UINT(tmp);
		}
	}

	cursor = SDL_CreateCursor(xordata, anddata, sx*8, sy, spotx, spoty);
	free(xordata);
	free(anddata);

	if(!cursor) SDL_RAISE;

	lastcursor = SDL_GetCursor();
	SDL_SetCursor(cursor);	
	SDL_FreeCursor(lastcursor);

	return self;
}

void initMouseClasses()
{
	classMouse=rb_define_class_under(moduleRUDL, "Mouse", rb_cObject);
	//rb_define_singleton_method(classMouse, "cursor", mouse_cursor, 0);
	rb_define_singleton_and_instance_method(classMouse, "set_cursor", mouse_set_cursor, 3);
	rb_define_singleton_and_instance_method(classMouse, "focused?", mouse_focused_, 0);
	rb_define_singleton_and_instance_method(classMouse, "pressed?", mouse_pressed_, 0);
	rb_define_singleton_and_instance_method(classMouse, "rel", mouse_rel, 0);
	rb_define_singleton_and_instance_method(classMouse, "pos", mouse_pos, 0);
	rb_define_singleton_and_instance_method(classMouse, "pos=", mouse_set_pos, 1);
	rb_define_singleton_and_instance_method(classMouse, "visible=", mouse_set_visible, 1);
/**
@class MouseMotionEvent
@method pos
An array [x, y] telling the position of the mouse.
*/
/**
@method rel
An array [dx, dy] telling how much the mouse moved.
*/
/**
@method button
An array of booleans, representing the states of the mousebuttons.
Currently, three buttons are supported.
*/
	classMouseMotionEvent=rb_define_class_under(moduleRUDL, "MouseMotionEvent", classEvent);
	rb_define_attr(classMouseMotionEvent, "pos", 1, 1);
	rb_define_attr(classMouseMotionEvent, "rel", 1, 1);
	rb_define_attr(classMouseMotionEvent, "button", 1, 1);

/**
@class MouseButtonUpEvent
@method pos
An array [x, y] telling the position of the mouse.
*/
/**
@method button
The number of the button that was released.
*/
	classMouseButtonUpEvent=rb_define_class_under(moduleRUDL, "MouseButtonUpEvent", classEvent);
	rb_define_attr(classMouseButtonUpEvent, "pos", 1, 1);
	rb_define_attr(classMouseButtonUpEvent, "button", 1, 1);

/**
@class MouseButtonDownEvent
@method pos
An array [x, y] telling the position of the mouse.
*/
/**
@method button
The number of the button that was pressed.
*/
	classMouseButtonDownEvent=rb_define_class_under(moduleRUDL, "MouseButtonDownEvent", classEvent);
	rb_define_attr(classMouseButtonDownEvent, "pos", 1, 1);
	rb_define_attr(classMouseButtonDownEvent, "button", 1, 1);

	rb_eval_string(
			"module RUDL class Mouse					\n"
			"	def inspect								\n"
			"		p=pos								\n"
			"		\"<Mouse: #{p.x},#{p.y} >\"			\n"
			"	end										\n"
			"end end									\n"
	);
}
