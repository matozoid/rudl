/*BitBitMask - a pixel perfect collision detection library
 *Copyright (C) 2002 Ulf Ekstrom
 *
 *This program is free software; you can redistribute it and/or
 *modify it under the terms of the GNU General Public License
 *as published by the Free Software Foundation; either version 2
 *of the License, or (at your option) any later version.
 *
 *This program is distributed in the hope that it will be useful,
 *but WITHOUT ANY WARRANTY; without even the implied warranty of
 *MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *GNU General Public License for more details.
 *
 *You should have received a copy of the GNU General Public License
 *along with this program; if not, write to the Free Software
 *Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "SDL_bitmask.h"
#include <assert.h>

  
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))

void BitMask_draw_SDL(BitMask *m,SDL_Surface *surface,int x,int y,Uint32 color)
{
  int i,j;
  int maxx;
  int maxy;
  
  maxx = MIN(m->w,surface->w - x);
  maxy = MIN(m->h,surface->h - y);

  switch (surface->format->BytesPerPixel) {
  case 1:  /* Assuming 8-bpp */	  
    for (j=0;j<maxy;j++)
      {
	for (i=0;i<maxx;i++)
	  {
	    if (BitMask_getbit(m,i,j))
	      *((Uint8 *)surface->pixels + (y + j)*surface->pitch + x + i) = color;
	  }
      }
    break;
  case 2:  /* Probably 15-bpp or 16-bpp */
    for (j=0;j<maxy;j++)
      {
	for (i=0;i<maxx;i++)
	  {
	    if (BitMask_getbit(m,i,j))
	      *((Uint16 *)surface->pixels + (y + j)*surface->pitch/2 + x + i) = color;
	  }
      }
    break;
  case 3:  /* Slow 24-bpp mode, usually not used */
    assert(0); /* Sorry. 24bpp packed mode not (yet) supported..*/
    break;    
  case 4:  /* Probably 32-bpp */
    for (j=0;j<maxy;j++)
      {
	for (i=0;i<maxx;i++)
	  {
	    if (BitMask_getbit(m,i,j))
		*((Uint32 *)surface->pixels + (y + j)*surface->pitch/4 + x + i) = color;
	  }
      }
    break;
  }
}

/* WARNING: This function throws core at some surfaces. Fix this! */
BitMask *BitMask_from_image_SDL(SDL_Surface *surface,SDL_Rect *r,Uint32 colorkey)
{
  BitMask *b;
  int i,j;
  SDL_Rect rr;

  assert(surface!=0);
  /*  printf("Surface: %ix%i, %i bpp\n",surface->w,surface->h,surface->format->BytesPerPixel); */
  if (!r)
    {
      rr.x = 0;
      rr.y = 0;
      rr.w = surface->w;
      rr.h = surface->h;
      r = &rr;
    }
  b = BitMask_create(r->w,r->h);
  if (!b) return 0;
  switch (surface->format->BytesPerPixel) {
  case 1: { /* Assuming 8-bpp */
    for (j=r->y;j<r->h+r->y;j++)
      {
	for (i=r->x;i<r->w+r->x;i++)
	  {
	    if (*((Uint8 *)surface->pixels + (j)*surface->pitch + i) 
		== colorkey)
	      BitMask_setbit(b,i-r->x,j-r->y,0);
	    else BitMask_setbit(b,i-r->x,j-r->y,1);
	  }
      }
  }
    break;
	  
    case 2: { /* Probably 15-bpp or 16-bpp */
      for (j=r->y;j<r->h+r->y;j++)
	{
	  for (i=r->x;i<r->w+r->x;i++)
		{
		  if (*((Uint16 *)surface->pixels + (j)*surface->pitch/2 + i)
		      == colorkey)
		    BitMask_setbit(b,i-r->x,j-r->y,0);
		  else 
		    BitMask_setbit(b,i-r->x,j-r->y,1);
		}
	    }
    }
      break;
        case 3: { /* Slow 24-bpp mode, usually not used */
      Uint8 *bufp;
      for (j=r->y;j<r->h+r->y;j++)
	{
	  for (i=r->x;i<r->w+r->x;i++)
	    {		
	      bufp = (Uint8 *)surface->pixels + (j)*surface->pitch + (i) * 3;
	      if(SDL_BYTEORDER == SDL_LIL_ENDIAN) {
		
		if ((bufp[0] == (colorkey & 255)) &&
		    (bufp[1] == ((colorkey >> 8) & 255)) &&
		    (bufp[2] == ((colorkey >> 16) & 255)))
		  BitMask_setbit(b,i-r->x,j-r->y,0);
		else 
		  BitMask_setbit(b,i-r->x,j-r->y,1);
	      } else 
		{
		  if ((bufp[2] == (colorkey & 255)) &&
		      (bufp[1] == ((colorkey >> 8) & 255)) &&
		      (bufp[0] == ((colorkey >> 16) & 255)))
		    BitMask_setbit(b,i-r->x,j-r->y,0);
		  else 
		    BitMask_setbit(b,i-r->x,j-r->y,1);
		}
	    }
	}
	}
      break;
  case 4:  /* Probably 32-bpp */
    for (j=r->y;j<r->h+r->y;j++)
      {
	for (i=r->x;i<r->w+r->x;i++)
	  {
	    if (*((Uint32 *)surface->pixels + (j)*surface->pitch/4 + i) 
		== colorkey)
	      BitMask_setbit(b,i-r->x,j-r->y,0);
	    else 
	      BitMask_setbit(b,i-r->x,j-r->y,1);
	  }
      }
    break;
  }
  return b;
}



