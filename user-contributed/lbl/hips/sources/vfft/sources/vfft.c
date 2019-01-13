/*	VFFT .C
#
%	Virtual-Very Fast Fourier Transform
%
%	Copyright (c)	Jin, Guojun -	All Rights Reserved
%
%	Every block has its own write_header() before its processing.
%
% AUTHOR:	Jin Guojun - LBL	5/10/91
% Last Modified Date:	12/5/94
*/

#include "complex.h"
#include "header.def"
#include "imagedef.h"

arg_fmt_list_string	arg_fmt[] =	{
	{"-D2", "%b", True, 1, 0, "\t2-dimension VFFT."},
	{"-VV", "%b", True, 1, 0,
		"\treal and imaginary are in separated planes"},
	{"-d[ouble]", "%b %b", True, 1, 0, "\boutput double VFFT"},
	{"	[<] image [> VFFT]", "0", 0, 0, 0, "End of Help"},
	NULL	};

U_IMAGE	uimg;

MType	dimen1len, dimen2size, fsize, vsize;

#define	row	uimg.height
#define	cln	uimg.width
#define	frm	uimg.frames


main(argc, argv)
int	argc;
char*	argv[];
{
bool	dflag=0, dimens=0, vflag=0, vfrms=0;
char	**fl;
int	i, j, f, hrows, hcols, logfrms, logrows, logcols;

format_init(&uimg, IMAGE_INIT_TYPE, HIPS, -1, *argv, "S20-1");

if ((i=parse_argus(&fl, argc, argv, arg_fmt,
	&dimens, &vflag, &dflag, &dflag)) < 0)	exit(i);
if (i && (in_fp=freopen(fl[0], "r", stdin)) == NULL)
	syserr("can't open %s as input", fl[0]);

io_test(fileno(in_fp), exit('i'));
#ifdef	_DEBUG_
INIT_PERFORM_TIMING();
#endif

if ((*uimg.header_handle)(HEADER_READ, &uimg, 0, 0) < 0)	exit('t');

if (uimg.in_form==IFMT_DOUBLE || uimg.in_form==IFMT_DBLCOM)
	dflag++;
if (uimg.in_form > IFMT_DBLCOM || uimg.in_form==IFMT_ASCII)
	syserr("undesired input image format");

if (frm < 2) {
	vflag = 0;
	dimens = 1;
}
if (vflag)	dimens=0;
fsize = row * cln;
hrows = row >> 1;
hcols = cln >> 1;
dimen1len = hcols + 1;
vsize = row * dimen1len;

logfrms=logrows=logcols= -1;
for (i=0,j=1;i<12;i++,j+=j) {
	if (j==row)	logrows=i;
	if (j==cln)	logcols=i;
	if (j==frm)	logfrms=i;
}

if (logrows == -1 || logcols == -1)	{
mesg("be patient for non power of 2 processing\n");
#include "realvfft.cxx"
exit(0);
}

if (logfrms == -1 && !dimens)	{
	for (logfrms=1, i=frm-1; i>>=1; logfrms++);
	vfrms = (1 << logfrms) - frm;
	frm += vfrms;	/* fake empty frames	*/
}

if (dflag)	{
DBCOMPLEX	*c_in = (DBCOMPLEX*)NZALLOC(fsize, sizeof(*c_in), "ibuf"),
	*c_out, *cvt;

	if (!dimens)	{
		c_out = (DBCOMPLEX*)nzalloc(frm*vsize, sizeof(*c_out), "obuf");
		cvt = c_in;
	}
	else	cvt = c_out = (DBCOMPLEX*)NZALLOC(fsize, sizeof(*cvt), "cvt");

	if (vflag)
		uimg.o_form = IFMT_DVVFFT3D;
	else	uimg.o_form = IFMT_DVFFT3D + dimens;
	uimg.pxl_out=16;
	(*uimg.header_handle)(HEADER_WRITE, &uimg, argc, argv, True);
	dw_init(hcols, MAX(hrows, 1<<logfrms-1));

	for (f=0; f<frm-vfrms; f++)	{
		upread(c_in, uimg.pxl_in, fsize, in_fp);
		switch (uimg.in_form)	{
		case IFMT_BYTE:	btodc(c_in, fsize);	break;
		case IFMT_SHORT:stodc(c_in, fsize);	break;
		case IFMT_LONG:	itodc(c_in, fsize);	break;
		case IFMT_FLOAT:ftodc(c_in, fsize);
		}

		dvfft_2d(c_in, logrows, logcols,
#ifdef	REGULAR_VFFT
			NULL);
#else
			c_out + (dimens ? 0 : f*vsize));
#endif

		if (dimens){	/* finish in 2D	*/
#ifdef	REGULAR_VFFT
			DBCOMPLEX	*temp = c_in;
			for (i=row; i--; temp += cln)
			    fwrite(temp, sizeof(*temp), dimen1len, out_fp);
#else
			fwrite(cvt, sizeof(*cvt), vsize, out_fp);
#endif
		}
#ifdef	REGULAR_VFFT
		else	{	/* prepare for 3D	*/
		register DBCOMPLEX	*c_c = c_out + f*vsize, *c_cin = c_in;
			for (i=0; i<row; i++)	{
#if	(POINTER_BITS > INT_BITS) || defined USE_MEMCPY
				memcpy(c_c, c_cin, dimen1len * sizeof(*c_c));
				c_c += dimen1len;
				c_cin += cln;
#else
			    for (j=dimen1len; j--;)
				*c_c++ = *c_cin++;
			    c_cin += hcols-1;
#endif
			}
		}
#endif
	}
	if (!dimens)	{
	    dw_load(1<<logfrms);	/* 3rd dimension VFFT	*/
	    if (vfrms)
		bzero(c_out + (frm-vfrms) * vsize, vfrms * vsize * uimg.pxl_out);
	    for (i=0, cvt=c_out; i<vsize; i++)
		dvfftn(c_out+i, logfrms, vsize, c_out+i, vsize);
	    if (vflag)	{
		double	*re = c_in, *im = re + vsize;
		for (f=0; f<frm; f++){
			for (i=vsize; i--;)
				re[i] = cvt[i].re,	im[i] = cvt[i].im;
			fwrite(c_in, sizeof(*c_in), vsize, out_fp);
		}
	    } else for (f=0; f<frm; f++)	{
		i = fwrite(cvt, sizeof(*cvt), vsize, out_fp);
		if (i != vsize)
			syserr("f%d [%d] write %d", f, vsize, i);
		cvt += vsize;
	    }
	}
}
else	{
COMPLEX	*c_in = (COMPLEX*)NZALLOC(fsize, sizeof(*c_in), "ibuf"),
	*c_out, *cvt;

	if (!dimens)	{
		c_out = (COMPLEX*)nzalloc(frm*vsize, sizeof(*c_out), "obuf");
		cvt = c_in;
	}
	else	cvt = c_out = (COMPLEX*)NZALLOC(fsize, sizeof(*cvt), "cvt");

	if (vflag)
		uimg.o_form = IFMT_VVFFT3D;
	else	uimg.o_form = IFMT_VFFT3D + dimens;
	uimg.pxl_out = 8;
	(*uimg.header_handle)(HEADER_WRITE, &uimg, argc, argv, True);
	w_init(hcols, MAX(hrows, 1<<logfrms-1));

	for (f=0; f<frm-vfrms; f++)	{
		upread(c_in, uimg.pxl_in, fsize, in_fp);
		switch(uimg.in_form){
		case IFMT_BYTE:	btoc(c_in, fsize);	break;
		case IFMT_SHORT:stoc(c_in, fsize);	break;
		case IFMT_LONG:	itoc(c_in, fsize);	break;
		default:{	float	*fp = (float*)c_in;
			for (i=fsize; i--;)
				c_in[i].re = fp[i],	c_in[i].im = 0.;
			}
		}

		vfft_2d(c_in, logrows, logcols,
#ifdef	REGULAR_VFFT
			NULL);
#else
			c_out + (dimens ? 0 : f*vsize));
#endif

		if (dimens){	/* finish in 2D	*/
#ifdef	REGULAR_VFFT
			COMPLEX	*temp = c_in;
			for (i=row; i--; temp += cln)
			    fwrite(temp, sizeof(*temp), dimen1len, out_fp);
#else
			fwrite(cvt, sizeof(*cvt), vsize, out_fp);
#endif
		}
#ifdef	REGULAR_VFFT
		else	{	/* prepare for 3D	*/
		register COMPLEX	*c_c = c_out + f*vsize, *c_cin = c_in;
			for (i=0; i<row; i++)	{
#if	(POINTER_BITS > INT_BITS) || defined USE_MEMCPY
				memcpy(c_c, c_cin, dimen1len * sizeof(*c_c));
				c_c += dimen1len;
				c_cin += cln;
#else
			    for (j=dimen1len; j--;)
				*c_c++ = *c_cin++;
			    c_cin += hcols-1;
#endif
			}
		}
#endif
	}
	if (!dimens)	{
	    w_load(1<<logfrms);	/* 3rd dimension VFFT	*/
		if (vfrms)
		bzero(c_out + (frm-vfrms) * vsize, vfrms * vsize * uimg.pxl_out);
	    for (i=0, cvt=c_out; i<vsize; i++)
		vfftn(c_out+i, logfrms, vsize, c_out+i, vsize);
	    if (vflag)	{
		float	*re = c_in, *im = re + vsize;
		for (f=0; f<frm; f++){
			for (i=vsize; i--;)
				re[i] = cvt[i].re,	im[i] = cvt[i].im;
			fwrite(c_in, sizeof(*c_in), vsize, out_fp);
		}
	    } else for (f=0; f<frm; f++)	{
		i = fwrite(cvt, sizeof(*cvt), vsize, out_fp);
		if (i != vsize)
			syserr("f%d [%d] write %d", f, vsize, i);
		cvt += vsize;
	    }
	}
}
}

#define	AtoC(toname, it, ot)	\
toname(buf, n)	\
register it	*buf;	\
register int	n;	\
{	register ot	*cplx = (ot *)buf + n;	\
	while (n--)	(--cplx)->re = buf[n],	cplx->im = 0.;	}

AtoC(btoc, byte, COMPLEX)

AtoC(stoc, short, COMPLEX)

AtoC(itoc, int, COMPLEX)

AtoC(btodc, byte, DBCOMPLEX)

AtoC(stodc, short, DBCOMPLEX)

AtoC(itodc, int, DBCOMPLEX)

AtoC(ftodc, float, DBCOMPLEX)

