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

#include "scale2xrows.h"
