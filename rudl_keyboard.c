/* RUDL - a C library wrapping SDL for use in Ruby. Copyright (C) 2001  Danny van Bruggen */
#include "rudl_keyboard.h"
#include "rudl_video.h"
#include "rudl_events.h"

/*
=begin
<<< docs/head
= Key
The Key class gives access to the keyboard without events.

You can either call the class methods: "Key.something" directly, 
or instantiate the class and lug along a reference to it and call
instance methods on that: "k=Key.new" "k.something" (they will call
class methods under the hood, so it's exactly the same)

EventQueue.pump will update the state of Key,
so if no keys seem to be pressed, call pump.
== Class and instance Methods
--- Key.focused?
--- Key#focused?
Returns true when the application has the keyboard input focus.
=end */
static VALUE key_getFocused(VALUE self)
{
	initVideo();
	return INT2BOOL((SDL_GetAppState()&SDL_APPINPUTFOCUS)!=0);
}

/*
=begin
--- Key.modifiers
--- Key#modifiers
Returns the current modifier keys state.
=end */
static VALUE key_getModifiers(VALUE self)
{
	initVideo();
	return UINT2NUM(SDL_GetModState());
}

/*
=begin
--- Key.pressed?
--- Key#pressed?
Returns a hash containing all keys that are set, set to true.
So, if K_b is pressed, the hash will contain a key,value pair of K_b => true.
=end */
static VALUE key_getPressed(VALUE self)
{
	int num_keys;
	Uint8* key_state;
	VALUE keys;
	int i;

	initVideo();

	key_state = SDL_GetKeyState(&num_keys);

	if(!key_state || !num_keys) return Qnil;

	keys=rb_hash_new();

	for(i=0; i<num_keys; i++){
		if(key_state[i]){
			rb_hash_aset(keys, UINT2NUM(i), Qtrue);
		}
	}
	return keys;
}

/*
=begin
--- Key.name( key )
--- Key#name( key )
Returns a string describing constant ((|key|))
=end */
static VALUE key_name(VALUE self, VALUE key)
{
	initVideo();
	return rb_str_new2(SDL_GetKeyName(NUM2UINT(key)));
}

/*
=begin
--- Key.modifiers=( modifiers )
--- Key#modifiers=( modifiers )
Sets the keyboard modifier state.
=end */
static VALUE key_setModifiers(VALUE self, VALUE mods)
{
	initVideo();
	SDL_SetModState(NUM2UINT(mods));
	return self;
}

/*
=begin
--- Key.set_repeat( delay, interval )
--- Key#set_repeat( delay, interval )
Sets the keyboard to wait for ((|delay|)) milliseconds before starting repeat with 
((|interval|)) delay between repeats.
Set both to zero to disable repeat.
=end */
static VALUE key_set_repeat(VALUE self, VALUE delay, VALUE interval)
{
	initVideo();
	SDL_EnableKeyRepeat(NUM2INT(delay),NUM2INT(interval));
	return self;
}



void initKeyClasses()
{
	classKey=rb_define_class_under(moduleRUDL, "Key", rb_cObject);
	rb_define_singleton_and_instance_method(classKey, "focused?", key_getFocused, 0);
	rb_define_singleton_and_instance_method(classKey, "modifiers", key_getModifiers, 0);
	rb_define_singleton_and_instance_method(classKey, "pressed?", key_getPressed, 0);
	rb_define_singleton_and_instance_method(classKey, "name", key_name, 1);
	rb_define_singleton_and_instance_method(classKey, "modifiers=", key_setModifiers, 1);
	rb_define_singleton_and_instance_method(classKey, "set_repeat", key_set_repeat, 2);

/*
=begin
= KeyUpEvent
This event is posted when a key is released.
--- KeyUpEvent#key
The keycode for the released key.
--- KeyUpEvent#mod
The modifier keys state.
--- KeyUpEvent#unicode
The Unicode version of the key.
=end */
	classKeyUpEvent=rb_define_class_under(moduleRUDL, "KeyUpEvent", classEvent);
	rb_define_attr(classKeyUpEvent, "key", 1, 1);
	rb_define_attr(classKeyUpEvent, "mod", 1, 1);
	rb_define_attr(classKeyUpEvent, "unicode", 1, 1);

/*
=begin
= KeyDownEvent
This event is posted when a key is pressed and when it gets repeated (see Key#set_repeat).
--- KeyDownEvent#key
The keycode for the pressed key.
--- KeyDownEvent#mod
The modifier keys state.
--- KeyDownEvent#unicode
The Unicode version of the key.
=end */
	classKeyDownEvent=rb_define_class_under(moduleRUDL, "KeyDownEvent", classEvent);
	rb_define_attr(classKeyDownEvent, "key", 1, 1);
	rb_define_attr(classKeyDownEvent, "mod", 1, 1);
	rb_define_attr(classKeyDownEvent, "unicode", 1, 1);

/*
=begin
= Constants
Keycodes in one handy big mess:

K_UNKNOWN, K_FIRST, K_BACKSPACE, K_TAB, K_CLEAR, K_RETURN, K_PAUSE, K_ESCAPE, K_SPACE, K_EXCLAIM, K_QUOTEDBL, 
K_HASH, K_DOLLAR, K_AMPERSAND, K_QUOTE, K_LEFTPAREN, K_RIGHTPAREN, K_ASTERISK, K_PLUS, K_COMMA, 
K_MINUS, K_PERIOD, K_SLASH, K_0, K_1, K_2, K_3, K_4, K_5, K_6, K_7, K_8, K_9, K_COLON, K_SEMICOLON, K_LESS, 
K_EQUALS, K_GREATER, K_QUESTION, K_AT, K_LEFTBRACKET, K_BACKSLASH, K_RIGHTBRACKET, K_CARET, K_UNDERSCORE, 
K_BACKQUOTE, K_a, K_b, K_c, K_d, K_e, K_f, K_g, K_h, K_i, K_j, K_k, K_l, K_m, K_n, K_o, K_p, K_q, K_r, K_s, K_t, 
K_u, K_v, K_w, K_x, K_y, K_z, K_DELETE, K_KP0, K_KP1, K_KP2, K_KP3, K_KP4, K_KP5, K_KP6, K_KP7, K_KP8, K_KP9, 
K_KP_PERIOD, K_KP_DIVIDE, K_KP_MULTIPLY, K_KP_MINUS, K_KP_PLUS, K_KP_ENTER, K_KP_EQUALS, K_UP, 
K_DOWN, K_RIGHT, K_LEFT, K_INSERT, K_HOME, K_END, K_PAGEUP, K_PAGEDOWN, K_F1, K_F2, K_F3, K_F4, K_F5, 
K_F6, K_F7, K_F8, K_F9, K_F10, K_F11, K_F12, K_F13, K_F14, K_F15, , K_NUMLOCK, K_CAPSLOCK, K_SCROLLOCK, 
K_RSHIFT, K_LSHIFT, K_RCTRL, K_LCTRL, K_RALT, K_LALT, K_RMETA, K_LMETA, K_LSUPER, K_RSUPER, K_MODE, 
K_HELP, K_PRINT, K_SYSREQ, K_BREAK, K_MENU, K_POWER, K_EURO, K_LAST, KMOD_NONE, KMOD_LSHIFT, KMOD_RSHIFT, 
KMOD_LCTRL, KMOD_RCTRL, KMOD_LALT, KMOD_RALT, KMOD_LMETA, KMOD_RMETA, KMOD_NUM, KMOD_CAPS, KMOD_MODE, 
KMOD_CTRL, KMOD_SHIFT, KMOD_ALT, KMOD_META
=end */
	DEC_CONSTK(K_UNKNOWN);
	DEC_CONSTK(K_FIRST);
	DEC_CONSTK(K_BACKSPACE);
	DEC_CONSTK(K_TAB);
	DEC_CONSTK(K_CLEAR);
	DEC_CONSTK(K_RETURN);
	DEC_CONSTK(K_PAUSE);
	DEC_CONSTK(K_ESCAPE);
	DEC_CONSTK(K_SPACE);
	DEC_CONSTK(K_EXCLAIM);
	DEC_CONSTK(K_QUOTEDBL);
	DEC_CONSTK(K_HASH);
	DEC_CONSTK(K_DOLLAR);
	DEC_CONSTK(K_AMPERSAND);
	DEC_CONSTK(K_QUOTE);
	DEC_CONSTK(K_LEFTPAREN);
	DEC_CONSTK(K_RIGHTPAREN);
	DEC_CONSTK(K_ASTERISK);
	DEC_CONSTK(K_PLUS);
	DEC_CONSTK(K_COMMA);
	DEC_CONSTK(K_MINUS);
	DEC_CONSTK(K_PERIOD);
	DEC_CONSTK(K_SLASH);
	DEC_CONSTK(K_0);
	DEC_CONSTK(K_1);
	DEC_CONSTK(K_2);
	DEC_CONSTK(K_3);
	DEC_CONSTK(K_4);
	DEC_CONSTK(K_5);
	DEC_CONSTK(K_6);
	DEC_CONSTK(K_7);
	DEC_CONSTK(K_8);
	DEC_CONSTK(K_9);
	DEC_CONSTK(K_COLON);
	DEC_CONSTK(K_SEMICOLON);
	DEC_CONSTK(K_LESS);
	DEC_CONSTK(K_EQUALS);
	DEC_CONSTK(K_GREATER);
	DEC_CONSTK(K_QUESTION);
	DEC_CONSTK(K_AT);
	DEC_CONSTK(K_LEFTBRACKET);
	DEC_CONSTK(K_BACKSLASH);
	DEC_CONSTK(K_RIGHTBRACKET);
	DEC_CONSTK(K_CARET);
	DEC_CONSTK(K_UNDERSCORE);
	DEC_CONSTK(K_BACKQUOTE);
	DEC_CONSTK(K_a);
	DEC_CONSTK(K_b);
	DEC_CONSTK(K_c);
	DEC_CONSTK(K_d);
	DEC_CONSTK(K_e);
	DEC_CONSTK(K_f);
	DEC_CONSTK(K_g);
	DEC_CONSTK(K_h);
	DEC_CONSTK(K_i);
	DEC_CONSTK(K_j);
	DEC_CONSTK(K_k);
	DEC_CONSTK(K_l);
	DEC_CONSTK(K_m);
	DEC_CONSTK(K_n);
	DEC_CONSTK(K_o);
	DEC_CONSTK(K_p);
	DEC_CONSTK(K_q);
	DEC_CONSTK(K_r);
	DEC_CONSTK(K_s);
	DEC_CONSTK(K_t);
	DEC_CONSTK(K_u);
	DEC_CONSTK(K_v);
	DEC_CONSTK(K_w);
	DEC_CONSTK(K_x);
	DEC_CONSTK(K_y);
	DEC_CONSTK(K_z);
	DEC_CONSTK(K_DELETE);

	DEC_CONSTK(K_KP0);
	DEC_CONSTK(K_KP1);
	DEC_CONSTK(K_KP2);
	DEC_CONSTK(K_KP3);
	DEC_CONSTK(K_KP4);
	DEC_CONSTK(K_KP5);
	DEC_CONSTK(K_KP6);
	DEC_CONSTK(K_KP7);
	DEC_CONSTK(K_KP8);
	DEC_CONSTK(K_KP9);
	DEC_CONSTK(K_KP_PERIOD);
	DEC_CONSTK(K_KP_DIVIDE);
	DEC_CONSTK(K_KP_MULTIPLY);
	DEC_CONSTK(K_KP_MINUS);
	DEC_CONSTK(K_KP_PLUS);
	DEC_CONSTK(K_KP_ENTER);
	DEC_CONSTK(K_KP_EQUALS);
	DEC_CONSTK(K_UP);
	DEC_CONSTK(K_DOWN);
	DEC_CONSTK(K_RIGHT);
	DEC_CONSTK(K_LEFT);
	DEC_CONSTK(K_INSERT);
	DEC_CONSTK(K_HOME);
	DEC_CONSTK(K_END);
	DEC_CONSTK(K_PAGEUP);
	DEC_CONSTK(K_PAGEDOWN);
	DEC_CONSTK(K_F1);
	DEC_CONSTK(K_F2);
	DEC_CONSTK(K_F3);
	DEC_CONSTK(K_F4);
	DEC_CONSTK(K_F5);
	DEC_CONSTK(K_F6);
	DEC_CONSTK(K_F7);
	DEC_CONSTK(K_F8);
	DEC_CONSTK(K_F9);
	DEC_CONSTK(K_F10);
	DEC_CONSTK(K_F11);
	DEC_CONSTK(K_F12);
	DEC_CONSTK(K_F13);
	DEC_CONSTK(K_F14);
	DEC_CONSTK(K_F15);

	DEC_CONSTK(K_NUMLOCK);
	DEC_CONSTK(K_CAPSLOCK);
	DEC_CONSTK(K_SCROLLOCK);
	DEC_CONSTK(K_RSHIFT);
	DEC_CONSTK(K_LSHIFT);
	DEC_CONSTK(K_RCTRL);
	DEC_CONSTK(K_LCTRL);
	DEC_CONSTK(K_RALT);
	DEC_CONSTK(K_LALT);
	DEC_CONSTK(K_RMETA);
	DEC_CONSTK(K_LMETA);
	DEC_CONSTK(K_LSUPER);
	DEC_CONSTK(K_RSUPER);
	DEC_CONSTK(K_MODE);

	DEC_CONSTK(K_HELP);
	DEC_CONSTK(K_PRINT);
	DEC_CONSTK(K_SYSREQ);
	DEC_CONSTK(K_BREAK);
	DEC_CONSTK(K_MENU);
	DEC_CONSTK(K_POWER);
	DEC_CONSTK(K_EURO);
	DEC_CONSTK(K_LAST);

	DEC_CONSTN(KMOD_NONE);
	DEC_CONSTN(KMOD_LSHIFT);
	DEC_CONSTN(KMOD_RSHIFT);
	DEC_CONSTN(KMOD_LCTRL);
	DEC_CONSTN(KMOD_RCTRL);
	DEC_CONSTN(KMOD_LALT);
	DEC_CONSTN(KMOD_RALT);
	DEC_CONSTN(KMOD_LMETA);
	DEC_CONSTN(KMOD_RMETA);
	DEC_CONSTN(KMOD_NUM);
	DEC_CONSTN(KMOD_CAPS);
	DEC_CONSTN(KMOD_MODE);

	DEC_CONSTN(KMOD_CTRL);
	DEC_CONSTN(KMOD_SHIFT);
	DEC_CONSTN(KMOD_ALT);
	DEC_CONSTN(KMOD_META);
}
