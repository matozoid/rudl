/* RUDL - a C library wrapping SDL for use in Ruby. Copyright (C) 2001  Danny van Bruggen */
#include "rudl_ttf.h"

#ifdef HAVE_SDL_TTF_H
static bool ttf_inited=false;
#include "rudl_video.h"
#endif

#define TTF_RAISE {rb_raise(classSDLError, TTF_GetError());}


void initTTF()
{
#ifdef HAVE_SDL_TTF_H
	if(!ttf_inited){
		if(TTF_Init()) TTF_RAISE;
		ttf_inited=true;
	}
#endif
}

void quitTTF()
{
#ifdef HAVE_SDL_TTF_H
	if(ttf_inited){
		TTF_Quit();
	}
#endif
}

void VALUE2SDL_COLOR(VALUE colorObject, SDL_Color* color)
{
	if(rb_obj_is_kind_of(colorObject, rb_cArray)){
		switch(RARRAY(colorObject)->len){
			case 4:	
			case 3:	color->r=(Uint8)NUM2UINT(rb_ary_entry(colorObject, 0)),
				color->g=(Uint8)NUM2UINT(rb_ary_entry(colorObject, 1)),
				color->b=(Uint8)NUM2UINT(rb_ary_entry(colorObject, 2));
				break;
			default:
				rb_raise(rb_eTypeError, "Need colorarray with 3 or 4 elements");
		}
	}else{
		rb_raise(rb_eTypeError, "Need a color array");
	}
}

#ifdef HAVE_SDL_TTF_H
/*
=begin
<<< docs/head
= TrueTypeFont
== Class Methods
--- TrueTypeFont.new( filename, size )
This will create a new font object.
The given file must be a filename of a TTF file.
The size represents the height of the font in pixels.
=end */
static VALUE truetypefont_new(VALUE self, VALUE filename, VALUE size)
{
	TTF_Font *font;
	initTTF();
	font=TTF_OpenFont(STR2CSTR(filename), NUM2INT(size));
	if(!font)TTF_RAISE;
	return Data_Wrap_Struct(classTTF, 0, TTF_CloseFont, font);
}

TTF_Font* retrieveTTFPointer(VALUE obj)
{
	TTF_Font* font;
	Data_Get_Struct(obj, TTF_Font, font);
	return font;
}

/*
=begin
== Instance Methods
--- TrueTypeFont#render(text, antialias, fore_RGBA)
--- TrueTypeFont#render(text, antialias, fore_RGBA, back_RGBA)
Render the given text onto a new image surface.
Antialiasing will smooth the edges of the font for a much cleaner look.
The foreground and background color are both RGBA, the alpha component is ignored if given.
If the background color is omitted, the text will have a transparent background.
=end */
static VALUE truetypefont_render(int argc, VALUE* argv, VALUE self)
{
	TTF_Font* font = retrieveTTFPointer(self);
	bool aa;
	SDL_Surface* surf;
	SDL_Color foreg, backg;
	char* string;

	VALUE textValue, aaValue, fg_rgbaValue, bg_rgbaValue;

	switch(rb_scan_args(argc, argv, "31", &textValue, &aaValue, &fg_rgbaValue, &bg_rgbaValue)){
		case 4: VALUE2SDL_COLOR(bg_rgbaValue, &backg);
	}
	VALUE2SDL_COLOR(fg_rgbaValue, &foreg);

	aa=NUM2BOOL(aaValue);

	string=STR2CSTR(textValue);
	if(aa){
		if(argc==3){
			surf = TTF_RenderText_Blended(font, string, foreg);
		}else{
			surf = TTF_RenderText_Shaded(font, string, foreg, backg);
		}
	}else{
		surf = TTF_RenderText_Solid(font, string, foreg);
	}

	if(!surf) SDL_RAISE;

	if(!aa && argc==4) /*turn off transparancy*/
	{			
		SDL_SetColorKey(surf, 0, 0);
		surf->format->palette->colors[0].r = backg.r;
		surf->format->palette->colors[0].g = backg.g;
		surf->format->palette->colors[0].b = backg.b;
	}

	return createSurfaceObject(surf);
}

/*
=begin
--- TrueTypeFont#ascent
Returns the ascent for the font.
The ascent is the number of pixels from the font baseline to the top of the font.
=end */
static VALUE truetypefont_ascent(VALUE self)
{
	return INT2NUM(TTF_FontAscent(retrieveTTFPointer(self)));
}

/*
=begin
--- TrueTypeFont#bold?
--- TrueTypeFont#bold=( onOrOff )
Controls the bold attribute for the font.
Making the font bold does not work as well as you expect.
=end */
static VALUE truetypefont_bold_(VALUE self)
{
	return INT2BOOL((TTF_GetFontStyle(retrieveTTFPointer(self))&&TTF_STYLE_BOLD)!=0);
}

static VALUE truetypefont_bold__(VALUE self, VALUE onOrOff)
{
	TTF_Font* font = retrieveTTFPointer(self);
	int style;

	style = TTF_GetFontStyle(font);
	if(NUM2BOOL(onOrOff)){
		style |= TTF_STYLE_BOLD;
	}else{
		style &= ~TTF_STYLE_BOLD;
	}
	TTF_SetFontStyle(font, style);

	return self;
}

/*
=begin
--- TrueTypeFont#italic?
--- TrueTypeFont#italic=( onOrOff )
Controls the italics attribute of the font.
=end */
static VALUE truetypefont_italic_(VALUE self)
{
	return INT2BOOL((TTF_GetFontStyle(retrieveTTFPointer(self))&&TTF_STYLE_ITALIC)!=0);
}

static VALUE truetypefont_italic__(VALUE self, VALUE onOrOff)
{
	TTF_Font* font = retrieveTTFPointer(self);
	int style;

	style = TTF_GetFontStyle(font);
	if(NUM2BOOL(onOrOff)){
		style |= TTF_STYLE_ITALIC;
	}else{
		style &= ~TTF_STYLE_ITALIC;
	}
	TTF_SetFontStyle(font, style);

	return self;
}

/*
=begin
--- TrueTypeFont#descent
Returns the descent for the font.
The descent is the number of pixels from the font baseline to the bottom of the font.
=end */
static VALUE truetypefont_descent(VALUE self)
{
	return INT2NUM(TTF_FontDescent(retrieveTTFPointer(self)));
}

/*
=begin
--- TrueTypeFont#h
Returns the average size of each glyph in the font.
=end */
static VALUE truetypefont_h(VALUE self)
{
	return INT2NUM(TTF_FontHeight(retrieveTTFPointer(self)));
}

/*
=begin
--- TrueTypeFont#linesize
Returns the linesize for the font.
Each font comes with it's own recommendation for the spacing number of pixels between 
each line of the font.
=end */
static VALUE truetypefont_linesize(VALUE self)
{
	return INT2NUM(TTF_FontLineSkip(retrieveTTFPointer(self)));
}

/*
=begin
--- TrueTypeFont#underline?
--- TrueTypeFont#underline=( onOrOff )
Controls the underline attribute of the font.
=end */
static VALUE truetypefont_underline_(VALUE self)
{
	return INT2BOOL((TTF_GetFontStyle(retrieveTTFPointer(self))&&TTF_STYLE_UNDERLINE)!=0);
}

static VALUE truetypefont_underline__(VALUE self, VALUE onOrOff)
{
	TTF_Font* font = retrieveTTFPointer(self);
	int style;

	style = TTF_GetFontStyle(font);
	if(NUM2BOOL(onOrOff)){
		style |= TTF_STYLE_UNDERLINE;
	}else{
		style &= ~TTF_STYLE_UNDERLINE;
	}
	TTF_SetFontStyle(font, style);

	return self;
}

/*
=begin
--- TrueTypeFont#size( text )
Returns the size in pixels that this text would need. [w,h]
=end */
static VALUE truetypefont_size(VALUE self, VALUE text)
{
	int w, h;
	TTF_SizeText(retrieveTTFPointer(self), STR2CSTR(text), &w, &h);
	return rb_ary_new3(2, INT2NUM(w), INT2NUM(h));
}
#endif

void initTrueTypeFontClasses()
{
#ifdef HAVE_SDL_TTF_H
	classTTF=rb_define_class_under(moduleRUDL, "TrueTypeFont", rb_cObject);
	rb_define_singleton_method(classTTF, "new", truetypefont_new, 2);
	rb_define_method(classTTF, "ascent", truetypefont_ascent, 0);
	rb_define_method(classTTF, "bold?", truetypefont_bold_, 0);
	rb_define_method(classTTF, "italic?", truetypefont_italic_, 0);
	rb_define_method(classTTF, "descent", truetypefont_descent, 0);
	rb_define_method(classTTF, "h", truetypefont_h, 0);
	rb_define_method(classTTF, "linesize", truetypefont_linesize, 0);
	rb_define_method(classTTF, "underline?", truetypefont_underline_, 0);
	rb_define_method(classTTF, "render", truetypefont_render, -1);
	rb_define_method(classTTF, "bold=", truetypefont_bold__, 1);
	rb_define_method(classTTF, "italic=", truetypefont_italic__, 1);
	rb_define_method(classTTF, "underline=", truetypefont_underline__, 1);
	rb_define_method(classTTF, "size", truetypefont_size, 1);

	rb_alias(classTTF, rb_intern("height"), rb_intern("h"));
#endif
}
