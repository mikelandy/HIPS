#ifndef lint
  static char SccSID[] = "@(#)to_sun.c	1.5 7/20/89";
#endif
/*
  Copyright 1988 Alan Shaw and Eric Schwartz.
  No part of this software may be distributed or sold without the prior
  agreement of Prof. Eric Schwartz, Dept. of Psychiatry, NYU School of
  Medicine, 550 1st Ave., New York, New York, 10016.
  to_sun.c
  Revision history:
  hek Thu Dec 20 13:51:26 EST 1990
  Added add32byte to pad the input column width to a multiple of
  32. Once again, hd.ocols is filtered through to32(), (see 
  history note in ctool.c).
  */

#include	<stdio.h>
#include	<pixrect/pixrect_hs.h>

#include	<hipl_format.h>

#define to32(a)         (((int)(a) + 3) & ~3)

to_sun(hd, imagebuf, rh, cp, image_out, magfactor)
  struct header		*hd;
  u_char			*imagebuf;
  struct rasterfile	*rh;
  colormap_t		*cp;
  u_char			*image_out;
  int			magfactor;
{
  int rowbytes_out, Mrowbytes_out, rowbytes_in, addabyte, add32byte,
  cmsize, i, j, size;
  u_char *s1, *s2, *pimage_in, *pimage_out, *too_many, *image_in;
  
  rh->ras_magic	= RAS_MAGIC;
  rh->ras_width	= magfactor * hd->ocols;
  rh->ras_height= magfactor * hd->orows;

  /* set the raster depth based on the pixel format; all pixel formats
     save PFSRLE/PFLSBF/PFMSBF are 8 bits deep */
  rh->ras_depth	= (hd->pixel_format == PFSRLE || hd->pixel_format == PFMSBF ||
	hd->pixel_format == PFLSBF) ? 1 : 8;

#ifdef	MSBFVERSION
  if (hd->pixel_format == PFLSBF)
    {
      fprintf(stderr,"to_sun: pixel_format must be MSBFIRST\n");
#else
  if (hd->pixel_format == PFMSBF) {
      fprintf(stderr,"to_sun: pixel_format must be LSBFIRST\n");
#endif	MSBFVERSION
      return(-1);
    }
/*	} */
      switch (rh->ras_depth) {
	case 1:
	  rowbytes_in = ((hd->ocols - 1) >> 3) + 1;
	  break;

	case 8:
	  rowbytes_in = hd->ocols;
	  break;

	default:
	  fprintf(stderr,"to_sun: bad depth\n");
	  return(-1);
	}
      rowbytes_out	= mpr_linebytes(rh->ras_width, rh->ras_depth);
      rh->ras_length	= to32(rowbytes_out) * rh->ras_height;
      rh->ras_type	= RT_STANDARD;
      
      if (cp != NULL && findparam(hd, "cmap"))
	{
	  u_char *colorbuf;
	  u_char *colorptr;
	  
	  cmsize = 2;
	  getparam(hd, "cmap", PFBYTE, &cmsize, &colorbuf);
	  
	  if (cmsize > 0)
	    {
	      rh->ras_maptype = cp->type = 1;
	      rh->ras_maplength = cmsize;
	      cp->length = cmsize / 3;
	      
	      colorptr = colorbuf;
	      for (i = 0; i < cp->length; i++) {
		cp->map[0][i] = colorptr[0];
		cp->map[1][i] = colorptr[cp->length];
		cp->map[2][i] = colorptr[(cp->length) * 2];
		colorptr++;
	      }
	      free(colorbuf);
	    }
	  else {
	    rh->ras_maptype = RMT_NONE;
	    rh->ras_maplength = 0;
	    cp = NULL;
	  }
	  
	}            
	  else {
	    rh->ras_maptype = RMT_NONE;
	    rh->ras_maplength = 0;
	    cp = NULL;
	  }
      
      image_in = imagebuf;
      addabyte = to32(rowbytes_out) - magfactor * rowbytes_in;
      too_many = image_out + rh->ras_length;
      Mrowbytes_out = magfactor * to32(rowbytes_out);
      
      /* compute how many extra 0 bytes must be added to the column of
	 pixels in order to have pixrect display it properly, ie. must
	 be a multiple of 32 
	 add32byte = (to32(hd->ocols) - hd->ocols); */
      
#ifdef	ULORIG
      for (pimage_in = image_in, pimage_out = image_out;
	   pimage_out < too_many;
	   pimage_out += Mrowbytes_out, pimage_in += rowbytes_in) {
#else
	  for (pimage_in = image_in +
	       (hd->orows - 1) * rowbytes_in * sizeof(u_char),
	       pimage_out = image_out;
	       pimage_out < too_many;
	       pimage_out += Mrowbytes_out, pimage_in -= rowbytes_in) {
#endif	ULORIG
	      s1 = pimage_out;
	      for (j = 0; j < magfactor; j++) {
		  s2 = pimage_in, size = rowbytes_in;
		  while (size--) {
		      for (i = 0; i < magfactor; i++)
			*s1++	= *s2;
		      s2++;
		    }
		  for (i = 0; i < addabyte; i++)
		    *s1++	= '\0';
		  
		  /* pad to 32-bit boundary 
		     for (i = 0; i < add32byte; i++)
		     *s1++	= '\0'; */
		}
	    }
	  
	  return(0);
	}
