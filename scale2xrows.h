
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
