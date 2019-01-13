#ifndef lint
  static char SccSID[] = "@(#)to_hips.c	1.4 7/18/89";
#endif
/*
  Copyright 1989 Alan Shaw and Eric Schwartz.
  No part of this software may be distributed or sold without the prior
  agreement of Prof. Eric Schwartz, Dept. of Psychiatry, NYU School of
  Medicine, 550 1st Ave., New York, New York, 10016.
  to_hips.c
  */

#include	<stdio.h>
#include	<pixrect/pixrect_hs.h>

#include	<hipl_format.h>

#define to32(a)         (((int)(a) + 3) & ~3)

to_hips(rh, cp, image_in, hd, image_out, magfactor)
  struct rasterfile	*rh;
  colormap_t		*cp;
  u_char			*image_in;
  struct	header		*hd;
  u_char			*image_out;
  int			magfactor;
{
  register u_char	*s1, *s2;
  register	size;
  int		rowbytes_out, rowbytes_in, Mrowbytes_in, addabyte;
  
  register u_char	*pimage_in, *pimage_out, *too_many;
  
  hd->orows = hd->rows = rh->ras_height / magfactor;
  hd->ocols = hd->cols = rh->ras_width / magfactor;
  
  switch(rh->ras_depth) {
    case 1:
#ifdef	MSBFVERSION
      hd->pixel_format = PFMSBF;
#else
      hd->pixel_format = PFLSBF;
#endif	MSBFVERSION
      rowbytes_out = ((hd->ocols - 1) >> 3) + 1;
      break;

    case 8:
      rowbytes_out = hd->ocols;
      break;

    default:
      fprintf(stderr, "to_hips: pixel format must be byte");
      fprintf(stderr, " or bit\n");
      return(-1);
    }
  
  rowbytes_in = mpr_linebytes(rh->ras_width, rh->ras_depth);
  addabyte = to32(rowbytes_out) - rh->ras_width / magfactor ;
  too_many = image_out + to32(rowbytes_out) * hd->orows;
  Mrowbytes_in = magfactor * to32(rowbytes_in);
  
  if ((cp != NULL) && (cp->length > 0)) {
      int i;
      u_char *ptr, *colorstring;
      
      /* make colormap 3 * cp->length long, no null char */
      colorstring = (u_char *) malloc(3 * cp->length);
      if (colorstring == NULL) {
	  fprintf(stderr, "%s: can't allocate core\n", "to_hips");
	  return(-1);
	}
      
      ptr = colorstring;
      for (i = 0; i < cp->length; i++) {
	  *ptr++ = cp->map[0][i];
	}
      for (i = 0; i < cp->length; i++) {
	  *ptr++ = cp->map[1][i];
	}
      for (i = 0; i < cp->length; i++) {
	  *ptr++ = cp->map[2][i];
	}
      setparam(hd, "cmap", PFBYTE, 3 * cp->length, colorstring);
      write_header(hd);

      free(colorstring);
    }
  
#ifdef	ULORIG
  for (pimage_in = image_in, pimage_out = image_out;
       pimage_out < too_many;
       pimage_out += rowbytes_out, pimage_in += Mrowbytes_in) {
#else
      for (pimage_in = image_in + (rh->ras_height - 1) * to32(rowbytes_in),
	   pimage_out = image_out;
	   pimage_out < too_many;
	   pimage_out += rowbytes_out, pimage_in -= Mrowbytes_in) {
#endif	ULORIG
	  register i;
	  
	  s1 = pimage_out, s2 = pimage_in, size = rowbytes_out; 
	  while (size--) {
	      *s1++ = *s2; 
	      s2 += magfactor;
	    }
	  
	  for (i = 0; i < addabyte; i++)
	    *s1++ = '\0';
	}
      pimage_out += addabyte;
      
      
      return(0);
    }
