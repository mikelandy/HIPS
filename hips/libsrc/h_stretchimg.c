/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_stretchimg.c - subroutines to stretch an image
 *
 * h_stretchimg changes the row and column dimensions of an image by stretching.
 * The algorithm is basically block averaging, where each pixel in the old
 * image is treated as square, and each pixel in the new image rectangular.
 * The new pixel's value is an average of the pixel's in the old image it
 * overlaps, weighted by the degree of overlap.
 *
 * pixel formats: BYTE
 *
 * Mike Landy - 6/11/87, based on code by Lou Salkind
 * HIPS 2 - msl - 6/29/91
 */

#include <hipl_format.h>

int h_stretchimg(hdi,hdo)

struct header *hdi,*hdo;

{
	switch(hdi->pixel_format) {
	case PFBYTE:	return(h_stretchimg_b(hdi,hdo));
	default:	return(perr(HE_FMTSUBR,"h_stretchimg",
				hformatname(hdi->pixel_format)));
	}
}

int h_stretchimg_b(hdi,hdo)

struct header *hdi,*hdo;

{
	return(h_stretchimg_B(hdi->firstpix,hdo->firstpix,hdi->rows,hdi->cols,
		hdi->ocols,hdo->rows,hdo->cols,hdo->ocols));
}

static h_boolean balloc = FALSE;
static int b_nr,b_nc,b_nor,b_noc;
static short *fxlo,*fxhi,*inxpix,*xlofract,*xhifract,*xmifract;
static short *fylo,*fyhi,*inypix,*ylofract,*yhifract,*ymifract;

int h_stretchimg_B(imagei,imageo,nr,nc,nlpi,nor,noc,nlpo)

byte *imagei,*imageo;
int nr,nc,nlpi,nor,noc,nlpo;

{
	int nexo;
	float xmag,ymag,xlo,xhi,ylo,yhi;
	float x0tmp,x1tmp,y0tmp,y1tmp;
	register int r,c,k;
	register int sum;
	register byte *ip3,*ip4;
	byte *ip,*op;
	int inxp,inyp,tmp,tmp2;

	if (!balloc || b_noc != noc) {
		if (balloc) {
			free(fxlo); free(fxhi); free(inxpix); free(xlofract);
			free(xhifract); free(xmifract);
		}
		if ((fxlo = (short *) memalloc(noc,sizeof(short))) ==
			(short *) HIPS_ERROR)
				return(HIPS_ERROR);
		if ((fxhi = (short *) memalloc(noc,sizeof(short))) ==
			(short *) HIPS_ERROR)
				return(HIPS_ERROR);
		if ((inxpix = (short *) memalloc(noc,sizeof(short))) ==
			(short *) HIPS_ERROR)
				return(HIPS_ERROR);
		if ((xlofract = (short *) memalloc(noc,sizeof(short))) ==
			(short *) HIPS_ERROR)
				return(HIPS_ERROR);
		if ((xhifract = (short *) memalloc(noc,sizeof(short))) ==
			(short *) HIPS_ERROR)
				return(HIPS_ERROR);
		if ((xmifract = (short *) memalloc(noc,sizeof(short))) ==
			(short *) HIPS_ERROR)
				return(HIPS_ERROR);
		b_nc = nc+1; /* force computation */
	}
	if (!balloc || b_nor != nor) {
		if (balloc) {
			free(fylo); free(fyhi); free(inypix); free(ylofract);
			free(yhifract); free(ymifract);
		}
		if ((fylo = (short *) memalloc(nor,sizeof(short))) ==
			(short *) HIPS_ERROR)
				return(HIPS_ERROR);
		if ((fyhi = (short *) memalloc(nor,sizeof(short))) ==
			(short *) HIPS_ERROR)
				return(HIPS_ERROR);
		if ((inypix = (short *) memalloc(nor,sizeof(short))) ==
			(short *) HIPS_ERROR)
				return(HIPS_ERROR);
		if ((ylofract = (short *) memalloc(nor,sizeof(short))) ==
			(short *) HIPS_ERROR)
				return(HIPS_ERROR);
		if ((yhifract = (short *) memalloc(nor,sizeof(short))) ==
			(short *) HIPS_ERROR)
				return(HIPS_ERROR);
		if ((ymifract = (short *) memalloc(nor,sizeof(short))) ==
			(short *) HIPS_ERROR)
				return(HIPS_ERROR);
		b_nr = nr+1; /* force computation */
	}
	balloc = TRUE;
	if (b_nc != nc || b_nr != nr || b_noc != noc || b_nor != nor) {
		b_nc = nc;
		b_nr = nr;
		b_noc = noc;
		b_nor = nor;
		xmag = ((float) nc)/noc;
		ymag = ((float) nr)/nor;

		/*
		 * precompute appropriate arrays
		 *
		 * we use integer arithmetic where possible,
		 * since floating point is so slow.
		 * fractions are represented as a byte value
		 * between 0 and 255
		 */

		for (c = 0; c < noc; c++) {
			if (nc == noc) {
				/* important special case */
				fxlo[c] = c;
				inxpix[c] = -1;
			} else {
				xlo = c * xmag;
				xhi = xlo + xmag;
				/* truncate */
				fxlo[c] = xlo;
				fxhi[c] = xhi;
				/* if on integer boundary, ignore */
				if ((float)fxhi[c] == xhi)
					fxhi[c]--;
				inxpix[c] = fxhi[c] - fxlo[c] - 1;
			}
			if (inxpix[c] == 0) {
				x0tmp = fxlo[c] + 1 - xlo;
				xlofract[c] = (255 * x0tmp / xmag) + .5;
				xhifract[c] = 255 - xlofract[c];
			} else if (inxpix[c] > 0) {
				x0tmp = fxlo[c] + 1 - xlo;
				x1tmp = xhi - fxhi[c];
				xlofract[c] = (255 * x0tmp / xmag) + .5;
				xhifract[c] = (255 * x1tmp / xmag) + .5;
				xmifract[c] = 255 - xlofract[c] - xhifract[c];
			}
		}

		for (r=0;r<nor;r++) {
			if (nor == nr) {
				/* important special case */
				fylo[r] = r;
				inypix[r] = -1;
			} else {
				ylo = r * ymag;
				yhi = ylo + ymag;
				/* truncate */
				fylo[r] = ylo;
				fyhi[r] = yhi;
				/* if on integer boundary, ignore */
				if ((float)fyhi[r] == yhi)
					fyhi[r]--;
				inypix[r] = fyhi[r] - fylo[r] - 1;
			}
			if (inypix[r] == 0) {
				y0tmp = fylo[r] + 1 - ylo;
				ylofract[r] = (255 * y0tmp / ymag) + .5;
				yhifract[r] = 255 - ylofract[r];
			} else if (inypix[r] > 0) {
				y0tmp = fylo[r] + 1 - ylo;
				y1tmp = yhi - fyhi[r];
				ylofract[r] = (255 * y0tmp / ymag) + .5;
				yhifract[r] = (255 * y1tmp / ymag) + .5;
				ymifract[r] = 255 - ylofract[r] - yhifract[r];
			}
		}
	}
	nexo = nlpo-noc;
	op = imageo;
	for (r=0;r<nor;r++) {
	    ip = imagei + fylo[r]*nlpi;
	    for (c=0;c<noc;c++) {
		/*
		 * sum up the appropriate pixels in the original
		 * image.  inxpix and inypix determine whether
		 * we will cross pixel boundaries in the original
		 * image.
		 */
		inxp = inxpix[c];
		inyp = inypix[r];

		/* start first the lower row */
		sum = 0;
		ip4 = ip + fxlo[c];
		ip3 = ip4 + 1;
		if (inxp > 0) {
			/* add in the middle region */
			/* this contains whole pixels across */
			for (k = inxp; k > 0; k--)
				sum += *ip3++;
			sum *= xmifract[c];	/* weight */
			sum /= inxp;
		}
		if (inxp >= 0) {
			/* add in partial regions */
			/* these are at the start and end */
			sum += (*ip4) * xlofract[c];
			sum += (*ip3) * xhifract[c];
		} else 		/* inxp < 0 */
			/* everything within one pixel across */
			sum += 255 * (*ip4);

		if (inyp < 0) {
			/* only one row to consider */
			/* everything fits in a y pixel */
			sum /= 255;
			*op++ = sum;
			continue;
		}

		sum *= ylofract[r];		/* weight */
		ip4 += nlpi;
		tmp = sum;

		/* add the middle rows */
		if (inyp > 0) {
			/* whole rows to add in */
			register int l;

			tmp2 = 0;
			for (l = inyp; l > 0; l--) {
				sum = 0;
				ip3 = ip4 + 1;
				if (inxp > 0) {
					for (k = inxp; k > 0; k--)
						sum += *ip3++;
					sum *= xmifract[c];
					sum /= inxp;
				}
				if (inxp >= 0) {
					/* add in partial regions */
					sum += (*ip4) * xlofract[c];
					sum += (*ip3) * xhifract[c];
				} else
					sum += 255 * (*ip4);
				ip4 += nlpi;
				tmp2 += sum;
			}
			tmp2 *= ymifract[r];
			tmp2 /= inyp;
			tmp += tmp2;
		}

		/* now the upper row */
		sum = 0;
		ip3 = ip4 + 1;
		if (inxp > 0) {
			/* add in the middle region */
			for (k = inxp; k > 0; k--)
				sum += *ip3++;
			sum *= xmifract[c];	/* weight */
			sum /= inxp;
		}
		if (inxp >= 0) {
			/* add in partial regions */
			sum += (*ip4) * xlofract[c];
			sum += (*ip3) * xhifract[c];
		} else 		/* inxp < 0 */
			sum += 255 * (*ip4);
		sum *= yhifract[r];

		/* combine and normalize */
		sum += tmp;
		sum /= 255*255;

		*op++ = sum;
	    }
	    op += nexo;
	}
	return(HIPS_OK);
}
