/*BitMask - a pixel perfect collision detection library
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

#ifndef BITMASK_H
#define BITMASK_H

#define BITMASK_TYPE unsigned int
#define BITMASK_WORD_LENGTH 32
#define BITMASK_WORD_BITMASK 31


/* If this is defined a test for at the midpoint of the rectangular
   intersection between the bounding boxes is performed. This is 
   usually a good idea, as objects tend to collide there. */
#define USE_MIDPOINT_GUESS


/* Defind this to collect statistics from the overlap tests. */
//#define DO_STATISTICS

/* ww is words per line + 1 word for padding */
typedef struct 
{
  int w,h;
  int ww;
  BITMASK_TYPE *bits;
} BitMask;

BitMask *BitMask_create(int h,int w);
void BitMask_destroy(BitMask *m);
void BitMask_clear(BitMask *m);

/* Returns 0 if the bit is not set and something != 0 if the bit is set */
int  BitMask_getbit(const BitMask *m,int x,int y);
/* Use value = 0 to clear the bit and somethin != 0 to set it */
void BitMask_setbit(BitMask *m,int x,int y,int value);

/*
 *  +----+----------..
 *  |A   | yoffset   
 *  |  +-+----------..
 *  +--|B        
 *  |xoffset      
 *  |  |
 *  :  :
 */
int BitMask_overlap(BitMask *A, BitMask *B,int xoffset, int yoffset);

/* Stores _a_ point of intersection in (x,y). This is as fast as BitMask_overlap()
 * when there is no overlap, and not much slower if there is. Use of this function
 * slows down the test cast with perhaps 0.5%
 */
int BitMask_overlap_pos(BitMask *A, BitMask *B, int xoffset, int yoffset, int *x, int *y);

/* Faster, 'unsafe' functions below. No bounds checking is done here. BEWARE! */

#define _BitMask_getbit(m, x, y) \
	(*(m->bits + (y)*(m->ww) + (x)/BITMASK_WORD_LENGTH) & (1 << (BITMASK_WORD_BITMASK & (x))))

/* get a whole word at once, starting at bit x. */
#define _BitMask_getbits(m, x, y)\
  (((x & BITMASK_WORD_BITMASK) == 0)?\
    *(m->bits + (y)*(m->ww) + (x)/BITMASK_WORD_LENGTH)\
  : (*(m->bits + (y)*(m->ww) + (x)/BITMASK_WORD_LENGTH) >> (BITMASK_WORD_BITMASK & (x)))\
      | (*(m->bits + (y)*(m->ww) + (x)/BITMASK_WORD_LENGTH + 1) << (32 - (BITMASK_WORD_BITMASK & (x)))))


#define _BitMask_setbit(m, x, y)\
	(*(m->bits + (y)*(m->ww) + (x)/BITMASK_WORD_LENGTH) |= (1 << (BITMASK_WORD_BITMASK & (x))))

#define _BitMask_clearbit(m, x, y)\
	(*(m->bits + (y)*(m->ww) + (x)/BITMASK_WORD_LENGTH) &=  ~(1 << (BITMASK_WORD_BITMASK & (x))))


#ifdef DO_STATISTICS
void BitMask_init_statistics();
void BitMask_print_statistics();
#endif

#endif 
