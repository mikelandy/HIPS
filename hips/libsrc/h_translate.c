/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_translate.c - subroutines to shift pixels by an integral amount
 *
 * pixel formats: BYTE, SHORT, INT, FLOAT, DOUBLE, COMPLEX, DBLCOM
 *
 * Michael Landy - 7/8/91
 * COMPLEX/DBLCOM - msl - 11/13/92
 */

#include <hipl_format.h>

int h_translate(hdi,hdo,shiftx,shifty,palinflag)

struct header *hdi,*hdo;
int shiftx,shifty;
h_boolean palinflag;

{
	switch(hdi->pixel_format) {
	case PFBYTE:	return(h_translate_b(hdi,hdo,shiftx,shifty,palinflag));
	case PFSHORT:	return(h_translate_s(hdi,hdo,shiftx,shifty,palinflag));
	case PFINT:	return(h_translate_i(hdi,hdo,shiftx,shifty,palinflag));
	case PFFLOAT:	return(h_translate_f(hdi,hdo,shiftx,shifty,palinflag));
	case PFDOUBLE:	return(h_translate_d(hdi,hdo,shiftx,shifty,palinflag));
	case PFCOMPLEX:	return(h_translate_c(hdi,hdo,shiftx,shifty,palinflag));
	case PFDBLCOM:	return(h_translate_dc(hdi,hdo,shiftx,shifty,palinflag));
	default:	return(perr(HE_FMTSUBR,"h_translate",
				hformatname(hdi->pixel_format)));
	}
}

int h_translate_b(hdi,hdo,shiftx,shifty,palinflag)

struct header *hdi,*hdo;
int shiftx,shifty;
h_boolean palinflag;

{
	return(h_translate_B(hdi->firstpix,hdo->firstpix,hdi->rows,hdi->cols,
		hdi->ocols,hdo->ocols,shiftx,shifty,palinflag));
}

int h_translate_s(hdi,hdo,shiftx,shifty,palinflag)

struct header *hdi,*hdo;
int shiftx,shifty;
h_boolean palinflag;

{
	return(h_translate_S((short *) hdi->firstpix,(short *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols,shiftx,
		shifty,palinflag));
}

int h_translate_i(hdi,hdo,shiftx,shifty,palinflag)

struct header *hdi,*hdo;
int shiftx,shifty;
h_boolean palinflag;

{
	return(h_translate_I((int *) hdi->firstpix,(int *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols,shiftx,shifty,
		palinflag));
}

int h_translate_f(hdi,hdo,shiftx,shifty,palinflag)

struct header *hdi,*hdo;
int shiftx,shifty;
h_boolean palinflag;

{
	return(h_translate_F((float *) hdi->firstpix,(float *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols,shiftx,shifty,
		palinflag));
}

int h_translate_d(hdi,hdo,shiftx,shifty,palinflag)

struct header *hdi,*hdo;
int shiftx,shifty;
h_boolean palinflag;

{
	return(h_translate_D((double *) hdi->firstpix,(double *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols,shiftx,shifty,
		palinflag));
}

int h_translate_c(hdi,hdo,shiftx,shifty,palinflag)

struct header *hdi,*hdo;
int shiftx,shifty;
h_boolean palinflag;

{
	return(h_translate_C((float *) hdi->firstpix,(float *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols,shiftx,shifty,
		palinflag));
}

int h_translate_dc(hdi,hdo,shiftx,shifty,palinflag)

struct header *hdi,*hdo;
int shiftx,shifty;
h_boolean palinflag;

{
	return(h_translate_DC((double *) hdi->firstpix,(double *) hdo->firstpix,
		hdi->rows,hdi->cols,hdi->ocols,hdo->ocols,shiftx,shifty,
		palinflag));
}

int h_translate_B(imagei,imageo,nr,nc,nlpi,nlpo,shiftx,shifty,palinflag)

byte *imagei,*imageo;
int nr,nc,nlpi,nlpo,shiftx,shifty;
h_boolean palinflag;

{

#ifdef ULORIG
	shifty = -shifty;
#endif
	if (palinflag) {
		shifty %= nr;
		if (shifty < 0)
			shifty += nr;
		shiftx %= nc;
		if (shiftx < 0)
			shiftx += nc;
		h_copy_B(imagei,imageo+(shifty*nlpo)+shiftx,nr-shifty,
			nc-shiftx,nlpi,nlpo);
		if (shifty != 0) {
			h_copy_B(imagei+(nr-shifty)*nlpi,imageo+shiftx,
				shifty,nc-shiftx,nlpi,nlpo);
			if (shiftx != 0)
				h_copy_B(imagei+(nr-shifty)*nlpi+nc-shiftx,
					imageo,shifty,shiftx,nlpi,
					nlpo);
		}
		if (shiftx != 0)
			h_copy_B(imagei+nc-shiftx,imageo+shifty*nlpo,nr-shifty,
				shiftx,nlpi,nlpo);
	}
	else {
		if (shifty >= nr || shifty <= -nr || shiftx >= nc ||
		    shiftx <= -nc) {	/* no overlap */
			h_setimage_B(imageo,nr,nc,nlpo,(byte) hips_lchar);
			return(HIPS_OK);
		}
		if (shifty < 0) {
		    if (shiftx < 0) {
			h_copy_B(imagei-shifty*nlpi-shiftx,imageo,nr+shifty,
				nc+shiftx,nlpi,nlpo);
			h_setimage_B(imageo+(nr+shifty)*nlpo,-shifty,nc,nlpo,
				(byte) hips_lchar);
			h_setimage_B(imageo+nc+shiftx,nr+shifty,-shiftx,nlpo,
				(byte) hips_lchar);
		    }
		    else {
			h_copy_B(imagei-shifty*nlpi,imageo+shiftx,nr+shifty,
				nc-shiftx,nlpi,nlpo);
			h_setimage_B(imageo+(nr+shifty)*nlpo,-shifty,nc,nlpo,
				(byte) hips_lchar);
			if (shiftx != 0)
				h_setimage_B(imageo,nr+shifty,shiftx,nlpo,
					(byte) hips_lchar);
		    }
		}
		else {
		    if (shiftx < 0) {
			h_copy_B(imagei-shiftx,imageo+(shifty*nlpo),nr-shifty,
				nc+shiftx,nlpi,nlpo);
			if (shifty != 0)
				h_setimage_B(imageo,shifty,nc,nlpo,
					(byte) hips_lchar);
			h_setimage_B(imageo+(shifty*nlpo)+nc+shiftx,nr-shifty,
				-shiftx,nlpo,(byte) hips_lchar);
		    }
		    else {
			h_copy_B(imagei,imageo+(shifty*nlpo)+shiftx,nr-shifty,
				nc-shiftx,nlpi,nlpo);
			if (shifty != 0)
				h_setimage_B(imageo,shifty,nc,nlpo,
					(byte) hips_lchar);
			if (shiftx != 0)
				h_setimage_B(imageo+(shifty*nlpo),nr-shifty,
					shiftx,nlpo,(byte) hips_lchar);
		    }
		}
	}
	return(HIPS_OK);
}

int h_translate_S(imagei,imageo,nr,nc,nlpi,nlpo,shiftx,shifty,palinflag)

short *imagei,*imageo;
int nr,nc,nlpi,nlpo,shiftx,shifty;
h_boolean palinflag;

{

#ifdef ULORIG
	shifty = -shifty;
#endif
	if (palinflag) {
		shifty %= nr;
		if (shifty < 0)
			shifty += nr;
		shiftx %= nc;
		if (shiftx < 0)
			shiftx += nc;
		h_copy_S(imagei,imageo+(shifty*nlpo)+shiftx,nr-shifty,
			nc-shiftx,nlpi,nlpo);
		if (shifty != 0) {
			h_copy_S(imagei+(nr-shifty)*nlpi,imageo+shiftx,
				shifty,nc-shiftx,nlpi,nlpo);
			if (shiftx != 0)
				h_copy_S(imagei+(nr-shifty)*nlpi+nc-shiftx,
					imageo,shifty,shiftx,nlpi,nlpo);
		}
		if (shiftx != 0)
			h_copy_S(imagei+nc-shiftx,imageo+shifty*nlpo,nr-shifty,
				shiftx,nlpi,nlpo);
	}
	else {
		if (shifty >= nr || shifty <= -nr || shiftx >= nc ||
		    shiftx <= -nc) {	/* no overlap */
			h_setimage_S(imageo,nr,nc,nlpo,(short) hips_lchar);
			return(HIPS_OK);
		}
		if (shifty < 0) {
		    if (shiftx < 0) {
			h_copy_S(imagei-shifty*nlpi-shiftx,imageo,nr+shifty,
				nc+shiftx,nlpi,nlpo);
			h_setimage_S(imageo+(nr+shifty)*nlpo,-shifty,nc,nlpo,
				(short) hips_lchar);
			h_setimage_S(imageo+nc+shiftx,nr+shifty,-shiftx,nlpo,
				(short) hips_lchar);
		    }
		    else {
			h_copy_S(imagei-shifty*nlpi,imageo+shiftx,nr+shifty,
				nc-shiftx,nlpi,nlpo);
			h_setimage_S(imageo+(nr+shifty)*nlpo,-shifty,nc,nlpo,
				(short) hips_lchar);
			if (shiftx != 0)
				h_setimage_S(imageo,nr+shifty,shiftx,nlpo,
					(short) hips_lchar);
		    }
		}
		else {
		    if (shiftx < 0) {
			h_copy_S(imagei-shiftx,imageo+(shifty*nlpo),nr-shifty,
				nc+shiftx,nlpi,nlpo);
			if (shifty != 0)
				h_setimage_S(imageo,shifty,nc,nlpo,
					(short) hips_lchar);
			h_setimage_S(imageo+(shifty*nlpo)+nc+shiftx,nr-shifty,
				-shiftx,nlpo,(short) hips_lchar);
		    }
		    else {
			h_copy_S(imagei,imageo+(shifty*nlpo)+shiftx,nr-shifty,
				nc-shiftx,nlpi,nlpo);
			if (shifty != 0)
				h_setimage_S(imageo,shifty,nc,nlpo,
					(short) hips_lchar);
			if (shiftx != 0)
				h_setimage_S(imageo+(shifty*nlpo),nr-shifty,
					shiftx,nlpo,(short) hips_lchar);
		    }
		}
	}
	return(HIPS_OK);
}

int h_translate_I(imagei,imageo,nr,nc,nlpi,nlpo,shiftx,shifty,palinflag)

int *imagei,*imageo;
int nr,nc,nlpi,nlpo,shiftx,shifty;
h_boolean palinflag;

{

#ifdef ULORIG
	shifty = -shifty;
#endif
	if (palinflag) {
		shifty %= nr;
		if (shifty < 0)
			shifty += nr;
		shiftx %= nc;
		if (shiftx < 0)
			shiftx += nc;
		h_copy_I(imagei,imageo+(shifty*nlpo)+shiftx,nr-shifty,
			nc-shiftx,nlpi,nlpo);
		if (shifty != 0) {
			h_copy_I(imagei+(nr-shifty)*nlpi,imageo+shiftx,
				shifty,nc-shiftx,nlpi,nlpo);
			if (shiftx != 0)
				h_copy_I(imagei+(nr-shifty)*nlpi+nc-shiftx,
					imageo,shifty,shiftx,nlpi,nlpo);
		}
		if (shiftx != 0)
			h_copy_I(imagei+nc-shiftx,imageo+shifty*nlpo,nr-shifty,
				shiftx,nlpi,nlpo);
	}
	else {
		if (shifty >= nr || shifty <= -nr || shiftx >= nc ||
		    shiftx <= -nc) {	/* no overlap */
			h_setimage_I(imageo,nr,nc,nlpo,(int) hips_lchar);
			return(HIPS_OK);
		}
		if (shifty < 0) {
		    if (shiftx < 0) {
			h_copy_I(imagei-shifty*nlpi-shiftx,imageo,nr+shifty,
				nc+shiftx,nlpi,nlpo);
			h_setimage_I(imageo+(nr+shifty)*nlpo,-shifty,nc,nlpo,
				(int) hips_lchar);
			h_setimage_I(imageo+nc+shiftx,nr+shifty,-shiftx,nlpo,
				(int) hips_lchar);
		    }
		    else {
			h_copy_I(imagei-shifty*nlpi,imageo+shiftx,nr+shifty,
				nc-shiftx,nlpi,nlpo);
			h_setimage_I(imageo+(nr+shifty)*nlpo,-shifty,nc,nlpo,
				(int) hips_lchar);
			if (shiftx != 0)
				h_setimage_I(imageo,nr+shifty,shiftx,nlpo,
					(int) hips_lchar);
		    }
		}
		else {
		    if (shiftx < 0) {
			h_copy_I(imagei-shiftx,imageo+(shifty*nlpo),nr-shifty,
				nc+shiftx,nlpi,nlpo);
			if (shifty != 0)
				h_setimage_I(imageo,shifty,nc,nlpo,
					(int) hips_lchar);
			h_setimage_I(imageo+(shifty*nlpo)+nc+shiftx,nr-shifty,
				-shiftx,nlpo,(int) hips_lchar);
		    }
		    else {
			h_copy_I(imagei,imageo+(shifty*nlpo)+shiftx,nr-shifty,
				nc-shiftx,nlpi,nlpo);
			if (shifty != 0)
				h_setimage_I(imageo,shifty,nc,nlpo,
					(int) hips_lchar);
			if (shiftx != 0)
				h_setimage_I(imageo+(shifty*nlpo),nr-shifty,
					shiftx,nlpo,(int) hips_lchar);
		    }
		}
	}
	return(HIPS_OK);
}

int h_translate_F(imagei,imageo,nr,nc,nlpi,nlpo,shiftx,shifty,palinflag)

float *imagei,*imageo;
int nr,nc,nlpi,nlpo,shiftx,shifty;
h_boolean palinflag;

{

#ifdef ULORIG
	shifty = -shifty;
#endif
	if (palinflag) {
		shifty %= nr;
		if (shifty < 0)
			shifty += nr;
		shiftx %= nc;
		if (shiftx < 0)
			shiftx += nc;
		h_copy_F(imagei,imageo+(shifty*nlpo)+shiftx,nr-shifty,
			nc-shiftx,nlpi,nlpo);
		if (shifty != 0) {
			h_copy_F(imagei+(nr-shifty)*nlpi,imageo+shiftx,
				shifty,nc-shiftx,nlpi,nlpo);
			if (shiftx != 0)
				h_copy_F(imagei+(nr-shifty)*nlpi+nc-shiftx,
					imageo,shifty,shiftx,nlpi,nlpo);
		}
		if (shiftx != 0)
			h_copy_F(imagei+nc-shiftx,imageo+shifty*nlpo,nr-shifty,
				shiftx,nlpi,nlpo);
	}
	else {
		if (shifty >= nr || shifty <= -nr || shiftx >= nc ||
		    shiftx <= -nc) {	/* no overlap */
			h_setimage_F(imageo,nr,nc,nlpo,(float) hips_lchar);
			return(HIPS_OK);
		}
		if (shifty < 0) {
		    if (shiftx < 0) {
			h_copy_F(imagei-shifty*nlpi-shiftx,imageo,nr+shifty,
				nc+shiftx,nlpi,nlpo);
			h_setimage_F(imageo+(nr+shifty)*nlpo,-shifty,nc,nlpo,
				(float) hips_lchar);
			h_setimage_F(imageo+nc+shiftx,nr+shifty,-shiftx,nlpo,
				(float) hips_lchar);
		    }
		    else {
			h_copy_F(imagei-shifty*nlpi,imageo+shiftx,nr+shifty,
				nc-shiftx,nlpi,nlpo);
			h_setimage_F(imageo+(nr+shifty)*nlpo,-shifty,nc,nlpo,
				(float) hips_lchar);
			if (shiftx != 0)
				h_setimage_F(imageo,nr+shifty,shiftx,nlpo,
					(float) hips_lchar);
		    }
		}
		else {
		    if (shiftx < 0) {
			h_copy_F(imagei-shiftx,imageo+(shifty*nlpo),nr-shifty,
				nc+shiftx,nlpi,nlpo);
			if (shifty != 0)
				h_setimage_F(imageo,shifty,nc,nlpo,
					(float) hips_lchar);
			h_setimage_F(imageo+(shifty*nlpo)+nc+shiftx,nr-shifty,
				-shiftx,nlpo,(float) hips_lchar);
		    }
		    else {
			h_copy_F(imagei,imageo+(shifty*nlpo)+shiftx,nr-shifty,
				nc-shiftx,nlpi,nlpo);
			if (shifty != 0)
				h_setimage_F(imageo,shifty,nc,nlpo,
					(float) hips_lchar);
			if (shiftx != 0)
				h_setimage_F(imageo+(shifty*nlpo),nr-shifty,
					shiftx,nlpo,(float) hips_lchar);
		    }
		}
	}
	return(HIPS_OK);
}

int h_translate_D(imagei,imageo,nr,nc,nlpi,nlpo,shiftx,shifty,palinflag)

double *imagei,*imageo;
int nr,nc,nlpi,nlpo,shiftx,shifty;
h_boolean palinflag;

{

#ifdef ULORIG
	shifty = -shifty;
#endif
	if (palinflag) {
		shifty %= nr;
		if (shifty < 0)
			shifty += nr;
		shiftx %= nc;
		if (shiftx < 0)
			shiftx += nc;
		h_copy_D(imagei,imageo+(shifty*nlpo)+shiftx,nr-shifty,
			nc-shiftx,nlpi,nlpo);
		if (shifty != 0) {
			h_copy_D(imagei+(nr-shifty)*nlpi,imageo+shiftx,
				shifty,nc-shiftx,nlpi,nlpo);
			if (shiftx != 0)
				h_copy_D(imagei+(nr-shifty)*nlpi+nc-shiftx,
					imageo,shifty,shiftx,nlpi,nlpo);
		}
		if (shiftx != 0)
			h_copy_D(imagei+nc-shiftx,imageo+shifty*nlpo,nr-shifty,
				shiftx,nlpi,nlpo);
	}
	else {
		if (shifty >= nr || shifty <= -nr || shiftx >= nc ||
		    shiftx <= -nc) {	/* no overlap */
			h_setimage_D(imageo,nr,nc,nlpo,(float) hips_lchar);
			return(HIPS_OK);
		}
		if (shifty < 0) {
		    if (shiftx < 0) {
			h_copy_D(imagei-shifty*nlpi-shiftx,imageo,nr+shifty,
				nc+shiftx,nlpi,nlpo);
			h_setimage_D(imageo+(nr+shifty)*nlpo,-shifty,nc,nlpo,
				(float) hips_lchar);
			h_setimage_D(imageo+nc+shiftx,nr+shifty,-shiftx,nlpo,
				(float) hips_lchar);
		    }
		    else {
			h_copy_D(imagei-shifty*nlpi,imageo+shiftx,nr+shifty,
				nc-shiftx,nlpi,nlpo);
			h_setimage_D(imageo+(nr+shifty)*nlpo,-shifty,nc,nlpo,
				(float) hips_lchar);
			if (shiftx != 0)
				h_setimage_D(imageo,nr+shifty,shiftx,nlpo,
					(float) hips_lchar);
		    }
		}
		else {
		    if (shiftx < 0) {
			h_copy_D(imagei-shiftx,imageo+(shifty*nlpo),nr-shifty,
				nc+shiftx,nlpi,nlpo);
			if (shifty != 0)
				h_setimage_D(imageo,shifty,nc,nlpo,
					(float) hips_lchar);
			h_setimage_D(imageo+(shifty*nlpo)+nc+shiftx,nr-shifty,
				-shiftx,nlpo,(float) hips_lchar);
		    }
		    else {
			h_copy_D(imagei,imageo+(shifty*nlpo)+shiftx,nr-shifty,
				nc-shiftx,nlpi,nlpo);
			if (shifty != 0)
				h_setimage_D(imageo,shifty,nc,nlpo,
					(float) hips_lchar);
			if (shiftx != 0)
				h_setimage_D(imageo+(shifty*nlpo),nr-shifty,
					shiftx,nlpo,(float) hips_lchar);
		    }
		}
	}
	return(HIPS_OK);
}

int h_translate_C(imagei,imageo,nr,nc,nlpi,nlpo,shiftx,shifty,palinflag)

float *imagei,*imageo;
int nr,nc,nlpi,nlpo,shiftx,shifty;
h_boolean palinflag;

{
	return(h_translate_F(imagei,imageo,nr,2*nc,2*nlpi,2*nlpo,2*shiftx,shifty,
		palinflag));
}

int h_translate_DC(imagei,imageo,nr,nc,nlpi,nlpo,shiftx,shifty,palinflag)

double *imagei,*imageo;
int nr,nc,nlpi,nlpo,shiftx,shifty;
h_boolean palinflag;

{
	return(h_translate_D(imagei,imageo,nr,2*nc,2*nlpi,2*nlpo,2*shiftx,shifty,
		palinflag));
}
