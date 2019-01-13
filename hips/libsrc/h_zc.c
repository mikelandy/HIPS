/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_zc.c - computer zero crossing slopes
 *
 * input pixel format: FLOAT
 * output pixel formats: FLOAT
 *
 * h_zc computes zero crossing slopes.  The error argument is a threshold.
 * Pixels with absolute value below this value are considered to be zero, and
 * hence are candidate zero crossings.  The routine calculates a crude
 * approximation to the slope of each zero crossing.  The program generally
 * places zero crossings where the image is actually zero and abuts pixels of
 * opposite signs, or at a positive pixel which abuts a negative pixel.  If
 * nflag is TRUE, then zero crossings are placed at actual zeroes and at
 * negative pixels which abut positive ones.  Lastly, if an actual zero (as
 * defined by "error") abuts pixels of one sign but not the other (as occurs
 * at the edges of a broad area of zeroes), no zero crossing is marked.  Such
 * a pixel will be marked, however, if zflag is TRUE.
 *
 * Yoav Cohen 7/6/82
 * totally rewritten/extended - Mike Landy 8/14/84
 * HIPS 2 - msl - 6/21/91
 */

#include <hipl_format.h>
#include <math.h>

int h_zc(hdi,hdo,error,nflag,zflag)

struct header *hdi,*hdo;
double error;
h_boolean nflag,zflag;

{
	switch(hdi->pixel_format) {
	case PFFLOAT:	return(h_zc_f(hdi,hdo,error,nflag,zflag));
	default:	return(perr(HE_FMTSUBR,"h_zc",
				hformatname(hdi->pixel_format)));
	}
}

int h_zc_f(hdi,hdo,error,nflag,zflag)

struct header *hdi,*hdo;
double error;
h_boolean nflag,zflag;

{
	return(h_zc_F((float *) hdi->firstpix,(float *) hdo->firstpix,hdi->rows,
		hdi->cols,hdi->ocols,hdo->ocols,error,nflag,zflag));
}

int h_zc_F(imagei,imageo,nr,nc,nlpi,nlpo,error,nflag,zflag)

float *imagei,*imageo;
double error;
int nr,nc,nlpi,nlpo;
h_boolean nflag,zflag;

{
	register int i,j,k,l,nexi,nexo;
	register float *pi,*po,merror;
	float w,val,nbig,pbig;

	merror = -error;
	nexi = nlpi-nc;
	nexo = nlpo-nc;
	pi = imagei;
	po = imageo;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			val= *pi;
			*po = 0;
			if (val>error && (!nflag)) {
				nbig = 0;
				for(k= -1;k<2;k++) {
				    for(l= -1;l<2;l++) {
					if(k==0 && l==0) continue;
					if (i+k<0 || i+k>=nr ||
					    j+l<0 || j+l>=nc) continue;
					w=(*(pi+k*nlpi+l));
					if (w<nbig)
						nbig = w;
				    }
				}
				if (nbig < merror)
					*po = (val - nbig) * 2;
			}
			else if (val<merror && nflag) {
				pbig = 0;
				for(k= -1;k<2;k++) {
				    for(l= -1;l<2;l++) {
					if(k==0 && l==0) continue;
					if (i+k<0 || i+k>=nr ||
					    j+l<0 || j+l>=nc) continue;
					w=(*(pi+k*nlpi+l));
					if (w>pbig)
						pbig = w;
				    }
				}
				if (pbig > error)
					*po = (pbig - val) * 2;
			}
			else if (val > merror && val < error) {
				pbig = 0;
				nbig = 0;
				for(k= -1;k<2;k++) {
				    for(l= -1;l<2;l++) {
					if(k==0 && l==0) continue;
					if (i+k<0 || i+k>=nr ||
					    j+l<0 || j+l>=nc) continue;
					w=(*(pi+k*nlpi+l));
					if (w>pbig)
						pbig = w;
					if (w<nbig)
						nbig = w;
				    }
				}
				if (pbig > error && nbig < merror)
					*po = pbig - nbig;
				else if (zflag && (pbig>error || nbig<merror))
					*po = pbig - nbig;
			}
			pi++; po++;
		}
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}
