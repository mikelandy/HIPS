/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_abdou.c - Abdou's edge fitting procedure
 *
 * pixel formats: BYTE
 *
 * Size is the length of a side of the nonoverlapping domains in which the
 * algorithm operates.  The program is an implementation of
 * the edge fitting algorithm described in I. E. Abdou's doctoral thesis,
 * "Quantitative Methods of Edge Detection", published by the USC Image
 * Processing Institute as USCIPI Report 830.  Input is in byte format, output
 * is in floating format, and gives either zero, or the signal-to-noise ratio
 * for each edge pixel.  All computations are done with pixels scaled by
 * size*size in order for integer calculations to be exact.  This yields the
 * same signal-to-noise ratio as nonscaled pixels would.
 *
 * Mike Landy 7/12/82
 * HIPS 2 - msl - 8/7/91
 */

#include <hipl_format.h>

static int *f,*cc1,*c2;
static int aalloc = FALSE;
static int savesize = -1;

int h_abdou(hdi,hdo,size)

struct header *hdi,*hdo;
int size;

{
	switch(hdi->pixel_format) {
	case PFBYTE:	return(h_abdou_b(hdi,hdo,size));
	default:	return(perr(HE_FMTSUBR,"h_abdou",
				hformatname(hdi->pixel_format)));
	}
}

int h_abdou_b(hdi,hdo,size)

struct header *hdi,*hdo;
int size;

{
	return(h_abdou_B(hdi->firstpix,(float *) hdo->firstpix,hdi->rows,
		hdi->cols,hdi->ocols,hdo->ocols,size));
}

int h_abdou_B(imagei,imageo,nr,nc,nlpi,nlpo,size)

byte *imagei;
float *imageo;
int nr,nc,nlpi,nlpo,size;

{
	int i,nexi,nexo,*cc1p,*c2p,nn,theta,rrr,ccc,k,*fp,c0,c1;
	int rr,cc,thetamin,dist,n,a,first;
	register byte *ip;
	float *op,e,delta,sn,emin,deltamin;

	n = size/2;
	if (size < 3 || size > 15 || n*2+1 != size)
		return(perr(HE_WINDSZ,"h_abdou_B",size));
	if (aalloc && savesize != size) {
		free(f); free(cc1); free(c2);
		aalloc = FALSE;
	}
	if (savesize != size) {
		if ((f = (int *) memalloc(size*size,sizeof(int)))
			== (int *) HIPS_ERROR)
				return(HIPS_ERROR);
		if ((cc1 = (int *) memalloc(size*size*n*4,sizeof(int)))
			== (int *) HIPS_ERROR)
				return(HIPS_ERROR);
		if ((c2 = (int *) memalloc(n*4,sizeof(int)))
			== (int *) HIPS_ERROR)
				return(HIPS_ERROR);
		/* compute c2 and cc1 */

		cc1p = cc1;
		c2p = c2;
		for (nn=1;nn<=n;nn++) {
		    for (theta=0;theta<=4;theta++) {
			switch (theta) {

			case 0:			/* | */
				for (rrr = -n;rrr<=n;rrr++) {
				    for (ccc = -n;ccc<=n;ccc++) {
					*cc1p++ = (ccc<=-nn) ? 2*nn :
						  ((ccc>=nn) ? -2*nn :
						  -2*ccc);
				    }
				}
				k = 2*(n-nn)*nn*nn;
				for (i = -nn;i<=nn;i++)
					k += i*i;
				*c2p++ = k*size;
				continue;
			case 1:
						/* / */
				for (rrr = -n;rrr<=n;rrr++) {
				    for (ccc = -n;ccc<=n;ccc++) {
					dist = ccc+rrr;
					*cc1p++ = (dist<-2*nn) ? 2*nn :
						  ((dist>2*nn) ? -2*nn :
						  -dist);
				    }
				}
				k = 2*(n-nn)*(2*(n-nn)+1)*nn*nn;
				for (dist=1;dist<=2*nn;dist++)
					k += (((2*n-dist)+1)*dist*dist)/2;
				*c2p++ = k;
				continue;
			case 2:			/* - */
				for (rrr = -n;rrr<=n;rrr++) {
				    for (ccc = -n;ccc<=n;ccc++) {
					*cc1p++ = (rrr<=-nn) ? 2*nn :
						  ((rrr>=nn) ? -2*nn :
						  -2*rrr);
				    }
				}
				k = 2*(n-nn)*nn*nn;
				for (i = -nn;i<=nn;i++)
					k += i*i;
				*c2p++ = k*size;
				continue;
			case 3:
						/* \ */
				for (rrr = -n;rrr<=n;rrr++) {
				    for (ccc = -n;ccc<=n;ccc++) {
					dist = ccc-rrr;
					*cc1p++ = (dist<-2*nn) ? 2*nn :
						  ((dist>2*nn) ? -2*nn :
						  -dist);
				    }
				}
				k = 2*(n-nn)*(2*(n-nn)+1)*nn*nn;
				for (dist=1;dist<=2*nn;dist++)
					k += (((2*n-dist)+1)*dist*dist)/2;
				*c2p++ = k;
				continue;
			}
		    }
		}
	}
	nexi = nlpi - size;
	nexo = nlpo - size;
	for (rr=0;rr<=nr-size;rr+=size) {
		for (cc=0;cc<=nc-size;cc+=size) {
			ip = imagei + rr*nlpi + cc;
			fp = f;
			a = 0;
			for (rrr=0;rrr<size;rrr++) {
				for (ccc=0;ccc<size;ccc++) {
					k = *ip++;
					*fp++ = k*size*size;
					a += k;
				}
				ip += nexi;
			}
			fp = f;
			c0 = 0;
			for (i=0;i<size*size;i++) {
				k = (*fp++ - a);
				c0 += k*k;
			}
			cc1p = cc1;
			c2p = c2;
			first = 1;
			for (nn=1;nn<=n;nn++) {
			    for (theta=0;theta<4;theta++) {
				fp = f;
				k = 0;
				for (i=0;i<size*size;i++)
					k += *cc1p++ * *fp++;
				c1 = k;
				e = c0 - (float) c1 * c1 / (4 * *c2p);
				delta = - (float) c1 / (2 * *c2p++);
				if (first || e < emin) {
					emin = e;
					thetamin = theta;
					deltamin = delta;
					first = 0;
				}
			    }
			}
			if (deltamin == 0)
				sn = 0;
			else if (emin == 0)
				sn = 100000000000000.;
			else
				sn = deltamin * deltamin / emin;
			op = imageo + rr*nlpo + cc;
			for (rrr = -n;rrr<=n;rrr++) {
				for (ccc = -n;ccc<=n;ccc++) {
					switch(thetamin) {
					case 0:
						dist = ccc;
						break;
					case 1:
						dist = ccc + rrr;
						break;
					case 2:
						dist = rrr;
						break;
					case 3:
						dist = ccc - rrr;
						break;
					}
					*op++ = (dist == 0) ? sn : 0.;
				}
				op += nexo;
			}
		}
	}
	return(HIPS_OK);
}

