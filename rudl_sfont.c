/* RUDL - a C library wrapping SDL for use in Ruby. Copyright (C) 2001  Danny van Bruggen */
#include "rudl_sfont.h"
#include "rudl_video.h"

typedef struct {
	SDL_Surface *Surface;
	int CharPos[512];
	int h;
} SFont_FontInfo;

void PutString(SDL_Surface *Surface, SFont_FontInfo *Font, Sint16 x, Sint16 y, char *text)
{
    Sint16 ofs;
    Sint16 i=0;
    SDL_Rect srcrect,dstrect;

    while (text[i]!='\0') {
        if (text[i]==' ') {
            x+=Font->CharPos[2]-Font->CharPos[1];
            i++;
		}else{
			ofs=(text[i]-33)*2+1;
			srcrect.w = dstrect.w = (Font->CharPos[ofs+2]+Font->CharPos[ofs+1])/2-(Font->CharPos[ofs]+Font->CharPos[ofs-1])/2;
			srcrect.h = dstrect.h = Font->Surface->h-1;
			srcrect.x = (Font->CharPos[ofs]+Font->CharPos[ofs-1])/2;
			srcrect.y = 1;
			dstrect.x = (Sint16)(x-(float)(Font->CharPos[ofs]-Font->CharPos[ofs-1])/2);
			dstrect.y = y;

			SDL_BlitSurface( Font->Surface, &srcrect, Surface, &dstrect);

			x+=Font->CharPos[ofs+1]-Font->CharPos[ofs];
			i++;
        }
    }
}

static SFont_FontInfo* retrieveFontInfoPointer(VALUE font)
{
	SFont_FontInfo* fontPtr;
	Data_Get_Struct(font, SFont_FontInfo, fontPtr);
	return fontPtr;
}

/**
@file Fonts
@class BitmapFont
BitmapFont is made with the SFont library and prints text with bitmaps.
It is maintained by Karl Bartel on <a href='http://www.linux-games.com/'>Linux Games</a>

<p>Here is Karl's README:

<p>FileFormat:

<p>The font file can be any type image file. The characters start with
ASCII symbol #33. They are seperated by pink(255,0,255) lines at the top
of the image. The space between these lines is the width of the caracter.
Just take a look at the image, and you'll be able to understand what I tried
to explain here.

<p>Example for the font file format is in this picture: <img src='sfont.gif'>

<p>The easiest way to create a new font is to use the GIMP's Logo function.
Use the following string as text (ASCII 33-127 with escape sequences and
spaces between the letters):

<code>! \" # $ % & ' ( ) * + , - . / 0 1 2 3 4 5 6 7 8 9 : ; < = > ? @@ A B C D E F G H I J K L M N O P Q R S T U V W X Y Z [ \\ ] ^ _ ` a b c d e f g h i j k l m n o p q r s t u v w x y z { | } ~</code>
*/
/**
@section Class Methods
@method new( surface ) -> BitmapFont
Creates a new BitmapFont from the drawn characters in @surface.
To load a font directly from disc,
use <code>Surface.load_new('fontfilename')</code> as the argument to @new.
*/
static VALUE sfont_new(VALUE self, VALUE surface)
{
	VALUE newFont;
	Sint16 x = 0, i = 0;

	SFont_FontInfo* font=(SFont_FontInfo*)malloc(sizeof(SFont_FontInfo));
	font->Surface=retrieveSurfacePointer(surface);

	SDL_VERIFY(font->Surface!=NULL);

	if (SDL_MUSTLOCK(font->Surface)) SDL_LockSurface(font->Surface);

	while ( x < font->Surface->w ) {
		if(internal_nonlocking_get(font->Surface,x,0)==SDL_MapRGB(font->Surface->format,255,0,255)) {
			font->CharPos[i++]=x;
			while (( x < font->Surface->w-1) && (internal_nonlocking_get(font->Surface,x,0)==SDL_MapRGB(font->Surface->format,255,0,255)))
				x++;
			font->CharPos[i++]=x;
		}
		x++;
	}
	if (SDL_MUSTLOCK(font->Surface)) SDL_UnlockSurface(font->Surface);

	font->h=font->Surface->h;
	SDL_SetColorKey(font->Surface, SDL_SRCCOLORKEY, internal_nonlocking_get(font->Surface, 0, font->Surface->h-1));

	newFont=Data_Wrap_Struct(classSFont, 0, free, font);
	rb_iv_set(newFont, "@referenceholder", surface);
	return newFont;
}

/**
@section Instance Methods
@method print( surface, coordinate, text ) -> self
Puts @text on @surface at @coordinate which is an array of [x, y].
*/
static VALUE sfont_print(VALUE self, VALUE surface, VALUE coord, VALUE text)
{
	Sint16 x,y;
	PARAMETER2COORD(coord, &x, &y);
	PutString(retrieveSurfacePointer(surface), retrieveFontInfoPointer(self), x, y, STR2CSTR(text));
	return self;
}

/**
@method print_centered( surface, y, text ) -> self
Puts @text on @surface at y-coordinate @y in the horizontal center of the screen.
*/
static void add_sfont_print_centered()
{
	rb_eval_string(
		"module RUDL class SFont						\n"
		"	def print_centered(surface, y, text)				\n"
		"		print(surface,[(surface.w-size(text)[0])/2,y],text)	\n"
        "       self                                                \n"
		"	end								\n"
		"end end								\n"
	);
}

/**
@method size( text ) -> [width, heigth]
Returns the size in pixels that @text will take when written.
*/
static VALUE sfont_size(VALUE self, VALUE ruby_text)
{
	SFont_FontInfo* font=retrieveFontInfoPointer(self);
	char* text=STR2CSTR(ruby_text);
	int ofs=0;
	int i=0,x=0;

	while (text[i]!='\0') {
		if (text[i]==' ') {
			x+=font->CharPos[2]-font->CharPos[1];
			i++;
		}else{
			ofs=(text[i]-33)*2+1;
			x+=font->CharPos[ofs+1]-font->CharPos[ofs];
			i++;
		}
	}
	return rb_ary_new3(2, INT2NUM(x), INT2NUM(font->h));
}



void initSFontClasses()
{
	classSFont=rb_define_class_under(moduleRUDL, "SFont", rb_cObject);
	rb_define_singleton_method(classSFont, "new", sfont_new, 1);
	rb_define_method(classSFont, "print", sfont_print, 3);
	rb_define_method(classSFont, "size", sfont_size, 1);

	add_sfont_print_centered();

	// Backward compatability:

	rb_alias(classSFont, rb_intern("puts"), rb_intern("print"));
	rb_alias(classSFont, rb_intern("puts_centered"), rb_intern("print_centered"));

	classBitmapFont=rb_define_class_under(moduleRUDL, "BitmapFont", classSFont);
	rb_eval_string(
			"module RUDL class BitmapFont					\n"
			"	def inspect									\n"
			"		\"<BitmapFont: #{size.x}x#{size.y}>\"	\n"
			"	end											\n"
			"end end										\n"
	);

}
