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
#ifndef SDL_BITMASK_H
#define SDL_BITMASK_H

#include <SDL.h>
#include "bitmask.h"

/* Creates a BitMask from area of SDL_Surface with colorkey as transparent (0) */ 
BitMask *BitMask_from_image_SDL(SDL_Surface *surface ,SDL_Rect *area, Uint32 colorkey);

/* Draws a BitMask onto a SDL_Surface. Very slow implementation, useful for debugging */
void BitMask_draw_SDL(BitMask *m, SDL_Surface *target, int x, int y, Uint32 color); 

#endif
