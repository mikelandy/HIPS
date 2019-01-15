/*
* dog.c 2.0 -	filter an image by applying difference of Gaussians mask.
*		The input is in byte or float format, and the output is in
*		floating point.	or in integer format.			*/
char	usage[]="\n\
* usage:	dog [-A esigma [masksize [ratio ]]] [-b #] [-n #]	\n\
*		[-p #] [-i [-c]] [-m] [-g #] [-w] [< input_seq] [> out_seq]\n\
*\n\
*	esigma [1.0] is the standard deviation of the \"excitatory\" Gaussian.\n\
*		A real positive number.	\n\
*	masksize [7] is the size of the mask (an integer).		\n\
*	ratio [1.6] is the ratio b/w the s.d.'s of the inhibitory and	\n\
*		excitatory Gaussians.	\n\
*	-p followed by a positive integer specifies the precision.	\n\
*		Defaults to 1.	\n\
*	-b begin from #th frame. default is 0.	\n\
*	-n number of frames will be processed, default is total.	\n\
*	-i implies output in IFMT_LONG format.	\n\
*	-c if -i is specified, causes checking of input to be in the	\n\
*	 range [-1024 to 1024].	\n\
*	-m output the Gaussian(s) only, do not convolve.		\n\
*	-g display size = filter size + #	\n\
*		otherwise, Gaussian filter display by using 2**n square.\n\
*	-w output without header.	\n\
*\n\
*	If input file is not given, the filtering is done on an impulse	\n\
*		response in a 7 x 7 frame.	\n";
/*
* compile:	cc -O -o DestPath/dog dog.c -lccs -lhips -lm
*  note:	The efficiency of convolution depends heavily on optimization
*	by	the compiler, hence option -O, and also need to define Faster.
*
* Author:	Yoav Cohen - 12/12/82
* modified by Jin, Guojun for convolution algrithm correction - 10/25/90
*/


#include "header.def"
#include "imagedef.h"
#include <math.h>

U_IMAGE	uimg;

#define	rows	uimg.height
#define	cols	uimg.width
#define	frm	uimg.frames
#define	k	num_offp
#define	bbuf	uimg.src
#define	fname	uimg.name

#define	Float	float
#define	Range	1024
#define	Faster

#ifndef	MType
#define	MType	int
#endif


main(argc, argv)
int	argc;
char**	argv;
{
MType	*int_inbp, *int_obp, *int_conv1,
	*int_emask, *int_imask, fsize;
int	fr, synthetic=0,
	woh=0, bgn_frm=1, num_offp=0,
	isint=0, checking=0, gaussonly=0,
	precision=1,
	masksize=7,
	i, edglen,
	ovf, unf;

#define	form_size	gaussonly
#define	c	edglen
	/*	using same variable at different parse	*/

char	*bp;	/* byte buffer & pointer	*/
Float	*inbuf, *obuf,	/* buffers	*/
	*conv1,		/* temp converting buffer	*/
	*obp, *inbp,	/* in & out put buffer pointers	*/
	gaussize=0,
	*emask, *imask;
double	esigma=1., ratio=1.6,	/* default value	*/
	gauss_mask();


format_init(&uimg, IMAGE_INIT_TYPE, HIPS, -1, *argv, "S20-1");

for (i = 1; i < argc; i++)
  if(*argv[i] == '-')
    {	c = 1;
args:	switch(argv[i][c++])
	{
	case 'p':
		if (!argv[i][c]) {	i++; c=0;	}
		precision = atoi(argv[i]+c);	break;
	case 'A':
		if (!argv[i][c]) {	i++; c=0;	}
		esigma	= atof(argv[i++]+c);
		if (i >= argc)	break;
		if (*argv[i] == '-')	goto	args;
		frm = masksize= atoi(argv[i++]);
		if (i >= argc)	break;
		if (*argv[i] == '-')	goto	args;
		ratio	= atof(argv[i]);	break;
	case 'i':
		isint++;	break;
	case 'c':
		checking++;	break;
	case 'b':
		if (!argv[i][c]) {	i++; c=0;	}
		bgn_frm = atoi(argv[i]+c);	break;
	case 'n':
		if (!argv[i][c]) {	i++; c=0;	}
		num_offp = atoi(argv[i]+c);	break;
#ifdef	_DEBUG_
	case 'd':
		debug++;	break;
#endif	_DEBUG_
	case 'm':
		gaussonly++;
		msg("%s no convolution; output: the Gaussian(s)\n", Progname);
		break;
	case 'g':
		if (!argv[i][c]) {	i++; c=0;	}
		gaussize=atof(argv[i]+c);	break;
	case 'w':
		woh++;	break;
	default:
info:		usage_n_options(usage, i, argv[i]);
	}
   }
   else	fname = argv[i];

io_test(fileno(out_fp), goto	info);

if (esigma <= 0.0)	prgmerr('e', "sigma must be positive");
if (masksize < 1)	prgmerr('m', "mask-size must be > 1");
if (ratio < 0.0)	prgmerr('r', "ratio must be >= 0");
if (ratio == 1)		ratio = .99995;

if (checking && !isint) {
	mesg("checking mode ignored; output is not INT.\n");
	checking=0;
	}

io_test(fileno(in_fp), synthetic= !iset);
if (fname)	{
	in_fp = freopen(fname, "rb", stdin);
	if (synthetic = !in_fp)	syserr("input file %s", fname);
	goto	rdh;
}
else if (synthetic)	{	/* no input file */
	edglen = 1;
	if (gaussize)
		edglen = masksize + gaussize;
	else	while(edglen <= masksize)	edglen <<= 1;
	if (!gaussonly)
		fprintf(stderr,"%s: output frame size = %dX%d.\n",
			Progname, edglen, edglen);
	rows = cols = edglen;
	uimg.pxl_out = sizeof(float);
	uimg.o_form = IFMT_FLOAT;
	uimg.frames = 1;
	(*uimg.std_swif)(FI_INIT_NAME, &uimg, "DOG mask", 0);
} else	{
rdh:	(*uimg.header_handle)(HEADER_READ, &uimg, 0, 0);
	if (uimg.in_form != IFMT_BYTE && uimg.in_form !=IFMT_FLOAT)
		prgmerr(1, "image pixel format must be bytes or floats");
    }

fsize = rows*cols;	/* size of one frame */

/* convert number of frames for process to last frame number,
	it has different meaning from 3d_dog.
*/
if (!bgn_frm || bgn_frm>frm)
	bgn_frm=0;
else	bgn_frm--;
if (num_offp && num_offp+bgn_frm < frm)	frm = num_offp + bgn_frm;

if (!ratio)	message("%s: blurring by a Gaussian mask\n", Progname);

if (!gaussonly)	{
	if (uimg.in_form==IFMT_BYTE)
		bbuf = nzalloc(fsize, 1L);
	inbuf = nzalloc(fsize, (MType)sizeof(Float));
	conv1 = nzalloc(fsize, (MType)sizeof(Float));
	obuf = nzalloc(fsize, (MType)sizeof(Float));
	uimg.o_form = IFMT_FLOAT;
	if (isint) {
		int_inbp = (MType *)inbuf;
		int_conv1 = (MType *)conv1;
		int_obp	= (MType *)obuf;
		int_emask = zalloc((MType)masksize, (MType)sizeof(MType));
		int_imask = zalloc((MType)masksize, (MType)sizeof(MType));
		uimg.o_form = IFMT_LONG;
	}
	uimg.pxl_out = 4;
	if (!woh)
		(*uimg.header_handle)(HEADER_WRITE, &uimg, argc, argv, True);
	else	(*uimg.header_handle)(HEADER_FWRITE, &uimg, argc, argv, stderr);
}

emask = zalloc((MType)masksize, (MType)sizeof(Float));
imask = zalloc((MType)masksize, (MType)sizeof(Float));
gauss_mask(esigma,masksize,emask,precision,gaussonly?out_fp:NULL);
if (ratio)
	gauss_mask(esigma*ratio,masksize,imask,precision,gaussonly?out_fp:NULL);

if (gaussonly)	return(0);	/* Done	*/

	if (isint)
	{  for(i=0; i < masksize; i++) {
		int_emask[i] = emask[i] * Range + .5;
		int_imask[i] = imask[i] * Range + .5;
		}
		form_size = sizeof(MType);
	}
	else	form_size = sizeof(Float);

if (bgn_frm)	fseek(in_fp, (bgn_frm)*fsize, 1);

for(fr = bgn_frm; fr < frm; fr++)
   {	if (frm > 1)
		fprintf(stderr,"%s: starting frame #%d\n", Progname, fr);
	if (synthetic) {
		if (edglen & 1)
			synthetic = (rows*cols >> 1) + 1;
		else	synthetic = (rows*cols + cols) >> 1;
		if (isint) {
			for(i=0; i < fsize; i++)	int_inbp[i] = 0;
			int_inbp[synthetic] = 1; /* center = 1 */
		}
		else {
			for(i=0; i < fsize; i++)	inbuf[i] = 0.0;
			inbuf[synthetic] = 1.0;
		}
	}
	else{	/* read one frame image --> memory	*/
		if (uimg.in_form==IFMT_BYTE)
		   {	(*uimg.std_swif)(FI_LOAD_FILE, &uimg, uimg.load_all=0, No);
			if (isint)	/* type convert	*/
			   for(i=0, bp=bbuf, int_inbp=(MType*)inbuf; i<fsize; i++)
				*int_inbp++ = *bp++ & 0xFF;
			else for(i=0, bp=bbuf, inbp=inbuf; i<fsize; i++)
				*inbp++ = *bp++ & 0xFF;
		   }
		else{
			if (upread(inbuf, form_size, fsize, in_fp) != fsize)
				syserr("unexpected EOF");
			if (isint)	 /* type convert; Float -> int */
			   for(i=0,int_inbp=(MType*)(inbp=inbuf); i<fsize; i++)
				*int_inbp++ = *inbp++;
		    }
	    }

	if (checking)
	   for(i=unf=ovf=0,int_inbp=(MType*)inbuf; i<fsize; i++){
		k = *int_inbp++;
		unf += (k < -Range);
		ovf += (k > Range);
	   }
	/*	starting convolution	*/
	if (isint) {
		int_vconvolve(int_emask,masksize,int_inbp,int_conv1,rows,cols);
		int_hconvolve(int_emask,masksize,int_conv1,int_obp,rows,cols);
		if (ratio!=0.0) {
			int_vconvolve(int_imask,masksize,int_inbp,int_conv1,rows,cols);
			int_hconvolve(int_imask,masksize,int_conv1,int_inbp,rows,cols);
		for(i=0, int_obp=(MType*)obuf, int_inbp=(MType*)inbuf; i<fsize; i++)
			*int_obp++ -= *int_inbp++;
		}
	}
	else {
		vconvolve(emask,masksize,inbuf,conv1,rows,cols);
		hconvolve(emask,masksize,conv1,obuf,rows,cols);
		if (ratio) {
			vconvolve(imask,masksize,inbuf,conv1,rows,cols);
			hconvolve(imask,masksize,conv1,inbuf,rows,cols);
			for(i=0, obp=obuf, inbp=inbuf; i < fsize; i++)
				*obp++ -= *inbp++;
		}
	}	/* end of convolution	*/
	fwrite(obuf, form_size, fsize, out_fp);
   }/*	end for frames	*/

if (checking && (ovf || unf))
	prgmerr(0, "%d overflows or %d underflows !!\n", ovf,unf);
}


#ifdef	Faster

int_hconvolve(mask,masksize, inb, outb, r,c)
int	*mask, *inb, *outb;
{
int	row_n,		/* row # -- position	*/
	v_lft_cln,	/* very left column	*/
	i, hrzn_p,	/* working point	*/
	*tmpp, *p, pc,	/* relative column pos	*/
	*bp=outb,
	vlpix, vrpix,
	sum, lweight, rweight,

last_cln= c - 1,
msk2	= masksize / 2,

lftregion = msk2 > c ? c : msk2,
rt_rgn_w = c - (masksize-msk2);	if (rt_rgn_w < lftregion) rt_rgn_w = 0;

for (row_n = v_lft_cln = 0; row_n < r; row_n++, v_lft_cln += c)
  {	tmpp = inb + v_lft_cln;	/* current row & first column	*/
	vlpix = *tmpp;		/* cur row very left side pixel	*/
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
		*bp++ = sum + vlpix*lweight + vrpix*rweight;
	}
	for(; hrzn_p <= rt_rgn_w; hrzn_p++) {
		p = tmpp + hrzn_p - msk2;
		for(i = sum = 0; i < masksize;i++,p++)
			sum += *p * mask[i];
		*bp++ = sum;
	}
	for(; hrzn_p < c; hrzn_p++) {
		pc = hrzn_p - msk2;
		p = tmpp + pc;
		for(i = sum = rweight = 0; i < masksize; i++,pc++,p++) {
			if (pc > last_cln)
				rweight += mask[i];
			else	sum += *p * mask[i];
		}
		*bp++ = sum + vrpix*rweight;
	}
   }
}

int_vconvolve(mask, masksize, inb, outb, r, c)
int	*mask, *inb, *outb;
{
int	row_n, hrzn_p, i,
	sum, bweight, tweight,
	*tmpp, *p, pr, *bp = outb,

msk2	= masksize / 2,
last_row= r - 1,
nd_r_p	= last_row * c,	/* last row (end row) first column	*/

topregion= msk2 > r ? r : msk2,
btm_rgn_h= r-(masksize-msk2); if (btm_rgn_h < topregion) btm_rgn_h = 0;

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
	*bp++ = sum + inb[hrzn_p]*bweight + inb[hrzn_p + nd_r_p]*tweight;
	}
for(; row_n <= btm_rgn_h; row_n++)
    for(hrzn_p = 0, tmpp = inb + (row_n-msk2)*c; hrzn_p < c; hrzn_p++) {
	for(i=sum= 0, p = tmpp + hrzn_p; i < masksize; i++, p+=c)
		sum += *p * mask[i];
	*bp++ = sum;
	}
for(; row_n < r; row_n++)
    for(hrzn_p = 0, tmpp = inb + (row_n-msk2)*c; hrzn_p < c; hrzn_p++) {
	p = tmpp + hrzn_p;
	for(i=sum=tweight= 0,pr = row_n - msk2; i < masksize;i++,pr++,p+=c)
		if (pr > last_row)
			tweight += mask[i];
		else 	sum += *p * mask[i];
	*bp++ = sum + tweight * inb[nd_r_p + hrzn_p];
	}
}

hconvolve(mask,masksize, inb, outb,r,c)
Float	*mask, *inb, *outb;
{
int	row_n, hrzn_p, i, pc,
	v_lft_cln;
Float	vlpix, vrpix,
	sum, lweight, rweight,
	*tmpp, *p,
	*bp = outb;

int
last_cln	= c - 1,
msk2	= masksize / 2,

lftregion= msk2 > c ? c : msk2,
rt_rgn_w= c-(masksize-msk2); if (rt_rgn_w < lftregion) rt_rgn_w = 0;

  for (row_n = v_lft_cln = 0; row_n < r; row_n++, v_lft_cln += c)
  {	tmpp = inb + v_lft_cln;	/* current row & first column	*/
	vlpix = inb[v_lft_cln];	/* cur row very left side pixel	*/
	vrpix = inb[v_lft_cln + c - 1];	/* very rlght side pix	*/
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
		*bp++ = sum + vlpix*lweight + vrpix*rweight;
	}
	for(; hrzn_p <= rt_rgn_w; hrzn_p++) {
		p = tmpp + hrzn_p - msk2;
		for(i = sum = 0; i < masksize;i++,p++)
			sum += *p * mask[i];
		*bp++ = sum;
	}
	for(; hrzn_p < c; hrzn_p++) {
		pc = hrzn_p - msk2;
		p = tmpp + pc;
		for(i = sum = rweight = 0; i < masksize; i++,pc++,p++) {
			if (pc > last_cln)
				rweight += mask[i];
			else	sum += mask[i] * inb[v_lft_cln+pc];
		}
		*bp++ = sum + vrpix*rweight;
	}
   }
}

vconvolve(mask,masksize,inb,outb,r,c)
Float	*mask, *inb,*outb;
{
int	row_n, hrzn_p, i,pr;
Float	sum , bweight , tweight,
	*tmpp, *p,
	*bp = outb;

int	last_row = r - 1,
msk2	= masksize/2,
nd_r_p	= last_row * c,	/* last row (end row) first column	*/

topregion= msk2 > r ? r : msk2,
btm_rgn_h= r-(masksize-msk2); if (btm_rgn_h < topregion) btm_rgn_h = 0;

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
	*bp++ = sum + inb[hrzn_p]*bweight + inb[hrzn_p + nd_r_p]*tweight;
	}
for(; row_n <= btm_rgn_h; row_n++)
    for(hrzn_p = 0, tmpp = inb + (row_n-msk2)*c; hrzn_p < c; hrzn_p++) {
	for(i=sum= 0, p = tmpp + hrzn_p; i < masksize; i++, p+=c)
		sum += *p * mask[i];
	*bp++ = sum;
	}
for(; row_n < r; row_n++)
    for(hrzn_p = 0, tmpp = inb + (row_n-msk2)*c; hrzn_p < c; hrzn_p++) {
	p = tmpp + hrzn_p;
	for(i=sum=tweight= 0,pr = row_n - msk2; i < masksize;i++,pr++,p+=c)
		if (pr > last_row)
			tweight += mask[i];
		else 	sum += *p * mask[i];
	*bp++ = sum + tweight * inb[nd_r_p + hrzn_p];
	}
}


#else	/* easy to read and to understand	*/

int_hconvolve(mask,masksize, inb, outb,r,c)
int	*mask, *inb, *outb;
{
int	row_n,		/* row # -- position	*/
	v_lft_cln,	/* very left column	*/
	i, hrzn_p,	/* working point	*/
	msk2, pc,	/* relative column pos	*/
	*bp=outb,
	vlpix, vrpix,
	sum, lweight, rweight;

msk2	= masksize / 2;

for(row_n = v_lft_cln = 0; row_n < r; row_n++, v_lft_cln += c)
   {
	vlpix = inb[v_lft_cln];		/* very left side pixel	*/
	vrpix = inb[v_lft_cln + c - 1];	/* very rlght side pix	*/
	for(hrzn_p = 0; hrzn_p < c; hrzn_p++) {
		pc = hrzn_p - msk2;
	/* PC moved form left half mask to right half mask	*/
		for(i = sum = lweight = rweight = 0; i < masksize; i++, pc++)
		   {
			if (pc < 0)
				lweight += mask[i];
			else	if (pc < c)
				    sum += mask[i] * inb[v_lft_cln + pc];
			else	rweight += mask[i];
		   }
		*bp++ = sum + vlpix*lweight + vrpix*rweight;
	}
    }
}

int_vconvolve(mask, masksize, inb, outb, r, c)
int	*mask, *inb, *outb;
{
int	row_n, hrzn_p, i, msk2,
	last_row,
	sum , lweight, tweight,
	pr, tp, *bp = outb;

msk2	= masksize / 2;
last_row= (r - 1) * c;

for(row_n = 0; row_n < r; row_n++)
   for(hrzn_p = 0; hrzn_p < c; hrzn_p++)
	{
	for(i=sum=lweight=tweight=0, tp=(row_n-msk2)*c; i<masksize; i++,tp+=c)
	   {	pr = row_n - msk2 + i;
		if (pr < 0)
			lweight += mask[i];
		else	if (pr < r)
			    sum += mask[i] * inb[tp + hrzn_p];
		else	tweight += mask[i];
	   }
	*bp++ = sum + inb[hrzn_p]*lweight + inb[hrzn_p + last_row]*tweight;
	}
}

hconvolve(mask,masksize, inb, outb,r,c)
Float	*mask, *inb, *outb;
{
int	row_n, hrzn_p, i, pc, msk2,
	v_lft_cln;
Float	vlpix, vrpix,
	sum, lweight, rweight,
	*bp = outb;

msk2	= masksize / 2;

for(row_n = v_lft_cln = 0; row_n < r; row_n++, v_lft_cln += c)
   {
	vlpix = inb[v_lft_cln];
	vrpix = inb[v_lft_cln + c - 1];
	for(hrzn_p = 0; hrzn_p < c; hrzn_p++) {
		pc = hrzn_p - msk2;	/* start from over left to right */
		for(i = sum = lweight = rweight = 0; i < masksize; i++, pc++) {
			if (pc < 0)
				lweight += mask[i];
			else	if (pc < c)
				    sum += mask[i] * inb[v_lft_cln + pc];
			else	rweight += mask[i];
		}
		*bp++ = sum + vlpix*lweight + vrpix*rweight;
	}
   }
}

vconvolve(mask,masksize,inb,outb,r,c)
Float	*mask, *inb,*outb;
{
int	row_n, hrzn_p,i, pr, msk2,
	last_row, tp;
Float	sum , bweight , tweight,
	*bp = outb;

msk2	= masksize/2;
last_row= (r-1)*c;

for(row_n = 0; row_n < r; row_n++)
   for(hrzn_p = 0; hrzn_p < c; hrzn_p++) {
	sum = bweight = tweight = 0;
	for(i = 0, tp = (row_n - msk2) * c; i < masksize; i++, tp+=c) {
		pr = row_n - msk2 + i;
		if (pr < 0)
			tweight += mask[i];
		else	if (pr < r)
			    sum += mask[i] * inb[tp + hrzn_p];
		else	bweight += mask[i];
	}
	*bp++ = sum + inb[hrzn_p]*tweight + inb[hrzn_p + last_row]*bweight;
   }
}
#endif	Faster
