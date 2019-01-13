/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_morphero.c  - apply the morphological operator of erosion
 *
 * pixel formats: BYTE
 *
 * H_morphero applies the erosion operation to an image.  Although it
 * applies to byte-formatted images, it effectively treats the image as a
 * binary image, where dark pixels (with grey levels below 128) are treated as
 * `foreground' or `object' pixels, and others are treated as background.  The
 * erosion of the input subimage is controlled by another subimage, specified
 * by hde, the structuring element.  For each foreground element
 * in the image, the structuring element is centered on that pixel.  All
 * other pixels lying at the same position as foreground pixels of the
 * structuring element will receive a `vote'.  After applying this procedure
 * to all foreground image elements, the program computes the maximum number
 * of `votes' received by any image pixel (the best foreground fit with the
 * structuring element).  Then, pixels with that many votes are set to the
 * darkest value that overlaps that pixel position (among the structural
 * element pixels that overlapped, and its former value in the input image).
 * Foreground pixels which received fewer votes are set to 255. Background
 * input image pixels which received fewer votes are left unchanged in the
 * output image.
 *
 * The program erodes the ROI of the input image by the ROI
 * of the structuring element, and stores the result in the ROI of the output
 * image.
 *
 * The element image is effectively
 * reflected about its vertical and horizontal axes before it is used.
 *
 * The center of the structure element as specified by centerr and centerc
 * is centered over the pixel, so the user may determine the shift caused by
 * the dilation operation.
 *
 * The votes are stored in the subimage specified by hdt, which must have
 * the same size as the input image and be in integer format.
 *
 * Ahmed. Abbood 19/10/1988
 * rewritten by Michael Landy 10/29/89
 * HIPS 2 - msl - 8/3/91
 */

#include <hipl_format.h>

#define inactive 255

int h_morphero(hdi,hde,hdt,hdo,centerr,centerc,gray)

struct header *hdi,*hde,*hdt,*hdo;
int centerr,centerc,gray;

{
	switch(hdi->pixel_format) {
	case PFBYTE:	return(h_morphero_b(hdi,hde,hdt,hdo,centerr,centerc,
				gray));
	default:	return(perr(HE_FMTSUBR,"h_morphero",
				hformatname(hdi->pixel_format)));
	}
}

int h_morphero_b(hdi,hde,hdt,hdo,centerr,centerc,gray)

struct header *hdi,*hde,*hdt,*hdo;
int centerr,centerc,gray;

{
	if (hdt->pixel_format != PFINT)
		return(perr(HE_FMT3SUBR,"h_morphero_b",
			hformatname(hdi->pixel_format),
			hformatname(hdt->pixel_format),
			hformatname(hdo->pixel_format)));
	return(h_morphero_B(hdi->firstpix,hde->firstpix,(int *) hdt->firstpix,
		hdo->firstpix,hdi->rows,hdi->cols,hdi->ocols,hdo->ocols,
		hdt->ocols,hde->rows,hde->cols,hde->ocols,
		centerr,centerc,gray));
}

int h_morphero_B(imagei,imagee,imaget,imageo,nr,nc,nlpi,nlpo,nlpt,nre,nce,nlpe,
	centerr,centerc,gray)

byte *imagei,*imagee,*imageo;
int *imaget,nr,nc,nlpi,nlpo,nlpt,nre,nce,nlpe,centerr,centerc,gray;

{
	int i,j,nexi,nexe,nexo,next,x,y,dx,dy,*pt,maxv;
	byte *pi,*pi2,*pe,*po;

	nexi = nlpi-nc;
	nexe = nlpe-nce;
	nexo = nlpo-nc;
	next = nlpt-nc;
	po = imageo;
	pt = imaget;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			*po++ = inactive;
			*pt++ = 0;
		}
		po += nexo;
		pt += next;
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
	      pt = imaget + nlpt*dy + dx;
	      for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
		  if (*pi <= gray) {
		    if (((i+dy)>=0) && ((i+dy)<nr) &&
			((j+dx)>=0) && ((j+dx)<nc)) {
			  *pt += 1;
			  if (*pe < *po)
			    *po = *pe;
			  if (*pi2 < *po)
			    *po = *pi2;
		    }
		  }
		  pi++; pi2++; po++; pt++;
		}
		pi += nexi;
		pi2 += nexi;
		po += nexo;
		pt += next;
	      }
	    }
	    pe++;
	  }
	  pe += nexe;
	}
	pt = imaget;
	maxv = 0;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			if (*pt > maxv)
				maxv = *pt;
			pt++;
		}
		pt += next;
	}
	po = imageo;
	pi = imagei;
	pt = imaget;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			if (*pt < maxv) {
				if (*pi <= gray)
					*po = inactive;
				else
					*po = *pi;
			}
			pi++; po++; pt++;
		}
		pi += nexi;
		po += nexo;
		pt += next;
	}
	return(HIPS_OK);
}
