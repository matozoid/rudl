/* RUDL - a C library wrapping SDL for use in Ruby. Copyright (C) 2001  Danny van Bruggen */
#include "rudl_timer.h"
#include "rudl_video.h"
#include "rudl_events.h"

void initTimer()
{
	if(!SDL_WasInit(SDL_INIT_TIMER)){

		DEBUG_S("Starting timer subsystem");
		SDL_VERIFY(SDL_Init(SDL_INIT_TIMER)!=-1);
	}
}
///////////////////////////////// TIME
/**
@file timer
@class Timer
@section Class Methods
@method delay( milliseconds ) -> nil
Will do nothing for @milliseconds milliseconds.
It is not guaranteed to be exact and has different resolution on different platforms.
Expect a resolution of 10 to 20 milliseconds at worst.
*/
static VALUE timer_delay(VALUE obj, VALUE delay)
{
	initTimer();
	
	SDL_Delay(NUM2INT(delay));
	return Qnil;
}

/**
@method ticks -> Number
Returns the time in milliseconds since RUDL was required.
*/
static VALUE timer_getTicks(VALUE obj)
{
	initTimer();

	return INT2NUM(SDL_GetTicks());
}

///////////////////////////////// EVENTTIMER
void freeEventTimer(SDL_TimerID freeMe)
{
	SDL_RemoveTimer(freeMe);
}

Uint32 timerCallback(Uint32 interval, void *param)
{
	SDL_Event user_event;
	
	user_event.type=RUDL_TIMEREVENT;
	user_event.user.code=(int)param;
	user_event.user.data1=NULL;
	user_event.user.data2=NULL;
	SDL_PushEvent(&user_event);

	return interval;
}

/**
@class EventTimer
A class that controls delivering @TimerEvent s on a regular basis.
*/
/**
@section Class Methods
@method new( interval, id ) -> EventTimer
Will create a EventTimer object that controls sending @TimerEvent s every @interval
milliseconds, having their id-field set to @id - which is a number.
*/
static VALUE eventTimer_new(VALUE obj, VALUE interval, VALUE id)
{
	SDL_TimerID timerID;

	initTimer();
	
	timerID=SDL_AddTimer(NUM2INT(interval), timerCallback, (void*)(NUM2INT(id)));

	SDL_VERIFY(timerID);
	return Data_Wrap_Struct(classEventTimer, 0, freeEventTimer, timerID);
}

/**
@section Instance Methods
@method stop -> boolean
Will stop this EventTimer from posting more @TimerEvent s.
*/
static VALUE eventTimer_stop(VALUE obj)
{
	SDL_TimerID timerID;

	// Data_Get_Struct hack because of unavailability of struct _SDL_TimerID
	Check_Type(obj, T_DATA);
	timerID=(SDL_TimerID)DATA_PTR(obj);

	return INT2BOOL(SDL_RemoveTimer(timerID));
}

void initTimerClasses()
{
	classTimer=rb_define_module_under(moduleRUDL, "Timer");
	rb_define_singleton_method(classTimer, "delay", timer_delay, 1);
	rb_define_singleton_method(classTimer, "ticks", timer_getTicks, 0);

	classEventTimer=rb_define_class_under(moduleRUDL, "EventTimer", rb_cObject);
	rb_define_singleton_method(classEventTimer, "new", eventTimer_new, 2);
	rb_define_method(classEventTimer, "stop", eventTimer_stop, 0);
/**
@class TimerEvent
This event is posted regularly by an @EventTimer object.
*/
/**
@method id -> Number
This is the EventTimer's @id.
*/
	classTimerEvent=rb_define_class_under(moduleRUDL, "TimerEvent", classEvent);
	rb_define_attr(classTimerEvent, "id", 1, 1);
}
