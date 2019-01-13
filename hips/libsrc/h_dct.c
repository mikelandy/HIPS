/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_dct.c - subroutines to compute a discrete cosine transform
 *
 * pixel formats: FLOAT, DOUBLE
 *
 * Michael Landy - 3/11/82
 * modified for HIPS 2 - msl - 1/3/91
 * upgraded to support ROI and header-calls - msl - 3/9/93
 */

#include <hipl_format.h>
#include <math.h>

int h_dct(ihd,ohd)

struct header *ihd,*ohd;

{
	switch(ihd->pixel_format) {
	case PFFLOAT:	return(h_dct_f(ihd,ohd));
	case PFDOUBLE:	return(h_dct_d(ihd,ohd));
	default:	return(perr(HE_FMTSUBR,"h_dct",
				hformatname(ihd->pixel_format)));
	}
}

int h_dct_f(ihd,ohd)

struct header *ihd,*ohd;

{
	return(h_dct_F((float *) ihd->firstpix,(float *) ohd->firstpix,
		ihd->rows,ihd->cols,ihd->ocols,ohd->ocols));
}

int h_dct_d(ihd,ohd)

struct header *ihd,*ohd;

{
	return(h_dct_D((double *) ihd->firstpix,(double *) ohd->firstpix,
		ihd->rows,ihd->cols,ihd->ocols,ohd->ocols));
}

int h_dct_F(imagei,imageo,nr,nc,nlpi,nlpo)

float *imagei,*imageo;
int nr,nc,nlpi,nlpo;

{
	int i,j,logrows,logcols;

	logrows = logcols = -1;
	i = 0; j = 1;
	while (1) {
		if (j == nr)
			logrows = i;
		if (j == nc)
			logcols = i;
		if (j >= nr && j >= nc)
			break;
		i++; j <<= 1;
	}
	if (logrows == -1 || logcols == -1)
		return(perr(HE_POW2));
	return(h_dct2d_f(imagei,imageo,logrows,logcols,nlpi,nlpo));
}

int h_dct_D(imagei,imageo,nr,nc,nlpi,nlpo)

double *imagei,*imageo;
int nr,nc,nlpi,nlpo;

{
	int i,j,logrows,logcols;

	logrows = logcols = -1;
	i = 0; j = 1;
	while (1) {
		if (j == nr)
			logrows = i;
		if (j == nc)
			logcols = i;
		if (j >= nr && j >= nc)
			break;
		i++; j <<= 1;
	}
	if (logrows == -1 || logcols == -1)
		return(perr(HE_POW2));
	return(h_dct2d_d(imagei,imageo,logrows,logcols,nlpi,nlpo));
}

int h_dctinv(ihd,ohd)

struct header *ihd,*ohd;

{
	switch(ihd->pixel_format) {
	case PFFLOAT:	return(h_dctinv_f(ihd,ohd));
	case PFDOUBLE:	return(h_dctinv_d(ihd,ohd));
	default:	return(perr(HE_FMTSUBR,"h_dctinv",
				hformatname(ihd->pixel_format)));
	}
}

int h_dctinv_f(ihd,ohd)

struct header *ihd,*ohd;

{
	return(h_dctinv_F((float *) ihd->firstpix,
		(float *) ohd->firstpix,ihd->rows,ihd->cols,
		ihd->ocols,ohd->ocols));
}

int h_dctinv_d(ihd,ohd)

struct header *ihd,*ohd;

{
	return(h_dctinv_D((double *) ihd->firstpix,
		(double *) ohd->firstpix,ihd->rows,ihd->cols,
		ihd->ocols,ohd->ocols));
}

int h_dctinv_F(imagei,imageo,nr,nc,nlpi,nlpo)

float *imagei,*imageo;
int nr,nc,nlpi,nlpo;

{
	int i,j,logrows,logcols;

	logrows = logcols = -1;
	i = 0; j = 1;
	while (1) {
		if (j == nr)
			logrows = i;
		if (j == nc)
			logcols = i;
		if (j >= nr && j >= nc)
			break;
		i++; j <<= 1;
	}
	if (logrows == -1 || logcols == -1)
		return(perr(HE_POW2));
	return(h_dctinv2d_f(imagei,imageo,logrows,logcols,nlpi,nlpo));
}

int h_dctinv_D(imagei,imageo,nr,nc,nlpi,nlpo)

double *imagei,*imageo;
int nr,nc,nlpi,nlpo;

{
	int i,j,logrows,logcols;

	logrows = logcols = -1;
	i = 0; j = 1;
	while (1) {
		if (j == nr)
			logrows = i;
		if (j == nc)
			logcols = i;
		if (j >= nr && j >= nc)
			break;
		i++; j <<= 1;
	}
	if (logrows == -1 || logcols == -1)
		return(perr(HE_POW2));
	return(h_dctinv2d_d(imagei,imageo,logrows,logcols,nlpi,nlpo));
}

/*
 * dct -- discrete cosine transform
 *
 * No division by N is performed.
 *
 * Calling sequence:
 *
 * h_dct2d_f(veci,veco,logrows,logcols,nlpi,nlpo) - performs a 2-dimensional
 *				dct where veci is the input image, veco is the
 *				output image, logrows is the log of the number
 *				of rows, logcols is the log of the number of
 *				columns, nlpi is the number of values per
 *				stored row of the input image (for ROI), and
 *				nlpo is the corresponding value for the output
 *				image
 * h_dctinv2d_f(veci,veco,logrows,logcols,nlpi,nlpo) - inverse transform
 *
 * h_complex veci,veco;
 * int logrows,logcols,nlpi,nlpo;
 *
 * h_dct2d_d and h_dctinv2d_d are the double versions.  h_dctfree deallocates
 * the working storage.
 */

static float *workvec,*iworkvec,*cconst,*iconst,*cconst2,*iconst2;
static int worksize = 0;
static int constsize = 0,constn = 0;
static int constsize2 = 0,constn2 = 0;
static double *workvec_d,*iworkvec_d,*cconst_d,*iconst_d,*cconst2_d,*iconst2_d;
static int worksize_d = 0;
static int constsize_d = 0,constn_d = 0;
static int constsize2_d = 0,constn2_d = 0;

int h_dct2d_f(veci,veco,logrows,logcols,nlpi,nlpo)

float *veci,*veco;
int logrows,logcols,nlpi,nlpo;

{
	int nr,nc,i,j,nexi,nexo,nr2,nc2,nrc2;
	float *w,*iw,*p,sq2,sqrc;

	nr = 1<<logrows;
	nc = 1<<logcols;
	nc2 = nc*2;
	nr2 = nr*2;
	nrc2 = nr*nc*2;
	nexi = nlpi - nc;
	nexo = nlpo - nc;
	if (worksize < nr*nc*4) {
		if (worksize) {
			free(workvec);
			free(iworkvec);
		}
		worksize = nr*nc*4;
		if ((workvec = (float *) hmalloc(worksize*sizeof(float))) ==
			(float *) HIPS_ERROR)
				return(HIPS_ERROR);
		if ((iworkvec = (float *) hmalloc(worksize*sizeof(float))) ==
			(float *) HIPS_ERROR)
				return(HIPS_ERROR);
	}
	if (constsize < nc) {
		if (constsize) {
			free(cconst);
			free(iconst);
		}
		constsize = nc;
		constn = 0;
		if ((cconst = (float *) hmalloc(nc*sizeof(float))) ==
			(float *) HIPS_ERROR)
				return(HIPS_ERROR);
		if ((iconst = (float *) hmalloc(nc*sizeof(float))) ==
			(float *) HIPS_ERROR)
				return(HIPS_ERROR);
	}
	if (nc != nr && constsize2 < nr) {
		if (constsize2) {
			free(cconst2);
			free(iconst2);
		}
		constsize2 = nr;
		constn2 = 0;
		if ((cconst2 = (float *) hmalloc(nr*sizeof(float))) ==
			(float *) HIPS_ERROR)
				return(HIPS_ERROR);
		if ((iconst2 = (float *) hmalloc(nr*sizeof(float))) ==
			(float *) HIPS_ERROR)
				return(HIPS_ERROR);
	}
	if (constn != nc) {
		constn = nc;
		w = cconst;
		iw = iconst;
		for (i=0;i<nc;i++) {
			*w++ = cos(-H_PI*i/nc2);
			*iw++ = sin(-H_PI*i/nc2);
		}
	}
	if (nc != nr && constn2 != nr) {
		constn2 = nr;
		w = cconst2;
		iw = iconst2;
		for (i=0;i<nr;i++) {
			*w++ = cos(-H_PI*i/nr2);
			*iw++ = sin(-H_PI*i/nr2);
		}
	}
	w = workvec;
	iw = iworkvec;
	p = veci;
	for (i=0;i<nr2;i++) {
		for (j=0;j<nc2;j++) {
			*iw++=0.;
			if (i<nr && j<nc)
				*w++ = *p++;
			else
				*w++ = 0.;
		}
		p += nexi;
	}
	for (i=0;i<nrc2;i+=nc2)
		h_fftn_ri_c(workvec+i,iworkvec+i,logcols+1,1);
	w = workvec;
	iw = iworkvec;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			*w = (*w * *(cconst+j)) - (*iw * *(iconst+j));
			w++;
			*iw++ = 0.;
		}
		w += nc;
		iw += nc;
	}
	for (i=0;i<nc;i++)
		h_fftn_ri_c(workvec+i,iworkvec+i,logrows+1,nc*2);
	p = veco;
	w = workvec;
	iw = iworkvec;
	sq2 = sqrt( (double) 2.);
	sqrc = sqrt( (double) (nr*nc));
	if (nr == nc) {
		for (i=0;i<nr;i++) {
			for (j=0;j<nc;j++) {
				*p = 2. * ((*w++ * *(cconst+i)) -
					(*iw++ * *(iconst+i)))/sqrc;
				if (j == 0 && i == 0)
					*p /= 2.;
				else if (j == 0 || i == 0)
					*p /= sq2;
				p++;
			}
			w += nc;
			iw += nc;
			p += nexo;
		}
	}
	else {
		for (i=0;i<nr;i++) {
			for (j=0;j<nc;j++) {
				*p = 2. * ((*w++ * *(cconst2+i)) -
					(*iw++ * *(iconst2+i)))/sqrc;
				if (j == 0 && i == 0)
					*p /= 2.;
				else if (j == 0 || i == 0)
					*p /= sq2;
				p++;
			}
			w += nc;
			iw += nc;
			p += nexo;
		}
	}
	return(HIPS_OK);
}

int h_dctinv2d_f(veci,veco,logrows,logcols,nlpi,nlpo)

float *veci,*veco;
int logrows,logcols,nlpi,nlpo;

{
	int nr,nc,i,j,nexi,nexo,nr2,nc2,nrc2;
	float *w,*iw,*p,sq2,sqrc;

	nr = 1<<logrows;
	nc = 1<<logcols;
	nc2 = nc*2;
	nr2 = nr*2;
	nrc2 = nr*nc*2;
	nexi = nlpi - nc;
	nexo = nlpo - nc;
	if (worksize < nr*nc*4) {
		if (worksize) {
			free(workvec);
			free(iworkvec);
		}
		worksize = nr*nc*4;
		if ((workvec = (float *) hmalloc(worksize*sizeof(float))) ==
			(float *) HIPS_ERROR)
				return(HIPS_ERROR);
		if ((iworkvec = (float *) hmalloc(worksize*sizeof(float))) ==
			(float *) HIPS_ERROR)
				return(HIPS_ERROR);
	}
	if (constsize < nc) {
		if (constsize) {
			free(cconst);
			free(iconst);
		}
		constsize = nc;
		constn = 0;
		if ((cconst = (float *) hmalloc(nc*sizeof(float))) ==
			(float *) HIPS_ERROR)
				return(HIPS_ERROR);
		if ((iconst = (float *) hmalloc(nc*sizeof(float))) ==
			(float *) HIPS_ERROR)
				return(HIPS_ERROR);
	}
	if (nc != nr && constsize2 < nr) {
		if (constsize2) {
			free(cconst2);
			free(iconst2);
		}
		constsize2 = nr;
		constn2 = 0;
		if ((cconst2 = (float *) hmalloc(nr*sizeof(float))) ==
			(float *) HIPS_ERROR)
				return(HIPS_ERROR);
		if ((iconst2 = (float *) hmalloc(nr*sizeof(float))) ==
			(float *) HIPS_ERROR)
				return(HIPS_ERROR);
	}
	if (constn != nc) {
		constn = nc;
		w = cconst;
		iw = iconst;
		for (i=0;i<nc;i++) {
			*w++ = cos(-H_PI*i/nc2);
			*iw++ = sin(-H_PI*i/nc2);
		}
	}
	if (nc != nr && constn2 != nr) {
		constn2 = nr;
		w = cconst2;
		iw = iconst2;
		for (i=0;i<nr;i++) {
			*w++ = cos(-H_PI*i/nr2);
			*iw++ = sin(-H_PI*i/nr2);
		}
	}
	w = workvec;
	iw = iworkvec;
	p = veci;
	sq2 = sqrt( (double) 2.0);
	for (i=0;i<nr2;i++) {
		for (j=0;j<nc2;j++) {
			if (i<nr && j<nc) {
				*w = *p++;
				if (i==0 && j==0)
					*w /= 2.0;
				else if (i==0 || j==0)
					*w /= sq2;
				*iw++ = *w * *(iconst+j);
				*w = *w * *(cconst+j);
				w++;
			}
			else {
				*w++ = 0.;
				*iw++ = 0.;
			}
		}
		p += nexi;
	}
	for (i=0;i<nrc2;i+=nc2)
		h_fftn_ri_c(workvec+i,iworkvec+i,logcols+1,1);
	w = workvec;
	iw = iworkvec;
	if (nr == nc) {
		for (i=0;i<nr;i++) {
			for (j=0;j<nc;j++) {
				*iw++ = *w * *(iconst+i);
				*w = *w * *(cconst+i);
				w++;
			}
			w += nc;
			iw += nc;
		}
	}
	else {
		for (i=0;i<nr;i++) {
			for (j=0;j<nc;j++) {
				*iw++ = *w * *(iconst2+i);
				*w = *w * *(cconst2+i);
				w++;
			}
			w += nc;
			iw += nc;
		}
	}
	for (i=0;i<nc;i++)
		h_fftn_ri_c(workvec+i,iworkvec+i,logrows+1,nc2);
	p = veco;
	w = workvec;
	sqrc = sqrt( (double) (nr*nc));
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++)
			*p++ = 2. * *w++ / sqrc;
		w += nc;
		p += nexo;
	}
	return(HIPS_OK);
}

int h_dct2d_d(veci,veco,logrows,logcols,nlpi,nlpo)

double *veci,*veco;
int logrows,logcols,nlpi,nlpo;

{
	int nr,nc,i,j,nexi,nexo,nr2,nc2,nrc2;
	double *w,*iw,*p,sq2,sqrc;

	nr = 1<<logrows;
	nc = 1<<logcols;
	nc2 = nc*2;
	nr2 = nr*2;
	nrc2 = nr*nc*2;
	nexi = nlpi - nc;
	nexo = nlpo - nc;
	if (worksize_d < nr*nc*4) {
		if (worksize_d) {
			free(workvec_d);
			free(iworkvec_d);
		}
		worksize_d = nr*nc*4;
		if ((workvec_d = (double *) hmalloc(worksize_d*sizeof(double)))
			== (double *) HIPS_ERROR)
				return(HIPS_ERROR);
		if ((iworkvec_d = (double *) hmalloc(worksize_d*sizeof(double)))
			== (double *) HIPS_ERROR)
				return(HIPS_ERROR);
	}
	if (constsize_d < nc) {
		if (constsize_d) {
			free(cconst_d);
			free(iconst_d);
		}
		constsize_d = nc;
		constn_d = 0;
		if ((cconst_d = (double *) hmalloc(nc*sizeof(double))) ==
			(double *) HIPS_ERROR)
				return(HIPS_ERROR);
		if ((iconst_d = (double *) hmalloc(nc*sizeof(double))) ==
			(double *) HIPS_ERROR)
				return(HIPS_ERROR);
	}
	if (nc != nr && constsize2_d < nr) {
		if (constsize2_d) {
			free(cconst2_d);
			free(iconst2_d);
		}
		constsize2_d = nr;
		constn2_d = 0;
		if ((cconst2_d = (double *) hmalloc(nr*sizeof(double))) ==
			(double *) HIPS_ERROR)
				return(HIPS_ERROR);
		if ((iconst2_d = (double *) hmalloc(nr*sizeof(double))) ==
			(double *) HIPS_ERROR)
				return(HIPS_ERROR);
	}
	if (constn_d != nc) {
		constn_d = nc;
		w = cconst_d;
		iw = iconst_d;
		for (i=0;i<nc;i++) {
			*w++ = cos(-H_PI*i/nc2);
			*iw++ = sin(-H_PI*i/nc2);
		}
	}
	if (nc != nr && constn2_d != nr) {
		constn2_d = nr;
		w = cconst2_d;
		iw = iconst2_d;
		for (i=0;i<nr;i++) {
			*w++ = cos(-H_PI*i/nr2);
			*iw++ = sin(-H_PI*i/nr2);
		}
	}
	w = workvec_d;
	iw = iworkvec_d;
	p = veci;
	for (i=0;i<nr2;i++) {
		for (j=0;j<nc2;j++) {
			*iw++=0.;
			if (i<nr && j<nc)
				*w++ = *p++;
			else
				*w++ = 0.;
		}
		p += nexi;
	}
	for (i=0;i<nrc2;i+=nc2)
		h_fftn_ri_dc(workvec_d+i,iworkvec_d+i,logcols+1,1);
	w = workvec_d;
	iw = iworkvec_d;
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++) {
			*w = (*w * *(cconst_d+j)) - (*iw * *(iconst_d+j));
			w++;
			*iw++ = 0.;
		}
		w += nc;
		iw += nc;
	}
	for (i=0;i<nc;i++)
		h_fftn_ri_dc(workvec_d+i,iworkvec_d+i,logrows+1,nc*2);
	p = veco;
	w = workvec_d;
	iw = iworkvec_d;
	sq2 = sqrt( (double) 2.);
	sqrc = sqrt( (double) (nr*nc));
	if (nr == nc) {
		for (i=0;i<nr;i++) {
			for (j=0;j<nc;j++) {
				*p = 2. * ((*w++ * *(cconst_d+i)) -
					(*iw++ * *(iconst_d+i)))/sqrc;
				if (j == 0 && i == 0)
					*p /= 2.;
				else if (j == 0 || i == 0)
					*p /= sq2;
				p++;
			}
			w += nc;
			iw += nc;
			p += nexo;
		}
	}
	else {
		for (i=0;i<nr;i++) {
			for (j=0;j<nc;j++) {
				*p = 2. * ((*w++ * *(cconst2_d+i)) -
					(*iw++ * *(iconst2_d+i)))/sqrc;
				if (j == 0 && i == 0)
					*p /= 2.;
				else if (j == 0 || i == 0)
					*p /= sq2;
				p++;
			}
			w += nc;
			iw += nc;
			p += nexo;
		}
	}
	return(HIPS_OK);
}

int h_dctinv2d_d(veci,veco,logrows,logcols,nlpi,nlpo)

double *veci,*veco;
int logrows,logcols,nlpi,nlpo;

{
	int nr,nc,i,j,nexi,nexo,nr2,nc2,nrc2;
	double *w,*iw,*p,sq2,sqrc;

	nr = 1<<logrows;
	nc = 1<<logcols;
	nc2 = nc*2;
	nr2 = nr*2;
	nrc2 = nr*nc*2;
	nexi = nlpi - nc;
	nexo = nlpo - nc;
	if (worksize_d < nr*nc*4) {
		if (worksize_d) {
			free(workvec_d);
			free(iworkvec_d);
		}
		worksize_d = nr*nc*4;
		if ((workvec_d = (double *) hmalloc(worksize_d*sizeof(double)))
			== (double *) HIPS_ERROR)
				return(HIPS_ERROR);
		if ((iworkvec_d = (double *) hmalloc(worksize_d*sizeof(double)))
			== (double *) HIPS_ERROR)
				return(HIPS_ERROR);
	}
	if (constsize_d < nc) {
		if (constsize_d) {
			free(cconst_d);
			free(iconst_d);
		}
		constsize_d = nc;
		constn_d = 0;
		if ((cconst_d = (double *) hmalloc(nc*sizeof(double))) ==
			(double *) HIPS_ERROR)
				return(HIPS_ERROR);
		if ((iconst_d = (double *) hmalloc(nc*sizeof(double))) ==
			(double *) HIPS_ERROR)
				return(HIPS_ERROR);
	}
	if (nc != nr && constsize2_d < nr) {
		if (constsize2_d) {
			free(cconst2_d);
			free(iconst2_d);
		}
		constsize2_d = nr;
		constn2_d = 0;
		if ((cconst2_d = (double *) hmalloc(nr*sizeof(double))) ==
			(double *) HIPS_ERROR)
				return(HIPS_ERROR);
		if ((iconst2_d = (double *) hmalloc(nr*sizeof(double))) ==
			(double *) HIPS_ERROR)
				return(HIPS_ERROR);
	}
	if (constn_d != nc) {
		constn_d = nc;
		w = cconst_d;
		iw = iconst_d;
		for (i=0;i<nc;i++) {
			*w++ = cos(-H_PI*i/nc2);
			*iw++ = sin(-H_PI*i/nc2);
		}
	}
	if (nc != nr && constn2_d != nr) {
		constn2_d = nr;
		w = cconst2_d;
		iw = iconst2_d;
		for (i=0;i<nr;i++) {
			*w++ = cos(-H_PI*i/nr2);
			*iw++ = sin(-H_PI*i/nr2);
		}
	}
	w = workvec_d;
	iw = iworkvec_d;
	p = veci;
	sq2 = sqrt( (double) 2.0);
	for (i=0;i<nr2;i++) {
		for (j=0;j<nc2;j++) {
			if (i<nr && j<nc) {
				*w = *p++;
				if (i==0 && j==0)
					*w /= 2.0;
				else if (i==0 || j==0)
					*w /= sq2;
				*iw++ = *w * *(iconst_d+j);
				*w = *w * *(cconst_d+j);
				w++;
			}
			else {
				*w++ = 0.;
				*iw++ = 0.;
			}
		}
		p += nexi;
	}
	for (i=0;i<nrc2;i+=nc2)
		h_fftn_ri_dc(workvec_d+i,iworkvec_d+i,logcols+1,1);
	w = workvec_d;
	iw = iworkvec_d;
	if (nr == nc) {
		for (i=0;i<nr;i++) {
			for (j=0;j<nc;j++) {
				*iw++ = *w * *(iconst_d+i);
				*w = *w * *(cconst_d+i);
				w++;
			}
			w += nc;
			iw += nc;
		}
	}
	else {
		for (i=0;i<nr;i++) {
			for (j=0;j<nc;j++) {
				*iw++ = *w * *(iconst2_d+i);
				*w = *w * *(cconst2_d+i);
				w++;
			}
			w += nc;
			iw += nc;
		}
	}
	for (i=0;i<nc;i++)
		h_fftn_ri_dc(workvec_d+i,iworkvec_d+i,logrows+1,nc2);
	p = veco;
	w = workvec_d;
	sqrc = sqrt( (double) (nr*nc));
	for (i=0;i<nr;i++) {
		for (j=0;j<nc;j++)
			*p++ = 2. * *w++ / sqrc;
		w += nc;
		p += nexo;
	}
	return(HIPS_OK);
}

void h_dctfree()

{
	if (worksize) {
		free(workvec);
		free(iworkvec);
	}
	if (constsize) {
		free(cconst);
		free(iconst);
	}
	if (constsize2) {
		free(cconst2);
		free(iconst2);
	}
	if (worksize_d) {
		free(workvec_d);
		free(iworkvec_d);
	}
	if (constsize_d) {
		free(cconst_d);
		free(iconst_d);
	}
	if (constsize2_d) {
		free(cconst2_d);
		free(iconst2_d);
	}
	worksize = constsize = constn = constsize2 = constn2 = 0;
	worksize_d = constsize_d = constn_d = constsize2_d = constn2_d = 0;
}
