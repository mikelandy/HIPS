/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_pyrdisp.c - convert an image pyramid to a single image for display
 *
 * input pixel formats: INTPYR, FLOATPYR
 * output pixel formats: INT, FLOAT
 *
 * H_pyrdisp converts an image pyramid to a single image for display
 * purposes.  There are two formats, either the default side-by-side format:
 *
 * 11111111 2222 33 4
 * 11111111 2222 33
 * 11111111 2222
 * 11111111 2222
 * 11111111
 * 11111111
 * 11111111
 * 11111111
 *
 * or a `compressed' format (cflag = TRUE):
 *
 * 11111111 2222
 * 11111111 2222
 * 11111111 2222
 * 11111111 2222
 * 11111111
 * 11111111 33 4
 * 11111111 33
 * 11111111
 *
 * The only difference is that the third and higher levels are shifted around
 * to a second row.  The user specifies the number of margin pixels between
 * each image. The background pixels are left untouched and should be
 * initialized in advance by the caller (e.g. using h_setimage).
 * Floating pyramids result in floating point images, and integer pyramids
 * result in integer pyramids.
 *
 * Mike Landy - 3/6/89
 * HIPS 2 - msl - 7/19/91
 */

#include <hipl_format.h>

int h_pyrdisp(pyr,botlev,toplev,hdo,cflag,margin)

FPYR pyr;
struct header *hdo;
h_boolean cflag;
int botlev,toplev,margin;

{
	switch(hdo->pixel_format) {
	case PFINT:	return(h_pyrdisp_i((IIMAGE *) pyr,botlev,toplev,hdo,
				cflag,margin));
	case PFFLOAT:	return(h_pyrdisp_f(pyr,botlev,toplev,hdo,cflag,margin));
	default:	return(perr(HE_FMTSUBR,"h_pyrdisp",
				hformatname(hdo->pixel_format)));
	}
}

int h_pyrdisp_i(pyr,botlev,toplev,hdo,cflag,margin)

IPYR pyr;
struct header *hdo;
h_boolean cflag;
int botlev,toplev,margin;

{
	return(h_pyrdisp_I(pyr,botlev,toplev,(int *) hdo->firstpix,hdo->ocols,
		cflag,margin));
}

int h_pyrdisp_f(pyr,botlev,toplev,hdo,cflag,margin)

FPYR pyr;
struct header *hdo;
h_boolean cflag;
int botlev,toplev,margin;

{
	return(h_pyrdisp_F(pyr,botlev,toplev,(float *) hdo->firstpix,hdo->ocols,
		cflag,margin));
}

int h_pyrdisp_I(pyr,botlev,toplev,imageo,nlpo,cflag,margin)

IPYR pyr;
int botlev,toplev,*imageo,nlpo,margin;
h_boolean cflag;

{
	int i,r,c,j,k,*op,*ip,fcol,frow,nexo;

	fcol = frow = 0;
	for (i=botlev;i<=toplev;i++) {
		r = pyr[i].nr;
		c = pyr[i].nc;
		nexo = nlpo - c;
		op = imageo + frow*nlpo + fcol;
		for (j=0;j<r;j++) {
			ip = pyr[i].ptr[j];
			for (k=0;k<c;k++)
				*op++ = *ip++;
			op += nexo;
		}
		if ((i==botlev) || (i>botlev+1) || (!cflag))
			fcol += margin + c;
		else
			frow += margin + r;
	}
	return(HIPS_OK);
}

int h_pyrdisp_F(pyr,botlev,toplev,imageo,nlpo,cflag,margin)

FPYR pyr;
int botlev,toplev,nlpo,margin;
float *imageo;
h_boolean cflag;

{
	int i,r,c,j,k,fcol,frow,nexo;
	float *op,*ip;

	fcol = frow = 0;
	for (i=botlev;i<=toplev;i++) {
		r = pyr[i].nr;
		c = pyr[i].nc;
		nexo = nlpo - c;
		op = imageo + frow*nlpo + fcol;
		for (j=0;j<r;j++) {
			ip = pyr[i].ptr[j];
			for (k=0;k<c;k++)
				*op++ = *ip++;
			op += nexo;
		}
		if ((i==botlev) || (i>botlev+1) || (!cflag))
			fcol += margin + c;
		else
			frow += margin + r;
	}
	return(HIPS_OK);
}
