/*
 copied from pngtopnm.c and modified by M.Itakura <itakura @ users.sourceforge.net>
 (mostly omitted and added a few lines)
 only color paletted image is handled
*/

/*
** pngtopnm.c -
** read a Portable Network Graphics file and produce a portable anymap
**
** Copyright (C) 1995,1998 by Alexander Lehmann <alex@hal.rhein-main.de>
**                        and Willem van Schaik <willem@schaik.com>
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
**
** modeled after giftopnm by David Koblas and
** with lots of bits pasted from libpng.txt by Guy Eric Schalnat
*/

#define VERSION "2.37.1 (3 July 1998)"

#include <stdio.h>
#include <stdlib.h>

#include "png.h"

#include <X11/Xlib.h>
#include <X11/X.h>
#include <X11/Xutil.h>

#define pm_message printf
#define pm_error(x) {fprintf(stderr,x);return NULL;}

//X11
extern unsigned long create_pixel(unsigned int red, unsigned int green, unsigned int blue);
//X11
extern XImage *ImgCreateSimple(int wx, int wy);

#  define TRUE 1
#  define FALSE 0
#  define NONE 0

#define SIG_CHECK_SIZE 4

XImage *read_png (char *fname)
{
  // port: libpng 1.6 accessors; 8-bit paletted or gray images only
  FILE *ifp = fopen(fname, "rb");
  if (!ifp) pm_error ("file not found");

  png_structp png_ptr = png_create_read_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  png_infop info_ptr = png_create_info_struct (png_ptr);
  if (setjmp (png_jmpbuf (png_ptr)))
    pm_error ("setjmp returns error condition");

  png_init_io (png_ptr, ifp);
  png_read_info (png_ptr, info_ptr);
  if (png_get_bit_depth (png_ptr, info_ptr) < 8)
    png_set_packing (png_ptr);

  unsigned int w = png_get_image_width (png_ptr, info_ptr);
  unsigned int h = png_get_image_height (png_ptr, info_ptr);
  unsigned long pix_table[256];
  unsigned int i;

  if (png_get_color_type (png_ptr, info_ptr) == PNG_COLOR_TYPE_PALETTE)
  {
      png_colorp pal;
      int npal;
      png_get_PLTE (png_ptr, info_ptr, &pal, &npal);
      for (i = 0; i < (unsigned int) npal; i++)
          pix_table[i] = create_pixel(pal[i].red, pal[i].green, pal[i].blue);
  }
  else
  {
      for (i = 0; i < 256; i++)
          pix_table[i] = create_pixel(i, i, i);
  }

  png_bytep *rows = (png_bytep *) malloc (h * sizeof (png_bytep));
  for (i = 0; i < h; i++)
      rows[i] = (png_bytep) malloc (png_get_rowbytes (png_ptr, info_ptr));
  png_read_image (png_ptr, rows);
  png_read_end (png_ptr, info_ptr);

  XImage *res = ImgCreateSimple(w, h);
  for (unsigned int y = 0; y < h; y++)
  {
      for (unsigned int x = 0; x < w; x++)
          XPutPixel(res, x, y, pix_table[rows[y][x]]);
      free (rows[y]);
  }
  free (rows);
  png_destroy_read_struct (&png_ptr, &info_ptr, NULL);
  fclose(ifp);
  return res;
}
