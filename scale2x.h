/*

This file should be included in a C source, and it will create the following functions:

Outside interface:
    static void scale2x(SDL_Surface* srcsurface, SDL_Surface* destsurface, int x, int y);

Internal functions:
    static void scale2x_row_8bit(Uint8* dest0, Uint8* dest1, Uint8* src0, Uint8* src1, Uint8* src2, int srcw);
    static void scale2x_row_16bit(Uint16* dest0, Uint16* dest1, Uint16* src0, Uint16* src1, Uint16* src2, int srcw);
    static void scale2x_row_24bit(Uint8* dest0, Uint8* dest1, Uint8* src0, Uint8* src1, Uint8* src2, int srcw);
    static void scale2x_row_32bit(Uint32* dest0, Uint32* dest1, Uint32* src0, Uint32* src1, Uint32* src2, int srcw);

    static void scale2x_8bit(SDL_Surface* src, SDL_Surface* dest, int x, int y);
    static void scale2x_16bit(SDL_Surface* src, SDL_Surface* dest, int x, int y);
    static void scale2x_24bit(SDL_Surface* src, SDL_Surface* dest, int x, int y);
    static void scale2x_32bit(SDL_Surface* src, SDL_Surface* dest, int x, int y);

File dependencies:
    scale2xrowfunc.h
    scale2xrowfunc_24bit.h
    scale2xrows.h

*/


#define PIXEL Uint8
#define SCALE2XROWFUNC scale2x_row_8bit
#define SCALE2XFUNC scale2x_8bit
#include "scale2xrowfunc.h"
#undef PIXEL
#undef SCALE2XROWFUNC
#undef SCALE2XFUNC

#define PIXEL Uint16
#define SCALE2XROWFUNC scale2x_row_16bit
#define SCALE2XFUNC scale2x_16bit
#include "scale2xrowfunc.h"
#undef PIXEL
#undef SCALE2XROWFUNC
#undef SCALE2XFUNC

#define PIXEL Uint32
#define SCALE2XROWFUNC scale2x_row_32bit
#define SCALE2XFUNC scale2x_32bit
#include "scale2xrowfunc.h"
#undef PIXEL
#undef SCALE2XROWFUNC
#undef SCALE2XFUNC

#include "scale2xrowfunc_24bit.h"

static void scale2x(SDL_Surface* srcsurface, SDL_Surface* destsurface, int x, int y)
{
    /* lock surfaces to ensure access to pixels */
    SDL_LockSurface(srcsurface);
    SDL_LockSurface(destsurface);

    /* choose the right function for this depth */
    switch (srcsurface->format->BytesPerPixel) {
        case 1:
            scale2x_8bit(srcsurface, destsurface, x, y);
            break;

        case 2:
            scale2x_16bit(srcsurface, destsurface, x, y);
            break;

        case 3:
            /* We need to pass 3*x because the pointer type is Uint8*. */
            scale2x_24bit(srcsurface, destsurface, 3*x, y);
            break;

        case 4:
            scale2x_32bit(srcsurface, destsurface, x, y);
            break;
    }

    SDL_UnlockSurface(srcsurface);
    SDL_UnlockSurface(destsurface);
}


