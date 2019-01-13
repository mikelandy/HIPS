/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_thicken.c - thicken a white-on-black image
 *
 * A pixel is set to hips_hchar if either it, its lower, right, or lower-right
 * neighbors is nonzero.  For LLORIG, then it is set if it, its upper, right,
 * or upper-right neighbors is nonzero.  Otherwise it is set to hips_lchar.
 *
 * pixel formats: BYTE
 *
 * Mike Landy - 12/20/82
 * HIPS 2 - msl - 8/4/91
 */

#include <hipl_format.h>

int h_thicken(hdi,hdo)

struct header *hdi,*hdo;

{
	switch(hdi->pixel_format) {
	case PFBYTE:	return(h_thicken_b(hdi,hdo));
	default:	return(perr(HE_FMTSUBR,"h_thicken",
				hformatname(hdi->pixel_format)));
	}
}

int h_thicken_b(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_thicken_B(hdi->firstpix,hdo->firstpix,hdi->rows,hdi->cols,
	    hdi->ocols,hdo->ocols));
}

int h_thicken_B(imagei,imageo,nr,nc,nlpi,nlpo)

byte *imagei,*imageo;
int nr,nc,nlpi,nlpo;

{
	register byte *ip1,*ip2,*ip3,*ip4,*op;
	register int i,j;
	int nc1,nr1,nexi,nexo;

	ip1 = imagei;
	ip2 = imagei+1;
	ip3 = imagei+nlpi;
	ip4 = imagei+nlpi+1;
	op = imageo;
	nexi = nlpi-(nc-1);
	nexo = nlpo-(nc-1);
	nc1 = nc - 1;
	nr1 = nr - 1;
	for (i=0;i<nr1;i++) {
		for (j=0;j<nc1;j++) {
			*op++ =  (*ip1 || *ip2 || *ip3 || *ip4) ? hips_hchar :
				hips_lchar;
			ip1++; ip2++; ip3++; ip4++;
		}
			/* last column */
		*op = (*ip1 || *ip3) ? hips_hchar : hips_lchar;
		ip1 += nexi;
		ip2 += nexi;
		ip3 += nexi;
		ip4 += nexi;
		op += nexo;
	}
	for (j=0;j<nc1;j++) {
		*op++ = (*ip1 || *ip2) ? hips_hchar : hips_lchar;
		ip1++; ip2++;
	}
	*op = *ip1 ? hips_hchar : hips_lchar;	/* last pixel */
	return(HIPS_OK);
}
