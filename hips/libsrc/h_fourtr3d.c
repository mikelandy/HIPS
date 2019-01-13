/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_fourtr3d.c - subroutines to compute a 3D Fourier transform
 *
 * The transform is computed `in-place'.
 *
 * pixel formats: COMPLEX
 *
 * Michael Landy - 7/10/91
 */

#include <hipl_format.h>
#include <math.h>

int h_fourtr3d(hd)

struct header *hd;

{
	switch(hd->pixel_format) {
	case PFCOMPLEX:	return(h_fourtr3d_c(hd));
	default:	return(perr(HE_FMTSUBR,"h_fourtr3d",
				hformatname(hd->pixel_format)));
	}
}

int h_fourtr3d_c(hd)

struct header *hd;

{
	return(h_fourtr3d_C((h_complex *) hd->image,hd->orows,hd->ocols,
		hd->num_frame));
}

int h_fourtr3d_C(image,nr,nc,nf)

h_complex *image;
int nr,nc,nf;

{
	int i,j,nrc,logrows,logcols,logframes;

	logframes = logrows = logcols = -1;
	i = 0; j = 1;
	while (1) {
		if (j == nr)
			logrows = i;
		if (j == nc)
			logcols = i;
		if (j == nf)
			logframes = i;
		if (j >= nr && j >= nc && j >= nf)
			break;
		i++; j <<= 1;
	}
	if (logrows == -1 || logcols == -1 || logframes == -1)
		return(perr(HE_POW2));
	for (i=0;i<nf;i++)
		if (h_fft2dgen_c(image+i*nr*nc,logrows,logcols,nc) ==
			HIPS_ERROR)
				return(HIPS_ERROR);
	nrc = nr*nc;
	for (i=0;i<nrc;i++)
		if (h_fftn_c(image+i,logframes,nrc) ==
			HIPS_ERROR)
				return(HIPS_ERROR);
	return(HIPS_OK);
}
