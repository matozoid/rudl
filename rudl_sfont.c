/* RUDL - a C library wrapping SDL for use in Ruby. Copyright (C) 2001  Danny van Bruggen */
#include "rudl_sfont.h"
#include "rudl_video.h"
#include "SFont.h"

static SFont_FontInfo* retrieveFontInfoPointer(VALUE font)
{
	SFont_FontInfo* fontPtr;
	Data_Get_Struct(font, SFont_FontInfo, fontPtr);
	return fontPtr;
}

/*
=begin
= SFont
An SFont is a font made of bitmaps.
It is maintained by Karl Bartel on ((<URL:http://www.linux-games.com/sfont/>)).
On his site, you can find fonts and some help.
== Class Methods
--- SFont.new( surface )
Creates a new SFont from the drawn characters in ((|surface|)).
To load a font directly from disc,
use Surface.load_new('fontfilename') as the argument to SFont.new.
=end */

static VALUE sfont_new(VALUE self, VALUE surface)
{
	VALUE newFont;
	SFont_FontInfo* info=(SFont_FontInfo*)malloc(sizeof(SFont_FontInfo));
	info->Surface=retrieveSurfacePointer(surface);
	InitFont2(info);
	newFont=Data_Wrap_Struct(classSFont, 0, free, info);
	rb_iv_set(newFont, "@referenceholder", surface);
	return newFont;
}

/*
=begin
== Instance Methods
--- SFont#puts( surface, coordinate, text )
Puts ((|text|)) on ((|surface|)) at ((|coordinate|)) which is an array of [x, y].
=end */
static VALUE sfont_puts(VALUE self, VALUE surface, VALUE coord, VALUE text)
{
	Sint16 x,y;
	PARAMETER2COORD(coord, &x, &y);
	PutString2(retrieveSurfacePointer(surface), retrieveFontInfoPointer(self), x, y, STR2CSTR(text));
	return self;
}

/*
=begin
== Instance Methods
--- SFont#pust_centered( surface, y, text )
Puts ((|text|)) on ((|surface|)) at y-coordinate ((|y|)) in the horizontal center of the screen.
=end */
static VALUE sfont_puts_centered(VALUE self, VALUE surface, VALUE y, VALUE text)
{
	XCenteredString2(retrieveSurfacePointer(surface), retrieveFontInfoPointer(self), NUM2INT(y), STR2CSTR(text));
	return self;
}

/*
=begin
== Instance Methods
--- SFont#size( text )
Returns the size [width, heigth] in pixels that ((|text|)) will take when written.
=end */
static VALUE sfont_size(VALUE self, VALUE text)
{
	SFont_FontInfo* fontinfo=retrieveFontInfoPointer(self);

	return rb_ary_new3(2, 
		INT2NUM(TextWidth2(fontinfo, STR2CSTR(text))),
		INT2NUM(fontinfo->h));
}

/*static VALUE sfont_gets(VALUE self, VALUE surface, VALUE coord, VALUE widthObject)
{
	Sint16 x,y;
	int width=NUM2INT(widthObject);
	char buffer[width]; // Presuming there are nog characters of width 0, this should be enough.
	PARAMETER2COORD(coord, &x, &y);
	SFont_Input2(retrieveSurfacePointer(surface), retrieveFontInfoPointer(self), x, y, width, buffer);
	return CSTR2STR(buffer);
}*/

void initSFontClasses()
{
	classSFont=rb_define_class_under(moduleRUDL, "SFont", rb_cObject);
	rb_define_singleton_method(classSFont, "new", sfont_new, 1);
	rb_define_method(classSFont, "puts", sfont_puts, 3);
	rb_define_method(classSFont, "puts_centered", sfont_puts_centered, 3);
	rb_define_method(classSFont, "size", sfont_size, 1);
	//rb_define_method(classSFont, "gets", sfont_gets, 3); Too bad, it uses UNICODE
}
