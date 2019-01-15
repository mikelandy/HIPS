/*	INV-VFFT . C
#
%	Copyright (c)	Jin, Guojun - All rights reserved
%
%	Inverse Virtual Fast Fourier Transform
%
%	Only one write_header() between header processing and inv_vfft.
%
% AUTHOR:	Jin Guojun - LBL	5/10/91
*/
char	usage[]="options\n\
-B	output BYTE format. Default is float.\n\
-D	2D test	\n\
-cut	cut both ends and output BYTE image\n\
[<] VFFT [> Image]\n";

#include "complex.h"
#include "header.def"
#include "imagedef.h"

U_IMAGE	uimg;

bool	cut;
MType	dimen1len, dimen2size, fsize, vsize;

#define	row	uimg.height
#define	cln	uimg.width
#define	frm	uimg.frames


main(argc, argv)
int	argc;
char*	argv[];
{
bool	dimens=0, vflag;
int	i, j, f, hrows, hcols, logfrms, logrows, logcols;

format_init(&uimg, IMAGE_INIT_TYPE, HIPS, -1, *argv, "D12-4");
uimg.o_form = IFMT_FLOAT;

for (i=1;i<argc;i++) {
    if (argv[i][0] == '-'){
	switch(argv[i][1]) {
	case 'c':	cut++;
	case 'B':	uimg.o_form = IFMT_BYTE;	break;
	case 'D':	dimens++;	break;
	default:
info:		usage_n_options(usage, i, argv[i]);
	}
    }
    else if (freopen(argv[i], "r", stdin) == NULL)
	syserr("can't open %s as input", argv[i]);
}

io_test(stdin_fd, goto	info);

(*uimg.header_handle)(HEADER_READ, &uimg, 0, 0);

vflag = uimg.in_form==IFMT_DVVFFT3D || uimg.in_form==IFMT_VVFFT3D;
if (uimg.o_form==IFMT_BYTE)
	uimg.pxl_out = 1;
else	uimg.pxl_out = 4;

hrows = row >> 1;
hcols = cln >> 1;
dimen1len = hcols+1;
vsize = row * dimen1len;
fsize = row * cln;
if (!vflag)	/* vvfft must be in 3D. */
	dimens |= !(uimg.in_form & 1);

logfrms=logrows=logcols = -1;
for (i=0,j=1;i<12;i++,j+=j) {
	if (j==row)	logrows=i;
	if (j==cln)	logcols=i;
	if (j==frm)	logfrms=i;
}

(*uimg.header_handle)(HEADER_WRITE, &uimg, argc, argv, True);

if (logfrms<0 || logrows<0 || logcols<0)	{
	mesg("not power of 2, be patient for slow processing\n");

#	include	"inrlvfft.cxx"
	exit(0);
}

if (uimg.in_form==IFMT_DVFFT3D || uimg.in_form==IFMT_DVFFT2D)	{
DBCOMPLEX	*c_out = (DBCOMPLEX *)NZALLOC(fsize, sizeof(*c_out), "ibuf"),
	*c_in = !dimens ? (DBCOMPLEX *)nzalloc(frm*vsize, sizeof(*c_in), "obuf")
		: c_out,
	*cvt;

	dw_init(hcols, MAX(hrows, 1<<logfrms-1));

	if (!dimens)	{
	register DBCOMPLEX*	fop = c_in;
		dw_load(1<<logfrms);
		if (vflag){
		double	*re_i = (double *) c_out, *im_i = re_i + vsize;
		    for (f=0; f<frm; f++)	{
			upread(c_out, sizeof(*c_out), vsize, stdin);
			for (i=vsize; i--;)
				fop[i].re = re_i[i],
				fop[i].im = -im_i[i];
		    }
		}
		else	{
			i = upread(fop, sizeof(*fop), vsize * frm, stdin);
			if (i != vsize * frm)
				syserr("f%d [%d] read %d", f, vsize<<3, i);
			while (i--)
				fop[i].im = -fop[i].im;
		}
		for (i=0; i<vsize; i++)
			dvfftn(c_in+i, logfrms, vsize, c_in+i, vsize);
	}

	for (f=0; f<frm; f++)	{
		if (dimens)	{
		    for (cvt=c_out, i=0; i<row; i++, cvt+=cln)	{
			upread(cvt, sizeof(*cvt), dimen1len, stdin);
			for (j=dimen1len; j--;)
				cvt[j].im = -cvt[j].im;
		    }
		} else
#ifdef	REGULAR_VFFT
		{
		register DBCOMPLEX	*temp = c_in + f * vsize;
			for (cvt=c_out, i=0; i<row; i++){
				memcpy(cvt, temp, sizeof(*cvt)*dimen1len);
				temp += dimen1len;
				cvt += cln;
			}
		}
		dvrft_2d(c_out, logrows, logcols, NULL);
#else
			cvt = c_in + f * vsize;
		dvrft_2d(cvt, logrows, logcols, c_out);
#endif

		if (uimg.o_form == IFMT_BYTE)
		dtob(c_out, c_in, fsize, 1./((double)fsize*(uimg.in_form&1?frm:1)));
		else	{
		register double	total = 1. / (fsize*(uimg.in_form&1?frm:1));
		register float	*op = (float*) c_in;
		    for (cvt=c_out, i=0; i<fsize; i++)
			op[i] = cvt[i].re * total;
		}
		fwrite(c_in, uimg.pxl_out, fsize, out_fp);
	}
}
else	{
COMPLEX	*c_out = (COMPLEX *)NZALLOC(fsize, sizeof(*c_out), "ibuf"),
	*c_in = !dimens ? (COMPLEX *)nzalloc(frm*vsize, sizeof(*c_in), "obuf")
		: c_out,
	*cvt;
	w_init(hcols, MAX(hrows, 1<<logfrms-1));

	if (!dimens)	{
	register COMPLEX*	fop = c_in;
		w_load(1<<logfrms);
		if (vflag)	{
		float	*re_i = (float *) c_out, *im_i = re_i + vsize;
		    for (f=0; f<frm; f++, fop+=vsize)	{
			upread(c_out, sizeof(*c_out), vsize, stdin);
			for (i=vsize; i--;)
				fop[i].re = re_i[i],
				fop[i].im = -im_i[i];
		    }
		}
		else	{
			i = upread(fop, sizeof(*fop), vsize * frm, stdin);
			if (i != vsize * frm)
				syserr("f%d [%d] read %d", f, vsize<<3, i);
			while (i--)
				fop[i].im = -fop[i].im;
		}
		for (i=0; i<vsize; i++)
			vfftn(c_in+i, logfrms, vsize, c_in+i, vsize);
	}

	for (f=0; f<frm; f++)	{
		if (dimens)	{	/* c_in == c_out	*/
		    for (i=0; i<row; i++, cvt+=cln)	{
			upread(cvt, sizeof(*cvt), dimen1len, stdin);
			for (j=dimen1len; j--;)
				cvt[j].im = -cvt[j].im;
		    }
		} else
#ifdef	REGULAR_VFFT
		{
		register COMPLEX	*temp = c_in + f * vsize;
			for (cvt=c_out, i=0; i<row; i++)	{
				memcpy(cvt, temp, sizeof(*cvt)*dimen1len);
				temp += dimen1len;
				cvt += cln;
			}
		}
		vrft_2d(c_out, logrows, logcols, NULL);
#else
			cvt = c_in + f * vsize;
		vrft_2d(cvt, logrows, logcols, c_out);
#endif

		if (uimg.o_form == IFMT_BYTE)
		ftob(c_out, c_in, fsize, 1./(fsize*(uimg.in_form&1?frm:1)));
		else	{
		register float	total = 1. / (fsize*(uimg.in_form&1?frm:1)),
			*tmp = (float *) c_in;
		    for (i=0; i<fsize; i++)
			tmp[i] = c_out[i].re * total;
		}
		fwrite(c_in, uimg.pxl_out, fsize, out_fp);
	}
}
}

#define	dtx_clip	for (;n--; obp++)	{	\
	register int	tmp = icp->re * scale;	icp++;	\
	if (tmp < 0)	\
		*obp = 0;	\
	else if (tmp>255)	\
		*obp = 255;	\
	else	*obp = tmp;	}

ftob(icp, obp, n, scale)
register COMPLEX*	icp;
register byte	*obp;
register int	n;
register float	scale;
{
#ifdef	_DEBUG_
message("%d FtoB, scale=%f\n", n, scale);
#endif
if (cut) while (n--)
	obp[n] = icp[n].re * scale;
else	dtx_clip
}

dtob(icp, obp, n, scale)
register DBCOMPLEX	*icp;
register byte	*obp;
register int	n;
register double	scale;
{
#ifdef	_DEBUG_
message("%d DtoB, scale=%lf\n", n, scale);
#endif
	dtx_clip
}

dtof(icp, obp, n, scale)	/* for non power of 2 processing */
register DBCOMPLEX	*icp;
register float	*obp;
register int	n;
register double	scale;
{
#ifdef	_DEBUG_
message("%d DtoF, scale=%lf\n", n, scale);
#endif
while (n--)
	*obp++ = icp->re * scale,	icp++;
}
