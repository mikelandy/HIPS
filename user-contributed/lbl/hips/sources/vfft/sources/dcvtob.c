/*	DCVtoB . C
#
%	Copyright (c)	Jin, Guojun - All Rights Reserved
%
%	This program convert following format image to Byte image:
%
%		float complex
%		double, double complex
%		float vfft 2D, float vfft 3D
%		double vfft 2D, double vfft 3D
%
%	Also	converting VFFT and double VFFT to corresponding complex FFT.
%
% compile:
%	cc -O -o $(DEST)/dcvtob dcvtob.c -lscs1 -lccs -lhips -lrle -ltiff -lm
%
% AUTHOR:	Guojun Jin - LBL	5/1/91
*/

#include <math.h>
#include "complex.h"
#include "header.def"
#include "imagedef.h"

#ifndef	MIN_DOUBLE
#define	MIN_DOUBLE	1.7e-308
#endif

arg_fmt_list_string	arg_fmt[] =	{
	{"-C", "%b", True, 1, 1, "VFFT to Complex (FFT)"},
	{"-F", "%b", sizeof(float), 1, 0,
		"output Float format instead of Bytes"},
	{"-di", "%D", 1.0, 1, 1,
		"maximum input value divident for auto scale.\n\
			The large value results a bright picture"},
	{"-ff", "%+ %b", True, 2, 0,
		"\bdo fast quad flip on regular complex format for view only"},
	{"-fl", "%b", True, 1, 0,
		"do regular quadrant flip on regular complex format"},
	{"-m", "%D", 0., 1, 1, "specify the maximum input value(range)"},
	{"-n", "%D", 1., 1, 0, "output non scaled conversion (%lf)"},
	{"-r", "%b", False, 1, 1,
		"output original VFFT spectrum, not quad flipped"},
	{"-s", "%b", True, 1, 1, "re-scaling for each frame	\n\
			(regular scale calculation done at first frame)"},
	{"-v", "%b", True, 1, 1, "verbose"},
	{"	[<] image [> output]", "0", 0, 0, 0, "End of Help"},
	NULL	};

U_IMAGE	uimg;

#define	inbuf	uimg.src
#define	obuf	uimg.dest
#define	frm	uimg.frames
#define	cln	uimg.width
#define	row	uimg.height

bool	verbose, spectrum, fflip, flip, selfscale, v2dtoc, v3d;
int	dimen1len, vfsize;


main(argc, argv)
int	argc;
char*	argv[];
{
int	i, f, fsize;
double	*dbuf, *vbuf, *cvt, maxi, scale=0., divident;
char	**fl;

format_init(&uimg, IMAGE_INIT_TYPE, HIPS, -1, *argv, "N30-4");
uimg.pxl_out = 1;

if ((i=parse_argus(&fl, argc, argv, arg_fmt,
	&v2dtoc, &uimg.pxl_out, &divident, &flip, &fflip, &flip, &maxi,
	&scale, &spectrum, &selfscale, &verbose)) < 0)	exit(i);
if (i && (in_fp=freopen(fl[0], "rb", stdin)) == NULL)
	prgmerr(1, "can not open input %s", fl[0]);
#ifdef	_DEBUG_
message("fl=%d, ffl=%d, maxi=%lf, scale=%lf\n", flip, fflip, maxi, scale);
#endif
io_test(fileno(in_fp), prgmerr(1, "needs input"));

(*uimg.header_handle)(HEADER_READ, &uimg, 0, 0);

if (uimg.in_form<IFMT_COMPLEX || uimg.in_form>IFMT_DBLCOM  &&
	(uimg.in_form < IFMT_VFFT3D || uimg.in_form > IFMT_DVVFFT3D))
	syserr("not in double, complex or vfft format");

if (uimg.pxl_out == 1)
	uimg.o_form = IFMT_BYTE;
else	uimg.o_form = IFMT_FLOAT;
if (v2dtoc)
    if (uimg.in_form < IFMT_VFFT3D && uimg.in_form > IFMT_DVFFT2D)
	v2dtoc = False;
    else	{
	v3d = uimg.in_form==IFMT_VFFT3D || uimg.in_form==IFMT_DVFFT3D;
#ifndef	ESPIPE
#define	ESPIPE	29
#endif
	if (v3d && fseek(in_fp, 0L, SEEK_CUR) < 0 && errno == ESPIPE)
		prgmerr('p', "no piping on 3D vfftofft");
	uimg.pxl_out = 8;
	if (uimg.in_form >= IFMT_DVFFT3D)	{
		uimg.pxl_out <<= 1;
		uimg.o_form = IFMT_DBLCOM;
	} else	uimg.o_form = IFMT_COMPLEX;
	msg("VFFT to Complex FFT (%d)\n", uimg.pxl_out);
    }
dimen1len = (cln>>1) + 1;
fsize = row * cln;
if (uimg.in_form > IFMT_DBLCOM)	/* input is VFFT's	*/
	vfsize = row * dimen1len;
else	vfsize = fsize;

dbuf = NZALLOC(vfsize, uimg.pxl_in, "dbuf");
obuf = NZALLOC(fsize, uimg.pxl_out, "obuf");
if (vfsize != fsize || flip && uimg.in_form < IFMT_VFFT3D)
	vbuf = NZALLOC(fsize, sizeof(*vbuf), "vbuf");

(*uimg.header_handle)(HEADER_WRITE, &uimg, argc, argv, True);

if (v2dtoc)	{	/* VFFT to complex. if 3D & odd frames, then no f_mid */
char*	Rbuf = v3d ? NZALLOC(vfsize, uimg.pxl_in, "Rbuf") : NULL;
int	r_len = row+1>>1, c_len = cln+1>>1, f_mid = frm&1 ? 0 : frm>>1;
	if (!scale)
		scale = 1. / (fsize * (uimg.in_form & 1 ? frm : 1));
    for (f=0; f<frm; f++)	{

	i=upread(dbuf, uimg.pxl_in, vfsize, in_fp);
	if (i != vfsize)
		prgmerr('v', "[%d] read %d", vfsize, i);
	switch (uimg.in_form)	{
	case	IFMT_VFFT2D:
		v2dtofft(dbuf, obuf, r_len, c_len, (float)scale);
		break;
	case	IFMT_VFFT3D:
		if (!f || f == f_mid)
			v2dtofft(dbuf, obuf, r_len, c_len, (float)scale);
		else	v3dtofft(dbuf, obuf, r_len, c_len, (float)scale,Rbuf,f);
		break;
	case	IFMT_DVFFT2D:
		dv2dtofft(dbuf, obuf, r_len, c_len, scale);
		break;
	case	IFMT_DVFFT3D:
		if (!f || f == f_mid)
			dv2dtofft(dbuf, obuf, r_len, c_len, scale);
		else	dv3dtofft(dbuf, obuf, r_len, c_len, scale, Rbuf, f);
	}
	if ((i=fwrite(obuf, uimg.pxl_out, fsize, out_fp)) != fsize)
		syserr("[%d] write %d", fsize, i);
    }
} else	for (f=0; f<frm; f++){
	register double	*dp;

	cvt = dbuf;
	if ((i=upread(dbuf, uimg.pxl_in, vfsize, in_fp)) != vfsize)
		syserr("[%d] Read %d", vfsize, i);
	if (uimg.in_form==IFMT_COMPLEX)
		complex_to_d(dbuf, vfsize);
	else if (uimg.in_form == IFMT_DBLCOM)
		dcomplex_to_d(dbuf, vfsize);
	else if (uimg.in_form > IFMT_DBLCOM)
		vfft_handle(uimg.in_form, dbuf, cvt=vbuf, row, cln, dimen1len);
	if (uimg.in_form < IFMT_VFFT3D)
	    if (fflip)
		Dcom_fflip(dbuf, cvt=vbuf, row, cln);
	    else if (flip)
		Dcom_flip(dbuf, cvt=vbuf, row, cln);

	dp = cvt;

	if (uimg.o_form == IFMT_BYTE){
	register byte	*bp = obuf;
	double	imin;

	    if (scale != 1. && f==0 || selfscale || scale > 1.7e308)	{
	    register double	min=1.7e308, max=MIN_DOUBLE, value;
		for (i=0; i<fsize; i++){
			value = dp[i];
			if (value < min)	min = value;
			else if (value > max)	max = value;
		}
		if (verbose || uimg.in_form > IFMT_DBLCOM)
		    message("%s: min_in=%f, max_in=%f\n", Progname, min, max);
		if (maxi)
			max = maxi;
		else	max /= divident;
		scale = 255. / (max - min);
		imin = min;
		if (verbose || uimg.in_form > IFMT_DBLCOM)
		    message("%s: max_in set to=%4.4f, scale=%f\n",
			Progname, max, scale);
	    }
	    {
	    register double	bscale = scale, min = imin;
		if (bscale > 1.7e308)
			bscale = 1.;
		for (i=0; i<fsize; i++){
		register int tmp = bscale * (dp[i] - min);
			if (tmp > 255)
				bp[i] = 255;
			else	bp[i] = tmp;
		}
	    }
	}
	else{
	register float	*fp = obuf;
	    if (uimg.in_form > IFMT_DBLCOM){
	    register double	fscale = 1. / fsize;
		for (i=0; i<fsize; i++)
			*fp++ = fscale * *dp++;
	    }
	    else for (i=0; i<fsize; i++)
		*fp++ = *dp++;
	}
	if ((i=fwrite(obuf, uimg.pxl_out, fsize, out_fp)) != fsize)
		syserr("[%d] write %d", fsize, i);
}
}


v3dtofft(c_in, cp, r_len, c_len, vscale, rbuf, f)
register COMPLEX	*c_in, *cp, *rbuf;
register float	vscale;
{
int	i;
long	fpos = ftell(in_fp);
	if (fseek(in_fp, -f * vfsize * uimg.pxl_in, SEEK_END) < 0)
		return	EOF;
	if ((i=upread(rbuf, uimg.pxl_in, vfsize, in_fp)) != vfsize)
		syserr("v3d [%d] Read %d", vfsize, i);
	fseek(in_fp, fpos, SEEK_SET);	/* set file pos. back to end of f */

    cp->re = c_in->re * vscale;
    cp->im = c_in->im * vscale;
    for (i=1; i<c_len; i++)	{
	cp[i].re = c_in[i].re * vscale;
	cp[i].im = c_in[i].im * vscale;
	cp[cln-i].re = rbuf[i].re * vscale;
	cp[cln-i].im = -rbuf[i].im * vscale;
    }
    if ((cln & 1)==0){
	cp[i].re = c_in[i].re * vscale;
	cp[i].im = c_in[i].im * vscale;
    }

    for (i=1; i<r_len; i++) {	/* dis for conjugate symmetry	*/
	register int	j, dis=(row-(i<<1))*dimen1len,
			disb=cln*(row-(i<<1)), dise=disb+cln;
	cp += cln;
	c_in += dimen1len;
	rbuf += dimen1len;
	cp->re = c_in[0].re * vscale;	/* 1st column in row[i]	*/
	cp->im = c_in[0].im * vscale;
	cp[disb].re = c_in[dis].re * vscale;	/* 1st column in row[d]	*/
	cp[disb].im = c_in[dis].im * vscale;
	for (j=1; j<c_len;j++)	{
			/* down-right <--> up-left	*/
		cp[j].re = c_in[j].re * vscale;
		cp[j].im = c_in[j].im * vscale;
		cp[dise-j].re = rbuf[j].re * vscale;
		cp[dise-j].im = - rbuf[j].im * vscale;
			/* up-right <--> down-left	*/
		cp[disb+j].re = c_in[dis+j].re * vscale;
		cp[disb+j].im = c_in[dis+j].im * vscale;
		cp[cln-j].re = rbuf[dis+j].re * vscale;
		cp[cln-j].im = -rbuf[dis+j].im * vscale;
	}
	if ((cln & 1)==0) {	/* neutral column	*/
		cp[j].re = c_in[j].re * vscale;
		cp[j].im = c_in[j].im * vscale;
		cp[disb+j].re = c_in[dis+j].re * vscale;
		cp[disb+j].im = c_in[dis+j].im * vscale;
	}
    }
    if ((row & 1)==0)	{
	cp += cln;
	c_in += dimen1len;
	cp->re = c_in->re * vscale;
	cp->im = c_in->im * vscale;
	for (i=1; i<c_len; i++)	{
		cp[i].re = c_in[i].re * vscale;
		cp[i].im = c_in[i].im * vscale;
		cp[cln-i].re = rbuf[i].re * vscale;
		cp[cln-i].im = -rbuf[i].im * vscale;
	}
	if ((cln & 1)==0){
		cp[i].re = c_in[i].re * vscale;
		cp[i].im = c_in[i].im * vscale;
	}
    }
}


dv3dtofft(c_in, cp, r_len, c_len, vscale, rbuf, f)
register DBCOMPLEX	*c_in, *cp, *rbuf;
register float	vscale;
{
int	i;
long	fpos = ftell(in_fp);
	if (fseek(in_fp, -f * vfsize * uimg.pxl_in, SEEK_END) < 0)
		return	EOF;
	if ((i=upread(rbuf, uimg.pxl_in, vfsize, in_fp)) != vfsize)
		syserr("dv3d [%d] Read %d", vfsize, i);
	fseek(in_fp, fpos, SEEK_SET);

    cp->re = c_in->re * vscale;
    cp->im = c_in->im * vscale;
    for (i=1; i<c_len; i++)	{
	cp[i].re = c_in[i].re * vscale;
	cp[i].im = c_in[i].im * vscale;
	cp[cln-i].re = rbuf[i].re * vscale;
	cp[cln-i].im = -rbuf[i].im * vscale;
    }
    if ((cln & 1)==0){
	cp[i].re = c_in[i].re * vscale;
	cp[i].im = c_in[i].im * vscale;
    }

    for (i=1; i<r_len; i++) {
	register int	j, dis=(row-(i<<1))*dimen1len,
			disb=cln*(row-(i<<1)), dise=disb+cln;
	cp += cln;
	c_in += dimen1len;
	rbuf += dimen1len;
	cp->re = c_in[0].re * vscale;
	cp->im = c_in[0].im * vscale;
	cp[disb].re = c_in[dis].re * vscale;
	cp[disb].im = c_in[dis].im * vscale;
	for (j=1; j<c_len;j++)	{
			/* d-r <--> u-l	*/
		cp[j].re = c_in[j].re * vscale;
		cp[j].im = c_in[j].im * vscale;
		cp[dise-j].re = rbuf[j].re * vscale;
		cp[dise-j].im = - rbuf[j].im * vscale;
			/* u-r <--> d-l	*/
		cp[disb+j].re = c_in[dis+j].re * vscale;
		cp[disb+j].im = c_in[dis+j].im * vscale;
		cp[cln-j].re = rbuf[dis+j].re * vscale;
		cp[cln-j].im = -rbuf[dis+j].im * vscale;
	}
	if ((cln & 1)==0) {
		cp[j].re = c_in[j].re * vscale;
		cp[j].im = c_in[j].im * vscale;
		cp[disb+j].re = c_in[dis+j].re * vscale;
		cp[disb+j].im = c_in[dis+j].im * vscale;
	}
    }
    if ((row & 1)==0)	{
	cp += cln;
	c_in += dimen1len;
	cp->re = c_in->re * vscale;
	cp->im = c_in->im * vscale;
	for (i=1; i<c_len; i++)	{
		cp[i].re = c_in[i].re * vscale;
		cp[i].im = c_in[i].im * vscale;
		cp[cln-i].re = rbuf[i].re * vscale;
		cp[cln-i].im = -rbuf[i].im * vscale;
	}
	if ((cln & 1)==0){
		cp[i].re = c_in[i].re * vscale;
		cp[i].im = c_in[i].im * vscale;
	}
    }
}


v2dtofft(c_in, cp, r_len, c_len, vscale)
register COMPLEX	*c_in, *cp;
register float	vscale;
{
int	i;
	/* The first row, column, and frame always symmetrical	*/
	/* same as the n / 2 + 1 r, c, f if they are even	*/
    cp->re = c_in->re * vscale;	/* [0, 0]	*/
    cp->im = c_in->im * vscale;
    for (i=1; i<c_len; i++)	{	/* 1st row --> Nyquist	*/
	cp[cln-i].re = cp[i].re = c_in[i].re * vscale;
	cp[cln-i].im = -(cp[i].im = c_in[i].im * vscale);
    }
    if ((cln & 1)==0){	/* neutral line	*/
	cp[i].re = c_in[i].re * vscale;
	cp[i].im = c_in[i].im * vscale;
    }

    for (i=1; i<r_len; i++) {	/* dis for conjugate symmetry	*/
	register int	j, dis=(row-(i<<1))*dimen1len,
			disb=cln*(row-(i<<1)), dise=disb+cln;
	cp += cln;
	c_in += dimen1len;
	cp->re = c_in[0].re * vscale;	/* 1st column in row[i]	*/
	cp->im = c_in[0].im * vscale;
	cp[disb].re = c_in[dis].re * vscale;	/* 1st column in row[d]	*/
	cp[disb].im = c_in[dis].im * vscale;
	for (j=1; j<c_len;j++)	{
			/* down-right <--> up-left	*/
		cp[dise-j].re = cp[j].re = c_in[j].re * vscale;
		cp[dise-j].im = -(cp[j].im = c_in[j].im * vscale);
			/* up-right <--> down-left	*/
		cp[cln-j].re = cp[disb+j].re = c_in[dis+j].re * vscale;
		cp[cln-j].im = -(cp[disb+j].im = c_in[dis+j].im * vscale);
	}
	if ((cln & 1)==0) {	/* neutral column	*/
		cp[j].re = c_in[j].re * vscale;
		cp[j].im = c_in[j].im * vscale;
		cp[disb+j].re = c_in[dis+j].re * vscale;
		cp[disb+j].im = c_in[dis+j].im * vscale;
	}
    }
    if ((row & 1)==0)	{
	cp += cln;
	c_in += dimen1len;
	cp->re = c_in->re * vscale;
	cp->im = c_in->im * vscale;
	for (i=1; i<c_len; i++)	{
		cp[cln-i].re = cp[i].re = c_in[i].re * vscale;
		cp[cln-i].im = -(cp[i].im = c_in[i].im * vscale);
	}
	if ((cln & 1)==0){
		cp[i].re = c_in[i].re * vscale;
		cp[i].im = c_in[i].im * vscale;
	}
    }
}


dv2dtofft(c_in, cp, r_len, c_len, vscale)
register DBCOMPLEX	*c_in, *cp;
register float	vscale;
{
int	i;
    cp->re = c_in->re * vscale;
    cp->im = c_in->im * vscale;
    for (i=1; i<c_len; i++)	{
	cp[cln-i].re = cp[i].re = c_in[i].re * vscale;
	cp[cln-i].im = -(cp[i].im = c_in[i].im * vscale);
    }
    if ((cln & 1)==0){
	cp[i].re = c_in[i].re * vscale;
	cp[i].im = c_in[i].im * vscale;
    }

    for (i=1; i<r_len; i++) {
	register int	j, dis=(row-(i<<1))*dimen1len,
			disb=cln*(row-(i<<1)), dise=disb+cln;
	cp += cln;
	c_in += dimen1len;
	cp->re = c_in[0].re * vscale;
	cp->im = c_in[0].im * vscale;
	cp[disb].re = c_in[dis].re * vscale;
	cp[disb].im = c_in[dis].im * vscale;
	for (j=1; j<c_len;j++)	{
		cp[dise-j].re = cp[j].re = c_in[j].re * vscale;
		cp[dise-j].im = -(cp[j].im = c_in[j].im * vscale);
		cp[cln-j].re = cp[disb+j].re = c_in[dis+j].re * vscale;
		cp[cln-j].im = -(cp[disb+j].im = c_in[dis+j].im * vscale);
	}
	if ((cln & 1)==0) {
		cp[j].re = c_in[j].re * vscale;
		cp[j].im = c_in[j].im * vscale;
		cp[disb+j].re = c_in[dis+j].re * vscale;
		cp[disb+j].im = c_in[dis+j].im * vscale;
	}
    }
    if ((row & 1)==0)	{
	cp += cln;
	c_in += dimen1len;
	cp->re = c_in->re * vscale;
	cp->im = c_in->im * vscale;
	for (i=1; i<c_len; i++)	{
		cp[cln-i].re = cp[i].re = c_in[i].re * vscale;
		cp[cln-i].im = -(cp[i].im = c_in[i].im * vscale);
	}
	if ((cln & 1)==0){
		cp[i].re = c_in[i].re * vscale;
		cp[i].im = c_in[i].im * vscale;
	}
    }
}


/*	float complex to double	*/
complex_to_d(buf, size)
VType	*buf;
{
register float	*fp = buf;
register double	*op = buf;
register int	i;

if (verbose)	mesg("converting complex to double\n");
for (i=0; i<size; i++){
register double	value = *fp * *fp;
	fp++;
	value += *fp * *fp;
	fp++;
	*op++ = sqrt(value);
}
}

/*	double complex to real double	*/
dcomplex_to_d(buf, size)
register double	*buf;
{
register double	*ibp = buf;
register int	i;

if (verbose)	mesg("converting double complex to double\n");
for (i=0; i<size; i++){
register double	value = *ibp * *ibp;
	ibp++;
	value += *ibp * *ibp;
	ibp++;
	*buf++ = sqrt(value);
}
}


/*===============================================
%	This routine convert vfft to spectrum.	%
%		It is exactly transform.	%
===============================================*/
vfft_handle(vform, ibuf, o_buf, rows, cols, dimen1len)
double	*ibuf, *o_buf;
{
int	vsize = rows * dimen1len;

switch(vform){
case IFMT_VFFT2D:
case IFMT_VFFT3D:	complex_to_d(ibuf, vsize);
	break;
case IFMT_DVFFT2D:
case IFMT_DVFFT3D:	dcomplex_to_d(ibuf, vsize);
	break;
}

if (spectrum)	{	/*	VFFT spectrum	*/
register int	i, j, dc=dimen1len-(dimen1len&1);
register double	*src=ibuf + (vsize>>1),	*dst=o_buf + dimen1len-1,
	*dstcs=o_buf + rows*cols - (dimen1len + (cols&1));
	/* like in v2dtoc, we can use integer dis instead of using *dstcs
		for computing conjugate symmetry distance */
    for (i=0; i<rows>>1; i++)	{
	*dst = *dstcs = *src++;
	for (j=1; j<dc; j++)
		dstcs[-j] = dst[j] = *src++;
	if (dimen1len & 1)	/* throw away the Nyquist since just display */
		src++;
	dst += cols;	dstcs -= cols;
    }

    for (src=ibuf; i<rows; i++)	{
	*dst = *dstcs = *src++;
	for (j=1; j<dc; j++)
		dstcs[-j] = dst[j] = *src++;
	if (dimen1len & 1)
		src++;
	dst += cols;	dstcs -= cols;
    }
}
else	{
register int	i, j, dis=cols*rows;	/* conjugately symmetric distance */
register double	*src=ibuf,	*dst=o_buf;
    for (i=0; i<rows; i++)	{
	*dst = *src++;
	for (j=1; j<dimen1len; j++)
		dst[dis-j] = dst[j] = *src++;
	dst += cols;	dis -= cols << 1;
    }
}
}

/*	Double Complex Flip Quadrant	*/
Dcom_flip(ibuf, o_buf, rows, cols)
double	*ibuf, *o_buf;
{
int	hcols=cols>>1;
register int	i, j;
register double	*src=ibuf + (rows*cols>>1),	*dst=o_buf + hcols;
    for (i=0; i<rows>>1; i++){
	for (j=0; j<hcols; j++)
		dst[-j-1] = dst[j] = *src++;
	dst += cols;	src += hcols;
    }
    for (src=ibuf; i<rows; i++){
	for (j=0; j<hcols; j++)
		dst[-j-1] = dst[j] = *src++;
	dst += cols;	src += hcols;
    }
}

/*===============================================
%	left half is one line higher than	%
%	right half. It does not effect view,	%
%	but may not use for real application	%
===============================================*/
Dcom_fflip(ibuf, o_buf, rows, cols)
double	*ibuf, *o_buf;
{
register int	i;
register double	*src=ibuf + (cols*(rows+1) >> 1), *dst = o_buf;

for (i=cols*(rows-1) >> 1; i--;)
	*dst++ = *src++;
src = ibuf;
for (i=cols*(rows+1) >> 1; i--;)
	*dst++ = *src++;
}

