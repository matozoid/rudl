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

#define GETPIX24(p) ((p[0] << 16) | (p[1] << 8) | p[2])
#define PUTPIX24(p,x) *p++ = x>>16; *p++ = x>>8 & 0xff; *p++ = x & 0xff

/* this function processes only one row of the input bitmap */
static void scale2x_row_24bit(Uint8* dest0, Uint8* dest1, Uint8* src0, Uint8* src1, Uint8* src2, int srcw)
{
    Uint32 b,d,e,f,h;
    Uint32 e0,e1,e2,e3;

    /* set up for the first pixel */
    /* since there is no pixel to the left, we reuse this first pixel */
    e = f = GETPIX24(src1); src1 += 3;

    /* this handles all pixels except the last one */
    for (srcw--; srcw > 0; srcw--) {
        b = GETPIX24(src0); src0 += 3;
        d = e; e = f; f = GETPIX24(src1); src1 += 3;
        h = GETPIX24(src2); src2 += 3;

        e0 = d == b && b != f && d != h ? d : e;
        e1 = b == f && b != d && f != h ? f : e;
        PUTPIX24(dest0, e0);
        PUTPIX24(dest0, e1);
        e2 = d == h && d != b && h != f ? d : e;
        e3 = h == f && d != h && b != f ? f : e;
        PUTPIX24(dest1, e2);
        PUTPIX24(dest1, e3);
    }

    /* last pixel - since there is no pixel to the right, we reuse this last pixel */
    b = GETPIX24(src0);
    d = e; e = f;
    h = GETPIX24(src2);

    e0 = d == b && b != f && d != h ? d : e;
    e1 = b == f && b != d && f != h ? f : e;
    PUTPIX24(dest0, e0);
    PUTPIX24(dest0, e1);
    e2 = d == h && d != b && h != f ? d : e;
    e3 = h == f && d != h && b != f ? f : e;
    PUTPIX24(dest1, e2);
    PUTPIX24(dest1, e3);

    return;
}

#define PIXEL Uint8
#define SCALE2XROWFUNC scale2x_row_24bit
#define SCALE2XFUNC scale2x_24bit

#include "scale2xrows.h"

