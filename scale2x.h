/*
Starting from this pattern:

src0:    B
src1:  D E F
src2:    H

The central pixel E is expanded into 4 new pixels:

dest0: E0 E1
dest1: E2 E3

with these rules:

E0 = D == B && B != F && D != H ? D : E;
E1 = B == F && B != D && F != H ? F : E;
E2 = D == H && D != B && H != F ? D : E;
E3 = H == F && D != H && B != F ? F : E;

*/

/* this function processes only one row of the input bitmap */
static void SCALE2XROWFUNC(PIXEL* dest0, PIXEL* dest1, PIXEL* src0, PIXEL* src1, PIXEL* src2, int srcw)
{
    PIXEL b,d,e,f,h;

    /* set up for the first pixel */
    /* since there is no pixel to the left, we reuse this first pixel */
    e = f = *src1++;

    /* this handles all pixels except the last one */
    for (srcw--; srcw > 0; srcw--) {
        b = *src0++;
        d = e; e = f; f = *src1++;
        h = *src2++;

        *dest0++ = d == b && b != f && d != h ? d : e;
        *dest0++ = b == f && b != d && f != h ? f : e;
        *dest1++ = d == h && d != b && h != f ? d : e;
        *dest1++ = h == f && d != h && b != f ? f : e;
    }

    /* last pixel - since there is no pixel to the right, we reuse this last pixel */
    b = *src0;
    d = e; e = f;
    h = *src2;

    *dest0++ = d == b && b != f && d != h ? d : e;
    *dest0++ = b == f && b != d && f != h ? f : e;
    *dest1++ = d == h && d != b && h != f ? d : e;
    *dest1++ = h == f && d != h && b != f ? f : e;

    return;
}


/* this function feeds all rows of the bitmap to the row processing function */
static void SCALE2XFUNC(SDL_Surface* src, SDL_Surface* dest)
{
    PIXEL* srcpix = src->pixels;
    PIXEL* destpix = dest->pixels;
    int srcpitch = src->pitch / sizeof(PIXEL);
    int destpitch = dest->pitch / sizeof(PIXEL);
    int w = src->w, h = src->h;
    PIXEL *dest0, *dest1, *src0, *src1, *src2;

    /* set up destination line pointers */
    dest0 = destpix; dest1 = destpix + destpitch;

    /* set up source line pointers */
    /* since there's no line above, we reuse this first line */
    src0 = src1 = srcpix; src2 = srcpix + srcpitch;

    /* all but the last row */
    for (h--; h > 0; h--) {
        SCALE2XROWFUNC(dest0, dest1, src0, src1, src2, w);
        /* move both destination line pointers down 2 rows */
        dest0 = dest1 + destpitch;
        dest1 = dest0 + destpitch;
        /* shift all source lines down */
        src0 = src1;
        src1 = src2;
        src2 += srcpitch;
    }

    /* last row - since there is no line below, we reuse this last line */
    SCALE2XROWFUNC(dest0, dest1, src0, src1, src1, w);

    return;
}
