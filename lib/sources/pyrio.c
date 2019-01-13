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
 * pyrio.c - image and pyramid I/O routines
 */

#include <stdio.h>
#include <hipl_format.h>

int _read_iimage(),_read_fimage(),_write_iimage(),_write_fimage();

int read_iimage(stream,img,fr,fname)

FILE *stream;
IIMAGE img;
int fr;
Filename fname;

{
	return(_read_iimage(stream,img.ptr,img.nr,img.nc,fr,fname));
}

int read_fimage(stream,img,fr,fname)

FILE *stream;
FIMAGE img;
int fr;
Filename fname;

{
	return(_read_fimage(stream,img.ptr,img.nr,img.nc,fr,fname));
}

int write_iimage(stream,img,fr)

FILE *stream;
IIMAGE img;
int fr;

{
	return(_write_iimage(stream,img.ptr,img.nr,img.nc,fr));
}

int write_fimage(stream,img,fr)

FILE *stream;
FIMAGE img;
int fr;

{
	return(_write_fimage(stream,img.ptr,img.nr,img.nc,fr));
}

int read_ipyr(stream,pyr,botlev,toplev,fr,fname)

FILE *stream;
IPYR pyr;
int botlev,toplev,fr;
Filename fname;

{
	int i;

	for (i=botlev; i <= toplev ; i++) {
		if (_read_iimage(stream,pyr[i].ptr,pyr[i].nr,pyr[i].nc,
			fr,fname) == HIPS_ERROR)
				return(HIPS_ERROR);
	}
	return(HIPS_OK);
}

int read_fpyr(stream,pyr,botlev,toplev,fr,fname)

FILE *stream;
FPYR pyr;
int botlev,toplev,fr;
Filename fname;

{
	int i;

	for (i=botlev; i <= toplev ; i++) {
		if (_read_fimage(stream,pyr[i].ptr,pyr[i].nr,pyr[i].nc,
			fr,fname) == HIPS_ERROR)
				return(HIPS_ERROR);
	}
	return(HIPS_OK);
}

int write_ipyr(stream,pyr,botlev,toplev,fr)

FILE *stream;
IPYR pyr;
int botlev,toplev,fr;

{
	int i;

	for (i=botlev; i <= toplev ; i++) {
		if (_write_iimage(stream,pyr[i].ptr,pyr[i].nr,pyr[i].nc,fr)
			== HIPS_ERROR)
				return(HIPS_ERROR);
	}
	return(HIPS_OK);
}

int write_fpyr(stream,pyr,botlev,toplev,fr)

FILE *stream;
FPYR pyr;
int botlev,toplev,fr;

{
	int i;

	for (i=botlev; i <= toplev ; i++) {
		if (_write_fimage(stream,pyr[i].ptr,pyr[i].nr,pyr[i].nc,fr)
			== HIPS_ERROR)
				return(HIPS_ERROR);
	}
	return(HIPS_OK);
}

int _read_iimage(stream,img,nr,nc,fr,fname)

FILE *stream;
int nr,nc,**img,fr;
Filename fname;

{
	register int **p,**fp;

	for (p=img, fp=p+nr; p < fp; p++)
		if (fread(*p,sizeof(**p),nc,stream) != nc)
			return(perr(HE_READFRFILE,fr,fname));
	return(HIPS_OK);
}

int _read_fimage(stream,img,nr,nc,fr,fname)

FILE *stream;
int nr,nc,fr;
float **img;
Filename fname;

{
	register float **p,**fp;

	for (p=img, fp=p+nr; p < fp; p++)
		if (fread(*p,sizeof(**p),nc,stream) != nc)
			return(perr(HE_READFRFILE,fr,fname));
	return(HIPS_OK);
}

int _write_iimage(stream,p,nr,nc,fr)

FILE *stream;
register int **p;
int nr,nc,fr;

{
	register int **fp;

	for (fp=p+nr; p < fp; p++)
		if (fwrite(*p,sizeof(**p),nc,stream) == 0)
			return(perr(HE_WRITEFR,fr));
	return(HIPS_OK);
}

int _write_fimage(stream,p,nr,nc,fr)

FILE *stream;
register float **p;
int nr,nc,fr;

{
	register float **fp;

	for (fp=p+nr; p < fp; p++)
		if (fwrite(*p,sizeof(**p),nc,stream) == 0)
			return(perr(HE_WRITEFR,fr));
	return(HIPS_OK);
}
