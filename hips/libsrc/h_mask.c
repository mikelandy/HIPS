/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_mask.c - apply a set of masks to an image
 *
 * A convolution mask set consists of 1 or more masks (arrays of floats or
 * ints), each with associated size and row and column offsets (to denote
 * which mask pixel is centered on the image pixel before cross-correlating
 * (***important note***, h_mask cross-correlates with the masks rather than
 * convolves; in other words, the rows and columns are not first reflected;
 * for the usual mirror symmetric masks this poses no complications).  The
 * masks are supplied as a mask structure, which gives the mask name, format
 * (PFFLOAT or PFINT), size, offsets, mask values, and function number.
 *
 * h_mask cross-correlates the input image with each of the supplied masks.
 * Each mask can be of a different size, and for each mask, the mask structure
 * designates the position of that mask with respect to the input image
 * (in other words, which pixel of the mask overlays a given input image pixel
 * in order to compute the cross-correlation corresponding to the
 * corresponding output image pixel).  The cross-correlation extends the edges
 * of the subimage whenever the mask extends beyond the subimage edges.
 *
 * Note that the earlier mask values are applied to earlier image
 * values.  Thus, the interpretation of the mask orientation depends on the
 * definition of ULORIG.  If ULORIG is defined (images have their origin at
 * the upper-left), then the first mask row is topmost relative to the image.
 * Otherwise, the last mask row is topmost relative to the image.  Otherwise
 * stated, a ULORIG coordinate system applies to masks as well as to images.
 *
 * Mask sets can include 1 or more masks.  h_mask applies a function to the
 * set of mask output values which results in the single pixel value placed in
 * a given position in the output image.  The function number in the mask
 * structure identifies which function will be applied from among:
 *
 *	MASKFUN_MAXABS	- the maximum absolute value of all mask outputs
 *	MASKFUN_MEANSQ  - the square root of the sum of the squares of all masks
 *	MASKFUN_SUMABS  - the sum of the absolute value of all mask outputs
 *	MASKFUN_MAX	- the maximum mask output
 *	MASKFUN_MAXFLR	- the maximum mask output, floored at zero
 *	MASKFUN_MXASFLR	- the larger of |mask-1| and |mask-2|, minus |mask-3|,
 *			  floored at zero
 *	MASKFUN_MUL	- the product of the mask outputs, each floored at zero
 *	MASKFUN_NORM	- the first mask output normalized by the sum of the
 *			  mask entries
 *	MASKFUN_DIFF	- the value of the pixel minus the normalized mask
 *			  output
 *	MASKFUN_ORIENT	- compute orientation: 360*atan(mask2/mask1)/2*PI
 *	MASKFUN_IDENT	- the value of the first mask output (simple
 *			  convolution)
 *
 * input pixel format (integer mask):  BYTE, INT
 * input pixel format (float mask):  BYTE, INT, FLOAT
 * output pixel format: FLOAT
 *
 * Based on HIPS-1 mask: Michael Landy - 4/21/82
 * HIPS-2 - msl - 7/12/91
 */

#include <hipl_format.h>
#include <math.h>

int h_mask_alloc(),h_mask_valloc();
float h_mask_value_i(),h_mask_value_f();

int h_mask(hdi,mask,hdo)

struct header *hdi,*hdo;
struct hips_mask *mask;

{
	switch(hdi->pixel_format) {
	case PFBYTE:	if (mask->pixel_format == PFINT)
				return(h_mask_bif(hdi,mask,hdo));
			else if (mask->pixel_format == PFFLOAT)
				return(h_mask_bff(hdi,mask,hdo));
			else
				return(perr(HE_FMT3SUBR,"h_mask",
					hformatname(hdi->pixel_format),
					hformatname(mask->pixel_format),
					hformatname(hdo->pixel_format)));
	case PFINT:	if (mask->pixel_format == PFINT)
				return(h_mask_iif(hdi,mask,hdo));
			else if (mask->pixel_format == PFFLOAT)
				return(h_mask_iff(hdi,mask,hdo));
			else
				return(perr(HE_FMT3SUBR,"h_mask",
					hformatname(hdi->pixel_format),
					hformatname(mask->pixel_format),
					hformatname(hdo->pixel_format)));
	case PFFLOAT:	if (mask->pixel_format == PFFLOAT)
				return(h_mask_fff(hdi,mask,hdo));
			else
				return(perr(HE_FMT3SUBR,"h_mask",
					hformatname(hdi->pixel_format),
					hformatname(mask->pixel_format),
					hformatname(hdo->pixel_format)));
	default:	return(perr(HE_FMTSUBR,"h_mask",
				hformatname(hdi->pixel_format)));
	}
}

int h_mask_bif(hdi,mask,hdo)

struct header *hdi,*hdo;
struct hips_mask *mask;

{
	return(h_mask_BIF(hdi->firstpix,(float *) hdo->firstpix,hdi->rows,
	    hdi->cols,hdi->ocols,hdo->ocols,mask->nmasks,mask->func_num,
	    mask->vals.i_values,mask->mask_rows,mask->mask_cols,
	    mask->row_offset,mask->col_offset));
}

int h_mask_bff(hdi,mask,hdo)

struct header *hdi,*hdo;
struct hips_mask *mask;

{
	return(h_mask_BFF(hdi->firstpix,(float *) hdo->firstpix,hdi->rows,
	    hdi->cols,hdi->ocols,hdo->ocols,mask->nmasks,mask->func_num,
	    mask->vals.f_values,mask->mask_rows,mask->mask_cols,
	    mask->row_offset,mask->col_offset));
}

int h_mask_iif(hdi,mask,hdo)

struct header *hdi,*hdo;
struct hips_mask *mask;

{
	return(h_mask_IIF((int *) hdi->firstpix,(float *) hdo->firstpix,
	    hdi->rows,hdi->cols,hdi->ocols,hdo->ocols,mask->nmasks,
	    mask->func_num,mask->vals.i_values,mask->mask_rows,mask->mask_cols,
	    mask->row_offset,mask->col_offset));
}

int h_mask_iff(hdi,mask,hdo)

struct header *hdi,*hdo;
struct hips_mask *mask;

{
	return(h_mask_IFF((int *) hdi->firstpix,(float *) hdo->firstpix,
	    hdi->rows,hdi->cols,hdi->ocols,hdo->ocols,mask->nmasks,
	    mask->func_num,mask->vals.f_values,mask->mask_rows,mask->mask_cols,
	    mask->row_offset,mask->col_offset));
}

int h_mask_fff(hdi,mask,hdo)

struct header *hdi,*hdo;
struct hips_mask *mask;

{
	return(h_mask_FFF((float *) hdi->firstpix,(float *) hdo->firstpix,
	    hdi->rows,hdi->cols,hdi->ocols,hdo->ocols,mask->nmasks,
	    mask->func_num,mask->vals.f_values,mask->mask_rows,mask->mask_cols,
	    mask->row_offset,mask->col_offset));
}

static h_boolean alloc = FALSE;
static int allocsize = 0;
static int *m_plusc,*m_minusc,*m_plusr,*m_minusr,*m_boundr,*m_boundb,*m_pinc;

static h_boolean ialloc = FALSE;
static int iallocsize = 0;
static int *m_ival;

static h_boolean falloc = FALSE;
static int fallocsize = 0;
static float *m_fval;

static int Nmasks,Maskfunc;

#define tcol(c)	(c<0? 0 : (c>=nc? nc-1 : c))
#define trow(c) (c<0? 0 : (c>=nr? nr-1 : c))

int h_mask_BIF(imagei,imageo,nr,nc,nlpi,nlpo,nmasks,maskfunc,masks,mrows,mcols,
	mrowoff,mcoloff)

byte *imagei;
float *imageo;
int nr,nc,nlpi,nlpo,nmasks,maskfunc,**masks,*mrows,*mcols,*mrowoff,*mcoloff;

{
	int r,c,dr,dc,tr,MMinusc,MMinusr,Boundb,Boundr;
	int k,m,*p,*maskp,Minusc,Plusc,Minusr,Plusr,Pinc,norm,nexo,nexi;
	float *op;
	byte *ip,*ip2,*ipul;
	h_boolean identflag,absflag,floorflag,onesizeflag,onemaskflag;

	if ((maskfunc == MASKFUN_MXASFLR && nmasks != 3) ||
	    (maskfunc == MASKFUN_NORM && nmasks != 1) ||
	    (maskfunc == MASKFUN_DIFF && nmasks != 1) ||
	    (maskfunc == MASKFUN_ORIENT && nmasks != 2) ||
	    (maskfunc == MASKFUN_IDENT && nmasks != 1))
		return(perr(HE_MSKCNT,"h_mask_BIF",maskfunc,nmasks));
	Nmasks = nmasks;
	Maskfunc = maskfunc;	/* avoid passing arguments in busiest loop */
	absflag = floorflag = onemaskflag = identflag = FALSE;
	if (nmasks == 1) {
		onemaskflag = TRUE;
		if (maskfunc == MASKFUN_MAXABS || maskfunc == MASKFUN_MEANSQ ||
		    maskfunc == MASKFUN_SUMABS)
			absflag = TRUE;
		else if (maskfunc == MASKFUN_MAXFLR || maskfunc == MASKFUN_MUL)
			floorflag = TRUE;
		else if (maskfunc == MASKFUN_MAX || maskfunc == MASKFUN_IDENT)
			identflag = TRUE;
	}
	else {
		onesizeflag = TRUE;
		for (m=1;m<nmasks;m++)
			if (mrows[m] != mrows[0] || mcols[m] != mcols[1] ||
			    mrowoff[m] != mrowoff[0] ||
			    mcoloff[m] != mcoloff[0]) {
				onesizeflag = FALSE;
				break;
			}
	}
	if (maskfunc == MASKFUN_DIFF || maskfunc == MASKFUN_NORM) {
		p = masks[0];
		norm = 0;
		for (dr=0;dr<mrows[0];dr++)
			for (dc=0;dc<mcols[0];dc++)
				norm += *p++;
	}
	if (onemaskflag || onesizeflag) {
		Plusc = mcols[0] - (1 + mcoloff[0]);
		Minusc = Plusc - mcols[0] + 1;
		Plusr = mrows[0] - (1 + mrowoff[0]);
		Minusr = Plusr - mrows[0] + 1;
		Boundr = nc - Plusc - 1;
		Boundb = nr - Plusr - 1;
		Pinc = nlpi - mcols[0];
		MMinusc = -Minusc;
		MMinusr = -Minusr;
	}
	else if (h_mask_alloc(nr,nc,nlpi,mrows,mcols,mrowoff,mcoloff)
		== HIPS_ERROR)
			return(HIPS_ERROR);
	if (!onemaskflag)
		if (h_mask_valloc(PFINT) == HIPS_ERROR)
			return(HIPS_ERROR);
	op = imageo;
	nexo = nlpo - nc;
	nexi = nlpi - nc;
	if (onemaskflag) {	/* Special case: only one mask, BE EFFICIENT */
	  maskp = masks[0];
	  ip = imagei;
	  ipul = imagei + Minusr*nlpi + Minusc;
	  for (r=0;r<nr;r++) {
	    for (c=0;c<nc;c++) {
		k = 0;
		p = maskp;
		if (r < MMinusr || r > Boundb ||
			c < MMinusc || c > Boundr) {
			    for (dr=Minusr;dr<=Plusr;dr++) {
				tr = trow(r+dr)*nlpi;
				for (dc=Minusc;dc<=Plusc;dc++)
					k += *p++ * imagei[tr + tcol(c+dc)];
			    }
		}
		else {
			ip2 = ipul;
			for (dr=Minusr;dr<=Plusr;dr++) {
			    for (dc=Minusc;dc<=Plusc;dc++) 
				k += *p++ * *ip2++;
			    ip2 += Pinc;
			}
		}
		if (identflag)
			*op++ = k;
		else if (absflag)
			*op++ = k < 0 ? -k : k;
		else if (floorflag)
			*op++ = k < 0 ? 0 : k;
		else if (maskfunc == MASKFUN_NORM)
			*op++ = ((float) k)/norm;
		else		/* MASKFUN_DIFF */
			*op++ = *ip++ - (((float) k)/norm);
		ipul++;
	    }
	    ip += nexi;
	    ipul += nexi;
	    op += nexo;
	  }
	}
	else if (onesizeflag) {	/* multiple masks, all same size */
	  ipul = imagei + Minusr*nlpi + Minusc;
	  for (r=0;r<nr;r++) {
	    for (c=0;c<nc;c++) {
		for (m=0;m<nmasks;m++) {
		    k = 0;
		    p = masks[m];
		    if (r < MMinusr || r > Boundb ||
			c < MMinusc || c > Boundr) {
			    for (dr=Minusr;dr<=Plusr;dr++) {
				tr = trow(r+dr)*nlpi;
				for (dc=Minusc;dc<=Plusc;dc++)
					k += *p++ * imagei[tr + tcol(c+dc)];
			    }
		    }
		    else {
			ip2 = ipul;
			for (dr=Minusr;dr<=Plusr;dr++) {
			    for (dc=Minusc;dc<=Plusc;dc++) 
				k += *p++ * *ip2++;
			    ip2 += Pinc;
			}
		    }
		    m_ival[m] = k;
		}
		*op++ = h_mask_value_i();
		ipul++;
	    }
	    op += nexo;
	    ipul += nexi;
	  }
	}
	else {	/* general case, multiple mask sizes */
	  for (r=0;r<nr;r++) {
	    for (c=0;c<nc;c++) {
		for (m=0;m<nmasks;m++) {
		    k = 0;
		    p = masks[m];
		    Minusr = m_minusr[m];
		    Plusr = m_plusr[m];
		    Minusc = m_minusc[m];
		    Plusc = m_plusc[m];
		    if (r < -Minusr || r > m_boundb[m] ||
			c < -Minusc || c > m_boundr[m]) {
			    for (dr=Minusr;dr<=Plusr;dr++) {
				tr = trow(r+dr)*nlpi;
				for (dc=Minusc;dc<=Plusc;dc++)
					k += *p++ * imagei[tr + tcol(c+dc)];
			    }
		    }
		    else {
			ip = imagei + (r + Minusr)*nlpi + c + Minusc;
			Pinc = m_pinc[m];
			for (dr=Minusr;dr<=Plusr;dr++) {
			    for (dc=Minusc;dc<=Plusc;dc++) 
				k += *p++ * *ip++;
			    ip += Pinc;
			}
		    }
		    m_ival[m] = k;
		}
		*op++ = h_mask_value_i();
	    }
	    op += nexo;
	  }
	}
	return(HIPS_OK);
}

int h_mask_BFF(imagei,imageo,nr,nc,nlpi,nlpo,nmasks,maskfunc,masks,mrows,mcols,
	mrowoff,mcoloff)

byte *imagei;
float *imageo,**masks;
int nr,nc,nlpi,nlpo,nmasks,maskfunc,*mrows,*mcols,*mrowoff,*mcoloff;

{
	int r,c,dr,dc,tr,MMinusc,MMinusr,Boundb,Boundr;
	int m,Minusc,Plusc,Minusr,Plusr,Pinc,nexo,nexi;
	float k,*maskp,*p,norm,*op;
	byte *ip,*ip2,*ipul;
	h_boolean identflag,absflag,floorflag,onesizeflag,onemaskflag;

	if ((maskfunc == MASKFUN_MXASFLR && nmasks != 3) ||
	    (maskfunc == MASKFUN_NORM && nmasks != 1) ||
	    (maskfunc == MASKFUN_DIFF && nmasks != 1) ||
	    (maskfunc == MASKFUN_ORIENT && nmasks != 2) ||
	    (maskfunc == MASKFUN_IDENT && nmasks != 1))
		return(perr(HE_MSKCNT,"h_mask_BFF",maskfunc,nmasks));
	Nmasks = nmasks;
	Maskfunc = maskfunc;	/* avoid passing arguments in busiest loop */
	absflag = floorflag = onemaskflag = identflag = FALSE;
	if (nmasks == 1) {
		onemaskflag = TRUE;
		if (maskfunc == MASKFUN_MAXABS || maskfunc == MASKFUN_MEANSQ ||
		    maskfunc == MASKFUN_SUMABS)
			absflag = TRUE;
		else if (maskfunc == MASKFUN_MAXFLR || maskfunc == MASKFUN_MUL)
			floorflag = TRUE;
		else if (maskfunc == MASKFUN_MAX || maskfunc == MASKFUN_IDENT)
			identflag = TRUE;
	}
	else {
		onesizeflag = TRUE;
		for (m=1;m<nmasks;m++)
			if (mrows[m] != mrows[0] || mcols[m] != mcols[1] ||
			    mrowoff[m] != mrowoff[0] ||
			    mcoloff[m] != mcoloff[0]) {
				onesizeflag = FALSE;
				break;
			}
	}
	if (maskfunc == MASKFUN_DIFF || maskfunc == MASKFUN_NORM) {
		p = masks[0];
		norm = 0;
		for (dr=0;dr<mrows[0];dr++)
			for (dc=0;dc<mcols[0];dc++)
				norm += *p++;
	}
	if (onemaskflag || onesizeflag) {
		Plusc = mcols[0] - (1 + mcoloff[0]);
		Minusc = Plusc - mcols[0] + 1;
		Plusr = mrows[0] - (1 + mrowoff[0]);
		Minusr = Plusr - mrows[0] + 1;
		Boundr = nc - Plusc - 1;
		Boundb = nr - Plusr - 1;
		Pinc = nlpi - mcols[0];
		MMinusc = -Minusc;
		MMinusr = -Minusr;
	}
	else if (h_mask_alloc(nr,nc,nlpi,mrows,mcols,mrowoff,mcoloff)
		== HIPS_ERROR)
			return(HIPS_ERROR);
	if (!onemaskflag)
		if (h_mask_valloc(PFFLOAT) == HIPS_ERROR)
			return(HIPS_ERROR);
	op = imageo;
	nexo = nlpo - nc;
	nexi = nlpi - nc;
	if (onemaskflag) {	/* Special case: only one mask, BE EFFICIENT */
	  maskp = masks[0];
	  ip = imagei;
	  ipul = imagei + Minusr*nlpi + Minusc;
	  for (r=0;r<nr;r++) {
	    for (c=0;c<nc;c++) {
		k = 0;
		p = maskp;
		if (r < MMinusr || r > Boundb ||
			c < MMinusc || c > Boundr) {
			    for (dr=Minusr;dr<=Plusr;dr++) {
				tr = trow(r+dr)*nlpi;
				for (dc=Minusc;dc<=Plusc;dc++)
					k += *p++ * imagei[tr + tcol(c+dc)];
			    }
		}
		else {
			ip2 = ipul;
			for (dr=Minusr;dr<=Plusr;dr++) {
			    for (dc=Minusc;dc<=Plusc;dc++) 
				k += *p++ * *ip2++;
			    ip2 += Pinc;
			}
		}
		if (identflag)
			*op++ = k;
		else if (absflag)
			*op++ = k < 0 ? -k : k;
		else if (floorflag)
			*op++ = k < 0 ? 0 : k;
		else if (maskfunc == MASKFUN_NORM)
			*op++ = ((float) k)/norm;
		else		/* MASKFUN_DIFF */
			*op++ = *ip++ - k/norm;
		ipul++;
	    }
	    ip += nexi;
	    ipul += nexi;
	    op += nexo;
	  }
	}
	else if (onesizeflag) {	/* multiple masks, all same size */
	  ipul = imagei + Minusr*nlpi + Minusc;
	  for (r=0;r<nr;r++) {
	    for (c=0;c<nc;c++) {
		for (m=0;m<nmasks;m++) {
		    k = 0;
		    p = masks[m];
		    if (r < MMinusr || r > Boundb ||
			c < MMinusc || c > Boundr) {
			    for (dr=Minusr;dr<=Plusr;dr++) {
				tr = trow(r+dr)*nlpi;
				for (dc=Minusc;dc<=Plusc;dc++)
					k += *p++ * imagei[tr + tcol(c+dc)];
			    }
		    }
		    else {
			ip2 = ipul;
			for (dr=Minusr;dr<=Plusr;dr++) {
			    for (dc=Minusc;dc<=Plusc;dc++) 
				k += *p++ * *ip2++;
			    ip2 += Pinc;
			}
		    }
		    m_fval[m] = k;
		}
		*op++ = h_mask_value_f();
		ipul++;
	    }
	    op += nexo;
	    ipul += nexi;
	  }
	}
	else {	/* general case, multiple mask sizes */
	  for (r=0;r<nr;r++) {
	    for (c=0;c<nc;c++) {
		for (m=0;m<nmasks;m++) {
		    k = 0;
		    p = masks[m];
		    Minusr = m_minusr[m];
		    Plusr = m_plusr[m];
		    Minusc = m_minusc[m];
		    Plusc = m_plusc[m];
		    if (r < -Minusr || r > m_boundb[m] ||
			c < -Minusc || c > m_boundr[m]) {
			    for (dr=Minusr;dr<=Plusr;dr++) {
				tr = trow(r+dr)*nlpi;
				for (dc=Minusc;dc<=Plusc;dc++)
					k += *p++ * imagei[tr + tcol(c+dc)];
			    }
		    }
		    else {
			ip = imagei + (r + Minusr)*nlpi + c + Minusc;
			Pinc = m_pinc[m];
			for (dr=Minusr;dr<=Plusr;dr++) {
			    for (dc=Minusc;dc<=Plusc;dc++) 
				k += *p++ * *ip++;
			    ip += Pinc;
			}
		    }
		    m_fval[m] = k;
		}
		*op++ = h_mask_value_f();
	    }
	    op += nexo;
	  }
	}
	return(HIPS_OK);
}

int h_mask_IIF(imagei,imageo,nr,nc,nlpi,nlpo,nmasks,maskfunc,masks,mrows,mcols,
	mrowoff,mcoloff)

int *imagei;
float *imageo;
int nr,nc,nlpi,nlpo,nmasks,maskfunc,**masks,*mrows,*mcols,*mrowoff,*mcoloff;

{
	int r,c,dr,dc,tr,MMinusc,MMinusr,Boundb,Boundr;
	int k,m,*p,*maskp,Minusc,Plusc,Minusr,Plusr,Pinc,norm,nexo,nexi;
	float *op;
	int *ip,*ip2,*ipul;
	h_boolean identflag,absflag,floorflag,onesizeflag,onemaskflag;

	if ((maskfunc == MASKFUN_MXASFLR && nmasks != 3) ||
	    (maskfunc == MASKFUN_NORM && nmasks != 1) ||
	    (maskfunc == MASKFUN_DIFF && nmasks != 1) ||
	    (maskfunc == MASKFUN_ORIENT && nmasks != 2) ||
	    (maskfunc == MASKFUN_IDENT && nmasks != 1))
		return(perr(HE_MSKCNT,"h_mask_IIF",maskfunc,nmasks));
	Nmasks = nmasks;
	Maskfunc = maskfunc;	/* avoid passing arguments in busiest loop */
	absflag = floorflag = onemaskflag = identflag = FALSE;
	if (nmasks == 1) {
		onemaskflag = TRUE;
		if (maskfunc == MASKFUN_MAXABS || maskfunc == MASKFUN_MEANSQ ||
		    maskfunc == MASKFUN_SUMABS)
			absflag = TRUE;
		else if (maskfunc == MASKFUN_MAXFLR || maskfunc == MASKFUN_MUL)
			floorflag = TRUE;
		else if (maskfunc == MASKFUN_MAX || maskfunc == MASKFUN_IDENT)
			identflag = TRUE;
	}
	else {
		onesizeflag = TRUE;
		for (m=1;m<nmasks;m++)
			if (mrows[m] != mrows[0] || mcols[m] != mcols[1] ||
			    mrowoff[m] != mrowoff[0] ||
			    mcoloff[m] != mcoloff[0]) {
				onesizeflag = FALSE;
				break;
			}
	}
	if (maskfunc == MASKFUN_DIFF || maskfunc == MASKFUN_NORM) {
		p = masks[0];
		norm = 0;
		for (dr=0;dr<mrows[0];dr++)
			for (dc=0;dc<mcols[0];dc++)
				norm += *p++;
	}
	if (onemaskflag || onesizeflag) {
		Plusc = mcols[0] - (1 + mcoloff[0]);
		Minusc = Plusc - mcols[0] + 1;
		Plusr = mrows[0] - (1 + mrowoff[0]);
		Minusr = Plusr - mrows[0] + 1;
		Boundr = nc - Plusc - 1;
		Boundb = nr - Plusr - 1;
		Pinc = nlpi - mcols[0];
		MMinusc = -Minusc;
		MMinusr = -Minusr;
	}
	else if (h_mask_alloc(nr,nc,nlpi,mrows,mcols,mrowoff,mcoloff)
		== HIPS_ERROR)
			return(HIPS_ERROR);
	if (!onemaskflag)
		if (h_mask_valloc(PFINT) == HIPS_ERROR)
			return(HIPS_ERROR);
	op = imageo;
	nexo = nlpo - nc;
	nexi = nlpi - nc;
	if (onemaskflag) {	/* Special case: only one mask, BE EFFICIENT */
	  maskp = masks[0];
	  ip = imagei;
	  ipul = imagei + Minusr*nlpi + Minusc;
	  for (r=0;r<nr;r++) {
	    for (c=0;c<nc;c++) {
		k = 0;
		p = maskp;
		if (r < MMinusr || r > Boundb ||
			c < MMinusc || c > Boundr) {
			    for (dr=Minusr;dr<=Plusr;dr++) {
				tr = trow(r+dr)*nlpi;
				for (dc=Minusc;dc<=Plusc;dc++)
					k += *p++ * imagei[tr + tcol(c+dc)];
			    }
		}
		else {
			ip2 = ipul;
			for (dr=Minusr;dr<=Plusr;dr++) {
			    for (dc=Minusc;dc<=Plusc;dc++) 
				k += *p++ * *ip2++;
			    ip2 += Pinc;
			}
		}
		if (identflag)
			*op++ = k;
		else if (absflag)
			*op++ = k < 0 ? -k : k;
		else if (floorflag)
			*op++ = k < 0 ? 0 : k;
		else if (maskfunc == MASKFUN_NORM)
			*op++ = ((float) k)/norm;
		else		/* MASKFUN_DIFF */
			*op++ = *ip++ - (((float) k)/norm);
		ipul++;
	    }
	    ip += nexi;
	    ipul += nexi;
	    op += nexo;
	  }
	}
	else if (onesizeflag) {	/* multiple masks, all same size */
	  ipul = imagei + Minusr*nlpi + Minusc;
	  for (r=0;r<nr;r++) {
	    for (c=0;c<nc;c++) {
		for (m=0;m<nmasks;m++) {
		    k = 0;
		    p = masks[m];
		    if (r < MMinusr || r > Boundb ||
			c < MMinusc || c > Boundr) {
			    for (dr=Minusr;dr<=Plusr;dr++) {
				tr = trow(r+dr)*nlpi;
				for (dc=Minusc;dc<=Plusc;dc++)
					k += *p++ * imagei[tr + tcol(c+dc)];
			    }
		    }
		    else {
			ip2 = ipul;
			for (dr=Minusr;dr<=Plusr;dr++) {
			    for (dc=Minusc;dc<=Plusc;dc++) 
				k += *p++ * *ip2++;
			    ip2 += Pinc;
			}
		    }
		    m_ival[m] = k;
		}
		*op++ = h_mask_value_i();
		ipul++;
	    }
	    op += nexo;
	    ipul += nexi;
	  }
	}
	else {	/* general case, multiple mask sizes */
	  for (r=0;r<nr;r++) {
	    for (c=0;c<nc;c++) {
		for (m=0;m<nmasks;m++) {
		    k = 0;
		    p = masks[m];
		    Minusr = m_minusr[m];
		    Plusr = m_plusr[m];
		    Minusc = m_minusc[m];
		    Plusc = m_plusc[m];
		    if (r < -Minusr || r > m_boundb[m] ||
			c < -Minusc || c > m_boundr[m]) {
			    for (dr=Minusr;dr<=Plusr;dr++) {
				tr = trow(r+dr)*nlpi;
				for (dc=Minusc;dc<=Plusc;dc++)
					k += *p++ * imagei[tr + tcol(c+dc)];
			    }
		    }
		    else {
			ip = imagei + (r + Minusr)*nlpi + c + Minusc;
			Pinc = m_pinc[m];
			for (dr=Minusr;dr<=Plusr;dr++) {
			    for (dc=Minusc;dc<=Plusc;dc++) 
				k += *p++ * *ip++;
			    ip += Pinc;
			}
		    }
		    m_ival[m] = k;
		}
		*op++ = h_mask_value_i();
	    }
	    op += nexo;
	  }
	}
	return(HIPS_OK);
}

int h_mask_IFF(imagei,imageo,nr,nc,nlpi,nlpo,nmasks,maskfunc,masks,mrows,mcols,
	mrowoff,mcoloff)

int *imagei;
float *imageo,**masks;
int nr,nc,nlpi,nlpo,nmasks,maskfunc,*mrows,*mcols,*mrowoff,*mcoloff;

{
	int r,c,dr,dc,tr,MMinusc,MMinusr,Boundb,Boundr;
	int m,Minusc,Plusc,Minusr,Plusr,Pinc,nexo,nexi;
	float k,*maskp,*p,norm,*op;
	int *ip,*ip2,*ipul;
	h_boolean identflag,absflag,floorflag,onesizeflag,onemaskflag;

	if ((maskfunc == MASKFUN_MXASFLR && nmasks != 3) ||
	    (maskfunc == MASKFUN_NORM && nmasks != 1) ||
	    (maskfunc == MASKFUN_DIFF && nmasks != 1) ||
	    (maskfunc == MASKFUN_ORIENT && nmasks != 2) ||
	    (maskfunc == MASKFUN_IDENT && nmasks != 1))
		return(perr(HE_MSKCNT,"h_mask_IFF",maskfunc,nmasks));
	Nmasks = nmasks;
	Maskfunc = maskfunc;	/* avoid passing arguments in busiest loop */
	absflag = floorflag = onemaskflag = identflag = FALSE;
	if (nmasks == 1) {
		onemaskflag = TRUE;
		if (maskfunc == MASKFUN_MAXABS || maskfunc == MASKFUN_MEANSQ ||
		    maskfunc == MASKFUN_SUMABS)
			absflag = TRUE;
		else if (maskfunc == MASKFUN_MAXFLR || maskfunc == MASKFUN_MUL)
			floorflag = TRUE;
		else if (maskfunc == MASKFUN_MAX || maskfunc == MASKFUN_IDENT)
			identflag = TRUE;
	}
	else {
		onesizeflag = TRUE;
		for (m=1;m<nmasks;m++)
			if (mrows[m] != mrows[0] || mcols[m] != mcols[1] ||
			    mrowoff[m] != mrowoff[0] ||
			    mcoloff[m] != mcoloff[0]) {
				onesizeflag = FALSE;
				break;
			}
	}
	if (maskfunc == MASKFUN_DIFF || maskfunc == MASKFUN_NORM) {
		p = masks[0];
		norm = 0;
		for (dr=0;dr<mrows[0];dr++)
			for (dc=0;dc<mcols[0];dc++)
				norm += *p++;
	}
	if (onemaskflag || onesizeflag) {
		Plusc = mcols[0] - (1 + mcoloff[0]);
		Minusc = Plusc - mcols[0] + 1;
		Plusr = mrows[0] - (1 + mrowoff[0]);
		Minusr = Plusr - mrows[0] + 1;
		Boundr = nc - Plusc - 1;
		Boundb = nr - Plusr - 1;
		Pinc = nlpi - mcols[0];
		MMinusc = -Minusc;
		MMinusr = -Minusr;
	}
	else if (h_mask_alloc(nr,nc,nlpi,mrows,mcols,mrowoff,mcoloff)
		== HIPS_ERROR)
			return(HIPS_ERROR);
	if (!onemaskflag)
		if (h_mask_valloc(PFFLOAT) == HIPS_ERROR)
			return(HIPS_ERROR);
	op = imageo;
	nexo = nlpo - nc;
	nexi = nlpi - nc;
	if (onemaskflag) {	/* Special case: only one mask, BE EFFICIENT */
	  maskp = masks[0];
	  ip = imagei;
	  ipul = imagei + Minusr*nlpi + Minusc;
	  for (r=0;r<nr;r++) {
	    for (c=0;c<nc;c++) {
		k = 0;
		p = maskp;
		if (r < MMinusr || r > Boundb ||
			c < MMinusc || c > Boundr) {
			    for (dr=Minusr;dr<=Plusr;dr++) {
				tr = trow(r+dr)*nlpi;
				for (dc=Minusc;dc<=Plusc;dc++)
					k += *p++ * imagei[tr + tcol(c+dc)];
			    }
		}
		else {
			ip2 = ipul;
			for (dr=Minusr;dr<=Plusr;dr++) {
			    for (dc=Minusc;dc<=Plusc;dc++) 
				k += *p++ * *ip2++;
			    ip2 += Pinc;
			}
		}
		if (identflag)
			*op++ = k;
		else if (absflag)
			*op++ = k < 0 ? -k : k;
		else if (floorflag)
			*op++ = k < 0 ? 0 : k;
		else if (maskfunc == MASKFUN_NORM)
			*op++ = ((float) k)/norm;
		else		/* MASKFUN_DIFF */
			*op++ = *ip++ - k/norm;
		ipul++;
	    }
	    ip += nexi;
	    ipul += nexi;
	    op += nexo;
	  }
	}
	else if (onesizeflag) {	/* multiple masks, all same size */
	  ipul = imagei + Minusr*nlpi + Minusc;
	  for (r=0;r<nr;r++) {
	    for (c=0;c<nc;c++) {
		for (m=0;m<nmasks;m++) {
		    k = 0;
		    p = masks[m];
		    if (r < MMinusr || r > Boundb ||
			c < MMinusc || c > Boundr) {
			    for (dr=Minusr;dr<=Plusr;dr++) {
				tr = trow(r+dr)*nlpi;
				for (dc=Minusc;dc<=Plusc;dc++)
					k += *p++ * imagei[tr + tcol(c+dc)];
			    }
		    }
		    else {
			ip2 = ipul;
			for (dr=Minusr;dr<=Plusr;dr++) {
			    for (dc=Minusc;dc<=Plusc;dc++) 
				k += *p++ * *ip2++;
			    ip2 += Pinc;
			}
		    }
		    m_fval[m] = k;
		}
		*op++ = h_mask_value_f();
		ipul++;
	    }
	    op += nexo;
	    ipul += nexi;
	  }
	}
	else {	/* general case, multiple mask sizes */
	  for (r=0;r<nr;r++) {
	    for (c=0;c<nc;c++) {
		for (m=0;m<nmasks;m++) {
		    k = 0;
		    p = masks[m];
		    Minusr = m_minusr[m];
		    Plusr = m_plusr[m];
		    Minusc = m_minusc[m];
		    Plusc = m_plusc[m];
		    if (r < -Minusr || r > m_boundb[m] ||
			c < -Minusc || c > m_boundr[m]) {
			    for (dr=Minusr;dr<=Plusr;dr++) {
				tr = trow(r+dr)*nlpi;
				for (dc=Minusc;dc<=Plusc;dc++)
					k += *p++ * imagei[tr + tcol(c+dc)];
			    }
		    }
		    else {
			ip = imagei + (r + Minusr)*nlpi + c + Minusc;
			Pinc = m_pinc[m];
			for (dr=Minusr;dr<=Plusr;dr++) {
			    for (dc=Minusc;dc<=Plusc;dc++) 
				k += *p++ * *ip++;
			    ip += Pinc;
			}
		    }
		    m_fval[m] = k;
		}
		*op++ = h_mask_value_f();
	    }
	    op += nexo;
	  }
	}
	return(HIPS_OK);
}

int h_mask_FFF(imagei,imageo,nr,nc,nlpi,nlpo,nmasks,maskfunc,masks,mrows,mcols,
	mrowoff,mcoloff)

float *imagei,*imageo,**masks;
int nr,nc,nlpi,nlpo,nmasks,maskfunc,*mrows,*mcols,*mrowoff,*mcoloff;

{
	int r,c,dr,dc,tr,MMinusc,MMinusr,Boundb,Boundr;
	int m,Minusc,Plusc,Minusr,Plusr,Pinc,nexo,nexi;
	float k,*maskp,*p,norm,*op;
	float *ip,*ip2,*ipul;
	h_boolean identflag,absflag,floorflag,onesizeflag,onemaskflag;

	if ((maskfunc == MASKFUN_MXASFLR && nmasks != 3) ||
	    (maskfunc == MASKFUN_NORM && nmasks != 1) ||
	    (maskfunc == MASKFUN_DIFF && nmasks != 1) ||
	    (maskfunc == MASKFUN_ORIENT && nmasks != 2) ||
	    (maskfunc == MASKFUN_IDENT && nmasks != 1))
		return(perr(HE_MSKCNT,"h_mask_FFF",maskfunc,nmasks));
	Nmasks = nmasks;
	Maskfunc = maskfunc;	/* avoid passing arguments in busiest loop */
	absflag = floorflag = onemaskflag = identflag = FALSE;
	if (nmasks == 1) {
		onemaskflag = TRUE;
		if (maskfunc == MASKFUN_MAXABS || maskfunc == MASKFUN_MEANSQ ||
		    maskfunc == MASKFUN_SUMABS)
			absflag = TRUE;
		else if (maskfunc == MASKFUN_MAXFLR || maskfunc == MASKFUN_MUL)
			floorflag = TRUE;
		else if (maskfunc == MASKFUN_MAX || maskfunc == MASKFUN_IDENT)
			identflag = TRUE;
	}
	else {
		onesizeflag = TRUE;
		for (m=1;m<nmasks;m++)
			if (mrows[m] != mrows[0] || mcols[m] != mcols[1] ||
			    mrowoff[m] != mrowoff[0] ||
			    mcoloff[m] != mcoloff[0]) {
				onesizeflag = FALSE;
				break;
			}
	}
	if (maskfunc == MASKFUN_DIFF || maskfunc == MASKFUN_NORM) {
		p = masks[0];
		norm = 0;
		for (dr=0;dr<mrows[0];dr++)
			for (dc=0;dc<mcols[0];dc++)
				norm += *p++;
	}
	if (onemaskflag || onesizeflag) {
		Plusc = mcols[0] - (1 + mcoloff[0]);
		Minusc = Plusc - mcols[0] + 1;
		Plusr = mrows[0] - (1 + mrowoff[0]);
		Minusr = Plusr - mrows[0] + 1;
		Boundr = nc - Plusc - 1;
		Boundb = nr - Plusr - 1;
		Pinc = nlpi - mcols[0];
		MMinusc = -Minusc;
		MMinusr = -Minusr;
	}
	else if (h_mask_alloc(nr,nc,nlpi,mrows,mcols,mrowoff,mcoloff)
		== HIPS_ERROR)
			return(HIPS_ERROR);
	if (!onemaskflag)
		if (h_mask_valloc(PFFLOAT) == HIPS_ERROR)
			return(HIPS_ERROR);
	op = imageo;
	nexo = nlpo - nc;
	nexi = nlpi - nc;
	if (onemaskflag) {	/* Special case: only one mask, BE EFFICIENT */
	  maskp = masks[0];
	  ip = imagei;
	  ipul = imagei + Minusr*nlpi + Minusc;
	  for (r=0;r<nr;r++) {
	    for (c=0;c<nc;c++) {
		k = 0;
		p = maskp;
		if (r < MMinusr || r > Boundb ||
			c < MMinusc || c > Boundr) {
			    for (dr=Minusr;dr<=Plusr;dr++) {
				tr = trow(r+dr)*nlpi;
				for (dc=Minusc;dc<=Plusc;dc++)
					k += *p++ * imagei[tr + tcol(c+dc)];
			    }
		}
		else {
			ip2 = ipul;
			for (dr=Minusr;dr<=Plusr;dr++) {
			    for (dc=Minusc;dc<=Plusc;dc++) 
				k += *p++ * *ip2++;
			    ip2 += Pinc;
			}
		}
		if (identflag)
			*op++ = k;
		else if (absflag)
			*op++ = k < 0 ? -k : k;
		else if (floorflag)
			*op++ = k < 0 ? 0 : k;
		else if (maskfunc == MASKFUN_NORM)
			*op++ = ((float) k)/norm;
		else		/* MASKFUN_DIFF */
			*op++ = *ip++ - k/norm;
		ipul++;
	    }
	    ip += nexi;
	    ipul += nexi;
	    op += nexo;
	  }
	}
	else if (onesizeflag) {	/* multiple masks, all same size */
	  ipul = imagei + Minusr*nlpi + Minusc;
	  for (r=0;r<nr;r++) {
	    for (c=0;c<nc;c++) {
		for (m=0;m<nmasks;m++) {
		    k = 0;
		    p = masks[m];
		    if (r < MMinusr || r > Boundb ||
			c < MMinusc || c > Boundr) {
			    for (dr=Minusr;dr<=Plusr;dr++) {
				tr = trow(r+dr)*nlpi;
				for (dc=Minusc;dc<=Plusc;dc++)
					k += *p++ * imagei[tr + tcol(c+dc)];
			    }
		    }
		    else {
			ip2 = ipul;
			for (dr=Minusr;dr<=Plusr;dr++) {
			    for (dc=Minusc;dc<=Plusc;dc++) 
				k += *p++ * *ip2++;
			    ip2 += Pinc;
			}
		    }
		    m_fval[m] = k;
		}
		*op++ = h_mask_value_f();
		ipul++;
	    }
	    op += nexo;
	    ipul += nexi;
	  }
	}
	else {	/* general case, multiple mask sizes */
	  for (r=0;r<nr;r++) {
	    for (c=0;c<nc;c++) {
		for (m=0;m<nmasks;m++) {
		    k = 0;
		    p = masks[m];
		    Minusr = m_minusr[m];
		    Plusr = m_plusr[m];
		    Minusc = m_minusc[m];
		    Plusc = m_plusc[m];
		    if (r < -Minusr || r > m_boundb[m] ||
			c < -Minusc || c > m_boundr[m]) {
			    for (dr=Minusr;dr<=Plusr;dr++) {
				tr = trow(r+dr)*nlpi;
				for (dc=Minusc;dc<=Plusc;dc++)
					k += *p++ * imagei[tr + tcol(c+dc)];
			    }
		    }
		    else {
			ip = imagei + (r + Minusr)*nlpi + c + Minusc;
			Pinc = m_pinc[m];
			for (dr=Minusr;dr<=Plusr;dr++) {
			    for (dc=Minusc;dc<=Plusc;dc++) 
				k += *p++ * *ip++;
			    ip += Pinc;
			}
		    }
		    m_fval[m] = k;
		}
		*op++ = h_mask_value_f();
	    }
	    op += nexo;
	  }
	}
	return(HIPS_OK);
}

int h_mask_alloc(nr,nc,nlpi,mrows,mcols,mrowoff,mcoloff)

int nr,nc,nlpi,*mrows,*mcols,*mrowoff,*mcoloff;

{
	int i;

	if (!alloc || allocsize < Nmasks) {
		if (alloc) {
			free(m_plusc); free(m_minusc); free(m_plusr);
			free(m_minusr); free(m_boundr);
			free(m_boundb); free(m_pinc);
		}
		if ((m_plusc = (int *) memalloc(Nmasks,sizeof(int)))
			== (int *) HIPS_ERROR)
				return(HIPS_ERROR);
		if ((m_minusc = (int *) memalloc(Nmasks,sizeof(int)))
			== (int *) HIPS_ERROR)
				return(HIPS_ERROR);
		if ((m_plusr = (int *) memalloc(Nmasks,sizeof(int)))
			== (int *) HIPS_ERROR)
				return(HIPS_ERROR);
		if ((m_minusr = (int *) memalloc(Nmasks,sizeof(int)))
			== (int *) HIPS_ERROR)
				return(HIPS_ERROR);
		if ((m_boundr = (int *) memalloc(Nmasks,sizeof(int)))
			== (int *) HIPS_ERROR)
				return(HIPS_ERROR);
		if ((m_boundb = (int *) memalloc(Nmasks,sizeof(int)))
			== (int *) HIPS_ERROR)
				return(HIPS_ERROR);
		if ((m_pinc = (int *) memalloc(Nmasks,sizeof(int)))
			== (int *) HIPS_ERROR)
				return(HIPS_ERROR);
		alloc = TRUE;
		allocsize = Nmasks;
	}
	for (i=0;i<Nmasks;i++) {
		m_plusc[i] = mcols[i] - (1 + mcoloff[i]);
		m_minusc[i] = m_plusc[i] - mcols[i] + 1;
		m_plusr[i] = mrows[i] - (1 + mrowoff[i]);
		m_minusr[i] = m_plusr[i] - mrows[i] + 1;
		m_boundr[i] = nc - m_plusc[i] - 1;
		m_boundb[i] = nr - m_plusr[i] - 1;
		m_pinc[i] = nlpi - mcols[i];
	}
	return(HIPS_OK);
}

int h_mask_valloc(format)

int format;

{
	if (format == PFINT) {
		if (!ialloc || iallocsize < Nmasks) {
			if (ialloc)
				free(m_ival);
			if ((m_ival = (int *) memalloc(Nmasks,sizeof(int)))
				== (int *) HIPS_ERROR)
					return(HIPS_ERROR);
			ialloc = TRUE;
			iallocsize = Nmasks;
		}
	}
	else {
		if (!falloc || fallocsize < Nmasks) {
			if (falloc)
				free(m_fval);
			if ((m_fval = (float *) memalloc(Nmasks,sizeof(float)))
				== (float *) HIPS_ERROR)
					return(HIPS_ERROR);
			falloc = TRUE;
			fallocsize = Nmasks;
		}
	}
	return(HIPS_OK);
}

float h_mask_value_i()

{
	int i,k;

	k = 0;
	switch(Maskfunc) {
	case MASKFUN_MAXABS:
		k = abs(m_ival[0]);
		for (i=1;i<Nmasks;i++)
			if (abs(m_ival[i])>k)
				k = abs(m_ival[i]);
		return((float) k);
	case MASKFUN_MEANSQ:
		for (i=0;i<Nmasks;i++)
			k += m_ival[i]*m_ival[i];
		return((float) sqrt((double) k));
	case MASKFUN_SUMABS:
		for (i=0;i<Nmasks;i++)
			k += abs(m_ival[i]);
		return((float) k);
	case MASKFUN_MAX:
		k = m_ival[0];
		for (i=1;i<Nmasks;i++)
			if (m_ival[i]>k)
				k = m_ival[i];
		return((float) k);
	case MASKFUN_MAXFLR:
		k = m_ival[0];
		for (i=1;i<Nmasks;i++)
			if (m_ival[i]>k)
				k=m_ival[i];
		if (k>0)
			return((float) k);
		return(0.);
	case MASKFUN_MXASFLR:
		k = abs(m_ival[0])>abs(m_ival[1])
			? abs(m_ival[0]) : abs(m_ival[1]);
		k -= abs(m_ival[2]);
		if (k>0)
			return((float) k);
		return(0.);
	case MASKFUN_MUL:
		k = 1;
		for (i=0;i<Nmasks;i++)
			k *= m_ival[i]>0 ? m_ival[i] : 0;
		return((float) k);
	case MASKFUN_ORIENT:
		if (m_ival[0] == 0 && m_ival[1] == 0)
			return(0.);
		else
			return((float) (H_180_PI*atan2((double) m_ival[0],
				(double) m_ival[1])));
	default:
		return(perr(HE_MSKFUNSUBR,"h_mask_value_i",Maskfunc));
	}
}

float h_mask_value_f()

{
	int i;
	float k;

	k = 0;
	switch(Maskfunc) {
	case MASKFUN_MAXABS:
		k = fabs((double) m_fval[0]);
		for (i=1;i<Nmasks;i++)
			if (fabs((double) m_fval[i])>k)
				k = fabs((double) m_fval[i]);
		return(k);
	case MASKFUN_MEANSQ:
		for (i=0;i<Nmasks;i++)
			k += m_fval[i]*m_fval[i];
		return((float) sqrt((double) k));
	case MASKFUN_SUMABS:
		for (i=0;i<Nmasks;i++)
			k += fabs((double) m_fval[i]);
		return(k);
	case MASKFUN_MAX:
		k = m_fval[0];
		for (i=1;i<Nmasks;i++)
			if (m_fval[i]>k)
				k = m_fval[i];
		return(k);
	case MASKFUN_MAXFLR:
		k = m_fval[0];
		for (i=1;i<Nmasks;i++)
			if (m_fval[i]>k)
				k=m_fval[i];
		if (k>0)
			return(k);
		return(0.);
	case MASKFUN_MXASFLR:
		k = fabs((double) m_fval[0])>fabs((double) m_fval[1])
			? fabs((double) m_fval[0]) : fabs((double) m_fval[1]);
		k -= fabs((double) m_fval[2]);
		if (k>0)
			return(k);
		return(0.);
	case MASKFUN_MUL:
		k = 1;
		for (i=0;i<Nmasks;i++)
			k *= m_fval[i]>0 ? m_fval[i] : 0;
		return(k);
	case MASKFUN_ORIENT:
		if (m_fval[0] == 0 && m_fval[1] == 0)
			return(0.);
		else
			return((float) (H_180_PI*atan2((double) m_fval[0],
				(double) m_fval[1])));
	default:
		return(perr(HE_MSKFUNSUBR,"h_mask_value_f",Maskfunc));
	}
}
