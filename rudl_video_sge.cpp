/* RUDL - a C library wrapping SDL for use in Ruby. Copyright (C) 2001, 2002  Danny van Bruggen */
#ifdef HAVE_SGE_H
#include "rudl_events.h"
#include "rudl_video.h"
#include "sge_primitives.h"

#ifdef __cplusplus
extern "C" {
#endif

static VALUE surface_horizontal_line(VALUE self, VALUE coord, VALUE endx, VALUE color)
{
	Sint16 x,y;
	DEBUG_S("sge");
	PARAMETER2COORD(coord, &x, &y);
	sge_HLine(retrieveSurfacePointer(self), x, NUM2Sint16(endx), y, VALUE2COLOR_NOMAP(color));
	return self;
}

static VALUE surface_vertical_line(VALUE self, VALUE coord, VALUE endy, VALUE color)
{
	Sint16 x,y;
	PARAMETER2COORD(coord, &x, &y);
	sge_VLine(retrieveSurfacePointer(self), x, y, NUM2Sint16(endy), VALUE2COLOR_NOMAP(color));
	return self;
}

///////////////////////////////// INIT
void initVideoSGEClasses()
{
	rb_undef_method(classSurface, "horizontal_line");
	rb_define_method(classSurface, "horizontal_line", VALUEFUNC(surface_horizontal_line), 3);
	rb_undef_method(classSurface, "vertical_line");
	rb_define_method(classSurface, "vertical_line", VALUEFUNC(surface_vertical_line), 3);
}

#ifdef __cplusplus
}
#endif

#endif
