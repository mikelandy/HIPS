/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_filter.c - subroutines to apply standard linear filters to Fourier
 *			spectra and transforms
 *
 * pixel formats: FLOAT, DOUBLE, COMPLEX, DBLCOM
 *
 * Float images are assumed to be spectra (with quadrants flipped) and complex
 * images are assumed to be transforms.
 *
 * Yoav Cohen 5/21/82
 * modified by Mike Landy 7/20/84
 * HIPS-2 - msl - 8/9/91
 */

#include <hipl_format.h>
#include <math.h>

static int savenr2,savenc2,savemethod,saveftype;
static double lowpower,highpower;
static double savelowcut,savehighcut;
static h_boolean rowalloc=FALSE,colalloc=FALSE;
static double *distr,*distc,distance;
static double saverscale,savecscale;
int calcdist();
double modulate();

int h_filter(hdi,hdo,filter)

struct header *hdi,*hdo;
struct hips_filter *filter;

{
	switch(hdi->pixel_format) {
	case PFFLOAT:	return(h_filter_f(hdi,hdo,filter));
	case PFDOUBLE:	return(h_filter_d(hdi,hdo,filter));
	case PFCOMPLEX:	return(h_filter_c(hdi,hdo,filter));
	case PFDBLCOM:	return(h_filter_dc(hdi,hdo,filter));
	default:	return(perr(HE_FMTSUBR,"h_filter",
				hformatname(hdi->pixel_format)));
	}
}

int h_filter_f(hdi,hdo,filter)

struct header *hdi,*hdo;
struct hips_filter *filter;

{
	return(h_filter_F((float *) hdi->firstpix,(float *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols,filter->method,
		filter->disttype,filter->ftype,filter->dmetric,filter->lowcut,
		filter->loworder,filter->highcut,filter->highorder));
}

int h_filter_d(hdi,hdo,filter)

struct header *hdi,*hdo;
struct hips_filter *filter;

{
	return(h_filter_D((double *) hdi->firstpix,(double *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols,filter->method,
		filter->disttype,filter->ftype,filter->dmetric,filter->lowcut,
		filter->loworder,filter->highcut,filter->highorder));
}

int h_filter_c(hdi,hdo,filter)

struct header *hdi,*hdo;
struct hips_filter *filter;

{
	return(h_filter_C((float *) hdi->firstpix,(float *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols,filter->method,
		filter->disttype,filter->ftype,filter->dmetric,filter->lowcut,
		filter->loworder,filter->highcut,filter->highorder));
}

int h_filter_dc(hdi,hdo,filter)

struct header *hdi,*hdo;
struct hips_filter *filter;

{
	return(h_filter_DC((double *) hdi->firstpix,(double *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols,filter->method,
		filter->disttype,filter->ftype,filter->dmetric,filter->lowcut,
		filter->loworder,filter->highcut,filter->highorder));
}

int h_filter_F(imagei,imageo,nr,nc,nlpi,nlpo,method,disttype,ftype,dmetric,lowcut,
	loworder,highcut,highorder)

float *imagei,*imageo;
int nr,nc,nlpi,nlpo,method,disttype,ftype,loworder,highorder;
float dmetric,lowcut,highcut;

{
	int nr2,nc2,i,j,nexi,nexo,y,x;
	float *pi,*po;
	double invmink,powy,powxy;

	nr2 = nr/2;
	nc2 = nc/2;
	if (calcdist(disttype,dmetric,nr2,nc2) == HIPS_ERROR)
		return(HIPS_ERROR);
	savemethod = method;
	saveftype = ftype;
	if (dmetric <= 0.)
		return(perr(HE_FILTPAR));
	if (method != FILTMETHOD_IDEAL && method != FILTMETHOD_BUTTERWORTH &&
		method != FILTMETHOD_EXPONENTIAL)
			return(perr(HE_FILTPAR));
	if (disttype != FILTDIST_ROW && disttype != FILTDIST_COL &&
		disttype != FILTDIST_BOTH)
			return(perr(HE_FILTPAR));
	if (ftype != FILTTYPE_LOWPASS && ftype != FILTTYPE_HIGHPASS &&
		ftype != FILTTYPE_BANDPASS && ftype != FILTTYPE_BANDREJ)
			return(perr(HE_FILTPAR));
	invmink = 1./dmetric;
	savelowcut = lowcut;
	lowpower = (method == FILTMETHOD_BUTTERWORTH) ? loworder*2 : loworder;
	savehighcut = highcut;
	highpower = (method == FILTMETHOD_BUTTERWORTH) ? highorder*2 :
		highorder;
	nexi = nlpi - nc;
	nexo = nlpo - nc;
	pi = imagei;
	po = imageo;
	for (i=0;i<nr;i++) {
		y = abs(i-nr2);
		powy = distr[y];
		for (j=0;j<nc;j++) {
			x = abs(j-nc2);
			powxy = powy + distc[x];
			distance = (powxy == 0.0) ? 0.0 :
				pow(powxy,invmink);
			*po++ = *pi++ * modulate();
		}
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_filter_D(imagei,imageo,nr,nc,nlpi,nlpo,method,disttype,ftype,dmetric,lowcut,
	loworder,highcut,highorder)

double *imagei,*imageo;
int nr,nc,nlpi,nlpo,method,disttype,ftype,loworder,highorder;
float dmetric,lowcut,highcut;

{
	int nr2,nc2,i,j,nexi,nexo,y,x;
	double *pi,*po;
	double invmink,powy,powxy;

	nr2 = nr/2;
	nc2 = nc/2;
	if (calcdist(disttype,dmetric,nr2,nc2) == HIPS_ERROR)
		return(HIPS_ERROR);
	savemethod = method;
	saveftype = ftype;
	if (dmetric <= 0.)
		return(perr(HE_FILTPAR));
	if (method != FILTMETHOD_IDEAL && method != FILTMETHOD_BUTTERWORTH &&
		method != FILTMETHOD_EXPONENTIAL)
			return(perr(HE_FILTPAR));
	if (disttype != FILTDIST_ROW && disttype != FILTDIST_COL &&
		disttype != FILTDIST_BOTH)
			return(perr(HE_FILTPAR));
	if (ftype != FILTTYPE_LOWPASS && ftype != FILTTYPE_HIGHPASS &&
		ftype != FILTTYPE_BANDPASS && ftype != FILTTYPE_BANDREJ)
			return(perr(HE_FILTPAR));
	invmink = 1./dmetric;
	savelowcut = lowcut;
	lowpower = (method == FILTMETHOD_BUTTERWORTH) ? loworder*2 : loworder;
	savehighcut = highcut;
	highpower = (method == FILTMETHOD_BUTTERWORTH) ? highorder*2 :
		highorder;
	nexi = nlpi - nc;
	nexo = nlpo - nc;
	pi = imagei;
	po = imageo;
	for (i=0;i<nr;i++) {
		y = abs(i-nr2);
		powy = distr[y];
		for (j=0;j<nc;j++) {
			x = abs(j-nc2);
			powxy = powy + distc[x];
			distance = (powxy == 0.0) ? 0.0 :
				pow(powxy,invmink);
			*po++ = *pi++ * modulate();
		}
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_filter_C(imagei,imageo,nr,nc,nlpi,nlpo,method,disttype,ftype,dmetric,lowcut,
	loworder,highcut,highorder)

float *imagei,*imageo;
int nr,nc,nlpi,nlpo,method,disttype,ftype,loworder,highorder;
float dmetric,lowcut,highcut;

{
	int nr2,nc2,i,j,nexi,nexo,y,x;
	float *pi,*po;
	double invmink,powy,powxy,mval;

	nr2 = nr/2;
	nc2 = nc/2;
	if (calcdist(disttype,dmetric,nr2,nc2) == HIPS_ERROR)
		return(HIPS_ERROR);
	savemethod = method;
	saveftype = ftype;
	if (dmetric <= 0.)
		return(perr(HE_FILTPAR));
	if (method != FILTMETHOD_IDEAL && method != FILTMETHOD_BUTTERWORTH &&
		method != FILTMETHOD_EXPONENTIAL)
			return(perr(HE_FILTPAR));
	if (disttype != FILTDIST_ROW && disttype != FILTDIST_COL &&
		disttype != FILTDIST_BOTH)
			return(perr(HE_FILTPAR));
	if (ftype != FILTTYPE_LOWPASS && ftype != FILTTYPE_HIGHPASS &&
		ftype != FILTTYPE_BANDPASS && ftype != FILTTYPE_BANDREJ)
			return(perr(HE_FILTPAR));
	invmink = 1./dmetric;
	savelowcut = lowcut;
	lowpower = (method == FILTMETHOD_BUTTERWORTH) ? loworder*2 : loworder;
	savehighcut = highcut;
	highpower = (method == FILTMETHOD_BUTTERWORTH) ? highorder*2 :
		highorder;
	nexi = 2*(nlpi - nc);
	nexo = 2*(nlpo - nc);
	pi = imagei;
	po = imageo;
	for (i=0;i<nr;i++) {
		y = (i+nr2)%nr;
		y = abs(y-nr2);
		powy = distr[y];
		for (j=0;j<nc;j++) {
			x = (j+nc2)%nc;
			x = abs(x-nc2);
			powxy = powy + distc[x];
			distance = (powxy == 0.0) ? 0.0 :
				pow(powxy,invmink);
			mval = modulate();
			*po++ = *pi++ * mval;
			*po++ = *pi++ * mval;
		}
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}

int h_filter_DC(imagei,imageo,nr,nc,nlpi,nlpo,method,disttype,ftype,dmetric,lowcut,
	loworder,highcut,highorder)

double *imagei,*imageo;
int nr,nc,nlpi,nlpo,method,disttype,ftype,loworder,highorder;
float dmetric,lowcut,highcut;

{
	int nr2,nc2,i,j,nexi,nexo,y,x;
	double *pi,*po;
	double invmink,powy,powxy,mval;

	nr2 = nr/2;
	nc2 = nc/2;
	if (calcdist(disttype,dmetric,nr2,nc2) == HIPS_ERROR)
		return(HIPS_ERROR);
	savemethod = method;
	saveftype = ftype;
	if (dmetric <= 0.)
		return(perr(HE_FILTPAR));
	if (method != FILTMETHOD_IDEAL && method != FILTMETHOD_BUTTERWORTH &&
		method != FILTMETHOD_EXPONENTIAL)
			return(perr(HE_FILTPAR));
	if (disttype != FILTDIST_ROW && disttype != FILTDIST_COL &&
		disttype != FILTDIST_BOTH)
			return(perr(HE_FILTPAR));
	if (ftype != FILTTYPE_LOWPASS && ftype != FILTTYPE_HIGHPASS &&
		ftype != FILTTYPE_BANDPASS && ftype != FILTTYPE_BANDREJ)
			return(perr(HE_FILTPAR));
	invmink = 1./dmetric;
	savelowcut = lowcut;
	lowpower = (method == FILTMETHOD_BUTTERWORTH) ? loworder*2 : loworder;
	savehighcut = highcut;
	highpower = (method == FILTMETHOD_BUTTERWORTH) ? highorder*2 :
		highorder;
	nexi = 2*(nlpi - nc);
	nexo = 2*(nlpo - nc);
	pi = imagei;
	po = imageo;
	for (i=0;i<nr;i++) {
		y = (i+nr2)%nr;
		y = abs(y-nr2);
		powy = distr[y];
		for (j=0;j<nc;j++) {
			x = (j+nc2)%nc;
			x = abs(x-nc2);
			powxy = powy + distc[x];
			distance = (powxy == 0.0) ? 0.0 :
				pow(powxy,invmink);
			mval = modulate();
			*po++ = *pi++ * mval;
			*po++ = *pi++ * mval;
		}
		pi += nexi;
		po += nexo;
	}
	return(HIPS_OK);
}

int calcdist(disttype,dmetric,nr2,nc2)

int disttype,nr2,nc2;
float dmetric;

{
	double *k,scale;
	int i;

	if (rowalloc && savenr2 != nr2) {
		free(distr);
		rowalloc = FALSE;
	}
	if (!rowalloc) {
		if ((distr = (double *) memalloc(1 + nr2,sizeof(double))) ==
			(double *) HIPS_ERROR)
				return(HIPS_ERROR);
		rowalloc = TRUE;
		savenr2 = nr2+1;	/* force recalculation */
	}
	scale = (disttype == FILTDIST_COL)
		? nc2 : nr2;
	if (scale != saverscale || nr2 != savenr2) {
		k = distr;
		*k++ = 0;
		for (i=1;i<=nr2;i++)
			*k++ = pow(((double) i)/scale,dmetric);
		savenr2 = nr2;
		saverscale = scale;
	}
	if (colalloc && savenc2 != nc2) {
		free(distc);
		colalloc = FALSE;
	}
	if (!colalloc) {
		if ((distc = (double *) memalloc(1 + nc2,sizeof(double))) ==
			(double *) HIPS_ERROR)
				return(HIPS_ERROR);
		colalloc = TRUE;
		savenc2 = nc2+1;	/* force recalculation */
	}
	scale = (disttype == FILTDIST_ROW)
		? nr2 : nc2;
	if (scale != savecscale || nc2 != savenc2) {
		k = distc;
		*k++ = 0;
		for (i=1;i<=nc2;i++)
			*k++=pow(((double) i)/scale,dmetric);
		savenc2 = nc2;
		savecscale = scale;
	}
	return(HIPS_OK);
}

double modulate() 

{
	double lowval,highval;

	switch(saveftype) {
	case FILTTYPE_LOWPASS:
				if (distance == 0.)
					return(1.);
				switch(savemethod) {
				case FILTMETHOD_IDEAL:
					return((distance <= savelowcut) ?
						1. : 0.);
				case FILTMETHOD_BUTTERWORTH:
					return(1./(1.+pow(distance/savelowcut,
						lowpower)));
				case FILTMETHOD_EXPONENTIAL:
					return(exp(-pow(distance/savelowcut,
						lowpower)));
				}
	case FILTTYPE_HIGHPASS:
				if (savehighcut == 0.)
					return(1.);
				switch(savemethod) {
				case FILTMETHOD_IDEAL:
					return((distance >= savehighcut) ?
						1. : 0.);
				case FILTMETHOD_BUTTERWORTH:
					if (distance == 0.)
						return(0.);
					return(1./(1.+pow(savehighcut/distance,
						highpower)));
				case FILTMETHOD_EXPONENTIAL:
					if (distance == 0.)
						return(0.);
					return(exp(-pow(savehighcut/distance,
						highpower)));
				}
	case FILTTYPE_BANDPASS:
				switch(savemethod) {
				case FILTMETHOD_IDEAL:
					return((distance <= savehighcut &&
						distance >= savelowcut) ?
							1. : 0.);
				case FILTMETHOD_BUTTERWORTH:
					if (distance == 0.)
						lowval = 1.;
					else
						lowval = 1. /
						  (1.+pow(distance/savehighcut,
						  highpower));
					if (savelowcut == 0.)
						highval = 1.;
					else if (distance == 0.)
						highval = 0.;
					else
						highval = 1. /
						  (1.+pow(savelowcut/distance,
						  lowpower));
					break;
				case FILTMETHOD_EXPONENTIAL:
					if (distance == 0.)
						lowval = 1.;
					else
						lowval =
						  exp(-pow(distance/savehighcut,
						  highpower));
					if (savelowcut == 0.)
						highval = 1.;
					else if (distance == 0.)
						highval = 0.;
					else
						highval = 
						  exp(-pow(savelowcut/distance,
						  lowpower));
					break;
				}
				return(lowval*highval);
	case FILTTYPE_BANDREJ:
				switch(savemethod) {
				case FILTMETHOD_IDEAL:
					return((distance <= savehighcut &&
						distance >= savelowcut) ?
							0. : 1.);
				case FILTMETHOD_BUTTERWORTH:
					if (distance == 0.)
						lowval = 1.;
					else
						lowval = 1. /
						  (1.+pow(distance/savehighcut,
						  highpower));
					if (savelowcut == 0.)
						highval = 1.;
					else if (distance == 0.)
						highval = 0.;
					else
						highval = 1. /
						  (1.+pow(savelowcut/distance,
						  lowpower));
					break;
				case FILTMETHOD_EXPONENTIAL:
					if (distance == 0.)
						lowval = 1.;
					else
						lowval =
						  exp(-pow(distance/savehighcut,
						  highpower));
					if (savelowcut == 0.)
						highval = 1.;
					else if (distance == 0.)
						highval = 0.;
					else
						highval = 
						  exp(-pow(savelowcut/distance,
						  lowpower));
					break;
				}
				return(1-(lowval*highval));
	}
	return(0.);	/* can't happen since filter type was already checked */
}
