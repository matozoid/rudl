/* RUDL - a C library wrapping SDL for use in Ruby. Copyright (C) 2001, 2002  Danny van Bruggen */
#include "rudl_events.h"
#include "rudl_video.h"

#ifdef HAVE_SDL_ROTOZOOM_H
#include "SDL_rotozoom.h"
#endif

#ifdef HAVE_SDL_GFXPRIMITIVES_H
#include "SDL_gfxPrimitives.h"
#endif

///////////////////////////////// SDL_GFX: ROTOZOOM
#ifdef HAVE_SDL_ROTOZOOM_H
/*
=begin
<<< docs/head
= SDL_gfx functions
SDL_gfx was written by Andreas Schiffler.
See ((<URL:http://de.ferzkopp.net/>))
== SDL_gfx: SDL_rotozoom
--- Surface#rotozoom( angle, zoom, smooth )
Returns a new surface that is rotated ((|angle|)) degrees and zoomed
((|zoom|)) times (fractions are OK).
This method returns a 32 bit surface.
Exception: for now it returns an 8 bit surface when fed an 8 bit surface.
If ((|smooth|)) is true and the surface is not 8 bits,
bilinear interpolation will be applied, resulting in a smoother image.
=end */
static VALUE surface_rotozoom(VALUE self, VALUE angle, VALUE zoom, VALUE smooth)
{
	return createSurfaceObject(rotozoomSurface(retrieveSurfacePointer(self), NUM2DBL(angle), NUM2DBL(zoom), NUM2BOOL(smooth)));
}

/*
=begin
--- Surface#zoom( zoom_horizontal, zoom_vertical, smooth )
Returns a new surface that is zoomed.
1.0 doesn't zoom, bigger than 1.0 zooms in, smaller than 1.0 zooms out.
This method returns a 32 bit surface.
Exception: for now it returns an 8 bit surface when fed an 8 bit surface.
If ((|smooth|)) is true and the surface is not 8 bits,
bilinear interpolation will be applied, resulting in a smoother image.
(The last two methods are from Andreas Schiffler's SDL_rotozoom, aschiffler@home.com)
=end */
static VALUE surface_zoom(VALUE self, VALUE zoom_x, VALUE zoom_y, VALUE smooth)
{
	return createSurfaceObject(zoomSurface(retrieveSurfacePointer(self), NUM2DBL(zoom_x), NUM2DBL(zoom_y), NUM2BOOL(smooth)));
}
#endif

///////////////////////////////// SDL_GFX: GFXPRIMITIVES
#ifdef HAVE_SDL_GFXPRIMITIVES_H
/*
=begin
== SDL_gfx: SDL_gfxPrimitives
For these methods, "antialiased" means that drawing is done with many shades of the
requested color to simulate 
--- Surface#plot( coordinate, color )
--- Surface#[ x, y ]= color
These methods access single pixels on a surface.
((|plot|)) or ((|[]=|)) set a pixel to ((|color|)) at ((|coordinate|)).
((|get|)) or ((|[]|)) get the color of a pixel.
These methods require the surface to be locked if neccesary.
((|[]=|)) and ((|[]|)) are the only methods in RUDL that take a seperate x and y coordinate.
=end */

static VALUE surface_plot(VALUE self, VALUE coordinate, VALUE color)
{
	Sint16 x,y;
	PARAMETER2COORD(coordinate, &x, &y);
	if(pixelColor(retrieveSurfacePointer(self), x, y, VALUE2COLOR_NOMAP(color))) SDL_RAISE_S("failed");
	return self;
}


static VALUE surface_array_plot(VALUE self, VALUE x, VALUE y, VALUE color)
{
	if(pixelColor(retrieveSurfacePointer(self), NUM2Sint16(x), NUM2Sint16(y), VALUE2COLOR_NOMAP(color))) SDL_RAISE_S("failed");
	return self;
}

/*
=begin
--- Surface#horizontal_line( coord, endx, color )
=end */
static VALUE surface_horizontal_line(VALUE self, VALUE coord, VALUE endx, VALUE color)
{
	Sint16 x,y;
	PARAMETER2COORD(coord, &x, &y);
	if(hlineColor(retrieveSurfacePointer(self), x, NUM2Sint16(endx), y, VALUE2COLOR_NOMAP(color))) SDL_RAISE_S("failed");
	return self;
}

/*
=begin
--- Surface#vertical_line( coord, endy, color )
=end */
static VALUE surface_vertical_line(VALUE self, VALUE coord, VALUE endy, VALUE color)
{
	Sint16 x,y;
	PARAMETER2COORD(coord, &x, &y);
	if(vlineColor(retrieveSurfacePointer(self), x, y, NUM2Sint16(endy), VALUE2COLOR_NOMAP(color))) SDL_RAISE_S("failed");
	return self;
}

/*
=begin
--- Surface#rectangle( rect, color )
=end */
static VALUE surface_rectangle(VALUE self, VALUE rectObject, VALUE color)
{
	SDL_Rect rect;
	PARAMETER2CRECT(rectObject, &rect);
	if(rectangleColor(retrieveSurfacePointer(self), rect.x, rect.y, (Sint16)(rect.x+rect.w-1), (Sint16)(rect.y+rect.h-1), VALUE2COLOR_NOMAP(color))) SDL_RAISE_S("failed");
	return self;
}

/*
=begin
--- Surface#filled_rectangle( rect, color )
=end */
static VALUE surface_filled_rectangle(VALUE self, VALUE rectObject, VALUE color)
{
	SDL_Rect rect;
	PARAMETER2CRECT(rectObject, &rect);
	if(boxColor(retrieveSurfacePointer(self), rect.x, rect.y, (Sint16)(rect.x+rect.w-1), (Sint16)(rect.y+rect.h-1), VALUE2COLOR_NOMAP(color))) SDL_RAISE_S("failed");
	return self;
}

/*
=begin
--- Surface#line( coord1, coord2, color )
=end */
static VALUE surface_line(VALUE self, VALUE coord1, VALUE coord2, VALUE color)
{
	Sint16 x1,y1,x2,y2;
	PARAMETER2COORD(coord1, &x1, &y1);
	PARAMETER2COORD(coord2, &x2, &y2);
	if(lineColor(retrieveSurfacePointer(self), x1, y1, x2, y2, VALUE2COLOR_NOMAP(color))) SDL_RAISE_S("failed");
	return self;
}

/*
=begin
--- Surface#antialiased_line( coord1, coord2, color )
=end */
static VALUE surface_antialiased_line(VALUE self, VALUE coord1, VALUE coord2, VALUE color)
{
	Sint16 x1,y1,x2,y2;
	PARAMETER2COORD(coord1, &x1, &y1);
	PARAMETER2COORD(coord2, &x2, &y2);
	if(aalineColor(retrieveSurfacePointer(self), x1, y1, x2, y2, VALUE2COLOR_NOMAP(color))) SDL_RAISE_S("failed");
	return self;
}

/*
=begin
--- Surface#circle( coord, radius, color )
=end */
static VALUE surface_circle(VALUE self, VALUE coord, VALUE r, VALUE color)
{
	Sint16 x,y;
	PARAMETER2COORD(coord, &x, &y);
	if(circleColor(retrieveSurfacePointer(self), x, y, NUM2Sint16(r), VALUE2COLOR_NOMAP(color))) SDL_RAISE_S("failed");
	return self;
}

/*
=begin
--- Surface#filled_circle( coord, radius, color )
=end */
static VALUE surface_filled_circle(VALUE self, VALUE coord, VALUE r, VALUE color)
{
	Sint16 x,y;
	PARAMETER2COORD(coord, &x, &y);
	if(filledCircleColor(retrieveSurfacePointer(self), x, y, NUM2Sint16(r), VALUE2COLOR_NOMAP(color))) SDL_RAISE_S("failed");
	return self;
}

/*
=begin
--- Surface#antialiased_circle( coord, radius, color )
=end */
static VALUE surface_antialiased_circle(VALUE self, VALUE coord, VALUE r, VALUE color)
{
	Sint16 x,y;
	PARAMETER2COORD(coord, &x, &y);
	if(aacircleColor(retrieveSurfacePointer(self), x, y, NUM2Sint16(r), VALUE2COLOR_NOMAP(color))) SDL_RAISE_S("failed");
	return self;
}

/*
=begin
--- Surface#filled_pie( coord, radius, start, end, color )
=end */
static VALUE surface_filled_pie(VALUE self, VALUE coord, VALUE r, VALUE start, VALUE end, VALUE color)
{
	Sint16 x,y;
	PARAMETER2COORD(coord, &x, &y);
	if(filledpieColor(retrieveSurfacePointer(self), x, y, NUM2Sint16(r), NUM2Sint16(start), NUM2Sint16(end), VALUE2COLOR_NOMAP(color))) SDL_RAISE_S("failed");
	return self;
}


/*
=begin
--- Surface#ellipse( coord, radius_x, radius_y, color )
=end */
static VALUE surface_ellipse(VALUE self, VALUE coord, VALUE rx, VALUE ry, VALUE color)
{
	Sint16 x,y;
	PARAMETER2COORD(coord, &x, &y);
	if(ellipseColor(retrieveSurfacePointer(self), x, y, NUM2Sint16(rx), NUM2Sint16(ry), VALUE2COLOR_NOMAP(color))) SDL_RAISE_S("failed");
	return self;
}

/*
=begin
--- Surface#antialiased_ellipse( coord, radius_x, radius_y, color )
=end */
static VALUE surface_antialiased_ellipse(VALUE self, VALUE coord, VALUE rx, VALUE ry, VALUE color)
{
	Sint16 x,y;
	PARAMETER2COORD(coord, &x, &y);
	if(aaellipseColor(retrieveSurfacePointer(self), x, y, NUM2Sint16(rx), NUM2Sint16(ry), VALUE2COLOR_NOMAP(color))) SDL_RAISE_S("failed");
	return self;
}

/*
=begin
--- Surface#filled_ellipse( coord, radius_x, radius_y, color )
These methods are thought to be self-explanatory.
Filled_rectangle is a lot like fill.
Fill comes from SDL, filled_rectangle from SDL_gfx,
choose whichever you like best.
=end */
static VALUE surface_filled_ellipse(VALUE self, VALUE coord, VALUE rx, VALUE ry, VALUE color)
{
	Sint16 x,y;
	PARAMETER2COORD(coord, &x, &y);
	if(filledEllipseColor(retrieveSurfacePointer(self), x, y, NUM2Sint16(rx), NUM2Sint16(ry), VALUE2COLOR_NOMAP(color))) SDL_RAISE_S("failed");
	return self;
}
/*
=begin
--- Surface#polygon( coord_list, color )
=end */

static VALUE surface_polygon(VALUE self, VALUE coordlist, VALUE color)
{
	int numpoints=RARRAY(coordlist)->len;
	Sint16 *x=malloc(sizeof(Sint16)*numpoints);
	Sint16 *y=malloc(sizeof(Sint16)*numpoints);
	int i;
	VALUE tmp;
	
	for(i=0; i<numpoints; i++){
		tmp=rb_ary_entry(rb_ary_entry(coordlist, i), 0);	x[i]=NUM2Sint16(tmp);
		tmp=rb_ary_entry(rb_ary_entry(coordlist, i), 1);	y[i]=NUM2Sint16(tmp);
	}

	if(polygonColor(retrieveSurfacePointer(self), x, y, numpoints, VALUE2COLOR_NOMAP(color)))  SDL_RAISE_S("failed");
	
	free(x);
	free(y);
	return self;
}

/*
=begin
--- Surface#filled_polygon( coord_list, color )
=end */
static VALUE surface_filled_polygon(VALUE self, VALUE coordlist, VALUE color)
{
	int numpoints=RARRAY(coordlist)->len;
	Sint16 *x=malloc(sizeof(Sint16)*numpoints);
	Sint16 *y=malloc(sizeof(Sint16)*numpoints);
	int i;
	VALUE tmp;
	
	for(i=0; i<numpoints; i++){
		tmp=rb_ary_entry(rb_ary_entry(coordlist, i), 0);	x[i]=NUM2Sint16(tmp);
		tmp=rb_ary_entry(rb_ary_entry(coordlist, i), 1);	y[i]=NUM2Sint16(tmp);
	}

	if(filledPolygonColor(retrieveSurfacePointer(self), x, y, numpoints, VALUE2COLOR_NOMAP(color)))  SDL_RAISE_S("failed");
	
	free(x);
	free(y);
	return self;
}

/*
=begin
--- Surface#antialiased_polygon( coord_list, color)
The polygon methods take an array of [x,y], like [[10,10],[40,60],[16,66]].
=end */
static VALUE surface_antialiased_polygon(VALUE self, VALUE coordlist, VALUE color)
{
	int numpoints=RARRAY(coordlist)->len;
	Sint16 *x=malloc(sizeof(Sint16)*numpoints);
	Sint16 *y=malloc(sizeof(Sint16)*numpoints);
	int i;
	VALUE tmp;
	
	for(i=0; i<numpoints; i++){
		tmp=rb_ary_entry(rb_ary_entry(coordlist, i), 0);	x[i]=NUM2Sint16(tmp);
		tmp=rb_ary_entry(rb_ary_entry(coordlist, i), 1);	y[i]=NUM2Sint16(tmp);
	}

	if(aapolygonColor(retrieveSurfacePointer(self), x, y, numpoints, VALUE2COLOR_NOMAP(color)))  SDL_RAISE_S("failed");
	
	free(x);
	free(y);
	return self;
}

/*
=begin
--- Surface#print( coord, text, color )
Puts ((|text|)) on the surface in a monospaced 8x8 standard old ASCII font.
=end */
static VALUE surface_print(VALUE self, VALUE coord, VALUE text, VALUE color)
{
	Sint16 x,y;
	PARAMETER2COORD(coord, &x, &y);
	if(stringColor(retrieveSurfacePointer(self), x, y, STR2CSTR(text), VALUE2COLOR_NOMAP(color))) SDL_RAISE_S("failed");
	return self;
}
#endif

#ifdef HAVE_SDL_IMAGEFILTER_H
/*
Unsupported for now (don't know how they work yet)
== SDL_gfx: SDL_imageFilter
--- Surface#filterMMXdetect
--- Surface#filterEnableMMX( true_or_false )
--- Surface#filterAdd: D = saturation255(S1 + S2)
--- Surface#filterMean: D = S1/2 + S2/2
SDL_imageFilterSub: D = saturation0(S1 - S2)
SDL_imageFilterAbsDiff: D = | S1 - S2 |
SDL_imageFilterMult: D = saturation(S1 * S2)
SDL_imageFilterMultNor: D = S1 * S2   (non-MMX)
SDL_imageFilterMultDivby2: D = saturation255(S1/2 * S2)
SDL_imageFilterMultDivby4: D = saturation255(S1/2 * S2/2)
SDL_imageFilterBitAnd: D = S1 & S2
SDL_imageFilterBitOr: D = S1 | S2
SDL_imageFilterDiv: D = S1 / S2   (non-MMX)
SDL_imageFilterBitNegation: D = !S
SDL_imageFilterAddByte: D = saturation255(S + C)
SDL_imageFilterAddByteToHalf: D = saturation255(S/2 + C)
SDL_imageFilterSubByte: D = saturation0(S - C)
SDL_imageFilterShiftRight: D = saturation0(S >> N)
SDL_imageFilterMultByByte: D = saturation255(S * C)
SDL_imageFilterShiftRightAndMultByByte: D = saturation255((S >> N) * C)
SDL_imageFilterShiftLeftByte: D = (S << N)
SDL_imageFilterShiftLeft: D = saturation255(S << N)
SDL_imageFilterBinarizeUsingThreshold: D = S >= T ? 255:0
SDL_imageFilterClipToRange: D = (S >= Tmin) & (S <= Tmax) 255:0
SDL_imageFilterNormalizeLinear: D = saturation255((Nmax - Nmin)/(Cmax - Cmin)*(S - Cmin) + Nmin)
 !!! NO C-ROUTINE FOR THESE FUNCTIONS YET !!!

SDL_imageFilterConvolveKernel3x3Divide: Dij = saturation0and255( ... )
SDL_imageFilterConvolveKernel5x5Divide: Dij = saturation0and255( ... )
SDL_imageFilterConvolveKernel7x7Divide: Dij = saturation0and255( ... )
SDL_imageFilterConvolveKernel9x9Divide: Dij = saturation0and255( ... )
SDL_imageFilterConvolveKernel3x3ShiftRight: Dij = saturation0and255( ... )
SDL_imageFilterConvolveKernel5x5ShiftRight: Dij = saturation0and255( ... )
SDL_imageFilterConvolveKernel7x7ShiftRight: Dij = saturation0and255( ... )
SDL_imageFilterConvolveKernel9x9ShiftRight: Dij = saturation0and255( ... )
SDL_imageFilterSobelX: Dij = saturation255( ... )

int SDL_imageFilterAdd (unsigned char *Src1, unsigned char *Src2, unsigned char *Dest, int length);
int SDL_imageFilterMean(unsigned char *Src1, unsigned char *Src2, unsigned char *Dest, int length);
int SDL_imageFilterSub(unsigned char *Src1, unsigned char *Src2, unsigned char *Dest, int length);
int SDL_imageFilterAbsDiff(unsigned char *Src1, unsigned char *Src2, unsigned char *Dest, int length);
int SDL_imageFilterMult(unsigned char *Src1, unsigned char *Src2, unsigned char *Dest, int length);
int SDL_imageFilterMultNor(unsigned char *Src1, unsigned char *Src2, unsigned char *Dest, int length);
int SDL_imageFilterMultDivby2(unsigned char *Src1, unsigned char *Src2, unsigned char *Dest, int length);
int SDL_imageFilterMultDivby4(unsigned char *Src1, unsigned char *Src2, unsigned char *Dest, int length);
int SDL_imageFilterBitAnd(unsigned char *Src1, unsigned char *Src2, unsigned char *Dest, int length);
int SDL_imageFilterBitOr(unsigned char *Src1, unsigned char *Src2, unsigned char *Dest, int length);
int SDL_imageFilterDiv(unsigned char *Src1, unsigned char *Src2, unsigned char *Dest, int length);
int SDL_imageFilterBitNegation(unsigned char *Src1, unsigned char *Dest, int length);
int SDL_imageFilterAddByte(unsigned char *Src1, unsigned char *Dest, int length, unsigned char C);
int SDL_imageFilterAddByteToHalf(unsigned char *Src1, unsigned char *Dest, int length, unsigned char C);
int SDL_imageFilterSubByte(unsigned char *Src1, unsigned char *Dest, int length, unsigned char C);
int SDL_imageFilterShiftRight(unsigned char *Src1, unsigned char *Dest, int length, unsigned char N);
int SDL_imageFilterMultByByte(unsigned char *Src1, unsigned char *Dest, int length, unsigned char C);
int SDL_imageFilterShiftRightAndMultByByte(unsigned char *Src1, unsigned char *Dest, int length, unsigned char N, unsigned char C);
int SDL_imageFilterShiftLeftByte(unsigned char *Src1, unsigned char *Dest, int length, unsigned char N);
int SDL_imageFilterShiftLeft(unsigned char *Src1, unsigned char *Dest, int length, unsigned char N);
int SDL_imageFilterBinarizeUsingThreshold(unsigned char *Src1, unsigned char *Dest, int length, unsigned char T);
int SDL_imageFilterClipToRange(unsigned char *Src1, unsigned char *Dest, int length, unsigned char Tmin, unsigned char Tmax);
int SDL_imageFilterNormalizeLinear(unsigned char *Src1, unsigned char *Dest, int length, int Cmin, int Cmax, int Nmin, int Nmax);
int SDL_imageFilterConvolveKernel3x3Divide(unsigned char *Src, unsigned char *Dest, int rows, int columns, signed short *Kernel, unsigned char Divisor);
int SDL_imageFilterConvolveKernel5x5Divide(unsigned char *Src, unsigned char *Dest, int rows, int columns, signed short *Kernel, unsigned char Divisor);
int SDL_imageFilterConvolveKernel7x7Divide(unsigned char *Src, unsigned char *Dest, int rows, int columns, signed short *Kernel, unsigned char Divisor);
int SDL_imageFilterConvolveKernel9x9Divide(unsigned char *Src, unsigned char *Dest, int rows, int columns, signed short *Kernel, unsigned char Divisor);
int SDL_imageFilterConvolveKernel3x3ShiftRight(unsigned char *Src, unsigned char *Dest, int rows, int columns, signed short *Kernel, unsigned char NRightShift);
int SDL_imageFilterConvolveKernel5x5ShiftRight(unsigned char *Src, unsigned char *Dest, int rows, int columns, signed short *Kernel, unsigned char NRightShift);
int SDL_imageFilterConvolveKernel7x7ShiftRight(unsigned char *Src, unsigned char *Dest, int rows, int columns, signed short *Kernel, unsigned char NRightShift);
int SDL_imageFilterConvolveKernel9x9ShiftRight(unsigned char *Src, unsigned char *Dest, int rows, int columns, signed short *Kernel, unsigned char NRightShift);
int SDL_imageFilterSobelX(unsigned char *Src, unsigned char *Dest, int rows, int columns);
*/
#endif
///////////////////////////////// INIT
void initVideoSDLGFXClasses()
{
	//classSurfaceArray=rb_define_class_under(moduleRUDL, "SurfaceArray", rb_cObject);
	#ifdef HAVE_SDL_GFXPRIMITIVES_H
	rb_define_method(classSurface, "plot", surface_plot, 2);
	rb_define_method(classSurface, "[]=", surface_array_plot, 3);
	rb_define_method(classSurface, "horizontal_line", surface_horizontal_line, 3);
	rb_define_method(classSurface, "vertical_line", surface_vertical_line, 3);
	rb_define_method(classSurface, "rectangle", surface_rectangle, 2);
	rb_define_method(classSurface, "filled_rectangle", surface_filled_rectangle, 2);
	rb_define_method(classSurface, "line", surface_line, 3);
	rb_define_method(classSurface, "antialiased_line", surface_antialiased_line, 3);
	rb_define_method(classSurface, "circle", surface_circle, 3);
	rb_define_method(classSurface, "filled_circle", surface_filled_circle, 3);
	rb_define_method(classSurface, "antialiased_circle", surface_antialiased_circle, 3);
	rb_define_method(classSurface, "filled_pie", surface_filled_pie, 5);
	rb_define_method(classSurface, "ellipse", surface_ellipse, 4);
	rb_define_method(classSurface, "filled_ellipse", surface_filled_ellipse, 4);
	rb_define_method(classSurface, "antialiased_ellipse", surface_antialiased_ellipse, 4);
	rb_define_method(classSurface, "polygon", surface_polygon, 2);
	rb_define_method(classSurface, "filled_polygon", surface_filled_polygon, 2);
	rb_define_method(classSurface, "antialiased_polygon", surface_antialiased_polygon, 2);
	rb_define_method(classSurface, "print", surface_print, 3);
	#endif
	#ifdef HAVE_SDL_ROTOZOOM_H
	rb_define_method(classSurface, "rotozoom", surface_rotozoom, 3);
	rb_define_method(classSurface, "zoom", surface_zoom, 3);
	#endif
	#ifdef HAVE_SDL_IMAGEFILTER_H
	#endif
}
