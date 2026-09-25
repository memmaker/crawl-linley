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
  char sig_buf [SIG_CHECK_SIZE];
  png_struct *png_ptr;
  png_info *info_ptr;
  png_byte **png_image;
  png_byte *png_pixel;
  unsigned int x, y;
  int linesize;
  png_uint_16 c;
  unsigned int i;

  //X11
  XImage *res;
  //X11
  unsigned long pix_table[256];


  FILE *ifp = fopen(fname,"r");

  if(!ifp) pm_error ("file not found");

  if (fread (sig_buf, 1, SIG_CHECK_SIZE, ifp) != SIG_CHECK_SIZE)
    pm_error ("input file empty or too short");
  if (png_sig_cmp ((unsigned char *)sig_buf, (png_size_t) 0, (png_size_t) SIG_CHECK_SIZE) != 0)
    pm_error ("input file not a PNG file");

  png_ptr = png_create_read_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (png_ptr == NULL) {
    pm_error ("cannot allocate LIBPNG structure");
  }
  info_ptr = png_create_info_struct (png_ptr);
  if (info_ptr == NULL) {
    png_destroy_read_struct (&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
    pm_error ("cannot allocate LIBPNG structures");
  }

  if (setjmp (png_ptr->jmpbuf)) {
    png_destroy_read_struct (&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
    free (png_ptr);
    free (info_ptr);
    pm_error ("setjmp returns error condition");
  }

  png_init_io (png_ptr, ifp);
  png_set_sig_bytes (png_ptr, SIG_CHECK_SIZE);
  png_read_info (png_ptr, info_ptr);



  png_image = (png_byte **)malloc (info_ptr->height * sizeof (png_byte*));
  if (png_image == NULL) {
    free (png_ptr);
    free (info_ptr);
    pm_error ("couldn't alloc space for image");
  }

  if (info_ptr->bit_depth == 16)
    linesize = 2 * info_ptr->width;
  else
    linesize = info_ptr->width;

  if (info_ptr->color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
    linesize *= 2;
  else
  if (info_ptr->color_type == PNG_COLOR_TYPE_RGB)
    linesize *= 3;
  else
  if (info_ptr->color_type == PNG_COLOR_TYPE_RGB_ALPHA)
    linesize *= 4;

  for (y = 0 ; y < info_ptr->height ; y++) {
    png_image[y] = (png_byte *)malloc (linesize);
    if (png_image[y] == NULL) {
      for (x = 0 ; x < y ; x++)
        free (png_image[x]);
      free (png_image);
      free (png_ptr);
      free (info_ptr);
      pm_error ("couldn't alloc space for image");
    }
  }

  if (info_ptr->bit_depth < 8)
    png_set_packing (png_ptr);

  /* sBIT handling is very tricky. If we are extracting only the image, we
     can use the sBIT info for grayscale and color images, if the three
     values agree. If we extract the transparency/alpha mask, sBIT is
     irrelevant for trans and valid for alpha. If we mix both, the
     multiplication may result in values that require the normal bit depth,
     so we will use the sBIT info only for transparency, if we know that only
     solid and fully transparent is used */

  if (info_ptr->valid & PNG_INFO_sBIT) {

        if ((info_ptr->color_type == PNG_COLOR_TYPE_PALETTE ||
             info_ptr->color_type == PNG_COLOR_TYPE_RGB ||
             info_ptr->color_type == PNG_COLOR_TYPE_RGB_ALPHA) &&
            (info_ptr->sig_bit.red != info_ptr->sig_bit.green ||
             info_ptr->sig_bit.red != info_ptr->sig_bit.blue) ) {
	  pm_message ("different bit depths for color channels not supported");
	  pm_message ("writing file with %d bit resolution", info_ptr->bit_depth);
        } else 
          if ((info_ptr->color_type == PNG_COLOR_TYPE_PALETTE) &&
	      (info_ptr->sig_bit.red < 255)) {
	    for (i = 0 ; i < info_ptr->num_palette ; i++) {
	      info_ptr->palette[i].red   >>= (8 - info_ptr->sig_bit.red);
	      info_ptr->palette[i].green >>= (8 - info_ptr->sig_bit.green);
	      info_ptr->palette[i].blue  >>= (8 - info_ptr->sig_bit.blue);
	    }

          } else 
          if ((info_ptr->color_type == PNG_COLOR_TYPE_GRAY ||
               info_ptr->color_type == PNG_COLOR_TYPE_GRAY_ALPHA) &&
	      (info_ptr->sig_bit.gray < info_ptr->bit_depth)) {
	    png_set_shift (png_ptr, &(info_ptr->sig_bit));
          }


      }

  if (info_ptr->color_type == PNG_COLOR_TYPE_PALETTE) 
  {
//X11
        for (i = 0 ; i < info_ptr->num_palette ; i++) 
          pix_table[i] = create_pixel(info_ptr->palette[i].red, 
           info_ptr->palette[i].green, info_ptr->palette[i].blue);
  }
  else
  if (info_ptr->color_type == PNG_COLOR_TYPE_GRAY)
  {
        for (i = 0 ; i < 256 ; i++)
//X11
          pix_table[i] = create_pixel(i, i, i);
  }

  png_read_image (png_ptr, png_image);
  png_read_end (png_ptr, info_ptr);

  res = ImgCreateSimple(info_ptr->width, info_ptr->height);

  for (y = 0 ; y < info_ptr->height ; y++) {
    png_pixel = png_image[y];
    for (x = 0 ; x < info_ptr->width ; x++) {
      c = *png_pixel;
      png_pixel++;
      XPutPixel(res, x, y, pix_table[c]);
    }
  }

  for (y = 0 ; y < info_ptr->height ; y++)
    free (png_image[y]);
  free (png_image);
  free (png_ptr);
  free (info_ptr);

  fclose(ifp);
  return res;
}

