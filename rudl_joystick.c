/* RUDL - a C library wrapping SDL for use in Ruby. Copyright (C) 2001  Danny van Bruggen */
#include "rudl_joystick.h"
#include "rudl_events.h"
#include "rudl_video.h"

void initJoystick()
{
	if(!SDL_WasInit(SDL_INIT_VIDEO)){
		initVideo();
	}

	if(!SDL_WasInit(SDL_INIT_JOYSTICK)){
		DEBUG_S("Starting joystick subsystem");
		SDL_VERIFY(SDL_Init(SDL_INIT_JOYSTICK)!=-1);
	}
}

void quitJoystick()
{
	if(SDL_WasInit(SDL_INIT_JOYSTICK)){
		DEBUG_S("Stopping joystick subsystem");
		rb_eval_string("ObjectSpace.each_object(RUDL::Joystick) {|x| x.close_hack}");
		SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
	}
}
/*
=begin
<<< docs/head
= Joystick
== Class Methods
--- Joystick.new( id )
Creates a new joystick object.
((|id|)) is a number smaller than ((<Joystick.count>)).
This will also start events for this joystick.
=end */
static VALUE joystick_new(VALUE self, VALUE id)
{
	SDL_Joystick* joystick;
	
	initJoystick();

	joystick=SDL_JoystickOpen(NUM2INT(id));
	if(joystick){
		return Data_Wrap_Struct(classJoystick, 0, 0, joystick);
	}
	SDL_RAISE;
	return Qnil;
}

SDL_Joystick* retrieveJoystickPointer(VALUE self)
{
	SDL_Joystick* joystick;
	Data_Get_Struct(self, SDL_Joystick, joystick);
	return joystick;
}

/*
=begin
--- Joystick.count
Returns the amount of joysticks attached to the computer.
=end */

static VALUE joystick_count(VALUE self)
{
	initJoystick();
	return INT2NUM(SDL_NumJoysticks());
}

/*
=begin
== Instance Methods
--- Joystick#id
Returns the id of the joystick as it was passed to ((<Joystick.new>)).
=end */
static VALUE joystick_id(VALUE self)
{
	return INT2NUM(SDL_JoystickIndex(retrieveJoystickPointer(self)));
}

/*
=begin
--- Joystick#axes
Returns the amount of axes the joystick has.
=end */
static VALUE joystick_axes(VALUE self)
{
	return INT2NUM(SDL_JoystickNumAxes(retrieveJoystickPointer(self)));
}

/*
=begin
--- Joystick#balls
Returns the amount of trackballs the joystick has.
=end */
static VALUE joystick_balls(VALUE self)
{
	return INT2NUM(SDL_JoystickNumBalls(retrieveJoystickPointer(self)));
}

/*
=begin
--- Joystick#hats
Returns the amount of hats the joystick has.
=end */
static VALUE joystick_hats(VALUE self)
{
	return INT2NUM(SDL_JoystickNumHats(retrieveJoystickPointer(self)));
}

/*
=begin
--- Joystick#buttons
Returns the amount of buttons the joystick has.
=end */
static VALUE joystick_buttons(VALUE self)
{
	return INT2NUM(SDL_JoystickNumButtons(retrieveJoystickPointer(self)));
}

/*
=begin
--- Joystick#axis( nr )
Returns the state of axis ((|nr|)), which is between -1 to 1.
=end */
static VALUE joystick_axis(VALUE self, VALUE nr)
{
	return DBL2NUM(SDL_JoystickGetAxis(retrieveJoystickPointer(self), NUM2INT(nr))/32768.0);
}

/*
=begin
--- Joystick#ball( nr )
Returns the state of ball ((|nr|)), which is an array of [dx, dy] where dx and dy are between -1 to 1.
=end */
static VALUE joystick_ball(VALUE self, VALUE nr)
{
	int dx, dy;
	if(SDL_JoystickGetBall(retrieveJoystickPointer(self), NUM2INT(nr), &dx, &dy)==-1)SDL_RAISE;
	return rb_ary_new3(2, DBL2NUM(dx/32768.0), DBL2NUM(dy/32768.0));
}

/*
=begin
--- Joystick#hat( nr )
Returns the state of hat ((|nr|)), which is an array of [dx, dy] where dx and dy can be -1, 0 or 1.
=end */
static VALUE joystick_hat(VALUE self, VALUE nr)
{
	int hx=0;
	int hy=0;
	Uint8 value=SDL_JoystickGetHat(retrieveJoystickPointer(self), NUM2INT(nr));

	if(value&SDL_HAT_UP) hy = 1;
	else if(value&SDL_HAT_DOWN) hy = -1;
	if(value&SDL_HAT_LEFT) hx = 1;
	else if(value&SDL_HAT_LEFT) hx = -1;

	return rb_ary_new3(2, INT2NUM(hx), INT2NUM(hy));
}
/*
=begin
--- Joystick#button( nr )
Returns the boolean state of button ((|nr|)).
=end */
static VALUE joystick_button(VALUE self, VALUE nr)
{
	return INT2BOOL(SDL_JoystickGetButton(retrieveJoystickPointer(self), NUM2INT(nr)));
}

static VALUE joystick_close_hack(VALUE self)
{
	SDL_JoystickClose(retrieveJoystickPointer(self));
	return Qnil;
}

void initJoystickClasses()
{
	classJoystick=rb_define_class_under(moduleRUDL, "Joystick", rb_cObject);
	rb_define_singleton_method(classJoystick, "new", joystick_new, 1);
	rb_define_singleton_method(classJoystick, "count", joystick_count, 0);
	rb_define_method(classJoystick, "id", joystick_id, 0);
	rb_define_method(classJoystick, "axes", joystick_axes, 0);
	rb_define_method(classJoystick, "balls", joystick_balls, 0);
	rb_define_method(classJoystick, "hats", joystick_hats, 0);
	rb_define_method(classJoystick, "buttons", joystick_buttons, 0);
	rb_define_method(classJoystick, "axis", joystick_axis, 1);
	rb_define_method(classJoystick, "ball", joystick_ball, 1);
	rb_define_method(classJoystick, "hat", joystick_hat, 1);
	rb_define_method(classJoystick, "button", joystick_button, 1);
	rb_define_method(classJoystick, "close_hack", joystick_close_hack, 0);

	rb_eval_string(
		"module RUDL class Joystick				\n"
		"	def to_s							\n"
		"		\"Joystick #{id}, #{axes} axes, #{balls} balls, #{buttons} buttons, #{hats} hats\"	\n"
		"	end									\n"
		"end end								\n"
	);
	
/*
=begin
= JoyAxisEvent
Contains ((|id|)) which is the joysticknumber,
((|value|)) which is the movement, ranging from -1 to 1 and
((|axis|)) which is the axis index.
= JoyBallEvent
Contains ((|id|)) which is the joysticknumber,
((|ball|)) which is a trackball index and
((|rel|)) which is a movement array of [dx, dy].
= JoyHatEvent
Contains ((|id|)) which is the joysticknumber,
((|hat|)) which is the hatnumber and
a movement array of [dx, dy] called ((|value|)) where dx and dy can be -1, 0 or 1.
= JoyButtonUpEvent
Contains ((|id|)) which is the joysticknumber and
((|button|)) which is the button index.
= JoyButtonDownEvent
Contains ((|id|)) which is the joysticknumber and
((|button|)) which is the button index.
=end */
	classJoyAxisEvent=rb_define_class_under(moduleRUDL, "JoyAxisEvent", classEvent);
	rb_define_attr(classJoyAxisEvent, "id", 1, 1);
	rb_define_attr(classJoyAxisEvent, "value", 1, 1);
	rb_define_attr(classJoyAxisEvent, "axis", 1, 1);
	classJoyBallEvent=rb_define_class_under(moduleRUDL, "JoyBallEvent", classEvent);
	rb_define_attr(classJoyBallEvent, "id", 1, 1);
	rb_define_attr(classJoyBallEvent, "ball", 1, 1);
	rb_define_attr(classJoyBallEvent, "rel", 1, 1);
	classJoyHatEvent=rb_define_class_under(moduleRUDL, "JoyHatEvent", classEvent);
	rb_define_attr(classJoyHatEvent, "id", 1, 1);
	rb_define_attr(classJoyHatEvent, "hat", 1, 1);
	rb_define_attr(classJoyHatEvent, "value", 1, 1);
	classJoyButtonUpEvent=rb_define_class_under(moduleRUDL, "JoyButtonUpEvent", classEvent);
	rb_define_attr(classJoyButtonUpEvent, "id", 1, 1);
	rb_define_attr(classJoyButtonUpEvent, "button", 1, 1);
	classJoyButtonDownEvent=rb_define_class_under(moduleRUDL, "JoyButtonDownEvent", classEvent);
	rb_define_attr(classJoyButtonDownEvent, "id", 1, 1);
	rb_define_attr(classJoyButtonDownEvent, "button", 1, 1);
	
	/*DEC_CONST(HAT_CENTERED);
	DEC_CONST(HAT_UP);
	DEC_CONST(HAT_RIGHTUP);
	DEC_CONST(HAT_RIGHT);
	DEC_CONST(HAT_RIGHTDOWN);
	DEC_CONST(HAT_DOWN);
	DEC_CONST(HAT_LEFTDOWN);
	DEC_CONST(HAT_LEFT);
	DEC_CONST(HAT_LEFTUP);*/
}
