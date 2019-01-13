/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_entropycnt.c - subroutines to count pixel values for an entropy computation
 *
 * If pairflag is set, entropy is computed across pairs of pixels
 * (horizontal neighbors), and if the number of columns is odd, the last
 * pixel in each column is ignored.
 *
 * pixel formats: BYTE
 *
 * Yoav Cohen - 9/20/82
 * HIPS 2 - Michael Landy - 7/5/91
 */

#include <hipl_format.h>
#include <math.h>

int h_entropycnt(hd,table,pairflag)

struct header *hd;
int *table;
h_boolean pairflag;

{
	switch(hd->pixel_format) {
	case PFBYTE:	return(h_entropycnt_b(hd,table,pairflag));
	default:	return(perr(HE_FMTSUBR,"h_entropycnt",
				hformatname(hd->pixel_format)));
	}
}

int h_entropycnt_b(hd,table,pairflag)

struct header *hd;
int *table;
h_boolean pairflag;

{
	return(h_entropycnt_B(hd->firstpix,hd->rows,hd->cols,hd->ocols,table,
			pairflag));
}

int h_entropycnt_B(image,nr,nc,nlp,table,pairflag)

byte *image;
int nr,nc,nlp,*table;
h_boolean pairflag;

{
	register int i,j,nex,col_limit,word;
	register byte *p;

	if (pairflag)
		col_limit = nc - (nc%2);
	else
		col_limit = nc;
	nex = nlp-col_limit;
	p = image;
	for (i=0;i<nr;i++) {
		if (pairflag) {
			for (j=0;j<col_limit;j += 2) {
				word = *p++;
				word = (word<<8) + *p++;
				table[word]++;
			}
		}
		else {
			for (j=0;j<col_limit;j++)
				table[*p++]++;
		}
		p += nex;
	}
	return(HIPS_OK);
}
