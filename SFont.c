#include "SDL.h"
#include "SFont.h"
#include "string.h"

SFont_FontInfo InternalFont;

Uint32 GetPixel(SDL_Surface *Surface, Sint32 X, Sint32 Y)
{

   Uint8  *bits;
   Uint32 Bpp;

   if (X<0) puts("SFONT ERROR: x too small in GetPixel. Report this to <karlb@gmx.net>");
   if (X>=Surface->w) puts("SFONT ERROR: x too big in GetPixel. Report this to <karlb@gmx.net>");
   
   Bpp = Surface->format->BytesPerPixel;

   bits = ((Uint8 *)Surface->pixels)+Y*Surface->pitch+X*Bpp;

   // Get the pixel
   switch(Bpp) {
      case 1:
         return *((Uint8 *)Surface->pixels + Y * Surface->pitch + X);
         break;
      case 2:
         return *((Uint16 *)Surface->pixels + Y * Surface->pitch/2 + X);
         break;
      case 3: { // Format/endian independent 
         Uint8 r, g, b;
         r = *((bits)+Surface->format->Rshift/8);
         g = *((bits)+Surface->format->Gshift/8);
         b = *((bits)+Surface->format->Bshift/8);
         return SDL_MapRGB(Surface->format, r, g, b);
         }
         break;
      case 4:
         return *((Uint32 *)Surface->pixels + Y * Surface->pitch/4 + X);
         break;
   }

    return -1;
}

void InitFont2(SFont_FontInfo *Font)
{
    int x = 0, i = 0;

    if ( Font->Surface==NULL ) {
	printf("The font has not been loaded!\n");
//	exit(1);
    }

    if (SDL_MUSTLOCK(Font->Surface)) SDL_LockSurface(Font->Surface);

    while ( x < Font->Surface->w ) {
	if(GetPixel(Font->Surface,x,0)==SDL_MapRGB(Font->Surface->format,255,0,255)) { 
    	    Font->CharPos[i++]=x;
    	    while (( x < Font->Surface->w-1) && (GetPixel(Font->Surface,x,0)==SDL_MapRGB(Font->Surface->format,255,0,255)))
		x++;
	    Font->CharPos[i++]=x;
	}
	x++;
    }
    if (SDL_MUSTLOCK(Font->Surface)) SDL_UnlockSurface(Font->Surface);

    Font->h=Font->Surface->h;
    SDL_SetColorKey(Font->Surface, SDL_SRCCOLORKEY, GetPixel(Font->Surface, 0, Font->Surface->h-1));
}

void InitFont(SDL_Surface *Font)
{
    InternalFont.Surface=Font;
    InitFont2(&InternalFont);
}

void PutString2(SDL_Surface *Surface, SFont_FontInfo *Font, int x, int y, char *text)
{
    int ofs;
    int i=0;
    SDL_Rect srcrect,dstrect; 

    while (text[i]!='\0') {
        if (text[i]==' ') {
            x+=Font->CharPos[2]-Font->CharPos[1];
            i++;
	}
	else {
//	    printf("-%c- %c - %u\n",228,text[i],text[i]);
	    ofs=(text[i]-33)*2+1;
//	    printf("printing %c %d\n",text[i],ofs);
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

void PutString(SDL_Surface *Surface, int x, int y, char *text)
{
    PutString2(Surface, &InternalFont, x, y, text);
}

int TextWidth2(SFont_FontInfo *Font, char *text)
{
    int ofs=0;
    int i=0,x=0;

    while (text[i]!='\0') {
        if (text[i]==' ') {
            x+=Font->CharPos[2]-Font->CharPos[1];
            i++;
	}
	else {
	    ofs=(text[i]-33)*2+1;
            x+=Font->CharPos[ofs+1]-Font->CharPos[ofs];
            i++;
        }
    }
//    printf ("--%d\n",x);
    return x;
}

int TextWidth(char *text)
{
    return TextWidth2(&InternalFont, text);
}

void XCenteredString2(SDL_Surface *Surface, SFont_FontInfo *Font, int y, char *text)
{
    PutString2(Surface, Font, Surface->w/2-TextWidth2(Font,text)/2, y, text);
}

void XCenteredString(SDL_Surface *Surface, int y, char *text)
{
    XCenteredString2(Surface, &InternalFont, y, text);
}

