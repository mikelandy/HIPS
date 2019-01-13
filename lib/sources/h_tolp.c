/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_tolp.c - conversions to LSBFIRST packed format
 *
 * Michael Landy - 1/4/91
 */

#include <hipl_format.h>

int h_tolp(hdi,hdo)

struct header *hdi,*hdo;

{
	switch(hdi->pixel_format) {
	case PFBYTE:	return(h_btolp(hdi,hdo));
	case PFINT:	return(h_itolp(hdi,hdo));
	default:	return(perr(HE_FMTSUBR,"h_tolp",
				hformatname(hdi->pixel_format)));
	}
}

int h_btolp(hdi,hdo)

struct header *hdi,*hdo;

{
	int i,j,nr,nc,nfullb,nextra;
	register byte val;
	byte *pi;
	byte *po;

	hips_lclip = hips_hclip = 0;
	pi = hdi->image;
	po = hdo->image;
	nr = hdi->orows;
	nc = hdi->ocols;
	nfullb = nc/8;
	nextra = nc%8;
	for (i=0;i<nr;i++) {
		for (j=0;j<nfullb;j++) {
			val = 0;
			if (*pi++) val |= 01;
			if (*pi++) val |= 02;
			if (*pi++) val |= 04;
			if (*pi++) val |= 010;
			if (*pi++) val |= 020;
			if (*pi++) val |= 040;
			if (*pi++) val |= 0100;
			if (*pi++) val |= 0200;
			*po++ = val;
		}
		switch(nextra) {
		case 0: break;
		case 1: val = 0;
			if (*pi++) val |= 01;
			*po++ = val;
			break;
		case 2: val = 0;
			if (*pi++) val |= 01;
			if (*pi++) val |= 02;
			*po++ = val;
			break;
		case 3: val = 0;
			if (*pi++) val |= 01;
			if (*pi++) val |= 02;
			if (*pi++) val |= 04;
			*po++ = val;
			break;
		case 4: val = 0;
			if (*pi++) val |= 01;
			if (*pi++) val |= 02;
			if (*pi++) val |= 04;
			if (*pi++) val |= 010;
			*po++ = val;
			break;
		case 5: val = 0;
			if (*pi++) val |= 01;
			if (*pi++) val |= 02;
			if (*pi++) val |= 04;
			if (*pi++) val |= 010;
			if (*pi++) val |= 020;
			*po++ = val;
			break;
		case 6: val = 0;
			if (*pi++) val |= 01;
			if (*pi++) val |= 02;
			if (*pi++) val |= 04;
			if (*pi++) val |= 010;
			if (*pi++) val |= 020;
			if (*pi++) val |= 040;
			*po++ = val;
			break;
		case 7: val = 0;
			if (*pi++) val |= 01;
			if (*pi++) val |= 02;
			if (*pi++) val |= 04;
			if (*pi++) val |= 010;
			if (*pi++) val |= 020;
			if (*pi++) val |= 040;
			if (*pi++) val |= 0100;
			*po++ = val;
			break;
		}
	}
	if (hdo->fcol % 8 != 0) {
		clearroi(hdo);
		return(perr(HE_ROI8C,"h_btolp"));
	}
	return(HIPS_OK);
}

int h_itolp(hdi,hdo)

struct header *hdi,*hdo;

{
	int i,j,nr,nc,nfullb,nextra;
	register byte val;
	int *pi;
	byte *po;

	hips_lclip = hips_hclip = 0;
	pi = (int *) hdi->image;
	po = hdo->image;
	nr = hdi->orows;
	nc = hdi->ocols;
	nfullb = nc/8;
	nextra = nc%8;
	for (i=0;i<nr;i++) {
		for (j=0;j<nfullb;j++) {
			val = 0;
			if (*pi++) val |= 01;
			if (*pi++) val |= 02;
			if (*pi++) val |= 04;
			if (*pi++) val |= 010;
			if (*pi++) val |= 020;
			if (*pi++) val |= 040;
			if (*pi++) val |= 0100;
			if (*pi++) val |= 0200;
			*po++ = val;
		}
		switch(nextra) {
		case 0: break;
		case 1: val = 0;
			if (*pi++) val |= 01;
			*po++ = val;
			break;
		case 2: val = 0;
			if (*pi++) val |= 01;
			if (*pi++) val |= 02;
			*po++ = val;
			break;
		case 3: val = 0;
			if (*pi++) val |= 01;
			if (*pi++) val |= 02;
			if (*pi++) val |= 04;
			*po++ = val;
			break;
		case 4: val = 0;
			if (*pi++) val |= 01;
			if (*pi++) val |= 02;
			if (*pi++) val |= 04;
			if (*pi++) val |= 010;
			*po++ = val;
			break;
		case 5: val = 0;
			if (*pi++) val |= 01;
			if (*pi++) val |= 02;
			if (*pi++) val |= 04;
			if (*pi++) val |= 010;
			if (*pi++) val |= 020;
			*po++ = val;
			break;
		case 6: val = 0;
			if (*pi++) val |= 01;
			if (*pi++) val |= 02;
			if (*pi++) val |= 04;
			if (*pi++) val |= 010;
			if (*pi++) val |= 020;
			if (*pi++) val |= 040;
			*po++ = val;
			break;
		case 7: val = 0;
			if (*pi++) val |= 01;
			if (*pi++) val |= 02;
			if (*pi++) val |= 04;
			if (*pi++) val |= 010;
			if (*pi++) val |= 020;
			if (*pi++) val |= 040;
			if (*pi++) val |= 0100;
			*po++ = val;
			break;
		}
	}
	if (hdo->fcol % 8 != 0) {
		clearroi(hdo);
		return(perr(HE_ROI8C,"h_itolp"));
	}
	return(HIPS_OK);
}
