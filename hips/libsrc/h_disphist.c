/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_disphist.c - display histograms as a bar graph
 *
 * Each histogram bar is barwidth pixels wide and barheight pixels tall.  The
 * entire histogram has a border of width borderw and grey level borderg, and a
 * stripe of the same width and greylevel separates the underflow and overflow
 * bins from the main histogram.  The main histogram bars have greylevel
 * hips_lchar on a background of hips_hchar.  The underflow and overflow bars
 * have a background of greylevel borderg so that they disappear into the
 * background if empty.  The output image must have the appropriate size to
 * accomodate the histogram:  barheight+2*borderw rows and
 * (nbins+2)*barwidth+4*borderw columns.  Histogram counts are scaled so that
 * a count of maxcnt corresponds to one full bar height.
 *
 * pixel formats: HISTOGRAM -> BYTE
 *
 * Michael Landy - 12/15/82
 * HIPS 2 - msl - 7/2/91
 * added borderg/borderw, etc. - msl - 1/20/92
 */

#include <hipl_format.h>

int h_disphist(histo,hd,barwidth,barheight,maxcnt,borderw,borderg)

struct hips_histo *histo;
struct header *hd;
int barwidth,barheight,maxcnt,borderg,borderw;

{
	switch(hd->pixel_format) {
	case PFBYTE:	return(h_disphist_b(histo,hd,barwidth,barheight,
				maxcnt,borderw,borderg));
	default:	return(perr(HE_FMTSUBR,"h_disphist",
				hformatname(hd->pixel_format)));
	}
}

int h_disphist_b(histo,hd,barwidth,barheight,maxcnt,borderw,borderg)

struct hips_histo *histo;
struct header *hd;
int barwidth,barheight,maxcnt,borderw,borderg;

{
	return(h_disphist_B(histo->histo,histo->nbins,hd->firstpix,hd->ocols,
		barwidth,barheight,maxcnt,borderw,borderg));
}

int h_disphist_B(histo,nbins,image,nlp,barwidth,barheight,maxcnt,borderw,borderg)

int *histo,nbins,nlp,barwidth,barheight,maxcnt,borderw,borderg;
byte *image;

{
	int nr,nc,nb2,leftc,rightc,nex,i,j,k,np,*hp;
	byte *op,*op2;

	nr = barheight+2*borderw;
	nb2 = nbins+2;
	nc = barwidth*nb2 + 4*borderw;
	leftc = barwidth + 2*borderw;
	rightc = nc - (barwidth + 2*borderw + 1);
	nex = nlp - nc;
	op = image;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			if (j >= leftc && j <= rightc && i >= borderw &&
			    i < barheight+borderw)
				*op++ = hips_hchar;
			else
				*op++ = borderg;
		}
		j += nex;
	}
	hp = histo;
#ifdef ULORIG
	op = image + nlp*(barheight+borderw-1) + borderw;
#else
	op = image + nlp*borderw + borderw;
#endif
	hp = histo;
	if (maxcnt == 0)
		maxcnt = 1;
	for (i=0;i<nb2;i++) {
	    if (i == 1 || i == nb2-1)
		op += borderw;
	    np = (((float) (*hp++)*barheight)/maxcnt) + .5;
	    if (np>barheight)
		np = barheight;
	    for (j=0;j<barwidth;j++) {
		op2 = op;
		for (k=0;k<np;k++) {
			*op2 = hips_lchar;
#ifdef ULORIG
			op2 -= nlp;
#else
			op2 += nlp;
#endif
		}
		op++;
	    }
	}
	return(HIPS_OK);
}
