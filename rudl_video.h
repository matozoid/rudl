/* RUDL - a C library wrapping SDL for use in Ruby. Copyright (C) 2001  Danny van Bruggen */
#include "rudl.h"

#ifdef __cplusplus
extern "C" {
#endif

VALUE classSurface;
VALUE classDisplaySurface;
VALUE classSurfaceArray;
VALUE classRect;
VALUE classUserRect;
VALUE classDraw;
VALUE classImage;
VALUE classCursors;
VALUE classRGBA;
VALUE classCollisionMap;

VALUE classSurfacesLostException;

VALUE classResizeEvent;
VALUE classActiveEvent;
VALUE classQuitEvent;

extern ID id_rect, id_atx, id_aty, id_atw, id_ath;

extern void initVideoClasses();
extern void initVideoDisplaySurfaceClasses();
extern void initVideoSurfaceClasses();
extern void initVideoRectClasses();
extern void initVideoSDLGFXClasses();
extern void initVideoSGEClasses();
extern void initVideo();

extern VALUE createSurfaceObject(SDL_Surface* surface);
extern SDL_Surface* retrieveSurfacePointer(VALUE self);
extern Uint32 internal_get(SDL_Surface* surface, Sint16 x, Sint16 y);
extern Uint32 internal_nonlocking_get(SDL_Surface* surface, Sint16 x, Sint16 y);

extern Uint32 VALUE2COLOR(VALUE colorObject, SDL_PixelFormat* format);
extern Uint32 VALUE2COLOR_NOMAP(VALUE colorObject);
extern VALUE COLOR2VALUE(Uint32 color, SDL_Surface* surface);
extern void RECT2CRECT(VALUE source, SDL_Rect* destination);
extern void CRECT2RECT(SDL_Rect* source, VALUE destination);
extern void PARAMETER2COORD(VALUE parameter, Sint16* x, Sint16* y);
extern void PARAMETER2CRECT(VALUE arg1, SDL_Rect* rect);

#ifdef __cplusplus
}
#endif
