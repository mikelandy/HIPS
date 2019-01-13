/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_stats.c - subroutines to compute statistics of an image
 *
 * pixel formats: BYTE, FLOAT
 *
 * Yoav Cohen 2/16/82
 * HIPS 2 - msl - 7/5/91
 */

#include <hipl_format.h>
#include <math.h>

int h_stats(hd,stats,nzflag)

struct header *hd;
struct hips_stats *stats;
h_boolean nzflag;

{
	switch(hd->pixel_format) {
	case PFBYTE:	return(h_stats_b(hd,stats,nzflag));
	case PFFLOAT:	return(h_stats_f(hd,stats,nzflag));
	default:	return(perr(HE_FMTSUBR,"h_stats",
				hformatname(hd->pixel_format)));
	}
}

int h_stats_b(hd,stats,nzflag)

struct header *hd;
struct hips_stats *stats;
h_boolean nzflag;

{
	h_stats_B(hd->firstpix,hd->rows,hd->cols,hd->ocols,&(stats->nelem),
		&(stats->statmin.v_byte),&(stats->statmax.v_byte),&(stats->sum),
		&(stats->ssq),nzflag);
	stats->pixel_format = PFBYTE;
	if (stats->nelem == 0)
		stats->mean = stats->var = stats->stdev = 0;
	else {
		stats->mean = (stats->sum)/(stats->nelem);
		stats->var = ((stats->ssq)/(stats->nelem)) -
			((stats->mean)*(stats->mean));
		stats->stdev = sqrt(stats->var);
	}
	return(HIPS_OK);
}

int h_stats_f(hd,stats,nzflag)

struct header *hd;
struct hips_stats *stats;
h_boolean nzflag;

{
	h_stats_F((float *) hd->firstpix,hd->rows,hd->cols,hd->ocols,
		&(stats->nelem),&(stats->statmin.v_float),
		&(stats->statmax.v_float),&(stats->sum),&(stats->ssq),nzflag);
	stats->pixel_format = PFFLOAT;
	if (stats->nelem == 0)
		stats->mean = stats->var = stats->stdev = 0;
	else {
		stats->mean = (stats->sum)/(stats->nelem);
		stats->var = ((stats->ssq)/(stats->nelem)) -
			((stats->mean)*(stats->mean));
		stats->stdev = sqrt(stats->var);
	}
	return(HIPS_OK);
}

int h_stats_B(image,nr,nc,nlp,rnelem,rmin,rmax,rsum,rssq,nzflag)

byte *image,*rmin,*rmax;
int nr,nc,nlp,*rnelem;
double *rsum,*rssq;
h_boolean nzflag;

{
	register int i,j,nex,nelem;
	register byte *p,min,max;
	register double sum,ssq;
	h_boolean notyet;

	nex = nlp-nc;
	p = image;
	sum = ssq = 0;
	if (nzflag) {
		nelem = min = max = 0;
		notyet = TRUE;
		for (i=0;i<nr;i++) {
			for (j=0;j<nc;j++) {
				if (*p != 0) {
					nelem++;
					if (notyet) {
						notyet = FALSE;
						min = max = *p;
					}
					else if (*p < min)
						min = *p;
					else if (*p > max)
						max = *p;
					sum += *p;
					ssq += *p * *p;
				}
				p++;
			}
			p += nex;
		}
	}
	else {
		nelem = nr*nc;
		min = max = *p;
		for (i=0;i<nr;i++) {
			for (j=0;j<nc;j++) {
				if (*p < min)
					min = *p;
				else if (*p > max)
					max = *p;
				sum += *p;
				ssq += *p * *p;
				p++;
			}
			p += nex;
		}
	}
	*rmin = min;
	*rmax = max;
	*rsum = sum;
	*rssq = ssq;
	*rnelem = nelem;
	return(HIPS_OK);
}

int h_stats_F(image,nr,nc,nlp,rnelem,rmin,rmax,rsum,rssq,nzflag)

float *image,*rmin,*rmax;
int nr,nc,nlp,*rnelem;
double *rsum,*rssq;
h_boolean nzflag;

{
	register int i,j,nex,nelem;
	register float *p,min,max;
	register double sum,ssq;
	h_boolean notyet;

	nex = nlp-nc;
	p = image;
	sum = ssq = 0;
	if (nzflag) {
		nelem = min = max = 0;
		notyet = TRUE;
		for (i=0;i<nr;i++) {
			for (j=0;j<nc;j++) {
				if (*p != 0) {
					nelem++;
					if (notyet) {
						notyet = FALSE;
						min = max = *p;
					}
					else if (*p < min)
						min = *p;
					else if (*p > max)
						max = *p;
					sum += *p;
					ssq += *p * *p;
				}
				p++;
			}
			p += nex;
		}
	}
	else {
		nelem = nr*nc;
		min = max = *p;
		for (i=0;i<nr;i++) {
			for (j=0;j<nc;j++) {
				if (*p < min)
					min = *p;
				else if (*p > max)
					max = *p;
				sum += *p;
				ssq += *p * *p;
				p++;
			}
			p += nex;
		}
	}
	*rmin = min;
	*rmax = max;
	*rsum = sum;
	*rssq = ssq;
	*rnelem = nelem;
	return(HIPS_OK);
}
