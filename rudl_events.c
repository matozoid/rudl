/* RUDL - a C library wrapping SDL for use in Ruby. Copyright (C) 2001  Danny van Bruggen */
#include "rudl_events.h"
#include "rudl_audio.h"
#include "rudl_video.h"
#include "rudl_joystick.h"
#include "rudl_keyboard.h"
#include "rudl_mouse.h"
#include "rudl_timer.h"

VALUE sDLEvent2RubyEvent(SDL_Event* event)
{
	VALUE newEvent=0;

	int hx,hy;

	switch(event->type){
		case SDL_ACTIVEEVENT:
			newEvent=rb_obj_alloc(classActiveEvent);
			rb_iv_set(newEvent, "@gain", UINT2NUM(event->active.gain));
			rb_iv_set(newEvent, "@state", UINT2NUM(event->active.state));
			break;
		case SDL_KEYDOWN:
			newEvent=rb_obj_alloc(classKeyDownEvent);
			rb_iv_set(newEvent, "@key", UINT2NUM(event->key.keysym.sym));
			rb_iv_set(newEvent, "@mod", UINT2NUM(event->key.keysym.mod));
			break;
		case SDL_KEYUP:
			newEvent=rb_obj_alloc(classKeyUpEvent);
			rb_iv_set(newEvent, "@key", UINT2NUM(event->key.keysym.sym));
			rb_iv_set(newEvent, "@mod", UINT2NUM(event->key.keysym.mod));
			break;
		case SDL_QUIT:
			newEvent=rb_obj_alloc(classQuitEvent);
			break;
		case SDL_MOUSEMOTION:
			newEvent=rb_obj_alloc(classMouseMotionEvent);
			rb_iv_set(newEvent, "@pos", rb_ary_new3(2, INT2NUM(event->motion.x), INT2NUM(event->motion.y)));
			rb_iv_set(newEvent, "@rel", rb_ary_new3(2, INT2NUM(event->motion.xrel), INT2NUM(event->motion.yrel)));
			rb_iv_set(newEvent, "@button", rb_ary_new3(3,
				INT2BOOL(event->motion.state&SDL_BUTTON(1)),
				INT2BOOL(event->motion.state&SDL_BUTTON(2)),
				INT2BOOL(event->motion.state&SDL_BUTTON(3))));
			break;
		case SDL_MOUSEBUTTONDOWN:
			newEvent=rb_obj_alloc(classMouseButtonDownEvent);
			rb_iv_set(newEvent, "@pos", rb_ary_new3(2, 
				INT2NUM(event->button.x), 
				INT2NUM(event->button.y)));
			rb_iv_set(newEvent, "@button", UINT2NUM(event->button.button));
			break;
		case SDL_MOUSEBUTTONUP:
			newEvent=rb_obj_alloc(classMouseButtonUpEvent);
			rb_iv_set(newEvent, "@pos", rb_ary_new3(2, 
				INT2NUM(event->button.x), 
				INT2NUM(event->button.y)));
			rb_iv_set(newEvent, "@button", UINT2NUM(event->button.button));
			break;
		case SDL_JOYAXISMOTION:
			newEvent=rb_obj_alloc(classJoyAxisEvent);
			rb_iv_set(newEvent, "@id", INT2NUM(event->jaxis.which));
			rb_iv_set(newEvent, "@value", DBL2NUM(event->jaxis.value/32767.0));
			rb_iv_set(newEvent, "@axis", INT2NUM(event->jaxis.axis));
			break;
		case SDL_JOYBALLMOTION:
			newEvent=rb_obj_alloc(classJoyBallEvent);
			rb_iv_set(newEvent, "@id", INT2NUM(event->jball.which));
			rb_iv_set(newEvent, "@ball", INT2NUM(event->jball.ball));
			rb_iv_set(newEvent, "@rel", rb_ary_new3(2,
				INT2NUM(event->jball.xrel),
				INT2NUM(event->jball.yrel)));
			break;
		case SDL_JOYHATMOTION:
			newEvent=rb_obj_alloc(classJoyHatEvent);
			rb_iv_set(newEvent, "@id", INT2NUM(event->jhat.which));
			rb_iv_set(newEvent, "@hat", INT2NUM(event->jhat.hat));
			hx = hy = 0;
			if(event->jhat.value&SDL_HAT_UP) hy = 1;
			else if(event->jhat.value&SDL_HAT_DOWN) hy = -1;
			if(event->jhat.value&SDL_HAT_LEFT) hx = 1;
			else if(event->jhat.value&SDL_HAT_LEFT) hx = -1;
			rb_iv_set(newEvent, "@value", rb_ary_new3(2, INT2NUM(hx), INT2NUM(hy)));
			break;
		case SDL_JOYBUTTONUP:
			newEvent=rb_obj_alloc(classJoyButtonUpEvent);
			rb_iv_set(newEvent, "@id", INT2NUM(event->jbutton.which));
			rb_iv_set(newEvent, "@button", INT2NUM(event->jbutton.button));
			break;
		case SDL_JOYBUTTONDOWN:
			newEvent=rb_obj_alloc(classJoyButtonDownEvent);
			rb_iv_set(newEvent, "@id", INT2NUM(event->jbutton.which));
			rb_iv_set(newEvent, "@button", INT2NUM(event->jbutton.button));
			break;
		case SDL_VIDEORESIZE:
			newEvent=rb_obj_alloc(classResizeEvent);
			rb_iv_set(newEvent, "@size", rb_ary_new3(2, 
				UINT2NUM(event->resize.w), 
				UINT2NUM(event->resize.h)));
			break;
		case RUDL_TIMEREVENT:
			newEvent=rb_obj_alloc(classTimerEvent);
			rb_iv_set(newEvent, "@id", INT2NUM(event->user.code));
			break;
		case RUDL_ENDMUSICEVENT:
			newEvent=rb_obj_alloc(classEndOfMusicEvent);
			break;
/*		else 
			if(event->type > USEREVENT && event->type < NUMEVENTS){
				newEvent=rb_obj_alloc(classEvent);
				rb_iv_set(newEvent, "@code", INT2NUM(event->user.code));
				rb_iv_set(newEvent, "@data1", INT2NUM(event->user.data1));
				rb_iv_set(newEvent, "@data2", INT2NUM(event->user.data2));
			}*/
	}

	if(newEvent==0){
		return Qnil;
	}
	return newEvent;
}

/*
=begin
<<< docs/head
= EventQueue
This class is the interface to the eventsystem in SDL.
Don't be put off by the amount of non-implemented methods, their absence doesn't bother
me and I don't plan on implementing them before someone comes up with a good reason for
their existence.
== Class Methods
--- EventQueue.get
Returns all events in the queue.
--- EventQueue.get( eventmask )
Not implemented.
=end
*/
static VALUE eventqueue_get(int argc, VALUE* argv, VALUE self)
{
	SDL_Event event;
	int mask = 0;
	VALUE retval;

	if(argc==0){
		mask = SDL_ALLEVENTS;
	}else{
		rb_notimplement();
	}
	
	retval=rb_ary_new();

	SDL_PumpEvents();

	while(SDL_PeepEvents(&event, 1, SDL_GETEVENT, mask)==1){
		rb_ary_push(retval, sDLEvent2RubyEvent(&event));
	}
	return retval;
}

/*
=begin
--- EventQueue.peek
Returns the next event without removing it from the queue,
or false when no events are available.
--- EventQueue.peek( eventmask )
Not implemented.
=end */
static VALUE eventqueue_peek(int argc, VALUE* argv, VALUE self)
{
	SDL_Event event;
	int mask = 0;

	if(argc==0){
		mask = SDL_ALLEVENTS;
	}else{
		rb_notimplement();
	}
	
	SDL_PumpEvents();

	if(SDL_PeepEvents(&event, 1, SDL_PEEKEVENT, mask)==1){
		return sDLEvent2RubyEvent(&event);
	}else{
		return Qfalse;
	}
}

/*
=begin
--- EventQueue.poll
Returns the next event, or nil if the queue is empty.
=end */
static VALUE eventqueue_poll(VALUE self)
{
	SDL_Event event;
	if(SDL_PollEvent(&event)){
		return sDLEvent2RubyEvent(&event);
	}
	return Qnil;
}

/*
=begin
--- EventQueue.post( event )
Not implemented.
=end */
static VALUE eventqueue_post(VALUE self, VALUE event)
{
	rb_notimplement();
	return Qnil;
}

/*
=begin
--- EventQueue.pump
This method is responsible for getting events from the operating system into the
SDL eventqueue.
If your application seems unresponsive, calling this method every now and then might help.
=end */
static VALUE eventqueue_pump(VALUE self)
{
	SDL_PumpEvents();
	return self;
}

/*
=begin
--- EventQueue.allowed=( eventtype )
Not implemented.
=end */
static VALUE eventqueue_allowed_(VALUE self, VALUE eventType)
{
	rb_notimplement();
	return Qnil;
}

/*
=begin
--- EventQueue.blocked=( eventtype )
Not implemented.
=end */
static VALUE eventqueue_blocked_(VALUE self, VALUE eventType)
{
	rb_notimplement();
	return Qnil;
}

/*
=begin
--- EventQueue.grab
--- EventQueue.grab=( grab )
Controls grabbing of all mouse and keyboard input for the display.
Grabbing the input is not neccessary to receive keyboard and mouse events, 
but it ensures all input will go to your application.
It also keeps the mouse locked inside your window.
It is best to not always grab the input, 
since it prevents the end user from doing anything else on their system.
((|grab|)) is true or false.
=end */
static VALUE eventqueue_grab_(VALUE self, VALUE grabOn)
{
	if(NUM2BOOL(grabOn)){
		SDL_WM_GrabInput(SDL_GRAB_ON);
	}else{
		SDL_WM_GrabInput(SDL_GRAB_OFF);
	}
	return self;
}

static VALUE eventqueue_grab(VALUE self)
{
	return INT2BOOL(SDL_WM_GrabInput(SDL_GRAB_QUERY)==SDL_GRAB_ON);
}

/*
=begin
--- EventQueue.wait
Wait for an event to arrive.
Returns that event.
=end */
static VALUE eventqueue_wait(VALUE self)
{
	SDL_Event event;
	if(!SDL_WaitEvent(&event)){
		SDL_RAISE
	}
	return sDLEvent2RubyEvent(&event);
}

void initEventsClasses()
{
	classEventQueue=rb_define_module_under(moduleRUDL, "EventQueue");
	rb_define_singleton_method(classEventQueue, "get", eventqueue_get, -1);
	rb_define_singleton_method(classEventQueue, "peek", eventqueue_peek, -1);
	rb_define_singleton_method(classEventQueue, "poll", eventqueue_poll, 0);
	rb_define_singleton_method(classEventQueue, "post", eventqueue_post, 1);
	rb_define_singleton_method(classEventQueue, "pump", eventqueue_pump, 0);
	rb_define_singleton_method(classEventQueue, "allowed=", eventqueue_allowed_, 1);
	rb_define_singleton_method(classEventQueue, "blocked=", eventqueue_blocked_, 1);
	rb_define_singleton_method(classEventQueue, "grab=", eventqueue_grab_, 1);
	rb_define_singleton_method(classEventQueue, "grab", eventqueue_grab, 0);
	rb_define_singleton_method(classEventQueue, "wait", eventqueue_wait, 0);
/*
=begin
--- EventQueue.flush
Flushes all events from the queue.
=end */
	rb_eval_string(
		"module RUDL module EventQueue				\n"
		"	def EventQueue.flush					\n"
		"		true while EventQueue.poll			\n"
		"	end										\n"
		"end end									\n");


/*
=begin
= Event
This is the baseclass for all event classes.
It contains nothing.
=end */
	classEvent=rb_define_class_under(moduleRUDL, "Event", rb_cObject);
	
	//classEvent=rb_define_class_under(moduleRUDL, "UserEvent", classEvent);
	//rb_define_attr(classEvent, "code", 1, 1);
	//rb_define_attr(classEvent, "data1", 1, 1);
	//rb_define_attr(classEvent, "data2", 1, 1);
	//classSysWMEvent=rb_define_class_under(moduleRUDL, "SysWMEvent", classEvent);

}
