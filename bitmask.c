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
#include "bitmask.h"
#include <malloc.h>
#include <assert.h>
#include <string.h>

#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define POINT_INSIDE(m,x,y) (((x) >= 0) && ((y) >= 0) && ((x) < m->w) && ((y) < m->h))


#ifdef DO_STATISTICS
#include <stdio.h>
static int nrcreated,nrdestroyed,nroverlaps,nroutside,nrdeepfailed,nrguessed;
void BitMask_init_statistics()
{
  nrcreated = 0;
  nrdestroyed = 0;
  nroverlaps = 0;
  nrdeepfailed = 0;
  nrguessed = 0;
} 
void BitMask_print_statistics()
{
  printf("BitMask:\n");
  printf("\tCreated %i, Destroyed %i\n",nrcreated,nrdestroyed);
  printf("\tDid %i overlap tests, %i%% were bounding box-discarded\n",
	 nroverlaps,(100*nroutside)/nroverlaps);
  printf("\tand %i%% did not overlap anyway.\n",(100*nrdeepfailed)/nroverlaps);
  printf("\tManaged to guess at point of intersection in %i%% of the cases.\n",
	 (100*nrguessed)/nroverlaps);
  printf("\twhich leaves %i%% which were discovered the 'hard' way.\n",
	 (100*(nroverlaps-nroutside-nrdeepfailed-nrguessed))/nroverlaps );
}
#endif

BitMask *BitMask_create(int w,int h)
{
  BitMask *m = (BitMask *)malloc(sizeof(BitMask));
#ifdef DO_STATISTICS
  nrcreated++;
#endif
  m->h = h;
  m->w = w;
  m->ww = (w / BITMASK_WORD_LENGTH) + 2; /* One extra padding word here */
  m->bits = (BITMASK_TYPE *)calloc((m->h)*(m->ww),sizeof(BITMASK_TYPE));
  if (m->bits)
    return m;
  else
    return 0;
}

void BitMask_destroy(BitMask *m)
{
  if (m)
    {
#ifdef DO_STATISTICS
      nrdestroyed++;
#endif
      free(m->bits);
      free(m);
    }  
}

void BitMask_clear(BitMask *m)
{
  memset((void*)m->bits,0,((m->h)*(m->ww)*sizeof(BITMASK_TYPE)));
}

int  BitMask_getbit(const BitMask *m,int x,int y)
{
  if (POINT_INSIDE(m,x,y))
    return _BitMask_getbit(m,x,y);
  else
    return 0;
}

void BitMask_setbit(BitMask *m,int x,int y,int value)
{
  /*assert(POINT_INSIDE(m,x,y));*/
  if (POINT_INSIDE(m,x,y))
    {
      if (value)
	_BitMask_setbit(m,x,y);
      else
	_BitMask_clearbit(m,x,y);
    }
}


int BitMask_overlap(BitMask *A, BitMask *B,int xoffset,int yoffset)
{
  int x,y;
  int axmin,aymin;
  int axmax,aymax;

#ifdef DO_STATISTICS
  nroverlaps++;
#endif

  axmin = MAX(0,xoffset);
  axmax = MIN(A->w,B->w + xoffset);
  if (axmax <=  axmin)
    {
#ifdef DO_STATISTICS
      nroutside++;
#endif
      return 0;
    }
  aymin = MAX(0,yoffset);
  aymax = MIN(A->h,B->h + yoffset);
  if (aymax <= aymin) 
    {
#ifdef DO_STATISTICS
      nroutside++;
#endif
      return 0;
    }  
#ifdef USE_MIDPOINT_GUESS
  /* Here we test if the BitMasks overlap at the midpoint of the rectangular intersection, 
     which is quite likely.*/
  x = (A->w/2 + B->w/2 + xoffset)/2;
  y = (A->h/2 + B->h/2 + yoffset)/2;
  if ((x>=axmin)&&(x<axmax)&&(y>=aymin)&&(y<aymax))
    {
      if (_BitMask_getbits(A,x,y) & _BitMask_getbits(B,x - xoffset,y - yoffset))
	{
#ifdef DO_STATISTICS
	  nrguessed++;
#endif
	  return 1;
	}
    } else return 0;
#endif
  
  for (y = aymin;y < aymax;y++)
    {
      for (x = axmin;x < axmax;x+=32)
	{
	  if (_BitMask_getbits(A,x,y) & _BitMask_getbits(B,x - xoffset,y - yoffset))
	    {
	      return 1;
	    }
	} 
    }
#ifdef DO_STATISTICS
  nrdeepfailed++; /* This means that we had to examine every single bit,
		   * obviously a pretty bad thing. 
		   * Can't think of any smart algorithm if we allow non-contigous BitMasks,
		   * except to use trees (i.e. lower resolution bitmaps)
		   */
#endif
  return 0;
}


int BitMask_overlap_pos(BitMask *A, BitMask *B,int xoffset,int yoffset,int *xp,int *yp)
{
  int x,y;
  int axmin,aymin;
  int axmax,aymax;

#ifdef DO_STATISTICS
  nroverlaps++;
#endif

  axmin = MAX(0,xoffset);
  axmax = MIN(A->w,B->w + xoffset);
  if (axmax <=  axmin)
    {
#ifdef DO_STATISTICS
      nroutside++;
#endif
      return 0;
    }
  aymin = MAX(0,yoffset);
  aymax = MIN(A->h,B->h + yoffset);
  if (aymax <= aymin) 
    {
#ifdef DO_STATISTICS
      nroutside++;
#endif
      return 0;
    }  

#ifdef USE_MIDPOINT_GUESS
  /* Here we test if the BitMasks overlap halfway between them, 
     which is quite likely. */
  x = (A->w/2 + xoffset + B->w/2)/2;
  y = (A->h/2 + yoffset + B->h/2)/2;
  if ((x>=axmin)&&(x<axmax)&&(y>=aymin)&&(y<aymax))
    {
      if (_BitMask_getbits(A,x,y) & _BitMask_getbits(B,x - xoffset,y - yoffset))
	{
#ifdef DO_STATISTICS
	  nrguessed++;
#endif
	  *yp = y;
	  for (;x<axmax;x++)
	    {
	      if ((_BitMask_getbit(A,x,y) != 0) && 
		  (_BitMask_getbit(B,x - xoffset,y - yoffset) != 0))
		{
		  *xp = x;
		  return 1;
		}
	    }
	  assert(0);
	  return 1;
	}
    } 
#endif
  
  for (y = aymin;y < aymax;y++)
    {
      for (x = axmin;x < axmax;x+=32)
	{
	  if (_BitMask_getbits(A,x,y) & _BitMask_getbits(B,x - xoffset,y - yoffset))
	    {
	      *yp = y;
	      for (;x<axmax;x++)
		{
		  if ((_BitMask_getbit(A,x,y) != 0) && 
		      (_BitMask_getbit(B,x - xoffset,y - yoffset) != 0))
		    {
		      *xp = x;
		      return 1;
		    }
		}
	      assert(0);
	      return 1;
	    }
	} 
    }
#ifdef DO_STATISTICS
  nrdeepfailed++; /* This means that we had to examine every single bit,
		   * obviously a pretty bad thing. 
		   * Can't think of any smart algorithm if we allow non-contigous BitMasks,
		   * except to use trees (i.e. lower resolution bitmaps)
		   */
#endif
  return 0;
}
