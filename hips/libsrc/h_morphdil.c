/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_morphdil.c  - apply the morphological operator of dilation
 *
 * pixel formats: BYTE
 *
 * H_morphdil applies the dilation operation to an image.  Although it
 * applies to byte-formatted images, it effectively treats the image as a
 * binary image, where dark pixels (with grey levels below that specified) are
 * treated as `foreground' or `object' pixels, and others are treated as
 * background.  The dilation of the input image is controlled by another HIPS
 * image, specified by hde, the structuring element.  For each foreground
 * element in the image, the structuring element is centered on that pixel.  All
 * other pixels lying at the same position as foreground pixels of the
 * structuring element will be set to foreground if they are not already
 * foreground pixels.  Actually, they are set to the darkest value that
 * overlaps that pixel position (among the structural element pixels that
 * can replace it, and its former value in the input image).  Background input
 * image pixels which can not be replaced by any structural element foreground
 * pixel are left unchanged in the output image.  Foreground input image
 * pixels which can not be replaced are set to 255.
 *
 * The center of the structure element as specified by centerr and centerc
 * is centered over the pixel, so the user may determine the shift caused by
 * the dilation operation.
 * The program dilates the ROI of the input image by the ROI
 * of the structuring element, and stores the result in the ROI of the output
 * image.
 *
 * Ahmed. Abbood 19/10/1988
 * rewritten by Michael Landy 10/29/89
 * HIPS 2 - msl - 8/3/91
 */

#include <hipl_format.h>

#define inactive 255

int h_morphdil(hdi,hde,hdo,centerr,centerc,gray)

struct header *hdi,*hde,*hdo;
int centerr,centerc,gray;

{
	switch(hdi->pixel_format) {
	case PFBYTE:	return(h_morphdil_b(hdi,hde,hdo,centerr,centerc,gray));
	default:	return(perr(HE_FMTSUBR,"h_morphdil",
				hformatname(hdi->pixel_format)));
	}
}

int h_morphdil_b(hdi,hde,hdo,centerr,centerc,gray)

struct header *hdi,*hde,*hdo;
int centerr,centerc,gray;

{
	return(h_morphdil_B(hdi->firstpix,hde->firstpix,
		hdo->firstpix,hdi->rows,hdi->cols,hdi->ocols,hdo->ocols,
		hde->rows,hde->cols,hde->ocols,centerr,centerc,gray));
}

int h_morphdil_B(imagei,imagee,imageo,nr,nc,nlpi,nlpo,nre,nce,nlpe,centerr,
	centerc,gray)

byte *imagei,*imagee,*imageo;
int nr,nc,nlpi,nlpo,nre,nce,nlpe,centerr,centerc,gray;

{
	int i,j,nexi,nexe,nexo,x,y,dx,dy;
	byte *pi,*pi2,*pe,*po;

	nexi = nlpi-nc;
	nexe = nlpe-nce;
	nexo = nlpo-nc;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++)
			*po++ = inactive;
		po += nexo;
	}
	pe = imagee;
	for (y=0;y<nre;y++) {
	  for (x=0;x<nce;x++) {
	    if (*pe <= gray) {
	      dy = y - centerr;
	      dx = x - centerc;
	      pi = imagei;
	      pi2 = imagei + nlpi*dy + dx;
	      po = imageo + nlpo*dy + dx;
	      for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
		  if (*pi <= gray) {
		    if (((i+dy)>=0) && ((i+dy)<nr) &&
			((j+dx)>=0) && ((j+dx)<nc)) {
			  if (*pe < *po)
			    *po = *pe;
			  if (*pi2 < *po)
			    *po = *pi2;
		    }
		  }
		  pi++; pi2++; po++;
		}
		pi += nexi;
		pi2 += nexi;
		po += nexo;
	      }
	    }
	    pe++;
	  }
	  pe += nexe;
	}
	po = imageo;
	pi = imagei;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			if ((*pi > gray) && (*po == inactive))
				*po = *pi;
			pi++; po++;
		}
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}
