/* RUDL - a C library wrapping SDL for use in Ruby. Copyright (C) 2001  Danny van Bruggen */
#include "rudl.h"

#ifdef __cplusplus
extern "C" {
#endif

VALUE classSurface;
VALUE classDisplaySurface;
VALUE classSurfaceArray;
VALUE classRect;
VALUE moduleRectangular;
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
extern void quitVideo();

extern VALUE createSurfaceObject(SDL_Surface* surface);
extern SDL_Surface* retrieveSurfacePointer(VALUE self);
extern Uint32 __inline__ internal_get(SDL_Surface* surface, Sint16 x, Sint16 y);
extern Uint32 __inline__ internal_nonlocking_get(SDL_Surface* surface, Sint16 x, Sint16 y);

extern VALUE surface_new(int argc, VALUE* argv, VALUE self);

extern Uint32 VALUE2COLOR(VALUE colorObject, SDL_PixelFormat* format);
extern Uint32 VALUE2COLOR_NOMAP(VALUE colorObject);

extern VALUE COLOR2VALUE(Uint32 color, SDL_Surface* surface);
extern void PARAMETER2COORD(VALUE parameter, Sint16* x, Sint16* y);
extern void PARAMETER2CRECT(VALUE arg1, SDL_Rect* rect);
extern VALUE new_rect(double x, double y, double w, double h);
extern VALUE new_rect_from_SDL_Rect(SDL_Rect* source);

#define GET_SURFACE SDL_Surface* surface; Data_Get_Struct(self, SDL_Surface, surface);

#ifdef __cplusplus
}
#endif
