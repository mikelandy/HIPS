/*
% mulmix.c - multiply each frame in a sequence by the corresponding frame
%	of another sequence pixel by pixel. If the file which has higher
%	hirechy consists of less frames, then that last frame is multiplied by each
%	later frames in the input sequence, otherwise corresponding frames are
%	used.  The following combinations of image formats are supported:
%
%		File sequence		Standard input sequence
%
%		any non pyramid format combinations
%		integer pyramid		integer pyramid
%		float pyramid		float pyramid
%
%	Copyright (c)	1990	Jin, Guojun
%
% usage:	mulmix filter_file < image > output
%
% AUTHOR	Guojun Jin - Lawrence Berkeley Laboratory	12/15/90
%
% last change:	Date 6/2/91 - adding VFFT format.
*/

#include "header.def"
#include "imagedef.h"

U_IMAGE	*img, *img1;

#define	ibuf	img1->dest
#define	obuf	img->dest
#define	itemp1	img1->src /* lower level filter frame to higher level transfer */
#define	otemp	img->src /* image BYTE or SHORT to INTEGER transfer buffer */
#define	in_fp0	img->IN_FP
#define	out_fp0	img->OUT_FP
#define	in_fp1	img1->IN_FP

MType	fsize, diffsize, dimen1len, vfsize;
int	ccf,	/* both file are complex */
	cf=0;	/* major file is complex */

main (argc, argv)
int	argc;
char**	argv;
{
int	toplev, i;

img = ZALLOC(sizeof(*img), 1, "img"),	/* img has higher hirechy than img1 */
img1 = ZALLOC(sizeof(*img1), 1, "img1");

format_init(img, IMAGE_INIT_TYPE, HIPS, -1, *argv, "S16-1");
format_init(img1, IMAGE_INIT_TYPE, HIPS, -1, *argv, "S16-1");

if (argv[argc-1][0]=='-' && argv[argc-1][1]=='D') argc--;
if (argc<2)
	syserr("file argument missing\n%s filter_file < image > output", *argv);
if ((in_fp1=fopen(argv[1], "rb")) == NULL)
	syserr("can't open frame file - %s", argv[1]);

(*img->header_handle)(HEADER_READ, img, 0, 0);
(*img1->header_handle)(HEADER_FREAD, img1, in_fp1, 0);
img->o_form = img->in_form;
img1->o_form = img1->in_form;

	if (img->in_form < img1->in_form) {
	register U_IMAGE*	itemp = img;	img = img1;	img1 = itemp;
	}
	diffsize = img->pxl_in - img1->pxl_in;
	if (diffsize)
		if (img->in_form == IFMT_COMPLEX ||
			img->in_form == IFMT_VFFT2D ||
			img->in_form == IFMT_VFFT3D)
			diffsize -= 4;
		else if (img->in_form == IFMT_DBLCOM ||
			img->in_form == IFMT_DVFFT2D ||
			img->in_form == IFMT_DVFFT3D)
			diffsize -= 12;
	if (img->in_form <= IFMT_SHORT)
	{
		if (!img->in_form)
			diffsize = 8;	/* byte to byte	*/
		else if(!diffsize)
			diffsize = 10;	/* short to short	*/
				/* otherwise is sht to byte diffsize=1	*/
		img->o_form = IFMT_LONG;
		img->pxl_out = sizeof(long);
	}
	else if	((img->in_form==IFMT_FLOAT || img->in_form==IFMT_COMPLEX) &&
		img1->in_form == IFMT_LONG)
		diffsize = 12;

	if (img->height != img1->height || img->width != img1->width)
		syserr("frame file and input header mismatch");

	if (!img->pxl_out)
		img->pxl_out = img->pxl_in;
	(*img->header_handle)(HEADER_WRITE, img, argc, argv, True);

#ifndef	HIPS_PUB
	if (img->in_form == IFMT_LONGPYR || img->in_form == IFMT_FLOATPYR)
	   if	(img->in_form != img1->in_form)
		syserr("format not matching");
	   else{
		if (upread(&toplev, 1, sizeof(toplev), in_fp0) != sizeof(toplev))
			syserr("error reading standard input number of levels");
		if (upread(&i, 1, sizeof(i), in_fp1) != sizeof(i))
			syserr("error reading file input number of levels");
		if (toplev != i)
			prgmerr(i, "pyramid number of levels mismatch");
		fsize = pyrnumpix(toplev,img->height,img->width);
		if (fwrite(&toplev,1,sizeof(toplev),out_fp0) != sizeof(toplev))
			syserr("error writing number of pyramid levels");
	   }
	else
#endif
		fsize = img->height*img->width;
	dimen1len = (img->width>>1) + 1;
	vfsize = img->height * dimen1len;
	if (img->in_form == IFMT_COMPLEX || img->in_form == IFMT_DBLCOM ||
	    img->in_form >= IFMT_VFFT3D && img->in_form < IFMT_VVFFT3D){
		cf++;
		if (img->in_form==img1->in_form)	ccf++;
	}

ibuf = NZALLOC(fsize, img->pxl_out, "mul_ibuf");
obuf = NZALLOC(fsize, img->pxl_out, "mul_obuf");
if (diffsize && diffsize < 12)
	itemp1 = ZALLOC(fsize, img1->pxl_in, "itemp1");
else	itemp1 = ibuf;

if (diffsize == 1 || diffsize == 8 || diffsize == 10)	{
	otemp = ZALLOC(fsize, img->pxl_in, "otemp");
	mulbs(img->frames, img1->frames);
}
else{
/*	ibuf = zalloc(fsize, img1->pxl_in + (diffsize&7));	*/
	otemp = obuf;
	switch(img->in_form)	{
	case IFMT_LONG:
	case IFMT_LONGPYR:
		mulint(img->frames, img1->frames);
		break;
	case IFMT_FLOAT:
	case IFMT_COMPLEX:
	case IFMT_FLOATPYR:
		mulfloat(img->frames,img1->frames);
		break;
	case IFMT_VFFT2D:
	case IFMT_VFFT3D:
		mulvfft(img->frames,img1->frames, img);
		break;
	case IFMT_DVFFT2D:
	case IFMT_DVFFT3D:
		muldvfft(img->frames,img1->frames, img);
		break;
	default:
		muldbl(img->frames,img1->frames);
	}
}
exit(0);
}

mulbs(num_frame, fnum_frame)
{
register int	i,*obp,*ibp;
int	f;

	for (f=0;f<num_frame;f++){
	    if (fnum_frame) {	/* read base (mask) frame(s)	*/
		fnum_frame--;
		if ((i=(*img1->std_swif)(FI_LOAD_FILE, img1, 0, No)) != fsize)
			syserr("reading error during bstoi transfer1 %d", i);
		switch(diffsize)
		{
		case 8:
		case 1:	btoi(itemp1, ibuf, fsize);	break;
		case 10:stoi(itemp1, ibuf, fsize);	break;
		default:	syserr("BS1 to int error %d", diffsize);
		}
	    }
	    if ((i=(*img->std_swif)(FI_LOAD_FILE, img, 0, No)) != fsize)
		syserr("reading error during bstoi transfer %d", i);
	    switch(diffsize)
		{
		case 8:	btoi(otemp, obuf, fsize);	break;
		case 1:
		case 10:stoi(otemp, obuf, fsize);	break;
		default:	syserr("BS0 to int error %d", diffsize);
		}

	    obp = (int*)obuf;
	    ibp = (int*)ibuf;
	    for (i=fsize; i--;)
		*obp++ *= *ibp++;
	    if (fwrite(obuf, img->pxl_out, fsize, out_fp0) != fsize)
			syserr("error during mulbs write int");
	}
}

mulint(num_frame,fnum_frame)
{
register int	i,*obp,*ibp;
int	f;

	for (f=0;f<num_frame;f++) {
	    if (fnum_frame) {
		fnum_frame--;
		if ((i=(*img1->std_swif)(FI_LOAD_FILE, img1, 0, No)) != fsize)
			syserr("reading error during btoi transfer %d", i);
		switch(diffsize)
		{
		case 0:	if (debug) mesg("int x int\n");	break;
		case 2:	stoi(itemp1, ibuf, fsize);	break;
		case 3: btoi(itemp1, ibuf, fsize);	break;
		default:prgmerr(diffsize, "X to int error %d", diffsize);
		}
	    }
	    if ((*img->std_swif)(FI_LOAD_FILE, img, 0, No) != fsize)
		syserr("error during last int sequence read");
	    obp = (int*)obuf;
	    ibp = (int*)ibuf;
	    for (i=fsize; i--;)
		*obp++ *= *ibp++;
	    if (fwrite(obuf,img->pxl_out,fsize,out_fp0) != fsize)
		syserr("error during write");
	}
}

mulfloat(num_frame,fnum_frame)
{
int	f;
register int	i;
register float	*obp,*ibp;
float	R,I;

	for (f=0;f<num_frame;f++) {
	    if (fnum_frame) {
		fnum_frame--;
		if ((i=(*img1->std_swif)(FI_LOAD_FILE, img1, 0, No)) != fsize)
			syserr("reading error during xtof transfer %d", i);
		else	message("read %d bytes\n", i);
		switch(diffsize)
		{
		case 12:itof(itemp1, ibuf, fsize);	break;
		case 3:	btof(itemp1, ibuf, fsize);	break;
		case 2:	stof(itemp1, ibuf, fsize);	break;
		case 0:	if (debug) mesg("fp x fp\n");	break;
		default:syserr("X to float error %d", diffsize);
		}
	    }
	    if ((*img->std_swif)(FI_LOAD_FILE, img, 0, No) != fsize)
			syserr("error during last float sequence read");
	    obp = (float*)obuf;
	    ibp = (float*)ibuf;
	    for (i=fsize; i--;) {
		if(ccf) {
			R = (*ibp)*(*obp) - (*(ibp+1))*(*(obp+1));
			I = (*ibp)*(*(obp+1)) + (*obp)*(*(ibp+1));
			*obp++ = R; *obp++ = I; ibp += 2;
		}
		else {	if (cf)	*obp++ *= *ibp;
			*obp++ *= *ibp++;
		}
	    }
	    if (fwrite(obuf, img->pxl_out, fsize, out_fp0) != fsize)
			syserr("error during write");
	}
}

muldbl(num_frame,fnum_frame)
{
register int	i;
int	f,s1=1<<ccf, s0=1<<cf;
register double	*obp, *ibp;
double	R,I;

	for (f=0;f<num_frame;f++) {
	    if (fnum_frame) {
		fnum_frame--;
		if (diffsize)
		{
		if((i=upread(itemp1, (sizeof(double)-(diffsize&7)), fsize, in_fp1))
			!= fsize)
			syserr("reading error during transfer");
		else	message("read %d bytes\n", i);
		switch(diffsize)
			{
			case 4:	ftod(itemp1, ibuf, fsize);	break;
			case 12:itod(itemp1, ibuf, fsize);	break;
			case 6:	stod(itemp1, ibuf, fsize);	break;
			case 7:	btod(itemp1, ibuf, fsize);	break;
			default:syserr("X to double error %d", diffsize);
			}
		}
		else if (upread(ibuf, s1 * sizeof(double), fsize, in_fp1) !=
			fsize)
				syserr("error during read");
	    }
	    if (upread(obuf, s0 * sizeof(double), fsize, in_fp0) != fsize)
			syserr("error during fixed sequence read");
	    obp = (double*)obuf;
	    ibp = (double*)ibuf;
	    for (i=fsize;i--;) {
		if(ccf) {
			R = (*ibp)*(*obp) - (*(ibp+1))*(*(obp+1));
			I = (*ibp)*(*(obp+1)) + (*obp)*(*(ibp+1));
			*obp++ = R; *obp++ = I; ibp += 2;
		}
		else {	if (cf)	*obp++ *= *ibp;
			*obp++ *= *ibp++;
		}
	    }
	    if (fwrite(obuf,s0*sizeof(double),fsize,out_fp0) != fsize)
			syserr("error during write");
	}
}

mulvfft(num_frame,fnum_frame, img)
U_IMAGE	*img;
{
int	f, s1=1<<ccf,	/* filter file size */
	s0=1<<cf;	/* major file size */
register int	i;
register float	*obp,*ibp;
float	R,I;

	for (f=0;f<num_frame;f++) {
	    if (fnum_frame) {
		fnum_frame--;
		if (diffsize)
		{
		    if((i=upread(itemp1, (sizeof(float)-diffsize%4), fsize, in_fp1))
			!= fsize)
			syserr("reading error during xtof transfer %d", i);
		    else	message("read %d bytes\n", i);
		    switch(diffsize)
			{
			case 12:itov(itemp1, ibuf, img);	break;
			case 3:	btov(itemp1, ibuf, img);	break;
			case 2:	stov(itemp1, ibuf, img);	break;
			default:syserr("X to float error %d", diffsize);
			}
		}
		else if (s1==1){
			if ((i=upread(ibuf, sizeof(float), fsize, in_fp1)) !=
			fsize)
				syserr("error during vfft read");
			else	ftov(ibuf, img);
		    }
		    else
			if ((i=upread(ibuf, sizeof(float)<<1, vfsize, in_fp1)) !=
			vfsize)
				syserr("error during vfft read");

	    }
	    if (upread(obuf, s0 * sizeof(float), vfsize, in_fp0) != vfsize)
			syserr("error during last vfft sequence read");
	    obp = (float*)obuf;
	    ibp = (float*)ibuf;
	    for (i=vfsize; i--;){
		if (ccf){
			R = (*ibp)*(*obp) - (*(ibp+1))*(*(obp+1));
			I = (*ibp)*(*(obp+1)) + (*obp)*(*(ibp+1));
			*obp++ = R; *obp++ = I; ibp += 2;
		}
		else {	if (cf)	*obp++ *= *ibp;
			*obp++ *= *ibp++;
		}
	    }
	    if (fwrite(obuf, s0 * sizeof(float), vfsize, out_fp0) != vfsize)
			syserr("error during v_write");
	}
}

muldvfft(num_frame,fnum_frame, img)
U_IMAGE	*img;
{
register int	i;
int	f,s1=1<<ccf, s0=1<<cf;
register double	*obp, *ibp;
double	R,I;

	for (f=0;f<num_frame;f++) {
	    if (fnum_frame) {
		fnum_frame--;
		if (diffsize)
		{
		if((i=upread(itemp1,(sizeof(double)-(diffsize&7)),fsize,in_fp1))
			!= fsize)
			syserr("reading error during transfer");
		else	message("read %d bytes\n", i);
		switch(diffsize)
			{
			case 4:	ftodv(itemp1, ibuf, img);	break;
			case 12:itodv(itemp1, ibuf, img);	break;
			case 6:	stodv(itemp1, ibuf, img);	break;
			case 7:	btodv(itemp1, ibuf, img);	break;
			default:syserr("X to double error %d", diffsize);
			}
		}
		else if (s1==1){
			if ((i=upread(ibuf, sizeof(double), fsize, in_fp1)) !=
			fsize)
				syserr("error during dvfft read");
			else	dtodv(ibuf, img);
		    }
		    else
			if ((i=upread(ibuf, sizeof(double)<<1, vfsize, in_fp1))
			    != vfsize)
				syserr("error during dvfft read");

	    }
	    if (upread(obuf, s0 * sizeof(double), vfsize, in_fp0) != vfsize)
			syserr("error during fixed dvfft sequence read");
	    obp = (double*)obuf;
	    ibp = (double*)ibuf;
	    for (i=vfsize; i--;) {
		if(ccf) {
			R = (*ibp)*(*obp) - (*(ibp+1))*(*(obp+1));
			I = (*ibp)*(*(obp+1)) + (*obp)*(*(ibp+1));
			*obp++ = R; *obp++ = I; ibp += 2;
		}
		else {	if (cf)	*obp++ *= *ibp;
			*obp++ *= *ibp++;
		}
	    }
	    if (fwrite(obuf, s0*sizeof(double), vfsize, out_fp0) != vfsize)
			syserr("error during dv_write");
	}
}

btoi(ibp, obp, n)
register byte*	ibp;
register int*	obp, n;
{
#ifdef	_DEBUG_
message("%d byte to integer\n", n);
#endif
while (n--)	*obp++ = *ibp++;
}

btof(ibp, obp, n)
register byte*	ibp;
register float*	obp;
register int	n;
{
#ifdef	_DEBUG_
message("%d byte to float\n", n);
#endif
while (n--)	*obp++ = (float)*ibp++;
}

btod(ibp, obp, n)
register byte*	ibp;
register double	*obp;
register int	n;
{
#ifdef	_DEBUG_
message("%d byte to double\n", n);
#endif
while (n--)	*obp++ = *ibp++;
}

stoi(ibp, obp, n)
register unsigned short	*ibp;
register int*	obp, n;
{
#ifdef	_DEBUG_
message("%d short to integer\n", n);
#endif
while (n--)	*obp++ = *ibp++;
}

stof(ibp, obp, n)
register unsigned short	*ibp;
register float*	obp;
register int	n;
{
#ifdef	_DEBUG_
message("%d short to float\n", n);
#endif
while (n--)	*obp++ = (float)*ibp++;
}

stod(ibp, obp, n)
register unsigned short	*ibp;
register double	*obp;
register int	n;
{
#ifdef	_DEBUG_
message("%d short to double\n", n);
#endif
while (n--)	*obp++ = *ibp++;
}

itof(ibp, obp, n)
register int*	ibp, n;
register float*	obp;
{
#ifdef	_DEBUG_
message("%d int to float\n", n);
#endif
while (n--)	*obp++ = *ibp++;
}

itod(ibp, obp, n)
register int*	ibp, n;
register double	*obp;
{
#ifdef	_DEBUG_
message("%d int to double\n", n);
#endif
while (n--)	*obp++ = *ibp++;
}

ftod(ibp, obp, n)
register float*	ibp;
register double	*obp;
register int	n;
{
#ifdef	_DEBUG_
message("%d float to double\n", n);
#endif
while (n--)	*obp++ = *ibp++;
}

btov(ibp, obp, img)
register byte	*ibp;
register float	*obp;
U_IMAGE	*img;
{
register int	c, r;

for (r=img->height; r--;){
    for (c=0; c<dimen1len; c++)
	*obp++ = *ibp++;
    ibp += img->width - dimen1len;
}
}

stov(ibp, obp, img)
register short	*ibp;
register float	*obp;
U_IMAGE	*img;
{
register int	c, r;

for (r=img->height; r--;){
    for (c=0; c<dimen1len; c++)
	*obp++ = *ibp++;
    ibp += img->width - dimen1len;
}
}

itov(ibp, obp, img)
register int	*ibp;
register float	*obp;
U_IMAGE	*img;
{
register int	c, r;

for (r=img->height; r--;){
    for (c=0; c<dimen1len; c++)
	*obp++ = *ibp++;
    ibp += img->width - dimen1len;
}
}

ftov(ibp, obp, img)
register float	*ibp, *obp;
U_IMAGE	*img;
{
register int	c, r;

for (r=img->height; r--;){
    for (c=0; c<dimen1len; c++)
	*obp++ = *ibp++;
    ibp += img->width - dimen1len;
}
}

btodv(ibp, obp, img)
register byte	*ibp;
register double	*obp;
U_IMAGE	*img;
{
register int	c, r;

for (r=img->height; r--;){
    for (c=0; c<dimen1len; c++)
	*obp++ = *ibp++;
    ibp += img->width - dimen1len;
}
}

stodv(ibp, obp, img)
register short	*ibp;
register double	*obp;
U_IMAGE	*img;
{
register int	c, r;

for (r=img->height; r--;){
    for (c=0; c<dimen1len; c++)
	*obp++ = *ibp++;
    ibp += img->width - dimen1len;
}
}

itodv(ibp, obp, img)
register int	*ibp;
register double	*obp;
U_IMAGE	*img;
{
register int	c, r;

for (r=img->height; r--;){
    for (c=0; c<dimen1len; c++)
	*obp++ = *ibp++;
    ibp += img->width - dimen1len;
}
}

ftodv(ibp, obp, img)
register float	*ibp;
register double	*obp;
U_IMAGE	*img;
{
register int	c, r;

for (r=img->height; r--;){
    for (c=0; c<dimen1len; c++)
	*obp++ = *ibp++;
    ibp += img->width - dimen1len;
}
}

dtodv(ibp, obp, img)
register double	*ibp, *obp;
U_IMAGE	*img;
{
register int	c, r;

for (r=img->height; r--;){
    for (c=0; c<dimen1len; c++)
	*obp++ = *ibp++;
    ibp += img->width - dimen1len;
}
}
