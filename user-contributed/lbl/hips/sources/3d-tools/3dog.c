/*	Copyright (c)	1990	Jin Guojun
%
% 3dog.c:
%	filter an image by applying Difference of Gaussians mask.
%	The input is in byte or float format, and the output is in
%	floating point or integer format.
*/

char	usage[]="\n\
3dog [-A esigma [masksize [ratio ]]] [-p #] [-i [-c]] [-m] [-w]\n\
	[-b #] [-n #] [-s #] [-g #] [-flvt] [< input_seq] [> out_seq]\n\
\n\
esigma [1.0] is the standard deviation of the \"excitatory\"\n\
	Gaussian - real positive number.\n\
masksize [7] is the size of the mask (linear).\n\
ratio [1.4] is the ratio b/w the s.d.'s of the inhibitory and\n\
	excitatory Gaussians.\n\
-p #	followed by a positive integer specifies the precision.\n\
		Defaults to 1.\n\
-b #	begin from #th frame. default is 0.\n\
-n #	number of frames will be processed, default is total.\n\
-s #	span -- frames between 2 slices.\n\
-i	implies output in integer format.\n\
-c	if -i is specified, causes checking of input to be in the\n\
		range [-1024 to 1024].\n\
-m	output the Gaussian(s) only.\n\
-l	force dog3d do level x-y 2d filter	\n\
		([x is a level axle torward face,\n\
		[ y axle is from left to right,	\n\
		[ z is a vertical axle.])	\n\
-v	force dog3d do vertical x-z 2d filter.	\n\
-f	force dog3d do vertical y-z 2d filter.	\n\
-g #	output the Gaussian(s) in filter_size + #\n\
		Otherwise, output the Gaussian(s) in method ( 2**n ).\n\
-w	output without header.\n\n\
	If input file is not applied, the filtering is done on\n\
		an impulse response in a 7 x 7 x 7 frame.\n";
/*
% compile:	cc -O -o $(DestDIR)/3dog 3dog.c -lccs -lhipsh -lhips -lm
%		The -O Optimize option will speed up process.
%
% Author:	Guojun Jin - LBL	11/25/90
*/

#include "header.def"
#include "imagedef.h"
#include <math.h>

int	span;		/* frames in two slices	*/
MType	fsize;
U_IMAGE	uimg;

#define	bbuf	uimg.src
#define	col	uimg.width
#define	row	uimg.height
#define	frm	uimg.frames
#define	in_fmt	uimg.in_form

#define	Float	float
#define	Range	1024
#define	Faster

#define	bgf	bgn_frm
#define	k	num_offp

#ifdef	_DEBUG_
extern	int	debug;
#endif	_DEBUG_

#define	GValue()	arget(argc, argv, &i, &c)


main(argc, argv)
int	argc;
char**	argv;
{
MType	*int_inbp, *int_obp, *int_conv,
	*int_emask, *int_imask;
int	synthetic,
	woh=0, bgn_frm=1, num_offp=0,
	isint=0, checking=0, gaussonly=0,
	precision=1, masksize=7,
	i, edglen,
	ovf, unf;
bool	Dir=0;

	/*	using same variable at different parse	*/
#define	form_size	gaussonly
#define	c	edglen

byte	*bp;	/* byte buffer & pointer	*/
char	*fname=NULL;
Float	*inbuf, *obuf,	/* buffers	*/
	*conv,		/* temp converting buffer	*/
	*obp, *inbp,	/* in & out put buffer pointers	*/
	*emask, *imask, gaussize=0;
double	esigma=1., ratio=1.4,
	gauss_mask();

format_init(&uimg, IMAGE_INIT_TYPE, HIPS, -1, *argv, "S20-1");

for (i = 1; i < argc; i++)
  if (*argv[i] == '-') {
	c = 1;
args:	switch(argv[i][c++])	{
	case 'p':
		precision = GValue();	break;
	case 'A':
		esigma	= GValue();
		if (!esigma)	goto	args;
		i++;
		frm = masksize= GValue();
		if (!frm)	goto	args;
		i++;
		ratio	= arget(argc, argv,&i,&c,0);	break;
	case 'i':
		isint++;	break;
	case 'c':
		checking++;	break;
	case 'b':
		bgn_frm = GValue();	break;
	case 'n':
		num_offp = GValue();	break;
#ifdef	_DEBUG_
	case 'd':
		debug = TRUE;	break;
#endif	_DEBUG_
	case 'm':
		gaussonly++;
		msg("%s no convolution; output the Gaussian(s)\n", Progname);
		break;
	case 's':
		span = GValue() + 1;	break;
	case 'l':
	case 'v':
	case 'f':
		Dir = *argv[i];	break;
	case 'g':
		gaussize = GValue();	break;
	case 'w':
		woh++;	break;
	default:
info:		usage_n_options(usage, i, argv[i]);
	}
   }
   else	fname = argv[i];

io_test(fileno(out_fp), goto	info);

if (esigma <= 0.0)	syserr("sigma must be positive");
if (masksize < 1)	syserr("mask-size must be > 1");
if (ratio < 0.0)	syserr("ratio must be >= 0");
if (ratio == 1)		ratio = 0.0;

if (checking && !isint) {
	mesg("checking mode ignored; output is not INT.\n");
	checking=0;
}

if (fname){
	if ((in_fp=freopen(fname, "rb", stdin))==NULL)
		syserr("input file %s", fname);
	goto	rdh;
}
else if (!system("test -t 0")) {	/* no input file */
	edglen = 1;
	if (gaussize)
		edglen = masksize + gaussize;
	else	while(edglen < masksize)	edglen <<= 1;
	if (frm > masksize)	frm = masksize>>1;
	else	if (!frm)	frm = masksize;
	if (!(frm % 2))	frm--;	/* must odd frames	*/
	if (Dir || frm<1)	frm = 1;
	if (!gaussonly)
	    message("%s: output %dx%dx%d sample\n", *argv,edglen,edglen,frm);
	col = row = edglen;
	uimg.pxl_out = sizeof(float);
	uimg.o_form = IFMT_FLOAT;
	synthetic = 1;
	(*uimg.std_swif)(FI_INIT_NAME, &uimg, "DOG mask", 0);
	}
else	{
rdh:	(*uimg.header_handle)(HEADER_READ, &uimg, 0, 0);
	if (uimg.in_form != IFMT_BYTE && uimg.in_form !=IFMT_FLOAT)
		syserr("image pixel format must be bytes or floats");
	if (uimg.frames < 2)
		syserr("Can not handle non3D images");
	synthetic = 0;
    }

fsize = row * col;	/* size of one frame	*/
if (!bgn_frm || bgn_frm > frm)	bgn_frm=0;
else	bgn_frm--;
if (!num_offp || num_offp+bgn_frm > frm) num_offp = frm-bgn_frm;
frm = num_offp;

if (!ratio)	msg("%s blurring by a Gaussian mask\n", Progname);

if (!gaussonly)
   {
	if (in_fmt==IFMT_BYTE)
		bbuf = (VType *) zalloc(fsize, (MType)frm, "bbuf");
	inbuf = zalloc(fsize, (MType)sizeof(Float) * frm, "inbuf");
	conv = zalloc(fsize, (MType)sizeof(Float) * frm, "conv");
	obuf = zalloc(fsize, (MType)sizeof(Float) * frm, "obuf");
	uimg.o_form = IFMT_FLOAT;
	uimg.pxl_out = sizeof(float);
	if (isint) {
		int_conv = (MType*)conv;
		int_emask = zalloc((MType)masksize, (MType)sizeof(MType));
		int_imask = zalloc((MType)masksize, (MType)sizeof(MType));
		uimg.o_form = IFMT_LONG;
	}
	if (woh)	/*	go onto screen	*/
		(*uimg.header_handle)(HEADER_FWRITE, &uimg, argc, argv, stderr);
	else	(*uimg.header_handle)(HEADER_WRITE, &uimg, argc, argv, True);
   }

emask = (Float *) zalloc((MType)masksize, (MType)sizeof(Float), "emask");
imask = (Float *) zalloc((MType)masksize, (MType)sizeof(Float), "imask");
	gauss_mask(esigma,masksize,emask,precision,gaussonly?out_fp:NULL);
if (ratio)
	gauss_mask(esigma*ratio,masksize,imask,precision,gaussonly?out_fp:NULL);
if (gaussonly)	return(0);	/* Done	*/

if (isint)	{
	for(i=0; i<masksize; i++) {
		int_emask[i] = emask[i] * Range + .5;
		int_imask[i] = imask[i] * Range + .5;
	}
	form_size = sizeof(MType);
}
else	form_size = sizeof(Float);

if (bgn_frm)	fseek(in_fp, (bgn_frm)*fsize, 1);

if (synthetic){
	if (edglen & 1)	/* is an odd# ?	*/
		synthetic = (frm >> 1)*fsize + (fsize >> 1) + 1;
	else	synthetic = (frm * fsize + col) >> 1;
	if (isint) {
		int_inbp = (MType *)inbuf;
		for(i=0; i < fsize*frm; i++)	int_inbp[i] = 0;
		int_inbp[synthetic] = 1; /* center=1 */
	}
	else {
		for(i=0; i<fsize; i++)	inbuf[i] = 0.0;
		inbuf[synthetic] = 1.0;
	}
}
else	/* read whole image --> memory	*/
	if (in_fmt==IFMT_BYTE)
	{	(*uimg.std_swif)(FI_LOAD_FILE, &uimg, uimg.load_all=frm, No);
		if (isint)	/* type convert	*/
		   for(i=0, bp=bbuf, int_inbp=(MType*)inbuf; i<fsize*frm; i++)
			*int_inbp++ = *bp++ & 0xFF;
		else for(i=0, bp=bbuf, inbp=inbuf; i<fsize*frm; i++)
			*inbp++ = *bp++ & 0xFF;
	}
	else{
		if (upread(inbuf, form_size, fsize * frm, in_fp) !=
			fsize * frm)
rerror:			syserr("unexpected EOF");
		if (isint)	 /* type convert; Float -> int */
		   for(i=0,int_inbp=(MType*)(inbp=inbuf); i<fsize*frm;i++)
			*int_inbp++ = *inbp++;
	    }

if (checking)
    for(i=unf=ovf=0,int_inbp=(MType*)inbuf; i<fsize*frm; i++){
	k = *int_inbp++;
	unf += (k < -Range);
	ovf += (k > Range);
    }
	/*	starting convolution	*/

if (isint) {
	msg("%s_S4-1: integer (%c) GAUSSIAN\n", Progname, Dir);
   if (!Dir)
	{
	int_vconvolve(int_emask,masksize,inbuf,obuf,row,col,frm);
	int_hconvolve(int_emask,masksize,obuf,int_conv,row,col,frm);
	int_fconvolve(int_emask,masksize,int_conv,obuf,row,col,frm);
	if (ratio)
	   {	int_vconvolve(int_imask,masksize,inbuf,int_conv,row,col,frm);
		int_hconvolve(int_imask,masksize,int_conv,inbuf,row,col,frm);
		int_fconvolve(int_imask,masksize,inbuf,int_conv,row,col,frm);
		free(inbuf);	inbuf = (Float*)int_conv;
	   }
	}
   else	switch(Dir)
	{
	case 'f':
	int_vconvolve(int_emask,masksize,inbuf,int_conv,row,col,frm);
	int_hconvolve(int_emask,masksize,int_conv,obuf,row,col,frm);
	if (ratio)
	   {	int_vconvolve(int_imask,masksize,inbuf,int_conv,row,col,frm);
		int_hconvolve(int_imask,masksize,int_conv,inbuf,row,col,frm);
	   }
	break;
	case 'l':
	int_hconvolve(int_emask,masksize,inbuf,int_conv,row,col,frm);
	int_fconvolve(int_emask,masksize,int_conv,obuf,row,col,bgf,frm);
	if (ratio)
	   {	int_hconvolve(int_imask,masksize,inbuf,int_conv,row,col,frm);
		int_fconvolve(int_imask,masksize,int_conv,obuf,row,col,frm);
	   }
	break;
	case 'v':
	int_vconvolve(int_emask,masksize,inbuf,int_conv,row,col,frm);
	int_fconvolve(int_emask,masksize,int_conv,obuf,row,col,frm);
	if (ratio)
	   {	int_vconvolve(int_imask,masksize,inbuf,int_conv,row,col,frm);
		int_fconvolve(int_imask,masksize,int_conv,obuf,row,col,frm);
	   }
	break;
	}
   if (ratio)
	for(i=0, int_obp=(MType*)obuf, int_inbp=(MType*)inbuf; i<fsize*frm; i++)
		*int_obp++ -= *int_inbp++;
}
else{	msg("%s_S4-1: regular [%c span=%d] GAUSSIAN\n", Progname, Dir, span);
   if (!Dir){
	vconvolve(emask,masksize,inbuf,obuf,row,col,frm);
	hconvolve(emask,masksize,obuf,conv,row,col,frm);
	fconvolve(emask,masksize,conv,obuf,row,col,frm);
	if (ratio) {
		vconvolve(imask,masksize,inbuf,conv,row,col,frm);
		hconvolve(imask,masksize,conv,inbuf,row,col,frm);
		fconvolve(imask,masksize,inbuf,conv,row,col,frm);
		inbuf = conv;
	}
   }
   else	switch(Dir)
	{
	case 'f':
		vconvolve(emask,masksize,inbuf,conv,row,col,frm);
		hconvolve(emask,masksize,conv, obuf,row,col,frm);
		if (ratio) {
			vconvolve(imask,masksize,inbuf,conv,row,col,frm);
			hconvolve(imask,masksize,conv,inbuf,row,col,frm);
		}
		break;
	case 'l':
		hconvolve(emask,masksize,inbuf,conv,row,col,frm);
		fconvolve(emask,masksize,conv, obuf,row,col,frm);
		if (ratio) {
			hconvolve(imask,masksize,inbuf,conv,row,col,frm);
			fconvolve(imask,masksize,conv,inbuf,row,col,frm);
		}
		break;
	case 'v':
		vconvolve(emask,masksize,inbuf,conv,row,col,frm);
		fconvolve(emask,masksize,conv, obuf,row,col,frm);
		if (ratio) {
			vconvolve(imask,masksize,inbuf,conv,row,col,frm);
			fconvolve(imask,masksize,conv,inbuf,row,col,frm);
		}
		break;
	}
   if (ratio)
	for(i=0, obp=obuf, inbp=inbuf; i < fsize*frm; i++)
		*obp++ -= *inbp++;
}	/* end of convolution	*/
fwrite(obuf, frm * form_size, fsize, out_fp);

if (checking && (ovf || unf))
	message("%s: %d overflows and %d underflows !!\n", Progname,ovf,unf);
}

#ifdef	Faster

int_hconvolve(mask,masksize, inb, outb,r,c,f)
MType	*mask, *inb, *outb;
{
MType	row_n,		/* row # -- position	*/
	v_lft_cln,	/* very left column	*/
	i, hrzn_p,	/* working point	*/
	*tmpp, *p, pc,	/* relative column pos	*/
	fn, *obp=outb,
	vlpix, vrpix,
	sum, lweight, rweight,

last_cln= c - 1,
msk2	= masksize >> 1,

lftregion = msk2 > c ? c : msk2,
mid_rgn = c - (masksize-msk2);	if (mid_rgn < lftregion) mid_rgn = 0;

for (fn=0; fn<f; fn++, inb+=fsize)
  for (row_n=v_lft_cln=0; row_n < r; row_n++, v_lft_cln+=c)
  {	tmpp = inb + v_lft_cln;	/* current row & first column	*/
	vlpix = *tmpp;		/* cur row very left side pixel	*/
	vrpix = *(tmpp + last_cln);	/* very rlght side pix	*/
	for(hrzn_p=0; hrzn_p < lftregion; hrzn_p++) {
		pc = hrzn_p - msk2;
		p = tmpp + pc;
		/* PC moved form left half mask to right half mask	*/
		for(i=sum=lweight=rweight=0; i < masksize; i++,pc++,p++)
		   {
			if (pc < 0)
				lweight += mask[i];
			else	if (pc < c)
				    sum += mask[i] * *p;
			else	rweight += mask[i];
		   }
		*obp++ = sum + vlpix*lweight + vrpix*rweight;
	}
	for(; hrzn_p <= mid_rgn; hrzn_p++) {
		p = tmpp + hrzn_p - msk2;
		for(i=sum=0; i < masksize;i++,p++)
			sum += *p * mask[i];
		*obp++ = sum;
	}
	for(; hrzn_p < c; hrzn_p++) {
		pc = hrzn_p - msk2;
		p = tmpp + pc;
		for(i=sum=rweight=0; i < masksize; i++,pc++,p++) {
			if (pc > last_cln)
				rweight += mask[i];
			else	sum += *p * mask[i];
		}
		*obp++ = sum + vrpix*rweight;
	}
   }
}

int_vconvolve(mask, masksize, inb, outb, r, c, f)
MType	*mask, *inb, *outb;
{
MType	row_n, hrzn_p, i, fn,
	sum,
	bweight, tweight,
	*tmpp, *p, pr, *obp = outb,

msk2	= masksize >> 1,
last_row= r - 1,
nd_r_p	= last_row * c,	/* last row (end row) first column	*/

topregion = msk2 > r ? r : msk2,
mid_rgn = r-(masksize-msk2); if (mid_rgn < topregion) mid_rgn = 0;

for (fn=0; fn < f; fn++, inb+=fsize) {
  for (row_n=0; row_n < topregion; row_n++)
    for (hrzn_p=0, tmpp = inb + (row_n-msk2)*c; hrzn_p < c; hrzn_p++)
	{
	p = tmpp + hrzn_p;	pr = row_n - msk2;
	for(i=sum=bweight=tweight=0; i<masksize; i++,p+=c,pr++)
	   {
		if (pr < 0)		bweight += mask[i];
		else	if (pr < r)    sum += *p * mask[i];
			else	tweight += mask[i];
	   }
	*obp++ = sum + inb[hrzn_p]*bweight + inb[hrzn_p + nd_r_p]*tweight;
	}
  for(; row_n <= mid_rgn; row_n++)
    for(hrzn_p=0, tmpp=inb + (row_n-msk2)*c; hrzn_p < c; hrzn_p++) {
	for(i=sum=0, p=tmpp + hrzn_p; i < masksize; i++, p+=c)
		sum += *p * mask[i];
	*obp++ = sum;
	}
  for(; row_n < r; row_n++)
    for(hrzn_p=0, tmpp = inb + (row_n-msk2)*c; hrzn_p < c; hrzn_p++) {
	p = tmpp + hrzn_p;
	for(i=sum=tweight=0,pr=row_n - msk2; i < masksize;i++,pr++,p+=c)
		if (pr > last_row)
			tweight += mask[i];
		else 	sum += *p * mask[i];
	*obp++ = sum + tweight * inb[nd_r_p + hrzn_p];
	}
  }
}

int_fconvolve(mask,masksize,inb,outb,r,c,f)
MType	*mask, *inb,*outb;
{
MType	row_n, hrzn_p,
	frm_n,
	msk2 = masksize >> 1,
	last_frm = f - 1,
	i, pf, *tmpp,*p,/* current point (temp)	*/
	sum, weight,	/* rear */
	fweight,	/* front */
	*obp = outb,

frtregion = msk2 > f ? f : msk2,
mid_rgn = f - (masksize-msk2);	if (mid_rgn < frtregion) mid_rgn = 0;

for (frm_n=0; frm_n < frtregion; frm_n++)
  for (row_n=0, tmpp=inb + (frm_n - msk2)*fsize; row_n < r; row_n++,tmpp+=c)
    for (hrzn_p=0; hrzn_p < c; hrzn_p++) {
	p = tmpp + hrzn_p;
	sum = fweight = weight = 0;
	for(i=0; i<masksize; i++, p+=fsize) {
		pf = frm_n - msk2 + i;
		if (pf < 0)		fweight += mask[i];
		else	if (pf < f)	sum += *p * mask[i];
		else	weight += mask[i];
	}
	*obp++ = sum + inb[row_n*c+hrzn_p]*fweight +
		inb[hrzn_p + (last_frm*r+row_n)*c]*weight;
   }
for (; frm_n < mid_rgn; frm_n++)
  for (row_n = 0, tmpp=inb + (frm_n - msk2)*fsize; row_n < r; row_n++,tmpp+=c)
    for (hrzn_p = 0; hrzn_p < c; hrzn_p++) {
	p = tmpp + hrzn_p;
	for(i = sum = 0; i < masksize; i++, p+=fsize)
		sum += *p * mask[i];
	*obp++ = sum;
   }
for (; frm_n < f; frm_n++)
  for (row_n = 0, tmpp=inb + (frm_n - msk2)*fsize; row_n < r; row_n++,tmpp+=c)
    for (hrzn_p = 0; hrzn_p < c; hrzn_p++) {
	p = tmpp + hrzn_p;
	sum = weight = 0;
	for(i = 0; i < masksize; i++, p+=fsize) {
		pf = frm_n - msk2 + i;
		if (pf < f)	sum += *p * mask[i];
		else	weight += mask[i];
	}
	*obp++ = sum + inb[hrzn_p + (last_frm*r+row_n)*c]*weight;
   }
}

hconvolve(mask,masksize, inb, outb,r,c,f)
Float	*mask, *inb, *outb;
{
MType	row_n, hrzn_p, i, pc,
	v_lft_cln, fn;
Float	vlpix, vrpix,
	sum, lweight, rweight,
	*tmpp, *p,
	*obp = outb;

int
last_cln	= c - 1,
msk2	= masksize >> 1,

lftregion= msk2 > c ? c : msk2,
mid_rgn= c-(masksize-msk2); if (mid_rgn < lftregion) mid_rgn = 0;

for (fn = 0; fn < f; fn++, inb += fsize)
  for (row_n = v_lft_cln = 0; row_n < r; row_n++, v_lft_cln += c)
  {	tmpp = inb + v_lft_cln;	/* current row & first column	*/
	vlpix = *tmpp;	/* current row & very left side pixel	*/
	vrpix = *(tmpp + last_cln);	/* very rlght side pix	*/
	for(hrzn_p = 0; hrzn_p < lftregion; hrzn_p++) {
		pc = hrzn_p - msk2;
		p = tmpp + pc;
		/* PC moved form left half mask to right half mask	*/
		for(i=sum=lweight=rweight = 0; i < masksize; i++,pc++,p++)
		   {
			if (pc < 0)
				lweight += mask[i];
			else	if (pc < c)
				    sum += mask[i] * *p;
			else	rweight += mask[i];
		   }
		*obp++ = sum + vlpix*lweight + vrpix*rweight;
	}
	for(; hrzn_p <= mid_rgn; hrzn_p++) {
		p = tmpp + hrzn_p - msk2;
		for(i = sum = 0; i < masksize;i++,p++)
			sum += *p * mask[i];
		*obp++ = sum;
	}
	for(; hrzn_p < c; hrzn_p++) {
		pc = hrzn_p - msk2;
		p = tmpp + pc;
		for(i = sum = rweight = 0; i < masksize; i++,pc++,p++) {
			if (pc > last_cln)
				rweight += mask[i];
			else	sum += *p * mask[i];
		}
		*obp++ = sum + vrpix*rweight;
	}
   }
}

vconvolve(mask,masksize,inb,outb,r,c,f)
Float	*mask, *inb,*outb;
{
MType	row_n, hrzn_p, i,
	pr, fn;
Float	sum , bweight , tweight,
	*tmpp, *p,
	*obp = outb;

MType	last_row = r - 1,
msk2	= masksize >> 1,
nd_r_p	= last_row * c,	/* last row (end row) first column	*/

topregion= msk2 > r ? r : msk2,
mid_rgn= r-(masksize-msk2); if (mid_rgn < topregion) mid_rgn = 0;

for (fn = 0; fn < f; fn++, inb += fsize) {
  for (row_n = 0; row_n < topregion; row_n++)
    for (hrzn_p = 0, tmpp = inb + (row_n-msk2)*c; hrzn_p < c; hrzn_p++)
	{
	p = tmpp + hrzn_p;	pr = row_n - msk2;
	for(i=sum=bweight=tweight=0; i<masksize; i++,p+=c,pr++)
	   {
		if (pr < 0)		bweight += mask[i];
		else	if (pr < r)    sum += *p * mask[i];
			else	tweight += mask[i];
	   }
	*obp++ = sum + inb[hrzn_p]*bweight + inb[hrzn_p + nd_r_p]*tweight;
	}
  for(; row_n <= mid_rgn; row_n++)
    for(hrzn_p = 0, tmpp = inb + (row_n-msk2)*c; hrzn_p < c; hrzn_p++) {
	for(i=sum= 0, p = tmpp + hrzn_p; i < masksize; i++, p+=c)
		sum += *p * mask[i];
	*obp++ = sum;
	}
  for(; row_n < r; row_n++)
    for(hrzn_p = 0, tmpp = inb + (row_n-msk2)*c; hrzn_p < c; hrzn_p++) {
	p = tmpp + hrzn_p;
	for(i=sum=tweight= 0,pr = row_n - msk2; i < masksize;i++,pr++,p+=c)
		if (pr > last_row)
			tweight += mask[i];
		else 	sum += *p * mask[i];
	*obp++ = sum + tweight * inb[nd_r_p + hrzn_p];
	}
  }
}

/*	interpolate from front to rear --
*	when the frame interpolated is front of certain frame, start at
*	more front one frame. If the frame is interpolated torwrad rear,
*	then just start at that frame.
*/
Float	frm_interpolation(masksize, mask, pp, span, fn, last_frm, Fv, Bv)
MType	fn, last_frm;
register Float	*mask, *pp;
{
MType	i, emf, realf, pf, num, Fw=0, Bw=0,
	sfs = masksize / (span << 1),
	smod = masksize % (span << 1);
Float	d, val, sum=0;

if (smod-1)
	sfs = (sfs + 1) << 1;
else	sfs = (sfs << 1) + 1;

for (i=0; i < sfs; i++)
  {
	if (i && i < sfs-1)	num = span;
	else if (!i)	num = smod >> 1;
	else	num = smod - (smod >> 1);
	if (!num)	num = span;

	pf =  i - (sfs >> 1);
	realf = pf + fn;
	if (realf<0)	for (emf=0; emf<num; emf++)	Fw += *mask++;
	else if (realf >= last_frm)
		for (emf=0; emf<num; emf++)
		{	if (realf == last_frm && !emf)
				sum += *(pp + pf*fsize) * *mask++;
			else	Bw += *mask++;
		}
	else {
		val = *(pp + pf*fsize);
		d = (*(pp + (pf + 1)*fsize) - val) / span;
		for (emf=0; emf<num; emf++)
			sum += (val + d*(span-num+emf)) * *mask++;
	}
  }
return	Fw *  Fv + Bw * Bv + sum;
}

fconvolve(mask,masksize,inb,outb,r,c,f)
Float	*mask, *inb,*outb;
{
MType	row_n, hrzn_p,
	frm_n,
	msk2 = masksize >> 1,
	last_frm = f - 1,
	i, pf;
Float	sum, weight,	/* rear */
	fweight,	/* front */
	*tmpp, *p,
	*obp = outb;

MType	frtregion = msk2 > f ? f : msk2,
mid_rgn = f - (masksize-msk2);	if (mid_rgn < frtregion) mid_rgn = 0;

if (span){
int	Fv, Bv;
/*
	pf = (msk2 % span) != 0;
	i = msk2 / span + pf;
	frtregion = i > f ? f : i;
	pf = ((masksize-msk2) % span) != 0;
	mid_rgn = f - (masksize-msk2) / span + pf;
	if (mid_rgn < frtregion) mid_rgn = 0;
	else if (mid_rgn == f)	mid_rgn--;
	wbuf = (Float*)zalloc((MType)masksize, (MType)sizeof(Float));
*/
	msg("interpolate %d frames\n", span-1);

for (frm_n = 0; frm_n < f; frm_n++)
  for (row_n = 0, tmpp=inb + frm_n*fsize; row_n < r; row_n++,tmpp+=c)
    for (hrzn_p = 0; hrzn_p < c; hrzn_p++) {
	p = tmpp + hrzn_p;
	Fv = inb[row_n * c + hrzn_p];
	Bv = inb[(last_frm*r + row_n) * c + hrzn_p];
	*obp++ = frm_interpolation(masksize, mask, p, span, frm_n, last_frm, Fv, Bv);
   }
/****	faniest: if have tmpp = inb + (frm_n - msk2) * fsize	****/
/*
for (; frm_n < mid_rgn; frm_n++)
  for (row_n = 0, tmpp=inb + frm_n*fsize; row_n < r; row_n++,tmpp+=c)
    for (hrzn_p = 0; hrzn_p < c; hrzn_p++) {
	p = tmpp + hrzn_p;
	if (frm_interpolation(masksize, wbuf, p, span, frm_n, last_frm))
		syserr("interpolation frame %d", frm_n);
	for(i = sum = 0; i < masksize; i++, p+=fsize)
		sum += wbuf[i] * mask[i];
	*obp++ = sum;
   }
for (; frm_n < f; frm_n++)
  for (row_n=0, tmpp=inb + frm_n*fsize; row_n < r; row_n++,tmpp+=c)
    for (hrzn_p=0; hrzn_p < c; hrzn_p++) {
	p = tmpp + hrzn_p;
	frm_interpolation(masksize, wbuf, p, span, frm_n, last_frm);
	for(i=sum=weight=0; i < masksize; i++, p+=fsize)
		if (wbuf > 0)	sum += wbuf[i] * mask[i];
		else	weight += mask[i];
	*obp++ = sum + inb[hrzn_p + (last_frm*r+row_n)*c]*weight;
   }
*/
}
else	{
for (frm_n=0; frm_n < frtregion; frm_n++)
  for (row_n=0, tmpp=inb + (frm_n - msk2)*fsize; row_n < r; row_n++,tmpp+=c)
    for (hrzn_p=0; hrzn_p < c; hrzn_p++) {
	p = tmpp + hrzn_p;
	sum = fweight = weight = 0;
	for(i = 0; i < masksize; i++, p+=fsize) {
		pf = frm_n - msk2 + i;	/* pf and p are consistent	*/
		if (pf < 0)		fweight += mask[i];
		else	if (pf < f)	sum += *p * mask[i];
		else	weight += mask[i];
	}
	*obp++ = sum + inb[row_n*c+hrzn_p]*fweight +
		inb[hrzn_p + (last_frm*r+row_n)*c]*weight;
   }
for (; frm_n < mid_rgn; frm_n++)
  for (row_n=0, tmpp=inb + (frm_n - msk2)*fsize; row_n < r; row_n++,tmpp+=c)
    for (hrzn_p=0; hrzn_p < c; hrzn_p++) {
	p = tmpp + hrzn_p;
	for(i=sum=0; i < masksize; i++, p+=fsize)
		sum += *p * mask[i];
	*obp++ = sum;
   }
for (; frm_n < f; frm_n++)
  for (row_n=0, tmpp=inb + (frm_n - msk2)*fsize; row_n < r; row_n++,tmpp+=c)
    for (hrzn_p=0; hrzn_p < c; hrzn_p++) {
	p = tmpp + hrzn_p;
	for(i=sum=weight=0; i < masksize; i++, p+=fsize) {
		pf = frm_n - msk2 + i;
		if (pf < f)	sum += *p * mask[i];
		else	weight += mask[i];
	}
	*obp++ = sum + inb[hrzn_p + (last_frm*r+row_n)*c]*weight;
   }
}
}


#else	/* easy to read and to understand	*/

int_hconvolve(mask,masksize, inb, outb,r,c,f)
MType	*mask, *inb, *outb;
{
MType	row_n,		/* row # -- position	*/
	v_lft_cln,	/* very left column	*/
	i, hrzn_p,	/* working point	*/
	msk2, pc,	/* relative column pos	*/
	fn, *obp=outb,
	vlpix, vrpix,
	sum, lweight, rweight;

msk2	= masksize >> 1;

for (fn = 0; fn < f; fn++, inb += fsize)
  for (row_n=v_lft_cln=0; row_n < r; row_n++, v_lft_cln+=c)
   {
	vlpix = inb[v_lft_cln];		/* very left side pixel	*/
	vrpix = inb[v_lft_cln + c - 1];	/* very rlght side pix	*/
	for(hrzn_p=0; hrzn_p < c; hrzn_p++) {
		pc = hrzn_p - msk2;
		/*	PC moved form left half mask to right half mask	*/
		for(i = sum = lweight = rweight = 0; i < masksize; i++, pc++)
		   {
			if (pc < 0)
				lweight += mask[i];
			else	if (pc < c)
				    sum += mask[i] * inb[v_lft_cln + pc];
			else	rweight += mask[i];
		   }
		*obp++ = sum + vlpix*lweight + vrpix*rweight;
	}
    }
}

int_vconvolve(mask, masksize, inb, outb, r, c, f)
MType	*mask, *inb, *outb;
{
MType	row_n, hrzn_p, i, fn,
	sum , lweight, tweight,
	pr, tp, *obp = outb,
	msk2	= masksize >> 1,
nd_r_p	= (r - 1) * c;	/* last row (end row) first column	*/

for (fn = 0; fn < f; fn++, inb += fsize)
  for (row_n = 0; row_n < r; row_n++)
    for (hrzn_p = 0; hrzn_p < c; hrzn_p++)
	{
	for(i=sum=lweight=tweight=0, tp=(row_n-msk2)*c+hrzn_p;
	i<masksize; i++,tp+=c)
	   {	pr = row_n - msk2 + i;
		if (pr < 0)		lweight += mask[i];
		else	if (pr < r)    sum += mask[i] * inb[tp];
			else	tweight += mask[i];
	   }
	*obp++ = sum + inb[hrzn_p]*lweight + inb[hrzn_p + nd_r_p]*tweight;
	}
}

int_fconvolve(mask,masksize,inb,outb,r,c,f)
MType	*mask, *inb,*outb;
{
MType	row_n, hrzn_p,
	frm_n,
	msk2 = masksize >> 1,
	last_frm = f - 1,
	i, pf, tp,	/* current point (temp)	*/
	sum, weight,	/* rear */
	fweight,	/* front */
	*obp = outb;

for (frm_n=0; frm_n < f; frm_n++)
  for (row_n=0; row_n < r; row_n++)
    for (hrzn_p=0; hrzn_p < c; hrzn_p++) {
	tp = ((frm_n - msk2)*r + row_n) * c + hrzn_p;
	sum = fweight = weight = 0;
	for(i = 0; i < masksize; i++, tp+=fsize) {
		pf = frm_n - msk2 + i;
		if (pf < 0)		fweight += mask[i];
		else	if (pf < f)	sum += mask[i] * inb[tp];
		else	weight += mask[i];
	}
	*obp++ = sum + inb[row_n*c+hrzn_p]*fweight +
		inb[hrzn_p + (last_frm*r+row_n)*c]*weight;
   }
}

hconvolve(mask,masksize, inb, outb,r,c,f)
Float	*mask, *inb, *outb;
{
MType	row_n, hrzn_p, i, pc,msk2,
	v_lft_cln, fn;
Float	vlpix, vrpix,
	sum, lweight, rweight,
	*obp = outb;

msk2	= masksize>>1;

for (fn=0; fn < f; fn++, inb+=fsize)
  for (row_n=v_lft_cln=0; row_n < r; row_n++, v_lft_cln+=c)
    {
	vlpix = inb[v_lft_cln];
	vrpix = inb[v_lft_cln + c - 1];
	for(hrzn_p=0; hrzn_p < c; hrzn_p++) {
		pc = hrzn_p - msk2;	/* start from over left to right */
		for(i=sum=lweight=rweight=0; i < masksize; i++, pc++) {
			if (pc < 0)	lweight += mask[i];
			else	if (pc < c)
				sum += mask[i] * inb[v_lft_cln + pc];
			else	rweight += mask[i];
		}
		*obp++ = sum + vlpix*lweight + vrpix*rweight;
	}
   }
}

vconvolve(mask,masksize,inb,outb,r,c,f)
Float	*mask, *inb,*outb;
{
MType	row_n, hrzn_p,i, pr, msk2,
	nd_r_p, fn, tp;	/* current point (temp)	*/
Float	sum , bweight , tweight,
	*obp = outb;

msk2	= masksize>>1;
nd_r_p	= (r-1)*c;	/* last row (end row) first column	*/

for (fn=0; fn < f; fn++, inb += fsize)
  for (row_n=0; row_n < r; row_n++)
    for (hrzn_p=0; hrzn_p < c; hrzn_p++) {
	sum = bweight = tweight = 0;
	for(i=0, tp = (row_n - msk2) * c + hrzn_p; i < masksize; i++, tp+=c)
	{	pr = row_n - msk2 + i;
		if (pr < 0)		tweight += mask[i];
		else	if (pr < r)	sum += mask[i] * inb[tp];
		else	bweight += mask[i];
	}
	*obp++ = sum + inb[hrzn_p]*tweight + inb[hrzn_p + nd_r_p]*bweight;
   }
}

fconvolve(mask,masksize,inb,outb,r,c,f)
Float	*mask, *inb,*outb;
{
MType	row_n, hrzn_p,
	frm_n,
	msk2 = masksize>>1,
	last_frm = f - 1,
	i, pf, tp;	/* current point (temp)	*/
Float	sum, weight,	/* rear */
	fweight,	/* front */
	*obp = outb;

for (frm_n=0; frm_n < f; frm_n++)
  for (row_n=0; row_n < r; row_n++)
    for (hrzn_p=0; hrzn_p < c; hrzn_p++) {
	tp = ((frm_n - msk2)*r + row_n) * c + hrzn_p;
	sum = fweight = weight = 0;
	for(i = 0; i < masksize; i++, tp+=fsize) {
		pf = frm_n - msk2 + i;
		if (pf < 0)		fweight += mask[i];
		else	if (pf < f)	sum += mask[i] * inb[tp];
		else	weight += mask[i];
	}
	*obp++ = sum + inb[row_n*c+hrzn_p]*fweight +
		inb[hrzn_p + (last_frm*r+row_n)*c]*weight;
   }
}
#endif	Faster
