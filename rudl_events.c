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
    VALUE newEvent=Qnil;

    int hx,hy;

    switch(event->type){
        case SDL_ACTIVEEVENT:
            newEvent=rb_funcall(classActiveEvent, id_new, 0);
            rb_iv_set(newEvent, "@gain", UINT2NUM(event->active.gain));
            rb_iv_set(newEvent, "@state", UINT2NUM(event->active.state));
            break;
        case SDL_KEYDOWN:
            newEvent=rb_funcall(classKeyDownEvent, id_new, 0);
            rb_iv_set(newEvent, "@key", UINT2NUM(event->key.keysym.sym));
            rb_iv_set(newEvent, "@mod", UINT2NUM(event->key.keysym.mod));
            rb_iv_set(newEvent, "@unicode", UINT2NUM(event->key.keysym.unicode));

            break;
        case SDL_KEYUP:
            newEvent=rb_funcall(classKeyUpEvent, id_new, 0);
            rb_iv_set(newEvent, "@key", UINT2NUM(event->key.keysym.sym));
            rb_iv_set(newEvent, "@mod", UINT2NUM(event->key.keysym.mod));
            rb_iv_set(newEvent, "@unicode", UINT2NUM(event->key.keysym.unicode));

            break;
        case SDL_QUIT:
            newEvent=rb_funcall(classQuitEvent, id_new, 0);
            break;
        case SDL_MOUSEMOTION:
            newEvent=rb_funcall(classMouseMotionEvent, id_new, 0);
            rb_iv_set(newEvent, "@pos", rb_ary_new3(2, INT2NUM(event->motion.x), INT2NUM(event->motion.y)));
            rb_iv_set(newEvent, "@rel", rb_ary_new3(2, INT2NUM(event->motion.xrel), INT2NUM(event->motion.yrel)));
            rb_iv_set(newEvent, "@button", rb_ary_new3(3,
                INT2BOOL(event->motion.state&SDL_BUTTON(1)),
                INT2BOOL(event->motion.state&SDL_BUTTON(2)),
                INT2BOOL(event->motion.state&SDL_BUTTON(3))));
            break;
        case SDL_MOUSEBUTTONDOWN:
            newEvent=rb_funcall(classMouseButtonDownEvent, id_new, 0);
            rb_iv_set(newEvent, "@pos", rb_ary_new3(2,
                INT2NUM(event->button.x),
                INT2NUM(event->button.y)));
            rb_iv_set(newEvent, "@button", UINT2NUM(event->button.button));
            break;
        case SDL_MOUSEBUTTONUP:
            newEvent=rb_funcall(classMouseButtonUpEvent, id_new, 0);
            rb_iv_set(newEvent, "@pos", rb_ary_new3(2,
                INT2NUM(event->button.x),
                INT2NUM(event->button.y)));
            rb_iv_set(newEvent, "@button", UINT2NUM(event->button.button));
            break;
        case SDL_JOYAXISMOTION:
            newEvent=rb_funcall(classJoyAxisEvent, id_new, 0);
            rb_iv_set(newEvent, "@id", INT2NUM(event->jaxis.which));
            rb_iv_set(newEvent, "@value", DBL2NUM(event->jaxis.value/32767.0));
            rb_iv_set(newEvent, "@axis", INT2NUM(event->jaxis.axis));
            break;
        case SDL_JOYBALLMOTION:
            newEvent=rb_funcall(classJoyBallEvent, id_new, 0);
            rb_iv_set(newEvent, "@id", INT2NUM(event->jball.which));
            rb_iv_set(newEvent, "@ball", INT2NUM(event->jball.ball));
            rb_iv_set(newEvent, "@rel", rb_ary_new3(2,
                INT2NUM(event->jball.xrel),
                INT2NUM(event->jball.yrel)));
            break;
        case SDL_JOYHATMOTION:
            newEvent=rb_funcall(classJoyHatEvent, id_new, 0);
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
            newEvent=rb_funcall(classJoyButtonUpEvent, id_new, 0);
            rb_iv_set(newEvent, "@id", INT2NUM(event->jbutton.which));
            rb_iv_set(newEvent, "@button", INT2NUM(event->jbutton.button));
            break;
        case SDL_JOYBUTTONDOWN:
            newEvent=rb_funcall(classJoyButtonDownEvent, id_new, 0);
            rb_iv_set(newEvent, "@id", INT2NUM(event->jbutton.which));
            rb_iv_set(newEvent, "@button", INT2NUM(event->jbutton.button));
            break;
        case SDL_VIDEORESIZE:
            {
                /* automatically call DisplaySurface.new to fix the clipping */
                VALUE newDS, oldDS = currentDisplaySurface;

                newDS = rb_funcall(classDisplaySurface, id_new, currDSnumargs,
                    rb_ary_new3(2, UINT2NUM(event->resize.w), UINT2NUM(event->resize.h)),
                    currDSflags, currDSdepth);

                /* keep the old object */
                currentDisplaySurface = oldDS;
                /* replace its SDL_Surface pointer with the new one */
                DATA_PTR(oldDS) = DATA_PTR(newDS);

                newEvent=rb_funcall(classResizeEvent, id_new, 0);
                rb_iv_set(newEvent, "@size", rb_ary_new3(2,
                    UINT2NUM(event->resize.w),
                    UINT2NUM(event->resize.h)));
                break;
            }
        case SDL_VIDEOEXPOSE:
            newEvent=rb_funcall(classVideoExposeEvent, id_new, 0);
            break;
        case RUDL_TIMEREVENT:
            newEvent=rb_funcall(classTimerEvent, id_new, 0);
            rb_iv_set(newEvent, "@id", INT2NUM(event->user.code));
            break;
        case RUDL_ENDMUSICEVENT:
            newEvent=rb_funcall(classEndOfMusicEvent, id_new, 0);
            break;
/*      else
            if(event->type > USEREVENT && event->type < NUMEVENTS){
                newEvent=rb_funcall(classEvent, id_new, 0);
                rb_iv_set(newEvent, "@code", INT2NUM(event->user.code));
                rb_iv_set(newEvent, "@data1", INT2NUM(event->user.data1));
                rb_iv_set(newEvent, "@data2", INT2NUM(event->user.data2));
            }*/
    }

    RUDL_ASSERT(newEvent!=Qnil, "Unknown event received from SDL (SDL too new for this RUDL version?)");

    return newEvent;
}

/**
@file eventqueue
@class EventQueue
This class is the interface to the eventsystem in SDL.
Don't be put off by the amount of non-implemented methods, their absence doesn't bother
me and I don't plan on implementing them before someone comes up with a good reason for
their existence.
*/
/**
@section Class and instance Methods
@method get -> [ Event, ... ]
Returns all events in the queue and removes them from the queue.
@method get( eventmask ) -> [ Event, ... ]
Not implemented.
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

/**
@method peek -> Event or nil
Returns the next event without removing it from the queue,
or nil when no events are available.
@method peek( eventmask )
Not implemented.
*/
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

/**
@method poll -> Event or nil
Returns the next event, or nil if the queue is empty.
*/
static VALUE eventqueue_poll(VALUE self)
{
    SDL_Event event;
    if(SDL_PollEvent(&event)){
        return sDLEvent2RubyEvent(&event);
    }
    return Qnil;
}

/**
@method post( event ) -> nil
Not implemented.
*/
static VALUE eventqueue_post(VALUE self, VALUE event)
{
    rb_notimplement();
    return Qnil;
}

/**
@method pump -> self
This method is responsible for getting events from the operating system into the
SDL eventqueue.
If your application seems unresponsive,
calling this method every now and then might help.
*/
static VALUE eventqueue_pump(VALUE self)
{
    SDL_PumpEvents();
    return self;
}

/**
@method allowed=( eventtype ) -> nil
Not implemented.
*/
static VALUE eventqueue_set_allowed(VALUE self, VALUE eventType)
{
    rb_notimplement();
    return Qnil;
}

/**
@method blocked=( eventtype ) -> nil
*/
static VALUE eventqueue_set_blocked(VALUE self, VALUE eventType)
{
    rb_notimplement();
    return Qnil;
}

/**
@method grab -> self
@method grab=( grab ) -> self

Controls grabbing of all mouse and keyboard input for the display.
Grabbing the input is not neccessary to receive keyboard and mouse events,
but it ensures all input will go to your application.
It also keeps the mouse locked inside your window.
It is best to not always grab the input,
since it prevents the end user from doing anything else on their system.
@grab is true or false.
*/
static VALUE eventqueue_set_grab(VALUE self, VALUE grabOn)
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

/**
@method wait -> Event

Wait for an event to arrive.
Returns that event.
*/
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
    classEventQueue=rb_define_class_under(moduleRUDL, "EventQueue", rb_cObject);
    rb_define_singleton_and_instance_method(classEventQueue, "get", eventqueue_get, -1);
    rb_define_singleton_and_instance_method(classEventQueue, "peek", eventqueue_peek, -1);
    rb_define_singleton_and_instance_method(classEventQueue, "poll", eventqueue_poll, 0);
    rb_define_singleton_and_instance_method(classEventQueue, "post", eventqueue_post, 1);
    rb_define_singleton_and_instance_method(classEventQueue, "pump", eventqueue_pump, 0);
    rb_define_singleton_and_instance_method(classEventQueue, "allowed=", eventqueue_set_allowed, 1);
    rb_define_singleton_and_instance_method(classEventQueue, "blocked=", eventqueue_set_blocked, 1);
    rb_define_singleton_and_instance_method(classEventQueue, "grab=", eventqueue_set_grab, 1);
    rb_define_singleton_and_instance_method(classEventQueue, "grab", eventqueue_grab, 0);
    rb_define_singleton_and_instance_method(classEventQueue, "wait", eventqueue_wait, 0);

/**
@method flush -> nil
Flushes all events from the queue.
*/

    rb_eval_string(
        "module RUDL class EventQueue               \n"
        "   def EventQueue.flush                    \n"
        "       true while EventQueue.poll          \n"
        "   end                                     \n"
        "   def flush                               \n"
        "       true while poll                     \n"
        "   end                                     \n"
        "end end                                    \n");

/*
@section Events
@class Event
This is the baseclass for all event classes.
It contains nothing.

Other events can be found in the documentation section they belong to.
*/
    classEvent=rb_define_class_under(moduleRUDL, "Event", rb_cObject);

    //classEvent=rb_define_class_under(moduleRUDL, "UserEvent", classEvent);
    //rb_define_attr(classEvent, "code", 1, 1);
    //rb_define_attr(classEvent, "data1", 1, 1);
    //rb_define_attr(classEvent, "data2", 1, 1);
    //classSysWMEvent=rb_define_class_under(moduleRUDL, "SysWMEvent", classEvent);
}
