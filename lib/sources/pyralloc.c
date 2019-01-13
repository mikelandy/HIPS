/*
 * The pyramid utilities are derived from code originally written by
 * Raj Hingorani at SRI/David Sarnoff Research Institute.  The original
 * Gaussian and Laplacian pyramid algorithms were designed by Peter Burt (also
 * currently at SRI/DSRC).  See:  Computer Graphics and Image Processing,
 * Volume 16, pp. 20-51, 1981, and IEEE Transactions on Communications,
 * Volume COM-31, pp. 532-540, 1983.
 *
 * modified for HIPS 2 - msl - 1/3/91
 */

/*
 * pyralloc.c - image and pyramid allocation routines
 *
 * note that all images are described by number of rows and columns, but
 * are allocated larger on all borders by an amount `Image_border' which
 * must be set by the calling program.  The image pointer points to an array
 * of pointers to the image rows.  The image pointer actually points to the
 * pointer corresponding to the first real row after the top border, and every
 * row pointer actually points to the first real column after the left-hand
 * border.
 *
 * Also note, we reduce the images in the same way originally described
 * by Burt.  The original algorithm was meant to deal with images of size
 * (2**m)+1 (or some multiple of that), but the code will in fact work for
 * any size image.  If there are nr rows and nc columns in one level of the
 * pyramid, then the next level will have (nr+1)/2 rows and (nc+1)/2 columns,
 * where each figure is truncated to be an integer.  So, a 65x65 image is
 * subsampled horizontally as follows:
 *
 *		Sample locations:
 * Level 7 *
 * Level 6 *                                                               *
 * Level 5 *                               *                               *
 * Level 4 *               *               *               *               *
 * Level 3 *       *       *       *       *       *       *       *       *
 * Level 2 *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *
 * Level 1 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Level 0 *****************************************************************
 */

#include <hipl_format.h>

int def_ipyr(pyr,lev,nr,nc)

IPYR pyr;
int lev,nr,nc;

{
	int j;

	pyr[lev].nr = nr;
	pyr[lev].nc = nc;
	for (j=lev-1; j >= 0; j--) {
		pyr[j].nr = pyr[j+1].nr*2;
		pyr[j].nc = pyr[j+1].nc*2;
	}
	for (j=lev+1; MIN(pyr[j-1].nr,pyr[j-1].nc) > 1; j++) {
		pyr[j].nr = (pyr[j-1].nr + 1)/2;
		pyr[j].nc = (pyr[j-1].nc + 1)/2;
	}
	return(j-1);
}

int alloc_ipyr(pyr,botlev,toplev)

IPYR pyr;
int botlev,toplev;

{
	int i;

	for (i=botlev; i <= toplev ; i++) {
		if ((pyr[i].ptr = _alloc_iimage(pyr[i].nr,pyr[i].nc))
			== (int **) HIPS_ERROR)
				return(HIPS_ERROR);
	}
	return(HIPS_OK);
}

int free_ipyr(pyr,botlev,toplev)

IPYR pyr;
int botlev,toplev;

{
	int i;

	for (i=botlev; i <= toplev ; i++) {
		if (free_iimage(pyr[i]) == HIPS_ERROR)
			return(HIPS_ERROR);
	}
	return(HIPS_OK);
}

int def_fpyr(pyr,lev,nr,nc)

FPYR pyr;
int lev,nr,nc;

{
	int j;

	pyr[lev].nr = nr;
	pyr[lev].nc = nc;
	for (j=lev-1; j >= 0; j--) {
		pyr[j].nr = pyr[j+1].nr*2;
		pyr[j].nc = pyr[j+1].nc*2;
	}
	for (j=lev+1; MIN(pyr[j-1].nr,pyr[j-1].nc) > 1; j++) {
		pyr[j].nr = (pyr[j-1].nr + 1)/2;
		pyr[j].nc = (pyr[j-1].nc + 1)/2;
	}
	return(j-1);
}

int alloc_fpyr(pyr,botlev,toplev)

FPYR pyr;
int botlev,toplev;

{
	int i;

	for (i=botlev; i <= toplev ; i++) {
		if ((pyr[i].ptr = _alloc_fimage(pyr[i].nr,pyr[i].nc)) ==
			(float **) HIPS_ERROR)
				return(HIPS_ERROR);
	}
	return(HIPS_OK);
}

int free_fpyr(pyr,botlev,toplev)

FPYR pyr;
int botlev,toplev;

{
	int i;

	for (i=botlev; i <= toplev ; i++) {
		if (free_fimage(pyr[i]) == HIPS_ERROR)
			return(HIPS_ERROR);
	}
	return(HIPS_OK);
}

int alloc_iimage(img)

IIMAGE *img;

{
	if ((img->ptr = _alloc_iimage(img->nr,img->nc)) == (int **) HIPS_ERROR)
		return(HIPS_ERROR);
	return(HIPS_OK);
}

int alloc_fimage(img)

FIMAGE *img;

{
	if ((img->ptr = _alloc_fimage(img->nr,img->nc)) ==
		(float **) HIPS_ERROR)
			return(HIPS_ERROR);
	return(HIPS_OK);
}

int **_alloc_iimage(nr,nc)

int nr,nc;

{
	register int **ptr,**p,**fp,*area;
	int rsize=nc+2*Image_border;		/* full length of a row */

	if ((ptr = (int **) memalloc((nr+2*Image_border),sizeof(*ptr)))
		== (int **) HIPS_ERROR)
			return((int **) HIPS_ERROR);
	if ((area = (int *) halloc((nr+2*Image_border)*(nc+2*Image_border),
	    sizeof(*area))) == (int *) HIPS_ERROR)
		return((int **) HIPS_ERROR);
	for (p=ptr,fp=p+nr+2*Image_border,area+=Image_border;
		p < fp; p++,area += rsize)
			*p = area;
	return(ptr+Image_border);
}

float **_alloc_fimage(nr,nc)

int nr,nc;

{
	register float **ptr,**p,**fp,*area;
	int rsize=nc+2*Image_border;		/* full length of a row */

	if ((ptr = (float **) memalloc((nr+2*Image_border),sizeof(*ptr)))
		== (float **) HIPS_ERROR)
			return((float **) HIPS_ERROR);
	if ((area = (float *) halloc((nr+2*Image_border)*(nc+2*Image_border),
		sizeof(*area))) == (float *) HIPS_ERROR)
			return((float **) HIPS_ERROR);
	for (p=ptr,fp=p+nr+2*Image_border,area+=Image_border;
		p < fp; p++,area += rsize)
			*p = area;
	return(ptr+Image_border);
}

int free_iimage(img)

IIMAGE img;

{
	int _free_iimage();

	return(_free_iimage(img.ptr));
}

int free_fimage(img)

FIMAGE img;

{
	int _free_fimage();

	return(_free_fimage(img.ptr));
}

int _free_iimage(img)

int **img;

{
	/* if (! */ free(*(img-Image_border)-Image_border) /* )
		return(perr(HE_FREE)) */ ;
	/* if (! */ free(img-Image_border) /* )
		return(perr(HE_FREE)) */ ;
	return(HIPS_OK);
}

int _free_fimage(img)

float **img;

{
	/* NOTE:  System V free() returns void */
	/* if (! */ free(*(img-Image_border)-Image_border) /* )
		return(perr(HE_FREE)) */ ;
	/* if (! */ free(img-Image_border) /* )
		return(perr(HE_FREE)) */ ;
	return(HIPS_OK);
}
