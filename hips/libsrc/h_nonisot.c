/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_nonisot.c - nonisotropic convolution
 *
 * H_nonisot performs nonisotropic convolution.  It is supplied with an array
 * of single frame convolution masks.  H_nonisot does the following.
 * For every row and column of the input image, the value of the chooser image
 * is used as an index into the list of masks.  The appropriate mask is scaled
 * by the input and added into the output image.  The chooser values are used
 * modulo the number of masks (i.e. a chooser value  of nmasks uses mask0).
 * The mask's first pixel is added to the corresponding output pixel position
 * (hence, for ULORIG, it is added rightward and downward, otherwise it is 
 * added rightward and upward).  Note that the output image is not first
 * zeroed; that is the caller's responsibility.
 *
 * pixel formats: INT
 *
 * Michael Landy - 7/9/88
 * Hips 2 - msl - 8/13/91
 */

#include <hipl_format.h>

static int savenmasks = -1;
static int **Mimage,*Mnr,*Mnc,*Mnlp;

int h_nonisot(hdi,hdc,nmasks,hdm,hdo)

struct header *hdi,*hdc,**hdm,*hdo;
int nmasks;

{
	switch(hdi->pixel_format) {
	case PFINT:	return(h_nonisot_i(hdi,hdc,nmasks,hdm,hdo));
	default:	return(perr(HE_FMTSUBR,"h_nonisot",
				hformatname(hdi->pixel_format)));
	}
}

int h_nonisot_i(hdi,hdc,nmasks,hdm,hdo)

struct header *hdi,*hdc,**hdm,*hdo;
int nmasks;

{
	int i;

	if (savenmasks < nmasks) {
		if (savenmasks > 0) {
			free(Mimage); free(Mnr); free(Mnc); free(Mnlp);
		}
		if ((Mimage = (int **) memalloc(nmasks,sizeof(int *))) ==
			(int **) HIPS_ERROR)
				return(HIPS_ERROR);
		if ((Mnr = (int *) memalloc(nmasks,sizeof(int))) ==
			(int *) HIPS_ERROR)
				return(HIPS_ERROR);
		if ((Mnc = (int *) memalloc(nmasks,sizeof(int))) ==
			(int *) HIPS_ERROR)
				return(HIPS_ERROR);
		if ((Mnlp = (int *) memalloc(nmasks,sizeof(int))) ==
			(int *) HIPS_ERROR)
				return(HIPS_ERROR);
		savenmasks = nmasks;
	}
	for (i=0;i<nmasks;i++) {
		Mimage[i] = (int *) (hdm[i]->firstpix);
		Mnr[i] = hdm[i]->rows;
		Mnc[i] = hdm[i]->cols;
		Mnlp[i] = hdm[i]->ocols;
	}
	return(h_nonisot_I((int *) hdi->firstpix,(int *) hdc->firstpix,nmasks,
		Mimage,(int *) hdo->firstpix,hdi->rows,hdi->cols,hdi->ocols,
		hdc->ocols,Mnr,Mnc,Mnlp,hdo->ocols));
}

int h_nonisot_I(imagei,imagec,nmasks,imagem,imageo,nr,nc,nlpi,nlpc,nrm,ncm,nlpm,
	nlpo)

int *imagei,*imagec,**imagem,*imageo;
int nmasks,nr,nc,nlpi,nlpc,*nrm,*ncm,*nlpm,nlpo;

{
	int nexi,nexc,r,c,ival,cval,nmrr,nmcc,nexm,nexo,mr,mc,nexo2,*po2;
	int *pi,*pc,*pm,*po;

	pi = imagei;
	pc = imagec;
	po2 = imageo;
	nexi = nlpi - nc;
	nexc = nlpc - nc;
	nexo2 = nlpo - nc;
	for (r=0;r<nr;r++) {
		for (c=0;c<nc;c++) {
			ival = *pi++;
			cval = *pc++ % nmasks;
			po = po2++;
			if (!ival)
				continue;
			nmrr = nrm[cval];
			nmcc = ncm[cval];
			pm = imagem[cval];
			nexm = nlpm[cval] - nmcc;
			nexo = nlpo - nmcc;
			for (mr=0;mr<nmrr;mr++) {
				if (mr+r >= nr)
					break;
				for (mc=0;mc<nmcc;mc++) {
					if (mc+c >= nc) {
						pm += nmcc - mc;
						po += nmcc - mc;
						break;
					}
					*po++ += ival * (*pm++);
				}
				po += nexo;
				pm += nexm;
			}
		}
		pi += nexi;
		pc += nexc;
		po2 += nexo2;
	}
	return(HIPS_OK);
}
