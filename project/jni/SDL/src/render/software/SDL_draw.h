/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997-2010 Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    Sam Lantinga
    slouken@libsdl.org
*/
#include "SDL_config.h"

#include "../../video/SDL_blit.h"

/* This code assumes that r, g, b, a are the source color,
 * and in the blend and add case, the RGB values are premultiplied by a.
 */

#define DRAW_MUL(_a, _b) (((unsigned)(_a)*(_b))/255)

#define DRAW_FASTSETPIXEL(type) \
    *pixel = (type) color

#define DRAW_FASTSETPIXEL1 DRAW_FASTSETPIXEL(Uint8)
#define DRAW_FASTSETPIXEL2 DRAW_FASTSETPIXEL(Uint16)
#define DRAW_FASTSETPIXEL4 DRAW_FASTSETPIXEL(Uint32)

#define DRAW_FASTSETPIXELXY(x, y, type, bpp, color) \
    *(type *)((Uint8 *)dst->pixels + (y) * dst->pitch \
                                   + (x) * bpp) = (type) color

#define DRAW_FASTSETPIXELXY1(x, y) DRAW_FASTSETPIXELXY(x, y, Uint8, 1, color)
#define DRAW_FASTSETPIXELXY2(x, y) DRAW_FASTSETPIXELXY(x, y, Uint16, 2, color)
#define DRAW_FASTSETPIXELXY4(x, y) DRAW_FASTSETPIXELXY(x, y, Uint32, 4, color)

#define DRAW_SETPIXEL(setpixel) \
do { \
    unsigned sr = r, sg = g, sb = b, sa = a; \
    setpixel; \
} while (0)

#define DRAW_SETPIXEL_BLEND(getpixel, setpixel) \
do { \
    unsigned sr, sg, sb, sa; sa; \
    getpixel; \
    sr = DRAW_MUL(inva, sr) + r; \
    sg = DRAW_MUL(inva, sg) + g; \
    sb = DRAW_MUL(inva, sb) + b; \
    setpixel; \
} while (0)

#define DRAW_SETPIXEL_ADD(getpixel, setpixel) \
do { \
    unsigned sr, sg, sb, sa; sa; \
    getpixel; \
    sr += r; if (sr > 0xff) sr = 0xff; \
    sg += g; if (sg > 0xff) sg = 0xff; \
    sb += b; if (sb > 0xff) sb = 0xff; \
    setpixel; \
} while (0)

#define DRAW_SETPIXELXY(x, y, type, bpp, op) \
do { \
    type *pixel = (type *)((Uint8 *)dst->pixels + (y) * dst->pitch \
                                                + (x) * bpp); \
    op; \
} while (0)

/*
 * Define draw operators for RGB555
 */

#define DRAW_SETPIXEL_RGB555 \
    DRAW_SETPIXEL(RGB555_FROM_RGB(*pixel, sr, sg, sb))

#define DRAW_SETPIXEL_BLEND_RGB555 \
    DRAW_SETPIXEL_BLEND(RGB_FROM_RGB555(*pixel, sr, sg, sb), \
                        RGB555_FROM_RGB(*pixel, sr, sg, sb))

#define DRAW_SETPIXEL_ADD_RGB555 \
    DRAW_SETPIXEL_ADD(RGB_FROM_RGB555(*pixel, sr, sg, sb), \
                      RGB555_FROM_RGB(*pixel, sr, sg, sb))

#define DRAW_SETPIXELXY_RGB555(x, y) \
    DRAW_SETPIXELXY(x, y, Uint16, 2, DRAW_SETPIXEL_RGB555)

#define DRAW_SETPIXELXY_BLEND_RGB555(x, y) \
    DRAW_SETPIXELXY(x, y, Uint16, 2, DRAW_SETPIXEL_BLEND_RGB555)

#define DRAW_SETPIXELXY_ADD_RGB555(x, y) \
    DRAW_SETPIXELXY(x, y, Uint16, 2, DRAW_SETPIXEL_ADD_RGB555)

/*
 * Define draw operators for RGB565
 */

#define DRAW_SETPIXEL_RGB565 \
    DRAW_SETPIXEL(RGB565_FROM_RGB(*pixel, sr, sg, sb))

#define DRAW_SETPIXEL_BLEND_RGB565 \
    DRAW_SETPIXEL_BLEND(RGB_FROM_RGB565(*pixel, sr, sg, sb), \
                        RGB565_FROM_RGB(*pixel, sr, sg, sb))

#define DRAW_SETPIXEL_ADD_RGB565 \
    DRAW_SETPIXEL_ADD(RGB_FROM_RGB565(*pixel, sr, sg, sb), \
                      RGB565_FROM_RGB(*pixel, sr, sg, sb))

#define DRAW_SETPIXELXY_RGB565(x, y) \
    DRAW_SETPIXELXY(x, y, Uint16, 2, DRAW_SETPIXEL_RGB565)

#define DRAW_SETPIXELXY_BLEND_RGB565(x, y) \
    DRAW_SETPIXELXY(x, y, Uint16, 2, DRAW_SETPIXEL_BLEND_RGB565)

#define DRAW_SETPIXELXY_ADD_RGB565(x, y) \
    DRAW_SETPIXELXY(x, y, Uint16, 2, DRAW_SETPIXEL_ADD_RGB565)

/*
 * Define draw operators for RGB888
 */

#define DRAW_SETPIXEL_RGB888 \
    DRAW_SETPIXEL(RGB888_FROM_RGB(*pixel, sr, sg, sb))

#define DRAW_SETPIXEL_BLEND_RGB888 \
    DRAW_SETPIXEL_BLEND(RGB_FROM_RGB888(*pixel, sr, sg, sb), \
                        RGB888_FROM_RGB(*pixel, sr, sg, sb))

#define DRAW_SETPIXEL_ADD_RGB888 \
    DRAW_SETPIXEL_ADD(RGB_FROM_RGB888(*pixel, sr, sg, sb), \
                      RGB888_FROM_RGB(*pixel, sr, sg, sb))

#define DRAW_SETPIXELXY_RGB888(x, y) \
    DRAW_SETPIXELXY(x, y, Uint32, 4, DRAW_SETPIXEL_RGB888)

#define DRAW_SETPIXELXY_BLEND_RGB888(x, y) \
    DRAW_SETPIXELXY(x, y, Uint32, 4, DRAW_SETPIXEL_BLEND_RGB888)

#define DRAW_SETPIXELXY_ADD_RGB888(x, y) \
    DRAW_SETPIXELXY(x, y, Uint32, 4, DRAW_SETPIXEL_ADD_RGB888)

/*
 * Define draw operators for ARGB8888
 */

#define DRAW_SETPIXEL_ARGB8888 \
    DRAW_SETPIXEL(ARGB8888_FROM_RGBA(*pixel, sr, sg, sb, sa))

#define DRAW_SETPIXEL_BLEND_ARGB8888 \
    DRAW_SETPIXEL_BLEND(RGBA_FROM_ARGB8888(*pixel, sr, sg, sb, sa), \
                        ARGB8888_FROM_RGBA(*pixel, sr, sg, sb, sa))

#define DRAW_SETPIXEL_ADD_ARGB8888 \
    DRAW_SETPIXEL_ADD(RGBA_FROM_ARGB8888(*pixel, sr, sg, sb, sa), \
                      ARGB8888_FROM_RGBA(*pixel, sr, sg, sb, sa))

#define DRAW_SETPIXELXY_ARGB8888(x, y) \
    DRAW_SETPIXELXY(x, y, Uint32, 4, DRAW_SETPIXEL_ARGB8888)

#define DRAW_SETPIXELXY_BLEND_ARGB8888(x, y) \
    DRAW_SETPIXELXY(x, y, Uint32, 4, DRAW_SETPIXEL_BLEND_ARGB8888)

#define DRAW_SETPIXELXY_ADD_ARGB8888(x, y) \
    DRAW_SETPIXELXY(x, y, Uint32, 4, DRAW_SETPIXEL_ADD_ARGB8888)

/*
 * Define draw operators for general RGB
 */

#define DRAW_SETPIXEL_RGB \
    DRAW_SETPIXEL(PIXEL_FROM_RGB(*pixel, fmt, sr, sg, sb))

#define DRAW_SETPIXEL_BLEND_RGB \
    DRAW_SETPIXEL_BLEND(RGB_FROM_PIXEL(*pixel, fmt, sr, sg, sb), \
                        PIXEL_FROM_RGB(*pixel, fmt, sr, sg, sb))

#define DRAW_SETPIXEL_ADD_RGB \
    DRAW_SETPIXEL_ADD(RGB_FROM_PIXEL(*pixel, fmt, sr, sg, sb), \
                      PIXEL_FROM_RGB(*pixel, fmt, sr, sg, sb))

#define DRAW_SETPIXELXY2_RGB(x, y) \
    DRAW_SETPIXELXY(x, y, Uint16, 2, DRAW_SETPIXEL_RGB)

#define DRAW_SETPIXELXY4_RGB(x, y) \
    DRAW_SETPIXELXY(x, y, Uint32, 4, DRAW_SETPIXEL_RGB)

#define DRAW_SETPIXELXY2_BLEND_RGB(x, y) \
    DRAW_SETPIXELXY(x, y, Uint16, 2, DRAW_SETPIXEL_BLEND_RGB)

#define DRAW_SETPIXELXY4_BLEND_RGB(x, y) \
    DRAW_SETPIXELXY(x, y, Uint32, 4, DRAW_SETPIXEL_BLEND_RGB)

#define DRAW_SETPIXELXY2_ADD_RGB(x, y) \
    DRAW_SETPIXELXY(x, y, Uint16, 2, DRAW_SETPIXEL_ADD_RGB)

#define DRAW_SETPIXELXY4_ADD_RGB(x, y) \
    DRAW_SETPIXELXY(x, y, Uint32, 4, DRAW_SETPIXEL_ADD_RGB)


/*
 * Define draw operators for general RGBA
 */

#define DRAW_SETPIXEL_RGBA \
    DRAW_SETPIXEL(PIXEL_FROM_RGBA(*pixel, fmt, sr, sg, sb, sa))

#define DRAW_SETPIXEL_BLEND_RGBA \
    DRAW_SETPIXEL_BLEND(RGBA_FROM_PIXEL(*pixel, fmt, sr, sg, sb, sa), \
                        PIXEL_FROM_RGBA(*pixel, fmt, sr, sg, sb, sa))

#define DRAW_SETPIXEL_ADD_RGBA \
    DRAW_SETPIXEL_ADD(RGBA_FROM_PIXEL(*pixel, fmt, sr, sg, sb, sa), \
                      PIXEL_FROM_RGBA(*pixel, fmt, sr, sg, sb, sa))

#define DRAW_SETPIXELXY4_RGBA(x, y) \
    DRAW_SETPIXELXY(x, y, Uint32, 4, DRAW_SETPIXEL_RGBA)

#define DRAW_SETPIXELXY4_BLEND_RGBA(x, y) \
    DRAW_SETPIXELXY(x, y, Uint32, 4, DRAW_SETPIXEL_BLEND_RGBA)

#define DRAW_SETPIXELXY4_ADD_RGBA(x, y) \
    DRAW_SETPIXELXY(x, y, Uint32, 4, DRAW_SETPIXEL_ADD_RGBA)

/*
 * Define line drawing macro
 */

#define ABS(_x) ((_x) < 0 ? -(_x) : (_x))

/* Horizontal line */
#define HLINE(type, op, draw_end) \
{ \
    int length; \
    int pitch = (dst->pitch / dst->format->BytesPerPixel); \
    type *pixel; \
    if (x1 <= x2) { \
        pixel = (type *)dst->pixels + y1 * pitch + x1; \
        length = draw_end ? (x2-x1+1) : (x2-x1); \
    } else { \
        pixel = (type *)dst->pixels + y1 * pitch + x2; \
        if (!draw_end) { \
            ++pixel; \
        } \
        length = draw_end ? (x1-x2+1) : (x1-x2); \
    } \
    while (length--) { \
        op; \
        ++pixel; \
    } \
}

/* Vertical line */
#define VLINE(type, op, draw_end) \
{ \
    int length; \
    int pitch = (dst->pitch / dst->format->BytesPerPixel); \
    type *pixel; \
    if (y1 <= y2) { \
        pixel = (type *)dst->pixels + y1 * pitch + x1; \
        length = draw_end ? (y2-y1+1) : (y2-y1); \
    } else { \
        pixel = (type *)dst->pixels + y2 * pitch + x1; \
        if (!draw_end) { \
            pixel += pitch; \
        } \
        length = draw_end ? (y1-y2+1) : (y1-y2); \
    } \
    while (length--) { \
        op; \
        pixel += pitch; \
    } \
}

/* Diagonal line */
#define DLINE(type, op, draw_end) \
{ \
    int length; \
    int pitch = (dst->pitch / dst->format->BytesPerPixel); \
    type *pixel; \
    if (y1 <= y2) { \
        pixel = (type *)dst->pixels + y1 * pitch + x1; \
        if (x1 <= x2) { \
            ++pitch; \
        } else { \
            --pitch; \
        } \
        length = (y2-y1); \
    } else { \
        pixel = (type *)dst->pixels + y2 * pitch + x2; \
        if (x2 <= x1) { \
            ++pitch; \
        } else { \
            --pitch; \
        } \
        if (!draw_end) { \
            pixel += pitch; \
        } \
        length = (y1-y2); \
    } \
    if (draw_end) { \
        ++length; \
    } \
    while (length--) { \
        op; \
        pixel += pitch; \
    } \
}

/* Bresenham's line algorithm */
#define BLINE(x1, y1, x2, y2, op, draw_end) \
{ \
    int i, deltax, deltay, numpixels; \
    int d, dinc1, dinc2; \
    int x, xinc1, xinc2; \
    int y, yinc1, yinc2; \
 \
    deltax = ABS(x2 - x1); \
    deltay = ABS(y2 - y1); \
 \
    if (deltax >= deltay) { \
        numpixels = deltax + 1; \
        d = (2 * deltay) - deltax; \
        dinc1 = deltay * 2; \
        dinc2 = (deltay - deltax) * 2; \
        xinc1 = 1; \
        xinc2 = 1; \
        yinc1 = 0; \
        yinc2 = 1; \
    } else { \
        numpixels = deltay + 1; \
        d = (2 * deltax) - deltay; \
        dinc1 = deltax * 2; \
        dinc2 = (deltax - deltay) * 2; \
        xinc1 = 0; \
        xinc2 = 1; \
        yinc1 = 1; \
        yinc2 = 1; \
    } \
 \
    if (x1 > x2) { \
        xinc1 = -xinc1; \
        xinc2 = -xinc2; \
    } \
    if (y1 > y2) { \
        yinc1 = -yinc1; \
        yinc2 = -yinc2; \
    } \
 \
    x = x1; \
    y = y1; \
 \
    if (!draw_end) { \
        --numpixels; \
    } \
    for (i = 0; i < numpixels; ++i) { \
        op(x, y); \
        if (d < 0) { \
            d += dinc1; \
            x += xinc1; \
            y += yinc1; \
        } else { \
            d += dinc2; \
            x += xinc2; \
            y += yinc2; \
        } \
    } \
}

/* Xiaolin Wu's line algorithm, based on Michael Abrash's implementation */
#define WULINE(x1, y1, x2, y2, opaque_op, blend_op, draw_end) \
{ \
    Uint16 ErrorAdj, ErrorAcc; \
    Uint16 ErrorAccTemp, Weighting; \
    int DeltaX, DeltaY, Temp, XDir; \
    unsigned r, g, b, a, inva; \
 \
    /* Draw the initial pixel, which is always exactly intersected by \
       the line and so needs no weighting */ \
    opaque_op(x1, y1); \
 \
    /* Draw the final pixel, which is always exactly intersected by the line \
       and so needs no weighting */ \
    if (draw_end) { \
        opaque_op(x2, y2); \
    } \
 \
    /* Make sure the line runs top to bottom */ \
    if (y1 > y2) { \
        Temp = y1; y1 = y2; y2 = Temp; \
        Temp = x1; x1 = x2; x2 = Temp; \
    } \
    DeltaY = y2 - y1; \
 \
    if ((DeltaX = x2 - x1) >= 0) { \
        XDir = 1; \
    } else { \
        XDir = -1; \
        DeltaX = -DeltaX; /* make DeltaX positive */ \
    } \
 \
    /* line is not horizontal, diagonal, or vertical */ \
    ErrorAcc = 0;  /* initialize the line error accumulator to 0 */ \
 \
    /* Is this an X-major or Y-major line? */ \
    if (DeltaY > DeltaX) { \
        /* Y-major line; calculate 16-bit fixed-point fractional part of a \
          pixel that X advances each time Y advances 1 pixel, truncating the \
          result so that we won't overrun the endpoint along the X axis */ \
        ErrorAdj = ((unsigned long) DeltaX << 16) / (unsigned long) DeltaY; \
        /* Draw all pixels other than the first and last */ \
        while (--DeltaY) { \
            ErrorAccTemp = ErrorAcc;   /* remember currrent accumulated error */ \
            ErrorAcc += ErrorAdj;      /* calculate error for next pixel */ \
            if (ErrorAcc <= ErrorAccTemp) { \
                /* The error accumulator turned over, so advance the X coord */ \
                x1 += XDir; \
            } \
            y1++; /* Y-major, so always advance Y */ \
            /* The IntensityBits most significant bits of ErrorAcc give us the \
             intensity weighting for this pixel, and the complement of the \
             weighting for the paired pixel */ \
            Weighting = ErrorAcc >> 8; \
            { \
                a = DRAW_MUL(_a, (Weighting ^ 255)); \
                r = DRAW_MUL(_r, a); \
                g = DRAW_MUL(_g, a); \
                b = DRAW_MUL(_b, a); \
                inva = (a ^ 0xFF); \
                blend_op(x1, y1); \
            } \
            { \
                a = DRAW_MUL(_a, Weighting); \
                r = DRAW_MUL(_r, a); \
                g = DRAW_MUL(_g, a); \
                b = DRAW_MUL(_b, a); \
                inva = (a ^ 0xFF); \
                blend_op(x1 + XDir, y1); \
            } \
        } \
    } else { \
        /* X-major line; calculate 16-bit fixed-point fractional part of a \
           pixel that Y advances each time X advances 1 pixel, truncating the \
           result to avoid overrunning the endpoint along the X axis */ \
        ErrorAdj = ((unsigned long) DeltaY << 16) / (unsigned long) DeltaX; \
        /* Draw all pixels other than the first and last */ \
        while (--DeltaX) { \
            ErrorAccTemp = ErrorAcc;   /* remember currrent accumulated error */ \
            ErrorAcc += ErrorAdj;      /* calculate error for next pixel */ \
            if (ErrorAcc <= ErrorAccTemp) { \
                /* The error accumulator turned over, so advance the Y coord */ \
                y1++; \
            } \
            x1 += XDir; /* X-major, so always advance X */ \
            /* The IntensityBits most significant bits of ErrorAcc give us the \
              intensity weighting for this pixel, and the complement of the \
              weighting for the paired pixel */ \
            Weighting = ErrorAcc >> 8; \
            { \
                a = DRAW_MUL(_a, (Weighting ^ 255)); \
                r = DRAW_MUL(_r, a); \
                g = DRAW_MUL(_g, a); \
                b = DRAW_MUL(_b, a); \
                inva = (a ^ 0xFF); \
                blend_op(x1, y1); \
            } \
            { \
                a = DRAW_MUL(_a, Weighting); \
                r = DRAW_MUL(_r, a); \
                g = DRAW_MUL(_g, a); \
                b = DRAW_MUL(_b, a); \
                inva = (a ^ 0xFF); \
                blend_op(x1, y1 + 1); \
            } \
        } \
    } \
}

#ifdef AA_LINES
#define AALINE(x1, y1, x2, y2, opaque_op, blend_op, draw_end) \
            WULINE(x1, y1, x2, y2, opaque_op, blend_op, draw_end)
#else
#define AALINE(x1, y1, x2, y2, opaque_op, blend_op, draw_end) \
            BLINE(x1, y1, x2, y2, opaque_op, draw_end)
#endif

/*
 * Define fill rect macro
 */

#define FILLRECT(type, op) \
do { \
    int width = rect->w; \
    int height = rect->h; \
    int pitch = (dst->pitch / dst->format->BytesPerPixel); \
    int skip = pitch - width; \
    type *pixel = (type *)dst->pixels + rect->y * pitch + rect->x; \
    while (height--) { \
        { int n = (width+3)/4; \
            switch (width & 3) { \
            case 0: do {   op; pixel++; \
            case 3:        op; pixel++; \
            case 2:        op; pixel++; \
            case 1:        op; pixel++; \
                    } while ( --n > 0 ); \
            } \
        } \
        pixel += skip; \
    } \
} while (0)

/* vi: set ts=4 sw=4 expandtab: */
