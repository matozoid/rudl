/* overlay.c 
**
**      name: Overlay library for Ruby / RUDL GUIs
**    author: Matthew Bloch <matthew@bytemark.co.uk>
** copyright: Bytemark Computer Consulting, 2002
*/

/********************************************** Declarations and includes  */

#include <limits.h>
#include <stdio.h>
#include "rudl.h"

#ifdef DEBUG_RUDL
#  define DEBUGF printf
#else
#  define DEBUGF if(0) __dummyprintf
void __dummyprintf(char *s, ...) { }
#endif

#ifdef DEBUG_RUDL
#  define ASSERT(c) _assert(__FILE__, __LINE__, (c))
#else
#  define ASSERT(c) 
#endif

#ifdef FORTIFY
#include "fortify.h"
#endif

static __inline__ void* MALLOC(size_t s)
{
  char *m;
  DEBUGF("malloc: malloc %d\n", s);
  m = malloc(s);
  DEBUGF("malloc: zero\n");
  memset(m, 0, s);
  DEBUGF("malloc: return\n");
  return m;
}

static __inline__ void* REALLOC(void *m, size_t s)
{
  char *nm = realloc(m, s);
  return nm;
}

static __inline__ void
_assert(char *f, int l, int cond)
{
  if (cond)
    return;
  rb_bug("assert failed at %s:%d", f, l);
}

#define FREE(a)  free(a)


/**************************************************************** ExtArray */

/** Structure describing an extensible array of objects */
typedef struct _ExtArray
{
  int dummy[8];
  int size;
  int cur;    /** The next free object in the list */
  int max;    /** The highest valid index in the list */
  void **list; /** The list */
}
ExtArray;

static __inline__ void
ExtArray_check_consistency(ExtArray *ea)
{
	DEBUG_I(100);
	ASSERT(Fortify_CheckPointer(ea->list));
	if (ea->list)
	{
		ASSERT(Fortify_CheckPointer(ea));
		ASSERT(Fortify_CheckPointer(ea->list));
	}
	else
	{
		DEBUG_I(666);
		ASSERT(ea->cur == 0);
		DEBUG_I(667);
		ASSERT(ea->max == 0);
		DEBUG_I(668);
	}
	DEBUG_I(101);
	ASSERT(ea->cur <= ea->max);
	ASSERT(ea->size < 256);
	ASSERT(ea->size > 0);
	ASSERT(ea->cur >= 0);
	ASSERT(ea->max >= 0);
	DEBUG_I(102);
	if (ea->size == sizeof(void*))
	{
		int c;
		for (c=0; c < ea->cur; c++)
		{
			ASSERT(Fortify_CheckPointer(ea->list[c]));
			ASSERT(ea->list[c]);
		}
	}
	DEBUG_I(103);
}

static void*
ExtArray_addr(ExtArray *ea, int i)
{
  ASSERT(i>=0);
  ASSERT(i<ea->cur);
  ExtArray_check_consistency(ea);
  
  return (((char*)ea->list) + ea->size * i);
}

static void*
ExtArray_addr_nocheck(ExtArray *ea, int i)
{
  ExtArray_check_consistency(ea);
  return (((char*)ea->list) + ea->size * i);
}

static __inline__ int
ExtArray_length(ExtArray *ea)
{
	DEBUG_I(15);
	ExtArray_check_consistency(ea);
	DEBUG_I(14);
	return ea->cur;
}

static void
ExtArray_add(ExtArray *ea, void *ptr)
{  
  ExtArray_check_consistency(ea);
  
  if (ea->max == ea->cur)
  {
    int newmax;
    
    /* Delayed initialisation really */
    if (!ea->size)
      ea->size = sizeof(void*);
    
    newmax = ea->max * 2;
    if (newmax == 0)
      newmax = 32;
    ea->list = REALLOC(ea->list, ea->size * newmax);
    DEBUGF("realloced to size %d * %d\n", ea->size, newmax);
    ea->max = newmax;
  }
  
  if (ea->size == sizeof(void*))
    ea->list[ea->cur] = ptr;
  else
  {
    char *c1 = ExtArray_addr_nocheck(ea, ea->cur);
    char *c2 = ptr;
    int i;
    
    for (i=0; i<ea->size; i++)
    {
      char tocopy = c2[i];
      c1[i] = tocopy;
    }
  }
        
  ea->cur++;
  ExtArray_check_consistency(ea);  
}

static ExtArray*
ExtArray_new(int object_size)
{
  ExtArray *ea = (ExtArray*) MALLOC(sizeof(ExtArray));
  ea->size = object_size;
  ea->cur = 0;
  ea->max = 10;
  ea->list = (void**) MALLOC(object_size * 10);

  ExtArray_check_consistency(ea);  
  
  return ea;
}

static void
ExtArray_free(ExtArray *ea)
{
  ExtArray_check_consistency(ea);  
  FREE(ea->list);
  FREE(ea);
}

static void
ExtArray_add_exclusive(ExtArray *ea, void *ptr)
{
  int c;

  ExtArray_check_consistency(ea);  
  
  if (ea->size == sizeof(void*))
  {
    for (c=0; c<ea->cur; c++)
      if (ea->list[c] == ptr)
	return;
  }
  else
  {
    for (c=0; c<ea->cur; c++)
      if (memcmp(ExtArray_addr(ea, c), ptr, ea->size) == 0)
	return;
  }
  
  ExtArray_add(ea, ptr);
  
}

static void
ExtArray_delete(ExtArray *ea, void *ptr)
{
  int c;
  
  ExtArray_check_consistency(ea);  

  DEBUGF("ea->size = %d\n", ea->size);
  if (ea->size == sizeof(void*))
  {
    DEBUGF("searching through %d for %08x\n", ea->cur, ptr);
    for (c=0; c<ea->cur; c++)
    {
      if (ea->list[c] == ptr)
      {
	int d;
	DEBUGF("found at %d\n", c);
	for (d = c; d < ea->cur-1; d++)
	{
	  ea->list[d] = ea->list[d+1];
	}
	ea->cur--;
	return;
      }
    }
  }
  else
  {
    for (c=0; c<ea->cur; c++)
    {
      if (!(memcmp((char*) ea->list + ea->size * c, ptr, ea->size)))
      {
	int d;
	for (d = c; d < ea->cur-1; d++)
	{
	  memcpy(ExtArray_addr(ea, d), ExtArray_addr(ea, d+1), ea->size);
	}
	ea->cur--;
	return;
      }
    }
  }
  
  ExtArray_check_consistency(ea);  
}

/*************************************************************** Rectangle */

/** Structure describing an individual point */
typedef struct _Pos
{
  int x;
  int y;
}
Pos;

/** Structure describing an area */
typedef struct _Rect
{
  Pos tl;
  Pos br;
}
Rect;

#define Rect_invalid(r) \
  ((r)->br.x - (r)->tl.x <= 0 || (r)->br.y - (r)->tl.y <= 0)

#define Rect_get_width(r)  ((r)->br.x - (r)->tl.x)
#define Rect_get_height(r) ((r)->br.y - (r)->tl.y)

#define RECT_END_MARKER INT_MAX

/* save typing with DEBUGFs */
#define PRECT(r) (r)->tl.x, (r)->tl.y, (r)->br.x, (r)->br.y

static __inline__ int
Rect_same(Rect *r1, Rect *r2)
{
  if (!r1 && !r2)
    return 1;
  
  if (r1 && r2)
    return 
      r1->tl.x == r2->tl.x &&
      r1->tl.y == r2->tl.y &&
      r1->br.x == r2->br.x &&
      r1->br.y == r2->br.y;
  
  
  return 0;
}

static void
Rect_subtract(Rect *dst, Rect *r1, Rect *r2)
{
  dst->tl.x = r1->tl.x - r2->tl.x;
  dst->tl.y = r1->tl.y - r2->tl.y;
  dst->br.x = r1->br.x - r2->tl.x;
  dst->br.y = r1->br.y - r2->tl.y;
  
  
}

static __inline__ void 
Rect_set_equals(Rect *to, Rect *from)
{
  memcpy(to, from, sizeof(Rect));
  
}

static __inline__ void 
Rect_set(Rect *r, int x0, int y0, int w, int h)
{
  r->tl.x = x0;
  r->tl.y = y0;
  r->br.x = x0 + w;
  r->br.y = y0 + h;
  
}

static __inline__ void 
Rect_set_width(Rect *r, int w)
{
  r->br.x = r->tl.x + w;
  
}

static __inline__ void 
Rect_set_height(Rect *r, int h)
{
  r->br.y = r->tl.y + h;
  
}

static __inline__ int 
sortandprune(int *list, int length)
{
  int a;
  int swapped;
  
  swapped = 1;
  
  while (swapped)
  {
    swapped = 0;
    for (a = 0; a < (length-1); a++)
    {
      if (list[a+1] < list[a])
      {
	int t = list[a];
	list[a] = list[a+1];
	list[a+1] = t;
	swapped = 1;
      }
    }
  }
  
  for (a = 0; a < (length-1); a++)
  {
    if (list[a] == list[a+1])
    {
      int b;
      
      for (b = a; b < (length-1); b++)
	list[b] = list[b+1];
      
      length--;
    }
  }
  
  
  return length;
}

static __inline__ void
Rect_overlap_details(Rect *self, Rect *other, 
                     Rect *overlap, Rect *inside, Rect *outside)
{
  int xc, yc, xl, yl, xr[4], yr[4];
  int sx0, sy0, sx1, sy1, ox0, oy0, ox1, oy1;
  int c;
  
  Rect *p_overlap = overlap, *p_inside = inside, *p_outside = outside;
    
  xr[0] = sx0 = self->tl.x;
  xr[1] = sx1 = self->br.x;
  xr[2] = ox0 = other->tl.x;
  xr[3] = ox1 = other->br.x;
  yr[0] = sy0 = self->tl.y;
  yr[1] = sy1 = self->br.y;
  yr[2] = oy0 = other->tl.y;
  yr[3] = oy1 = other->br.y;

  DEBUGF("overlap details: self (%d,%d,%d,%d) other (%d,%d,%d,%d)\n", sx0, sy0, sx1, sy1, ox0, oy0, ox1, oy1);
  DEBUGF("pointers at start: %p %p %p\n", overlap, outside, inside);

  xl = sortandprune(xr, 4);
  yl = sortandprune(yr, 4);
  DEBUGF("sorted X: "); for (c=0; c<xl; c++) DEBUGF("%d ", xr[c]); DEBUGF("\n");
  DEBUGF("sorted Y: "); for (c=0; c<yl; c++) DEBUGF("%d ", yr[c]); DEBUGF("\n");
  
  for (xc = 0; xc < xl-1; xc++)
  {
    for (yc = 0; yc < yl-1; yc++)
    {
      int is_inside  = (sx0 <= xr[xc] && sx1 >= xr[xc+1] && sy0 <= yr[yc] && sy1 >= yr[yc+1]);
      int is_outside = (ox0 <= xr[xc] && ox1 >= xr[xc+1] && oy0 <= yr[yc] && oy1 >= yr[yc+1]);
      
      DEBUGF("checking against %d,%d,%d,%d: self %d other %d\n", 
        xr[xc], yr[yc], xr[xc+1], yr[yc+1], is_inside, is_outside);
      
      if (is_inside)
      {
	if (is_outside)
	{
	  DEBUGF("storing both rectangle at %p: %d,%d,%d,%d\n", p_overlap, xr[xc], yr[yc], xr[xc+1], yr[yc+1]);
	  p_overlap->tl.x = xr[xc]; 
	  p_overlap->tl.y = yr[yc]; 
	  p_overlap->br.x = xr[xc+1];
	  p_overlap->br.y = yr[yc+1];
	  p_overlap++;
	}
	else
	{
	  DEBUGF("storing self rectangle at %p: %d,%d,%d,%d\n", p_inside, xr[xc], yr[yc], xr[xc+1], yr[yc+1]);
	  p_inside->tl.x = xr[xc]; 
	  p_inside->tl.y = yr[yc]; 
	  p_inside->br.x = xr[xc+1];
	  p_inside->br.y = yr[yc+1];
	  p_inside++;
	}
      }
      else if (is_outside)
      {
        DEBUGF("storing other rectangle at %p: %d,%d,%d,%d\n", p_outside, xr[xc], yr[yc], xr[xc+1], yr[yc+1]);
	p_outside->tl.x = xr[xc]; 
	p_outside->tl.y = yr[yc]; 
	p_outside->br.x = xr[xc+1];
	p_outside->br.y = yr[yc+1];
	p_outside++;
      }
    }
  }
   
  DEBUGF("storing end markers at both %p other %p self %p\n", p_overlap, p_outside, p_inside);
  p_inside->tl.x  = p_outside->tl.x = p_overlap->tl.x = RECT_END_MARKER;
  
  DEBUGF("overlap: ");  p_overlap = overlap; while (p_overlap->tl.x != RECT_END_MARKER) { DEBUGF("%d,%d,%d,%d ", PRECT(p_overlap)); p_overlap++; } DEBUGF("\n");
  DEBUGF("inside: ");  p_inside = inside; while (p_inside->tl.x != RECT_END_MARKER) { DEBUGF("%d,%d,%d,%d ", PRECT(p_inside)); p_inside++; } DEBUGF("\n");
  DEBUGF("outside: "); p_outside = outside; while (p_outside->tl.x != RECT_END_MARKER) { DEBUGF("%d,%d,%d,%d ", PRECT(p_outside)); p_outside++; } DEBUGF("\n");
  
}

/******************************************************************** Area */

typedef struct _Area
{
  ExtArray rectangles;
}
Area;

static __inline__ void
Area_Init(Area *area)
{
  area->rectangles.size = sizeof(Rect);
}

static __inline__ Area*
Area_new()
{
  Area *area;
  
  DEBUGF("area_new: malloc\n");
  area = (Area*) MALLOC(sizeof(Area));
  DEBUGF("area_new: sizeof\n");
  area->rectangles.size = sizeof(Rect);
  DEBUGF("area_new: return\n");
  
  return area;
}

static void
Area_free(Area *area)
{
  if (area->rectangles.list)
    FREE(area->rectangles.list);
  FREE(area);  
}

#define Area_is_empty(a) ((a)->rectangles.cur == 0)

static __inline__ void
Area_clear(Area *area)
{
  area->rectangles.cur = 0;
  
}

/** Adds a rectangle to an area, without taking other rectangles into account */
static __inline__ void 
Area_append_rectangle(Area *area, Rect *rect)
{
  ExtArray_add_exclusive(&area->rectangles, rect);
  
}

/** Returns the total number of rectangles in an area */
static __inline__ int
Area_rectangles(Area *area)
{
  return ExtArray_length(&area->rectangles);
}

/** Returns a particular rectangle number in an area */
static __inline__ Rect*
Area_rectangle(Area *area, int num)
{
  return (Rect*) ExtArray_addr(&area->rectangles, num);
}

/** Returns a particular rectangle number in an area */
static __inline__ int
Area_length(Area *area)
{
  return ExtArray_length(&area->rectangles);
}

/** Adds a rectangle to an area, merging with nearby rectangles if appropriate 
  */
static int 
Area_add(Area *area, Rect *rect)
{
  int rx0 = rect->tl.x;
  int rx1 = rect->br.x;
  int ry0 = rect->tl.y;
  int ry1 = rect->br.y;
  int merged = 0;
  int at;
  
  if (rx1 - rx0 <= 0 || ry1 - ry0 <= 0)
    return 0; /* bad rectangle! */
  
  /*printf("Adding rect %d,%d,%d,%d to area %08x\n", PRECT(rect), area);*/
  
  if (rect->tl.x < 0)
    return 0;
  
  for (at = 0; at  < Area_rectangles(area); at++)
  {
    Rect *at_rect = Area_rectangle(area, at);
    int lx0 = at_rect->tl.x;
    int ly0 = at_rect->tl.y;
    int lx1 = at_rect->br.x;
    int ly1 = at_rect->br.y;
    
    if (rx0 >= lx0 && rx1 < lx1 && ry0 >= ly0 && ry1 < ly1)
    {
      merged = 1;
      break;
    }
    
    if (lx0 >= rx0 && lx1 < rx1 && ly0 >= ry0 && ly1 < ry1)
    {
      if (!merged)
      {
	at_rect->tl.x = rx0;
	at_rect->br.x = rx1;
	at_rect->tl.y = ry0;
	at_rect->br.y = ry1;
      }
      else
      {
	at_rect->tl.x =
	at_rect->br.x =
	at_rect->tl.y =
	at_rect->br.y = 0;
      }
      continue;
    }
    
    if (lx0 == rx0 && lx1 == rx1 && ((ly0 >= ry0 && ly0 < ry1) || (ly1 >= ry0 && ly1 < ry1)))
    {
      at_rect->tl.y = ry0 < ly0 ? ry0 : ly0;
      at_rect->br.y = ry1 > ly1 ? ry1 : ly1;
      merged = 1;
    }
    
    if (ly0 == ry0 && ly1 == ry1 && ((lx0 >= rx0 && lx0 < rx1) || (lx1 >= rx0 && lx1 < rx1)))
    {
      at_rect->tl.x = rx0 < lx0 ? rx0 : lx0;
      at_rect->br.x = rx1 > lx1 ? rx1 : lx1;
      merged = 1;      
    }
  }
    
  if (!merged)
    Area_append_rectangle(area, rect);

  /*{
    int r;
    printf("area dump:\n");
    for (r = 0; r < Area_length(area); r++)
      printf("rectangle %d/%d = %d,%d,%d,%d\n", r, Area_length(area), PRECT(Area_rectangle(area, r)));
    printf("end area dump\n");
  }*/

    
  
  return !merged;
}

/** Calculates three lists of rectangles for the union of an area and a
  * rectangle: those rectangles in the area, those only in the rectangle
  * and those in both.
  */
static void
Area_overlap_details(Area *area, Rect *rect, 
                     Area *overlap, Area *inside, Area *outside)
{
  Rect overlapTmp[9], insideTmp[9], outsideTmp[9];
  int c;
  
  DEBUGF("area_overlap_details: %d length\n", Area_length(area));
  for (c=0; c < Area_length(area); c++)
  {
    int d;
    Rect *cmp;
    
    DEBUGF("getting rectangle #%d\n", c);
    cmp = Area_rectangle(area, c);
    
    DEBUGF("rect_overlap_details %d,%d,%d,%d with %d,%d,%d,%d\n",
      cmp->tl.x,
      cmp->tl.y,
      cmp->br.x,
      cmp->br.y,
      rect->tl.x,
      rect->tl.y,
      rect->br.x,
      rect->br.y);
    Rect_overlap_details(cmp, rect, overlapTmp, insideTmp, outsideTmp);
    
    DEBUGF("area_overlap_details: appending from buffers\n");
    
    for (d=0; overlapTmp[d].tl.x != RECT_END_MARKER; d++)
      Area_append_rectangle(overlap, overlapTmp+d);
    
    for (d=0; insideTmp[d].tl.x != RECT_END_MARKER; d++)
      Area_append_rectangle(inside, insideTmp+d);
    
    for (d=0; outsideTmp[d].tl.x != RECT_END_MARKER; d++)
      Area_append_rectangle(outside, outsideTmp+d);
    
  }
  
}


/***************************************************************** Overlay */

struct _Overlay;
typedef int(OverlaySizeFunc)(struct _Overlay *ov);
typedef void(OverlayDrawCallback)(struct _Overlay *ov, void *paint_arg, Rect *r);

/** Point on a compass */
typedef enum _Anchor
{
  NW=1, N, NE, W, C, E, SW, S, SE
}
Anchor;

/** Main Overlay structure; a 'magic' area of the screen which can have 
 *  other child overlays associated with it.
 */
typedef struct _Overlay
{
  struct _Overlay   *parent;
  ExtArray          *children;  
  
  Rect               position;
  Rect               position_painted;
  int                xoff;
  int                yoff;
  int                w;
  int                h;
  Anchor             anchor_parent;
  Anchor             anchor;
  
  int                visible;
  int                force_paint;
  int                moved;
  int                show_dirty;
      
  /* Stuff below is initially set to NULL for all but the root Overlay;
  ** child overlays of the root use these fields as a cache, and update
  ** them from the root where necessary.
  */
  Area                *dirty;
  ExtArray            *reposition;

  OverlaySizeFunc     *width;
  OverlaySizeFunc     *height;
  OverlayDrawCallback *update;
  OverlayDrawCallback *paint;
  
#ifdef RUBY_H
  VALUE              ruby;
#endif
}
Overlay;

#define Overlay_is_root(o) !((o)->dirty == 0)

static __inline__ ExtArray* 
Overlay_get_reposition(Overlay *ov)
{
	DEBUG_I(22);
	while (ov->parent){
		ov = ov->parent;
		DEBUG_I(13);
	}
	DEBUG_I(33);
  
  return ov->reposition;
}

static __inline__ Area*
Overlay_get_dirty(Overlay *ov)
{
  while (ov->parent)
    ov = ov->parent;
  
  return ov->dirty;
}

void
Overlay_flag_moved(Overlay *ov)
{
  /* Don't bother adding it to the reposition list if it's not attached to
  ** a parent; it'll be processed when the parent is eventually drawn.
  */
  if (Overlay_get_reposition(ov))
    ExtArray_add_exclusive(Overlay_get_reposition(ov), ov);
  ov->moved = 1;
}

Overlay*
Overlay_new()
{
  Overlay *ov = (Overlay*) MALLOC(sizeof(Overlay));
  memset(ov, 0, sizeof(ov));
  
  ov->children = ExtArray_new(sizeof(Overlay*));
  ov->visible  = 1;
  Overlay_flag_moved(ov);
  
  return ov;
}

static __inline__ Overlay*
Overlay_child(Overlay *ov, int n)
{
  ASSERT(n < ExtArray_length(ov->children));
  ASSERT(n >= 0);
  
  return *((Overlay**)ExtArray_addr(ov->children, n));
}

static __inline__ int
Overlay_width(Overlay *ov)
{
  return ov->width ? ov->width(ov) : ov->w;
}

static __inline__ int
Overlay_height(Overlay *ov)
{
  return ov->height ? ov->height(ov) : ov->h;
}

static __inline__ Overlay*
Overlay_reposition_element(Overlay *ov, int n)
{
  ASSERT(n < ExtArray_length(Overlay_get_reposition(ov)));
  ASSERT(n >= 0);
  
  return *((Overlay**)ExtArray_addr(Overlay_get_reposition(ov), n));
}

void
Overlay_add_child(Overlay *parent, Overlay *child)
{
  
  if (Overlay_is_root(child))
    abort();
  
  ExtArray_add(parent->children, child);
  child->parent = parent;
  Overlay_flag_moved(parent);
}

void
Overlay_del_child(Overlay *parent, Overlay *child)
{
  ExtArray_delete(parent->children, child);
  Area_add(Overlay_get_dirty(parent), &child->position_painted);
}

void
Overlay_free(Overlay *ov)
{
  int c;
  
  if (ov->parent)
    ExtArray_delete(ov->parent->children, ov);
  
  for (c=0; c < ExtArray_length(ov->children); c++)
    Overlay_child(ov, c)->parent = NULL;
  
  if (Overlay_get_reposition(ov))
    ExtArray_delete(Overlay_get_reposition(ov), ov);
  
  if (Overlay_is_root(ov))
  {
    Area_free(Overlay_get_dirty(ov));
    ExtArray_free(Overlay_get_reposition(ov));
  }

  ExtArray_free(ov->children);  
  FREE(ov);
  
}

void
Overlay_calculate_position(Overlay *ov)
{
  int o_width = Overlay_width(ov);
  int o_height = Overlay_height(ov);
  int p_x = (ov->parent ? ov->parent : ov)->position.tl.x;
  int p_y = (ov->parent ? ov->parent : ov)->position.tl.y;
  int p_width = ov->parent ? Overlay_width(ov->parent) : o_width;
  int p_height = ov->parent ? Overlay_height(ov->parent) : o_height;
    
  ASSERT(!ov->parent || !Rect_invalid(&ov->parent->position));

  ov->position.tl.x = ov->xoff + p_x;
  ov->position.tl.y = ov->yoff + p_y;
  
  switch (ov->anchor_parent)
  {
    case N: case C: case S:   ov->position.tl.x += p_width / 2; break;
    case NE: case E: case SE: ov->position.tl.x += p_width;     break;
    default: /* avoid warning */;
  }
  switch (ov->anchor_parent)
  {
    case W: case C: case E:   ov->position.tl.y += p_height / 2; break;
    case SW: case S: case SE: ov->position.tl.y += p_height;     break;
    default: /* avoid warning */;
  }
  switch (ov->anchor)
  {
    case N: case C: case S:   ov->position.tl.x -= o_width / 2;  break;
    case NE: case E: case SE: ov->position.tl.x -= o_width;      break;
    default: /* avoid warning */;
  }
  switch (ov->anchor)
  {
    case W: case C: case E:   ov->position.tl.y -= o_height / 2;  break;
    case SW: case S: case SE: ov->position.tl.y -= o_height;      break;
    default: /* avoid warning */;
  }
  
  
  ov->position.br.x = ov->position.tl.x + o_width;
  ov->position.br.y = ov->position.tl.y + o_height;
  
  /*ASSERT(ov->position.br.x > ov->position.tl.x);
  ASSERT(ov->position.br.y > ov->position.tl.y);*/
  
  DEBUGF("%08x: positioned at %d,%d\n", ov, ov->position.tl.x, ov->position.tl.y);
  
}

static void 
Overlay_reposition_recurse(Overlay *ov)
{
  int valid_position = !Rect_invalid(&ov->position);
  int valid_position_painted = !Rect_invalid(&ov->position_painted);
  
  DEBUGF("%08x: calculating position\n", ov);
  Overlay_calculate_position(ov);
  DEBUGF("%08x: calculated %d,%d,%d,%d", ov, PRECT(&ov->position));
  if (ov->parent)
  {
    DEBUGF(" parent %d,%d,%d,%d", PRECT(&ov->parent->position));
  }
  else
  {
    DEBUGF(" no parent");
  }
  DEBUGF("\n");
  
  if (!valid_position || !Rect_same(&ov->position, &ov->position_painted))
  {
    int c;
    
    DEBUGF("%08x: adding to dirty list %08x\n", ov, Overlay_get_dirty(ov));
    if (valid_position_painted)
      Area_add(Overlay_get_dirty(ov), &ov->position_painted);
    DEBUGF("%08x: adding to dirty list %08x\n", ov, Overlay_get_dirty(ov));
    if (ov->visible)
      Area_add(Overlay_get_dirty(ov), &ov->position);
    
    DEBUGF("%08x: recursing over %d children\n", ov, ExtArray_length(ov->children));
    for (c = 0; c < ExtArray_length(ov->children); c++)
    {
      Overlay *ch = Overlay_child(ov, c);
      DEBUGF("%08x: child is %08x\n", ov, ch);
      Overlay_reposition_recurse(ch);
    }
  }
  
  DEBUGF("%08x: deleting from repos list\n", ov);
  ov->moved = 0;
  if (Overlay_get_reposition(ov))
    ExtArray_delete(Overlay_get_reposition(ov), ov);
  
}

static void
Overlay_reposition(Overlay *ov)
{
  Overlay *start = ov;
  
  DEBUGF("repositioning %08x\n", ov);
  while (ov->parent)
  {
    if (ov->moved)
      start = ov;
    ov = ov->parent;
  }
  
  DEBUGF("repositioning (recurse) %08x\n", start);
  
  Overlay_reposition_recurse(start);
  
}

void Overlay_paint_self(Overlay *ov, void *paint_arg, Area *updated)
{
  Area *intersect, *outside, *inside;
  int r;

  DEBUGF("paint_self %08x\n", ov);
  
  /* Don't bother painting Overlay or its children if visible flag not set */
  if (!ov->visible)
    return;
  
  intersect = Area_new();
  outside = Area_new();
  inside = Area_new();

  for (r = 0; r < Area_length(Overlay_get_dirty(ov)); r++)
    DEBUGF("dirty rectangle %d/%d = %d,%d,%d,%d\n", r, Area_length(Overlay_get_dirty(ov)), PRECT(Area_rectangle(Overlay_get_dirty(ov), r)));
    
  /* See where this Overlay's area intersects with the global dirty area 
  ** then call the repaint method on that part of the overlay, as well as
  ** adding the area to the list of updated rectangles.
  */
  DEBUGF("getting details for %08x @ %d,%d,%d,%d\n", ov, PRECT(&ov->position));
  Area_overlap_details(Overlay_get_dirty(ov), &ov->position, intersect, inside, outside);
  
  for (r = 0; r < Area_length(intersect); r++)
  {
    Rect *sect_rect = Area_rectangle(intersect, r);
    Rect paint_rect;
    
    Rect_subtract(&paint_rect, sect_rect, &ov->position);

    DEBUGF("%d/%d: dirty intersect with %d,%d,%d,%d = %d,%d,%d,%d\n", r+1, Area_length(intersect), PRECT(&ov->position), PRECT(sect_rect));
      
    Area_add(updated, sect_rect);
    ov->paint(ov, paint_arg, &paint_rect);
  }
  
  /* Recursively paint children */
  for (r = 0; r < ExtArray_length(ov->children); r++)
    Overlay_paint_self(Overlay_child(ov, r), paint_arg, updated);
  
  /* Set last-painted position */
  memcpy(&ov->position_painted, &ov->position, sizeof(Rect));
  
  Area_free(intersect);
  Area_free(outside);
  Area_free(inside);
  
}

static void
Overlay_check_delayed_repositioning(Overlay *ov)
{
  int r;
  
  //!!!
  DEBUGF("checking through %d reposition elements (%08x)\n", ExtArray_length(Overlay_get_reposition(ov)), Overlay_get_reposition(ov));
  
  /* Check whether any element repositioning has been delayed until now */
  for (r=0; r < ExtArray_length(Overlay_get_reposition(ov)); r++)
  {
    Overlay *repos = Overlay_reposition_element(ov, r);
    
    DEBUGF("checking %08x (%d)\n", repos, repos->moved);
    
    if (repos->moved)
      Overlay_reposition(repos);
  }  
  
}

void 
Overlay_paint_all(Overlay *ov, void *paint_arg)
{
  Area *updated, *dirty = Overlay_get_dirty(ov);
  int r;
  int c;

  DEBUG_I(1);
  
  Overlay_check_delayed_repositioning(ov);
  
  /* Don't bother if there are no dirty rectangles */
  if (Area_length(Overlay_get_dirty(ov)) == 0)
    return;

  if (ov->show_dirty)
    printf("dirty : ");
  for (c=0; c<Area_length(dirty); c++)
  {
    int d;
    Rect *r = Area_rectangle(dirty, c);

    if (d < c-1)
      for (d=c+1; d<Area_length(dirty); d++)
        if (Rect_same(r, Area_rectangle(dirty, d)))
	  ExtArray_delete(&dirty->rectangles, r);

    if (ov->show_dirty)
      printf("(%d,%d,%d,%d) ", PRECT(r));
  }
  if (ov->show_dirty)
    printf("\n");
  
  DEBUGF("paint_all: Area_new\n");
  updated = Area_new();
  
  /* Repaint ourself and children, building list of changed rectangles */
  DEBUGF("paint_all: paint ourself\n");
  Overlay_paint_self(ov, paint_arg, updated);

  /* Force updated rectangles to be output to the screen */
  if (ov->update)
    for (r = 0; r < Area_length(updated); r++)
      ov->update(ov, paint_arg, Area_rectangle(updated, r));
  
  Area_free(updated);
  Area_clear(Overlay_get_dirty(ov));
  
}


void Overlay_paint_self_nodirty(Overlay *ov, void *paint_arg)
{
  int r;
  Rect all;
  Rect_set(&all, 0, 0, 1000, 1000);
  
  Overlay_check_delayed_repositioning(ov);

   /* Don't bother painting Overlay or its children if visible flag not set */
  if (!ov->visible)
    return;
  
  ov->paint(ov, paint_arg, &all);  
  for (r = 0; r < ExtArray_length(ov->children); r++)
    Overlay_paint_self_nodirty(Overlay_child(ov, r), paint_arg);
  
  /* Set last-painted position */
  memcpy(&ov->position_painted, &ov->position, sizeof(Rect));
  
}

void
Overlay_paint_all_nodirty(Overlay *ov, void *paint_arg)
{
  Overlay_check_delayed_repositioning(ov);
  Overlay_paint_self_nodirty(ov, paint_arg);
  
  ov->update(ov, paint_arg, &ov->position);
  Area_clear(Overlay_get_dirty(ov));  
}

void
Overlay_set_root(Overlay *ov, 
                 OverlayDrawCallback  *paint_f,
                 OverlayDrawCallback *update_f)
{
  Rect large;
  
  large.tl.x = 0;
  large.tl.y = 0;
  large.br.x = ov->width(ov);
  large.br.y = ov->height(ov);
  
  ov->update             = update_f;
  ov->paint              = paint_f;
  ov->dirty              = Area_new();
  ov->reposition         = ExtArray_new(sizeof(Overlay*));
  
  Area_append_rectangle(Overlay_get_dirty(ov), &large);
}
  
/****************************************************************************
***************************************************************** Ruby API **
****************************************************************************/

/************************************************************ Ruby statics */

static VALUE Overlay_module;
static VALUE Overlay_Rect_class;
static VALUE Overlay_Base_class;

static ID intern_w;
static ID intern_h;
static ID intern_paint;
static ID intern_update;

/***************************************************** Ruby Rect interface */

static VALUE 
rb_Rect_initialize(VALUE self, VALUE x0, VALUE y0, VALUE x1, VALUE y1)
{
  Rect *r;
  DEBUGF("new rect: unwrap\n");
  Data_Get_Struct(self, Rect, r);
  DEBUGF("new rect: set values from %08x,%08x,%08x,%08x\n", x0, y0, x1, y1);
  r->tl.x = NUM2INT(x0);
  r->tl.y = NUM2INT(y0);
  r->br.x = NUM2INT(x1);
  r->br.y = NUM2INT(y1);
  DEBUGF("new rect: return from init\n");
  return self;
}

static VALUE
rb_Rect_new(VALUE self, VALUE x0, VALUE y0, VALUE x1, VALUE y1)
{
  VALUE argv[4];
  Rect *r;
  VALUE data;
  
  DEBUGF("new rect: malloc\n");
  r = MALLOC(sizeof(Rect));
  DEBUGF("new rect: wrap struct\n");
  data = Data_Wrap_Struct(Overlay_Rect_class, 0, free, r);
  DEBUGF("new rect: call init\n");
  argv[0] = x0;
  argv[1] = y0;
  argv[2] = x1;
  argv[3] = y1;
  rb_obj_call_init(data, 4, argv);
  return data;
}

static VALUE rb_Rect_x0_read(VALUE self) { Rect *r; Data_Get_Struct(self, Rect, r); return INT2FIX(r->tl.x); }
static VALUE rb_Rect_y0_read(VALUE self) { Rect *r; Data_Get_Struct(self, Rect, r); return INT2FIX(r->tl.y); }
static VALUE rb_Rect_x1_read(VALUE self) { Rect *r; Data_Get_Struct(self, Rect, r); return INT2FIX(r->br.x); }
static VALUE rb_Rect_y1_read(VALUE self) { Rect *r; Data_Get_Struct(self, Rect, r); return INT2FIX(r->br.y); }
static VALUE rb_Rect_w_read(VALUE self)  { Rect *r; Data_Get_Struct(self, Rect, r); return INT2FIX(r->br.x - r->tl.x); }
static VALUE rb_Rect_h_read(VALUE self)  { Rect *r; Data_Get_Struct(self, Rect, r); return INT2FIX(r->br.y - r->tl.y); }
static VALUE rb_Rect_x0_write(VALUE self, VALUE v) { Rect *r; Data_Get_Struct(self, Rect, r); r->tl.x = NUM2INT(v); return self; }
static VALUE rb_Rect_y0_write(VALUE self, VALUE v) { Rect *r; Data_Get_Struct(self, Rect, r); r->tl.y = NUM2INT(v); return self; }
static VALUE rb_Rect_x1_write(VALUE self, VALUE v) { Rect *r; Data_Get_Struct(self, Rect, r); r->br.x = NUM2INT(v); return self; }
static VALUE rb_Rect_y1_write(VALUE self, VALUE v) { Rect *r; Data_Get_Struct(self, Rect, r); r->br.y = NUM2INT(v); return self; }

static VALUE rb_Rect_w_write(VALUE self, VALUE v)  { Rect *r; Data_Get_Struct(self, Rect, r); r->br.x = NUM2INT(v)+r->tl.x; return self; }
static VALUE rb_Rect_h_write(VALUE self, VALUE v)  { Rect *r; Data_Get_Struct(self, Rect, r); r->br.y = NUM2INT(v)+r->tl.y; return self; }

static VALUE rb_Rect_to_s(VALUE self)
{
  char tmp[256];
  Rect *r; 
  Data_Get_Struct(self, Rect, r);
  DEBUGF(tmp, "<Rect:%d,%d,%d,%d>", PRECT(r));
  
  return rb_str_new2(tmp);
}

/************************************************** Ruby Overlay interface */

static void
rb_paint_callback(Overlay *ov, void *arg, Rect *r)
{
  VALUE rubyrect = rb_Rect_new(Overlay_Rect_class, INT2NUM(r->tl.x), INT2NUM(r->tl.y), INT2NUM(r->br.x), INT2NUM(r->br.y));
  DEBUGF("> paint callback! %08x %08x %d,%d,%d,%d\n", ov, ov->ruby, r->tl.x, r->tl.y, r->br.x, r->br.y);
  rb_funcall(ov->ruby, rb_intern("paint"), 2, (VALUE) arg, rubyrect);
  DEBUGF("< paint callback!\n");
}

static void
rb_update_callback(Overlay *ov, void *arg, Rect *r)
{
  VALUE rubyrect = rb_Rect_new(Overlay_Rect_class, INT2NUM(r->tl.x), INT2NUM(r->tl.y), INT2NUM(r->br.x), INT2NUM(r->br.y));
  DEBUGF("> update callback!\n");
  rb_funcall(ov->ruby, rb_intern("update"), 2, (VALUE) arg, rubyrect);
  DEBUGF("< update callback!\n");
}

static void
rb_Overlay_mark(Overlay *ov)
{
  int c;  
  if (ov->parent)
    rb_gc_mark(ov->parent->ruby);
  for (c=0; c < ExtArray_length(ov->children); c++)
    rb_gc_mark(Overlay_child(ov, c)->ruby);
}

static VALUE 
rb_Overlay_check_consistency(VALUE self)
{
  Overlay *ov; 
  Data_Get_Struct(self, Overlay, ov); 

  ASSERT(!ov->parent || Fortify_CheckPointer(ov->parent));
  ExtArray_check_consistency(ov->children);
  if (Overlay_is_root(ov))
  {
    ExtArray_check_consistency(Overlay_get_reposition(ov));
    ExtArray_check_consistency(&Overlay_get_dirty(ov)->rectangles);
  }
  
  return self;
}

static VALUE 
rb_Overlay_initialize(VALUE self)
{
  return self;
}

static int
rb_Overlay_width_callback(Overlay *ov)
{
  return NUM2INT(rb_funcall(ov->ruby, rb_intern("w"), 0));
}

static int
rb_Overlay_height_callback(Overlay *ov)
{
  return NUM2INT(rb_funcall(ov->ruby, rb_intern("h"), 0));  
}

static VALUE 
rb_Overlay_new(int argc, VALUE *argv, VALUE clazz)
{
  Overlay *ov = Overlay_new();
  ov->ruby = Data_Wrap_Struct(clazz, rb_Overlay_mark, Overlay_free, ov);
  ov->width = rb_Overlay_width_callback;
  ov->height = rb_Overlay_height_callback;
  ov->paint = rb_paint_callback;
  ov->update = rb_update_callback;
  
  rb_obj_call_init(ov->ruby, argc, argv); 
  return ov->ruby;
}

static VALUE
rb_Overlay_anchor_parent_read(VALUE self)
  { Overlay *ov; Data_Get_Struct(self, Overlay, ov); return INT2NUM(ov->anchor_parent); }

static VALUE
rb_Overlay_anchor_read(VALUE self)
  { Overlay *ov; Data_Get_Struct(self, Overlay, ov); return INT2NUM(ov->anchor); }

static VALUE
rb_Overlay_visible_read(VALUE self)
  { Overlay *ov; Data_Get_Struct(self, Overlay, ov); return ov->visible ? Qtrue : Qfalse; }

static VALUE
rb_Overlay_moved_read(VALUE self)
  { Overlay *ov; Data_Get_Struct(self, Overlay, ov); return ov->moved ? Qtrue : Qfalse; }

static VALUE    
rb_Overlay_xoff_read(VALUE self)
  { Overlay *ov; Data_Get_Struct(self, Overlay, ov); return INT2NUM(ov->xoff); }

static VALUE  
rb_Overlay_yoff_read(VALUE self)
  { Overlay *ov; Data_Get_Struct(self, Overlay, ov); return INT2NUM(ov->yoff); }

static VALUE  
rb_Overlay_pos_read(VALUE self)
{ 
  Overlay *ov; 
  Data_Get_Struct(self, Overlay, ov); 
  DEBUGF("pos_read: %d,%d,%d,%d\n", PRECT(&ov->position));
  return rb_Rect_new(Overlay_Rect_class, 
    INT2NUM(ov->position.tl.x), INT2NUM(ov->position.tl.y), 
    INT2NUM(ov->position.br.x), INT2NUM(ov->position.br.y)); 
}

static VALUE  
rb_Overlay_w_read(VALUE self)
  { Overlay *ov; Data_Get_Struct(self, Overlay, ov); return INT2NUM(ov->w); }

static VALUE  
rb_Overlay_h_read(VALUE self)
  { Overlay *ov; Data_Get_Struct(self, Overlay, ov); return INT2NUM(ov->h); }
  
static VALUE  
rb_Overlay_inside(VALUE self, VALUE xx, VALUE yy)
{ 
  Overlay *ov; 
  int x = NUM2INT(xx);
  int y = NUM2INT(yy);
  int inside;
  
  Data_Get_Struct(self, Overlay, ov); 

  inside = ov->position.tl.x <= x && 
           ov->position.tl.y <= y && 
           ov->position.br.x >  x && 
           ov->position.br.y >  y;
  
  DEBUGF("inside?(%d,%d) -> %d,%d,%d,%d = %d\n", x, y, PRECT(&ov->position), inside);
  
  return inside ? Qtrue : Qfalse;
}

static VALUE
rb_Overlay_anchor_parent_write(VALUE self, VALUE v)
  { Overlay *ov; Data_Get_Struct(self, Overlay, ov); return (ov->anchor_parent = NUM2INT(v)); }
  
static VALUE
rb_Overlay_anchor_write(VALUE self, VALUE v)
  { Overlay *ov; Data_Get_Struct(self, Overlay, ov); return (ov->anchor = NUM2INT(v)); }

static VALUE
rb_Overlay_visible_write(VALUE self, VALUE v)
{ 
  int c;
  Overlay *ov; 
  Data_Get_Struct(self, Overlay, ov); 
  
  ov->visible = (v == Qtrue ? 1 : 0); 
  Overlay_flag_moved(ov);
  
  Area_add(Overlay_get_dirty(ov), &ov->position_painted);
  
  for (c=0; c<ExtArray_length(ov->children); c++)
    rb_Overlay_visible_write(Overlay_child(ov, c)->ruby, v);

  return v; 
}

static VALUE
rb_Overlay_xoff_write(VALUE self, VALUE v)
  { Overlay *ov; Data_Get_Struct(self, Overlay, ov); ov->xoff = NUM2INT(v); Overlay_flag_moved(ov); return v; }

static VALUE
rb_Overlay_yoff_write(VALUE self, VALUE v)
  { Overlay *ov; Data_Get_Struct(self, Overlay, ov); ov->yoff = NUM2INT(v); Overlay_flag_moved(ov); return v; }

static VALUE
rb_Overlay_w_write(VALUE self, VALUE v)
  { Overlay *ov; Data_Get_Struct(self, Overlay, ov); ov->w = NUM2INT(v); Overlay_flag_moved(ov); return v; }

static VALUE
rb_Overlay_h_write(VALUE self, VALUE v)
  { Overlay *ov; Data_Get_Struct(self, Overlay, ov); ov->h = NUM2INT(v); Overlay_flag_moved(ov); return v; }

static VALUE
rb_Overlay_show_dirty_write(VALUE self, VALUE v)
  { Overlay *ov; Data_Get_Struct(self, Overlay, ov); ov->show_dirty = NUM2INT(v); return v; }

static VALUE
rb_Overlay_flag_dirty(VALUE self)
{ 
  Overlay *ov; 
  Data_Get_Struct(self, Overlay, ov);
  Area_add(Overlay_get_dirty(ov), &ov->position);
  return self;
}

static VALUE
rb_Overlay_dirty_hack(VALUE self)
{ 
  Rect r;
  Overlay *ov; 
  r.tl.x = 0;
  r.tl.y = 0;
  r.br.x = 800;
  r.br.y = 600;
  Data_Get_Struct(self, Overlay, ov);
  Area_clear(Overlay_get_dirty(ov));
  Area_add(Overlay_get_dirty(ov), &r);
  return self;
}

static VALUE
rb_Overlay_flag_moved(VALUE self)
{ 
  Overlay *ov; 
  Data_Get_Struct(self, Overlay, ov);
  Overlay_flag_moved(ov);
  return self;
}

static VALUE
rb_Overlay_set_root(VALUE self)
{ 
  Overlay *ov; 
  Data_Get_Struct(self, Overlay, ov);
  Overlay_set_root(ov, rb_paint_callback, rb_update_callback);
  return self;
}

static VALUE
rb_Overlay_parent(VALUE self)
{
  Overlay *ov;
  Data_Get_Struct(self, Overlay, ov);
  
  if (!ov->parent)
    return Qnil;
  
  return ov->parent->ruby;
}

static VALUE
rb_Overlay_add_child(VALUE self, VALUE child)
{
  Overlay *ov, *ovc; 
  Data_Get_Struct(self, Overlay, ov);
  Data_Get_Struct(child, Overlay, ovc);
  
  Overlay_add_child(ov, ovc);
  
  return self;
}

static VALUE
rb_Overlay_del_child(VALUE self, VALUE child)
{
  Overlay *ov, *ovc; 
  Data_Get_Struct(self, Overlay, ov);
  Data_Get_Struct(child, Overlay, ovc);
  
  Overlay_del_child(ov, ovc);
  
  return self;
}

static VALUE
rb_Overlay_paint(VALUE self, VALUE surface, VALUE area)
{
  return self;
}

static VALUE
rb_Overlay_paint_all(VALUE self, VALUE surface)
{
  Overlay *ov; 
  Data_Get_Struct(self, Overlay, ov);
  
  Overlay_paint_all(ov, (void*) surface);
  
  return self;
}

static VALUE
rb_Overlay_paint_all_nodirty(VALUE self, VALUE surface)
{
  Overlay *ov; 
  Data_Get_Struct(self, Overlay, ov);
  
  Overlay_paint_all_nodirty(ov, (void*) surface);
  
  return self;
}

static void
rb_Overlay_each_child_recurse(Overlay *ov)
{
  int c;
  
  /* DEBUGF("asking to recurse over %08x with %d children\n", ov, ExtArray_length(ov->children)); */
  for (c=0; c<ExtArray_length(ov->children); c++)
  {
    Overlay *child = Overlay_child(ov, c);
    rb_Overlay_each_child_recurse(child);
  }
  
  /* Yielding here, at the lowest level objects, means that we're going
  ** from front-to-back which is more appropriate for handling user interaction.
  */
  rb_yield(ov->ruby);
}
  
static VALUE
rb_Overlay_each_child(VALUE self)
{
  Overlay *ov; 
  Data_Get_Struct(self, Overlay, ov);
  
  rb_Overlay_each_child_recurse(ov);
  
  return self;
}

/***************************************************** Ruby initialization */

void initOverlay()
{
  /* module Overlay */
  Overlay_module = rb_define_module("Overlay");
  
  /* class Overlay::Rect */
  Overlay_Rect_class = rb_define_class_under(Overlay_module, "Rect", rb_cObject);
  
  rb_define_singleton_method(Overlay_Rect_class, "new", rb_Rect_new, 4);
  rb_define_method(Overlay_Rect_class, "initialize", rb_Rect_initialize, 4);
  
  rb_define_method(Overlay_Rect_class, "x0", rb_Rect_x0_read, 0);
  rb_define_method(Overlay_Rect_class, "x1", rb_Rect_x1_read, 0);
  rb_define_method(Overlay_Rect_class, "y0", rb_Rect_y0_read, 0);
  rb_define_method(Overlay_Rect_class, "y1", rb_Rect_y1_read, 0);
  rb_define_method(Overlay_Rect_class, "w", rb_Rect_w_read, 0);
  rb_define_method(Overlay_Rect_class, "h", rb_Rect_h_read, 0);
  rb_define_method(Overlay_Rect_class, "x0=", rb_Rect_x0_write, 1);
  rb_define_method(Overlay_Rect_class, "x1=", rb_Rect_x1_write, 1);
  rb_define_method(Overlay_Rect_class, "y0=", rb_Rect_y0_write, 1);
  rb_define_method(Overlay_Rect_class, "y1=", rb_Rect_y1_write, 1);
  rb_define_method(Overlay_Rect_class, "w=", rb_Rect_w_write, 1);
  rb_define_method(Overlay_Rect_class, "h=", rb_Rect_h_write, 1);
  rb_define_method(Overlay_Rect_class, "to_s", rb_Rect_to_s, 0);
  
  /* class Overlay::Base */
  Overlay_Base_class = rb_define_class_under(Overlay_module, "Base", rb_cObject);
    
  rb_define_const(Overlay_Base_class, "NW", INT2NUM(NW));
  rb_define_const(Overlay_Base_class, "N", INT2NUM(N));
  rb_define_const(Overlay_Base_class, "NE", INT2NUM(NE));
  rb_define_const(Overlay_Base_class, "W", INT2NUM(W));
  rb_define_const(Overlay_Base_class, "C", INT2NUM(C));
  rb_define_const(Overlay_Base_class, "E", INT2NUM(E));
  rb_define_const(Overlay_Base_class, "SW", INT2NUM(SW));
  rb_define_const(Overlay_Base_class, "S", INT2NUM(S));
  rb_define_const(Overlay_Base_class, "SE", INT2NUM(SE));

  rb_define_singleton_method(Overlay_Base_class, "new", rb_Overlay_new, -1);
  rb_define_method(Overlay_Base_class, "initialize", rb_Overlay_initialize, 0);
  
  rb_define_method(Overlay_Base_class, "check_consistency", rb_Overlay_check_consistency, 0);

    rb_define_method(Overlay_Base_class, "anchor_parent", rb_Overlay_anchor_parent_read, 0);
  rb_define_method(Overlay_Base_class, "anchor", rb_Overlay_anchor_read, 0);
  rb_define_method(Overlay_Base_class, "visible", rb_Overlay_visible_read, 0);
  rb_define_method(Overlay_Base_class, "moved", rb_Overlay_moved_read, 0);
  rb_define_method(Overlay_Base_class, "xoff", rb_Overlay_xoff_read, 0);
  rb_define_method(Overlay_Base_class, "yoff", rb_Overlay_yoff_read, 0);
  rb_define_method(Overlay_Base_class, "pos", rb_Overlay_pos_read, 0);
  rb_define_method(Overlay_Base_class, "w", rb_Overlay_w_read, 0);
  rb_define_method(Overlay_Base_class, "h", rb_Overlay_h_read, 0);
  rb_define_method(Overlay_Base_class, "inside?", rb_Overlay_inside, 2);
  
  rb_define_method(Overlay_Base_class, "anchor_parent=", rb_Overlay_anchor_parent_write, 1);
  rb_define_method(Overlay_Base_class, "anchor=", rb_Overlay_anchor_write, 1);
  rb_define_method(Overlay_Base_class, "visible=", rb_Overlay_visible_write, 1);
  rb_define_method(Overlay_Base_class, "xoff=", rb_Overlay_xoff_write, 1);
  rb_define_method(Overlay_Base_class, "yoff=", rb_Overlay_yoff_write, 1);
  rb_define_method(Overlay_Base_class, "w=", rb_Overlay_w_write, 1);
  rb_define_method(Overlay_Base_class, "h=", rb_Overlay_h_write, 1);

  rb_define_method(Overlay_Base_class, "show_dirty=", rb_Overlay_show_dirty_write, 1);
  
  rb_define_method(Overlay_Base_class, "set_root", rb_Overlay_set_root, 0);
  rb_define_method(Overlay_Base_class, "flag_moved", rb_Overlay_flag_moved, 0);
  rb_define_method(Overlay_Base_class, "flag_dirty", rb_Overlay_flag_dirty, 0);
  rb_define_method(Overlay_Base_class, "dirty_hack", rb_Overlay_dirty_hack, 0);
  
  rb_define_method(Overlay_Base_class, "parent", rb_Overlay_parent, 0);
  rb_define_method(Overlay_Base_class, "add_child", rb_Overlay_add_child, 1);
  rb_define_method(Overlay_Base_class, "del_child", rb_Overlay_del_child, 1);
  
  rb_define_method(Overlay_Base_class, "paint", rb_Overlay_paint, 1);
  rb_define_method(Overlay_Base_class, "paint_all", rb_Overlay_paint_all, 1);
  rb_define_method(Overlay_Base_class, "paint_all_nodirty", rb_Overlay_paint_all_nodirty, 1);
  rb_define_method(Overlay_Base_class, "each_child", rb_Overlay_each_child, 0);
  
  /* Cache some symbol IDs for faster Ruby method calling later */
  intern_w = rb_intern("w");
  intern_h = rb_intern("h");
  intern_paint = rb_intern("paint");
  intern_update = rb_intern("update");
}





