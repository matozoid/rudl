/* RUDL - a C library wrapping SDL for use in Ruby. Copyright (C) 2001  Danny van Bruggen */
#include "rudl_mouse.h"
#include "rudl_video.h"
#include "rudl_events.h"

/*
=begin
<<< docs/head
= Mouse
The mouse class methods can also be used as instance methods once you
instantiate the class. However, there is no need to do that, it's just
for convenience.
== Class and instance Methods
--- Mouse.focused?
--- Mouse#focused?
Returns true when the application is receiving the mouse input focus.
=end */
static VALUE mouse_focused_(VALUE self)
{
	initVideo();
	return INT2BOOL((SDL_GetAppState()&SDL_APPMOUSEFOCUS) != 0);
}
/*
=begin
--- Mouse.pos
--- Mouse#pos
Returns the current position of the mouse cursor.
This is the absolute mouse position inside your game window.
=end */
static VALUE mouse_pos(VALUE self)
{
	int x, y;

	initVideo();

	SDL_GetMouseState(&x, &y);
	return rb_ary_new3(2, INT2NUM(x), INT2NUM(y));
}
/*
=begin
--- Mouse.pressed?
--- Mouse#pressed?
This will return an array containing the pressed state of each mouse button.
=end */
static VALUE mouse_pressed_(VALUE self)
{
	Uint32 state = SDL_GetMouseState(NULL, NULL);

	return rb_ary_new3(3,
		INT2BOOL((state&SDL_BUTTON(1)) != 0),
		INT2BOOL((state&SDL_BUTTON(2)) != 0),
		INT2BOOL((state&SDL_BUTTON(3)) != 0));
}
/*
=begin
--- Mouse.rel
--- Mouse#rel
Returns the total distance the mouse has moved since your last call to ((<Mouse.rel>)).
On the first call to get_rel the movement will always be 0,0.
When the mouse is at the edges of the screen, the relative movement will be stopped.
See ((<Mouse.visible>)) for a way to resolve this.
=end */
static VALUE mouse_rel(VALUE self)
{
	int x, y;

	initVideo();

	SDL_GetRelativeMouseState(&x, &y);
	return rb_ary_new3(2, INT2NUM(x), INT2NUM(y));
}
/*
=begin
--- Mouse.pos=( pos )
--- Mouse#pos=( pos )
Moves the mouse cursor to the specified position.
This will generate a MouseMotionEvent on the input queue.
=end */
static VALUE mouse_set_pos(VALUE self, VALUE pos)
{
	Sint16 x,y;
	initVideo();
	PARAMETER2COORD(pos, &x, &y);
	SDL_WarpMouse(x, y);
	return self;
}
/*
=begin
--- Mouse.visible=( onOrOff )
--- Mouse#visible=( onOrOff )
Shows or hides the mouse cursor.
This will return the previous visible state of the mouse cursor.

Note that when the cursor is hidden and the application has grabbed the input.
SDL will force the mouse to stay in the center of the screen.
Since the mouse is hidden it won't matter that it's not moving,
but it will keep the mouse from the edges of the screen so the relative mouse 
position will always be true.
=end */
static VALUE mouse_set_visible(VALUE self, VALUE yes)
{
	initVideo();
	return INT2BOOL(SDL_ShowCursor(NUM2BOOL(yes)));
}

/*
=begin
--- Mouse.set_cursor( hotspot, xormasks, andmasks )
--- Mouse#get_cursor( hotspot, xormasks, andmasks )
When the mouse cursor is visible, it will be displayed as a black and white bitmap 
using the given bitmask arrays.
Hotspot is an array containing the cursor hotspot position.
xormasks is an array of arrays of bytes containing the cursor xor data masks.
Lastly is andmasks, an array of arrays of bytes containting the cursor bitmask data.

The example "mousecursor.rb" explains this much better.
=end */
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
	classMouse=rb_define_module_under(moduleRUDL, "Mouse");
	//rb_define_singleton_method(classMouse, "cursor", mouse_cursor, 0);
	rb_define_singleton_and_instance_method(classMouse, "set_cursor", mouse_set_cursor, 3);
	rb_define_singleton_and_instance_method(classMouse, "focused?", mouse_focused_, 0);
	rb_define_singleton_and_instance_method(classMouse, "pressed?", mouse_pressed_, 0);
	rb_define_singleton_and_instance_method(classMouse, "rel", mouse_rel, 0);
	rb_define_singleton_and_instance_method(classMouse, "pos", mouse_pos, 0);
	rb_define_singleton_and_instance_method(classMouse, "pos=", mouse_set_pos, 1);
	rb_define_singleton_and_instance_method(classMouse, "visible=", mouse_set_visible, 1);
/*
=begin
= MouseMotionEvent
--- MouseMotionEvent.pos
An array [x, y] telling the position of the mouse.
--- MouseMotionEvent.rel
An array [dx, dy] telling how much the mouse moved.
--- MouseMotionEvent.button
An array of booleans, representing the states of the mousebuttons.
Currently, three buttons are supported.
=end */
	classMouseMotionEvent=rb_define_class_under(moduleRUDL, "MouseMotionEvent", classEvent);
	rb_define_attr(classMouseMotionEvent, "pos", 1, 1);
	rb_define_attr(classMouseMotionEvent, "rel", 1, 1);
	rb_define_attr(classMouseMotionEvent, "button", 1, 1);

/*
=begin
= MouseButtonUpEvent
--- MouseButtonUpEvent.pos
An array [x, y] telling the position of the mouse.
--- MouseButtonUpEvent.button
The number of the button that was released.
=end */
	classMouseButtonUpEvent=rb_define_class_under(moduleRUDL, "MouseButtonUpEvent", classEvent);
	rb_define_attr(classMouseButtonUpEvent, "pos", 1, 1);
	rb_define_attr(classMouseButtonUpEvent, "button", 1, 1);

/*
=begin
= MouseButtonDownEvent
--- MouseButtonDownEvent.pos
An array [x, y] telling the position of the mouse.
--- MouseButtonDownEvent.button
The number of the button that was pressed.
=end */
	classMouseButtonDownEvent=rb_define_class_under(moduleRUDL, "MouseButtonDownEvent", classEvent);
	rb_define_attr(classMouseButtonDownEvent, "pos", 1, 1);
	rb_define_attr(classMouseButtonDownEvent, "button", 1, 1);
}
