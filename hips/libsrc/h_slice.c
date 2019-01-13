/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_slice.c - display slices through an image as a graph
 *
 * If vflag is FALSE, then a horizontal slice through the image is computed,
 * otherwise a vertical slice is computed.  The grey values in the row
 * or column specified are displayed as a bar graph.  For horizontal slices,
 * the number of columns in the output subimage is 2 wider than the input
 * subimage, and there are 257 rows (one for each possible nonzero grey level
 * plus 2 for a border).  For vertical slices, the same thing is reflected.
 *
 * pixel formats: BYTE
 *
 * Michael Landy - 8/4/87
 * HIPS 2 - msl - 6/29/91
 */

#include <hipl_format.h>

int h_slice(hdi,hdo,rowcol,vflag)

struct header *hdi,*hdo;
int rowcol;
h_boolean vflag;

{
	switch(hdi->pixel_format) {
	case PFBYTE:	return(h_slice_b(hdi,hdo,rowcol,vflag));
	default:	return(perr(HE_FMTSUBR,"h_slice",
				hformatname(hdi->pixel_format)));
	}
}

int h_slice_b(hdi,hdo,rowcol,vflag)

struct header *hdi,*hdo;
int rowcol;
h_boolean vflag;

{
	return(h_slice_B(hdi->firstpix,hdo->firstpix,hdi->rows,hdi->cols,
		hdi->ocols,hdo->ocols,rowcol,vflag));
}

int h_slice_B(imagei,imageo,nr,nc,nlpi,nlpo,rowcol,vflag)

byte *imagei,*imageo;
int nr,nc,nlpi,nlpo;
int rowcol;
h_boolean vflag;

{
	int or,oc,or1,oc1,valinc,ipinc,opinc,iorig,oorig,nslice,val,i,j;
	byte *op,*ip,*op2;

	if (vflag) {
		oc = 257;		/* vertical slice */
		or = nr + 2;
		if (rowcol < 0 || rowcol >= nc)
			return(perr(HE_RCSELN,"h_slice_B"));
		valinc = 1;
		ipinc = nlpi;
		opinc = nlpo;
		iorig = rowcol;
		oorig = nlpo+1;
		nslice = nr;
	}
	else {
		oc = nc + 2;		/* horizontal slice */
		or = 257;
		if (rowcol < 0 || rowcol >= nr)
			return(perr(HE_RCSELN,"h_slice_B"));
#ifdef ULORIG
		valinc = -nlpo;
#else
		valinc = nlpo;
#endif
		ipinc = 1;
		opinc = 1;
		iorig = rowcol*nlpi;
#ifdef ULORIG
		oorig = ((or-2)*nlpo) + 1;
#else
		oorig = nlpo+1;
#endif
		nslice = nc;
	}

	oc1 = oc-1; or1 = or-1;
	op = imageo;
	for (i=0;i<or;i++)
		for (j=0;j<oc;j++) {
			if (j==0 || j==oc1 || i==0 || i==or1)
				*op++ = 128;
			else
				*op++ = hips_lchar;
		}
	ip = imagei + iorig;
	op = imageo + oorig;
	for (i=0;i<nslice;i++) {
		val = *ip;
		op2 = op;
		for (j=0;j<val;j++) {
			*op2 = hips_hchar;
			op2 += valinc;
		}
		ip += ipinc;
		op += opinc;
	}
	return(HIPS_OK);
}
