/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_minroi.c - subroutines to compute a minimum useful region
 *
 * These routines compute the minimum portion of an image which bounds the
 * image pixels whose values are less than or equal to a supplied value.  It
 * is used to compute the useful portion of a structuring element for
 * h_morphdil and h_morphero.  The high level routine resets the ROI to that
 * region.  The low-level routine returns the first and last useful row and
 * column.
 *
 * pixel formats: BYTE
 *
 * HIPS 2 - msl - 8/3/91
 */

#include <hipl_format.h>

int h_minroi(hd,gray)

struct header *hd;
int gray;

{
	switch(hd->pixel_format) {
	case PFBYTE:	return(h_minroi_b(hd,gray));
	default:	return(perr(HE_FMTSUBR,"h_minroi",
				hformatname(hd->pixel_format)));
	}
}

int h_minroi_b(hd,gray)

struct header *hd;
int gray;

{
	int frow,fcol,lrow,lcol,nrow,ncol;

	h_minroi_B(hd->firstpix,hd->rows,hd->cols,hd->ocols,&frow,&fcol,
		&lrow,&lcol,gray);
	nrow = (lrow-frow)+1;
	ncol = (lcol-fcol)+1;
	if (nrow < 0)
		nrow = ncol = 0;
	setroi(hd,frow,fcol,nrow,ncol);
	return(HIPS_OK);
}

int h_minroi_B(image,nr,nc,nlp,frow,fcol,lrow,lcol,gray)

byte *image;
int nr,nc,nlp,*frow,*fcol,*lrow,*lcol,gray;

{
	register int i,j,nex,sr,sc,er,ec;
	register byte *p;

	nex = nlp-nc;
	sr = nr;
	sc = nc;
	er = 0;
	ec = 0;
	p = image;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			if (*p++ <= gray) {
				if (i<sr)
					sr = i;
				if (j<sc)
					sc = i;
				if (i>er)
					er = i;
				if (j>ec)
					ec = j;
			}
		}
		p += nex;
	}
	*frow = sr;
	*lrow = er;
	*fcol = sc;
	*lcol = ec;
	return(HIPS_OK);
}
