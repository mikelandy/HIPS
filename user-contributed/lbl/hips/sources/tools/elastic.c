/*
%  ELASTIC . C - High performance enhancement tool
%
%	Copyright (c) 1991	Jin, Guojun
%
*/
char	usage[]="options\n\
-o option is used for PC kind computer which requires binary output	\n\
	file modes. No space allowed between switch and filename.	\n\
-v #	maximum value for output.	\n\
-n	negative elastic operation.	\n\
-b	let Elastic scaling to emphasis background. Regularly emphasis	\n\
	the foreground.\n\
-f #	scale factor in percentage number(in integer). default = 0%	\n\
	(useful range from -255[gentle] to +450[sharp])			\n\
-g[#]	base value at bottom. No output value will less than it.	\n\
-a[#]	adjust float input automatically. Default=196. Smaller number	\n\
	yields narrow output range.	\n\
-B -F	force BYTE or FP format output.	\n\
-e	emphasize in high frequency domain.	\n\
-l	linear scaling	\n\
-L name_of_LKT	user Look-Up Table -- 256 entries\n\
-R #	interpolation Regions for both X and Y	\n\
-Rx #	X interpolation regions.\n\
-Ry #	Y interpolation regions.\n\
-S	treat all slices as Single 3D Image (one frame).\n\
-t [#n [#v]]	\n\
	clip top #n(default=1) value to 91%(default) or given value #v.	\n\
-z	option to count zero valued pixels as frequency factor(for -e).	\n\
-r	option only for FAST_Version to get relaxed elastic scale.	\n\
-E	modify Edges on right and bottom\n\
-I	Interpolation in main peak area for BYTE format output. [2 x 2]	\n\
-M	Message switch on.	\n\
-T	generate Table for plot	\n\
[<] in_file [> [-o] output_file]\n", *u_lkt;

/* Note:
%	input can be any format.
%	FP input can only output either BYTE or FP without interpolation.
%
% compile: cc -O -o elastic elastic.c -lscs3 -lccs -lhips -lrle -ltiff -lm
%
% AUTHOR:	Jin Guojun - LBL	1/3/91
*/

#include "header.def"
#include "imagedef.h"
#include <math.h>	/* new_curve required	*/

#define	ADJ_fact	196
#define	GValue()	arget(argc, argv, &i, &j)

/*	GLOBAL variables	*/

bool	adj, bgrd, flr, interpolate, neg, Msg, top, Table;
#ifdef	FAST
bool	revs=0;
#define	VER	"Fast"		/* strange function	*/
#else
#define	VER	"Regular"	/* spring law	*/
#endif
#ifndef	VSC_BASE
#define	VSC_BASE	.01
#endif

bool	D3, edge, hfeq, zflag, zcount;
int	regions, rgn_w, rgn_h, xfrac, yfrac;
float	h_avg, hint, maxout, rel_val;
MType	fsize, sc=0, topv, maxval;
U_IMAGE	uimg;

#define	cols	uimg.width
#define	rows	uimg.height
#define	nfrm	uimg.frames
#define	ibuf	uimg.src
#define	obuf	uimg.dest


main(argc, argv)
int	argc;
char **	argv;
{
#define	p	i

byte	*obp;
int	*ip, *tmpip, ffac;
float	*ofp, fmin, fmin_1, *fip;
InterpMap	*IM;	/* Interpolation Matrix */
MType	frmp, i, j, x_regions=2, y_regions=2;
LKT*	lktp;

format_init(&uimg, IMAGE_INIT_TYPE, HIPS, uimg.o_form = -1, *argv, "S20-1");

for (i=1; i<argc; i++)
    if (*argv[i] == '-')	{
	j = 1;
	switch(argv[i][j++])
	{
	case 'a':if ((adj=GValue()) == 0)
			adj=ADJ_fact;	break;
	case 'b':	bgrd++;	break;
	case 'e':	bgrd--;	break;
	case 'f':	sc = GValue();	break;
	case 'g':	flr = GValue();
		if (!flr)	flr=10;	break;
	case 'E':	edge++;	break;
	case 'L':
		if (avset(argc, argv, &i, &j, 1))
			u_lkt = argv[i] + j;
		break;
	case 'R':{
	register int	who=argv[i][j], num=GValue();
		switch(who){
		case 'x':	x_regions = num;	break;
		case 'y':	y_regions = num;	break;
		default:	x_regions = y_regions = num;
		}
	}
	case 'I':	interpolate++;	break;
	case 'B':	uimg.o_form=IFMT_BYTE;	break;
	case 'F':	uimg.o_form=IFMT_FLOAT;	break;
	case 'M':	message("%s: %s\n", *argv, VER);
		Msg++;	break;
#ifdef	_DEBUG_
	case 'D':	debug++;	break;
#endif
	case 'l':	bgrd=2;	break;	/* linear op	*/
	case 'n':	neg++;	break;
#ifdef	FAST
	case 'r':	revs++;	break;
#endif
	case 'S':	D3++;	break;
	case 't':	top = GValue()-1;
			topv = GValue();
		break;
	case 'T':	Table++;	break;
	case 'v':	maxval = GValue();	break;
	case 'z':
	case 'Z':	zflag++;	break;
	case 'o':
		if (avset(argc, argv, &i, &j, 1) &&
			freopen(argv[i]+j, "wb", stdout))	break;
		message("output file - %s", argv[i]);
	default:
info:		usage_n_options(usage, i, argv[i]);
	}
    }else if ((in_fp=freopen(argv[i], "rb", stdin)) == 0)
		syserr("input file - %s", argv[i]);

io_test(fileno(in_fp), goto	info);

(*uimg.header_handle)(HEADER_READ, &uimg, 0, 0);
fsize = cols * rows;

if (D3)	fsize *= nfrm,	nfrm = 1;

if (uimg.o_form==IFMT_BYTE)
	uimg.pxl_out = 1;
else if (uimg.o_form==IFMT_FLOAT)
	uimg.pxl_out = sizeof(float);
else	uimg.o_form = uimg.in_form,
	uimg.pxl_out = uimg.pxl_in;

if (!maxval)
	switch(uimg.pxl_in){
	case 1:	maxval=255;	break;
	default:maxval=65535;	break;
	}
if (top<0 || top>maxval)	top = 1;
if (top){
	top = maxval - top;
	if (!topv || topv>=maxval)
		topv = maxval * .91;
}
if (bgrd<0 && uimg.in_form>IFMT_BYTE)
    if (uimg.in_form==IFMT_FLOAT)	bgrd = 1;
    else	interpolate++;
interpolate &= (uimg.o_form<=IFMT_LONG && !u_lkt);

ibuf = NZALLOC(fsize, uimg.pxl_in, "ibuf");
uimg.hist = (int*)nzalloc(maxval+1,sizeof(*(uimg.hist)), "hist");

if (!interpolate)
	lktp = (LKT*)(uimg.lkt = ZALLOC(maxval, sizeof(LKT), "lkt"));
if (uimg.in_form != IFMT_LONG)
	tmpip = (int*)ZALLOC(fsize, sizeof(*tmpip), "tmp");
if (u_lkt){
register int	t_l;
FILE	*lkt_fp = fopen(u_lkt, "r");
	if (!lkt_fp)	syserr("user lkt %s", u_lkt);
	t_l = getw(lkt_fp);
	if (t_l > maxval)	t_l = maxval;
	for (i=0; i<t_l; i++)
		fscanf(lkt_fp, "%f", &lktp[i]);
}

if (uimg.pxl_out==1)
	maxout = 255;
else	maxout = maxval;

if (!Table)	(*uimg.header_handle)(HEADER_WRITE, &uimg, argc, argv, True);

if (interpolate){
	if (y_regions<2 || y_regions>(rows>>4) || y_regions>16)
		y_regions = 4;
	if (x_regions<2 || x_regions>(cols>>4) || x_regions>16)
		x_regions = 4;
	message("x regions=%d,	y regions=%d\n", x_regions, y_regions);
	rgn_h = rows / y_regions;
	rgn_w = cols / x_regions;
	xfrac = cols - rgn_w * x_regions;
	yfrac = rows - rgn_h * y_regions;
	regions = (x_regions + (xfrac!=0)) * (y_regions + (yfrac!=0));
	IM = (InterpMap*)NZALLOC(regions, sizeof(*IM), "IM");
	for (i=0; i<regions; i++)
		IM[i].lkt = (LKT*)ZALLOC(maxval, sizeof(LKT), "lkts");
}

if (bgrd>=0 && !interpolate)
	obuf = NZALLOC(fsize, uimg.pxl_out, "obuf");
else	obuf = ibuf;

for (frmp=0; frmp < nfrm; frmp++)
{
int	min=uimg.o_form,	max=uimg.pxl_out;

	if (uimg.in_type==FITS)
		uimg.o_form = uimg.in_form,	uimg.pxl_out = uimg.pxl_in;
	(*uimg.std_swif)(FI_LOAD_FILE, &uimg, NULL, No);
	uimg.o_form = min,	uimg.pxl_out = max;

    if (bgrd>=0 || interpolate){
	ip = tmpip;
	switch(uimg.in_form){
	case IFMT_BYTE:	obp = (byte*)ibuf;
		for (i=fsize; i--;)	*ip++ = *obp++;
		break;
	case IFMT_SHORT: {
	register short	*osp = (short*)ibuf;
		for (i=fsize; i--;)	*ip++ = *osp++;
	}	break;
	case IFMT_LONG:	tmpip = (int*)ibuf;
		break;
	case IFMT_FLOAT:
		maxval = ffind_min_max(ibuf, &fmin, &fmin_1, &max, &ffac);
		uimg.lkt = ZALLOC(maxval, (MType)sizeof(LKT), "fp_lkt");
		if (!u_lkt)	new_curve(uimg.lkt, maxval, 0, max, fsize);
		fip = (float*)ibuf;
		switch(uimg.o_form){
		case IFMT_BYTE:
		    for (p=fsize, obp=(byte*)obuf; p--; fip++)
			*obp++ = lktp[(int)(ffac*(*fip-fmin) * fmin_1)];
		break;
		case IFMT_FLOAT:
		    for (p=fsize, ofp=(float*)obuf; p--; fip++)
			*ofp++ = lktp[(int)(ffac*(*fip-fmin) * fmin_1)];
		}
		free(uimg.lkt);
		goto	wout;
	default:syserr("Only accept B S I F format");
	}
	ip = tmpip;
    }
    else	ip = (int*)ibuf;	/* histo equalization */
    if (interpolate)
	interpolation(ip, obuf, x_regions, y_regions, IM);
    else{
	maxval = Find_min_max(ip, &min, &max);

	/* building new histogram table	*/
	if (!u_lkt)	new_curve(uimg.lkt, maxval, min, max, fsize);

	/* generate new histogram	*/
	if (bgrd<0){
	register byte	*bp = PtrCAST ip;
		for (p=fsize; p--;)
			*bp = lktp[*bp++];
	}
	else switch (uimg.o_form)
	{
	case IFMT_BYTE:
		for (p=fsize, obp=(byte*)obuf; p--;)
			*obp++ = lktp[*ip++ - min] + flr;
		break;
	case IFMT_SHORT: {
	register short	*osp = (short*)obuf;
		for (p=fsize; p--;)
			*osp++ = lktp[*ip++ - min] + flr;
		} break;
	case IFMT_LONG:{
	register int	*iobp=(int*)obuf;
		for (p=fsize; p--;)
			*iobp++ = lktp[*ip++ - min] + flr;
		} break;
	case IFMT_FLOAT:
	default:for (p=fsize, ofp=(float*)obuf; p--;)
			*ofp++ = lktp[*ip++ - min] + flr;
	}
    }
wout:	i = (*uimg.std_swif)(FI_SAVE_FILE, &uimg, uimg.dest, uimg.save_all=0);
}
return	(D3);
}


/* calculate the histogram */
histogram(vbuf, histp, hsize, bytes)
void	*vbuf;
register int	*histp;
{
register int	i;

for (i=maxval; i--;)	histp[i]=0;
switch(bytes){
case 1:	{
register byte	*bp = vbuf;
	for (p=hsize; p--;)	++histp[*bp++];
	}break;
case 2:	{
register short	*sp = vbuf;
	for (p=hsize; p--;)	++histp[*sp++];
	}break;
case 4:	{
register int	*ip = vbuf;
	for (p=hsize; p--;)	++histp[*ip++];
	}break;
default:	syserr("no histo for this format %d", uimg.in_form);
}
}

/*==============================================*
*	computing the min & max for integer	*
*	& trans float to integer for scaling	*
*==============================================*/
find_min_max(bufp, imp, r_width, r_height)
register int	*bufp;
InterpMap	*imp;
{
register unsigned	i, j=0;
register int	*histp=uimg.hist;
for (i=maxval; i--;)
	histp[i] = j;
for (i=r_height; i--; bufp+=cols)
    for (j=r_width; j--;)
	histp[bufp[j]]++;
for (i=0, j=maxval; histp[i]==0 && i<j; i++);
while (histp[--j]==0);
imp->min = i;
imp->max = j;
return	imp->diff = j - i + 1;
}

Find_min_max(buf, min, max)
void	*buf;
int	*min, *max;
{
register int	i, j;

if (uimg.pxl_in==4){
register unsigned	min=65536, max=0;
register int	*ip = buf;
	for (i=fsize; i--;){
		j = *ip++;
		if (j>max)	max = j;
		else if (j<min)	min = j;
	}
	i = min,	j = max;
}
else {
register int	*histp=uimg.hist;
	histogram(buf, histp, fsize, bgrd<0?uimg.pxl_in:sizeof(MType));
	for (i=0, j=maxval; histp[i]==0 && i<j; i++);
	while (histp[--j]==0);
}
*min = i;
*max = j;
return	j - i + 1;
}

ffind_min_max(bufp, fmin, fmin_1, max, ffac)
register float	*bufp, *fmin, *fmin_1;
register int	*max, *ffac;
{
register int	i;
register float	fmax = -3.4e38;
*fmin = 3.4e38;

for (i=0; i<fsize; i++, bufp++)
	if (*bufp > fmax)	fmax = *bufp;
	else if (*bufp < *fmin)	*fmin = *bufp;
if (fabs(fmax) > 1 && fabs(*fmin) > 1)
	*fmin_1 = 1;
else	if (!*fmin)	*fmin_1 = 1e7;
	else	*fmin_1 = fabs(1. / *fmin);
fmax = fabs(*fmin_1 * (fmax - *fmin));

if (adj)	*ffac = adj / fmax;
else if (fmax < 64)	*ffac = ADJ_fact /fmax;
	else	*ffac = 1;
#ifdef	_DEBUG_
message("fmin=%f, fm_1=%f, fmax=%f, fac=%d\n", *fmin, *fmin_1, fmax, *ffac);
#endif
return	*max = *ffac * fmax;
}

/*======================================================*
*	computing the average new value of pixels	*
*======================================================*/

#ifdef	Fast
new_curve(lkt, maxdiff, min, max)
LKT*	lkt;
int	maxdiff;
register int	min, max;
{
int	i;
register float	scale, vsf;

#ifdef	_DEBUG_
if (Msg)
    message("in -> min=%d, max=%d, maxdiff=%d : max_out=%f",
	min, max, maxdiff, maxout);
#endif
	vsf = 0.01 * sc * max;
	scale = maxout / (vsf + ((min + max)*maxdiff >> 1));
#ifdef	_DEBUG_
	if(Msg)	message("	scale=%f, sc=%f\n", scale, vsf);
#endif
	if (revs)
	   if (!bgrd)
		for(i=rel_val=0; i<maxdiff; i++)
		{
		rel_val += (maxdiff-i+vsf) * scale;
		if (rel_val > hint)	lkt[i]=hint;
		else	lkt[i] = rel_val;
		}
	   else for (i=maxdiff, rel_val=hint; i; i--)
		{
		lkt[i] = rel_val;
		rel_val -= (i+vsf)*scale;
		if (rel_val<0 && !uimg.o_form)	rel_val=0;
		}
	else if (neg)
	   if (!bgrd)
		for(i=maxdiff, rel_val=0; i; i--)
		{
		rel_val += (maxdiff-i+vsf) * scale;
		if (rel_val > maxout)	lkt[i]=maxout;
		else	lkt[i] = rel_val;
		}
	   else for (i=0, rel_val=maxout; i<maxdiff; i++)
		{
		lkt[i] = rel_val;
		rel_val -= (i+vsf)*scale;
		if (rel_val<0 && !uimg.o_form)	rel_val=0;
		}
	else if (!bgrd)
		for(i=rel_val=0; i<maxdiff; i++)
		{
		rel_val += (i+vsf) * scale;
		if (rel_val > maxout)	lkt[i]=maxout;
		else	lkt[i] = rel_val;
		}
	   else for (i=maxdiff, rel_val=maxout; i; i--)
		{
		lkt[i] = rel_val;
		rel_val -= (maxdiff-i+vsf)*scale;
		if (rel_val<0 && !uimg.o_form)	rel_val=0;
		}
#else
/*===============================
*	maxout is float point	*
*	maxdiff is max-min+1	*
*	the 1 is a cell for max	*
===============================*/
new_curve(lkt, maxdiff, min, max, hsize)
LKT*	lkt;
MType	maxdiff;
register int	min, max;
{
int	i;

if (bgrd < 0){
int	z, incr;
register int	*histp = uimg.hist;

	/* calculate the average number of pixels per bin */
		if (!zflag){	zcount = histp[0];	histp[0] = 0;	}
		h_avg = (float) (hsize - zcount)/maxout;

	/*==============================================================*
	*	Re-calculate the average number of pixels per bin	*
	*==============================================================*/

		for (z = rel_val = hint = 0; z < maxdiff; z++) {
			hint += histp[z];
			incr = hint/h_avg;
			lkt[z] = rel_val + (incr>>2);
			rel_val += incr;
			hint -= incr*h_avg;
#ifdef	_DEBUG_
		if(Msg)	message("%6.2f(%4d)    ", lkt[z], histp[z]);
#endif
		};
		if (Table)	GTable(maxval, uimg.lkt);
}
else if (bgrd==2){
register float	scale = (maxout - flr) / (max - min);
	for (i=0; i < maxdiff; i++)
		lkt[i] = i * scale + flr;
}
else{
register float	scale, vsc=VSC_BASE*sc/max + 1;
	if (vsc<=0.)	vsc = VSC_BASE;
	{
	register double	tmp;
	if (vsc != 1.)
	{
		for (i=tmp=maxdiff; i--;)	tmp = tmp*vsc + i;
		scale = (maxout-flr) / tmp;
	}
	else	scale = (maxout-flr) / ((maxdiff-1)*maxdiff >> 1);
	if (Msg)
	message("min=%d, max=%d, flr=%d, top=%.3f\n", min, max, flr, maxout);
#	ifdef	_DEBUG_
	message("Scale=%f, C=%f, sc=%f\n", scale, tmp, vsc);
#	endif
	}
	if (neg)
	   if (!bgrd)
		for(i=maxdiff, rel_val=0; i--;)
		{
		rel_val += (maxdiff-i) * (scale*=vsc);
		if (rel_val > maxout)	lkt[i]=maxout;
		else	lkt[i] = rel_val;
		}
	   else for (i=0, rel_val=maxout; i<maxdiff; i++)
		{
		lkt[i] = rel_val;
		rel_val -= i * (scale*=vsc);
		if (rel_val<0 && !uimg.o_form)	rel_val=0;
		}
	else if (!bgrd)
		for(i=1, rel_val=0; i<maxdiff; i++)
		{
		rel_val += i * (scale*=vsc);
		lkt[i] = rel_val;
		}
	   else for (i=maxdiff, rel_val=maxout; i--;)
		{
		lkt[i] = rel_val;
		rel_val -= (maxdiff-i) * (scale*=vsc);
		}
#endif	Fast
	i = maxdiff - 1;
	if (uimg.in_form==IFMT_BYTE){
		if (*lkt < 0){
#	ifdef	_DEBUG_
			msg("lkt[0]=%d\n", *lkt);
#	endif
			*lkt=0;
		}
		else if (*lkt > 255){
#	ifdef	_DEBUG_
			msg("lkt[0]=%d\n", *lkt);
#	endif
			*lkt=255;
		}
		if (lkt[i] > 255)	lkt[i]=255;
		else if (lkt[i]<0)	lkt[i]=0;
	}
}
if (top)
    for (;lkt[i]>top; i--)
	lkt[i] = topv;
if (Table)	GTable(maxdiff, lkt);
if (Msg)	dump_lkt(maxdiff, lkt);
}

dump_lkt(n, lkt)
LKT*	lkt;
{
register int	i;
for (i=0; i<n;){
	message("%10.3f", lkt[i]);	if (!(++i & 7))	mesg("\n");
}
msg("\n%d items\n", i);
}

GTable(max, lkt)
LKT*	lkt;
{
register int i;
register float *lp=lkt;
for (i=0; i<max; i++)
	printf("%d %f\n", i, *lp++);
exit(0);
}

interpolation(in, out, x_regions, y_regions, imp)
int	*in;
byte	*out;
InterpMap	*imp;
{
int	i, j, maxdiff, x, y, mx, my, region;
float	xinc, yinc, xfracinc, yfracinc, xrate, yrate, xcomp, ycomp;
InterpMap	*mlu, *mld, *mru, *mrd, *imu, *imd;

{
register int	*tmpp=in, diff;
for (i=region=maxdiff=0; i<y_regions; i++, tmpp+=(rgn_h-1)*cols){
    for (j=0; j<x_regions; j++, tmpp+=rgn_w, region++){
	diff=find_min_max(tmpp, imp+region, rgn_w, rgn_h);
	if (maxdiff < diff)
		maxdiff = diff;
	/* building new histogram table	*/
	new_curve(imp[region].lkt, imp[region].diff,
		imp[region].min, imp[region].max, rgn_h*rgn_w);
    }
    if (xfrac){
	diff=find_min_max(tmpp, imp+region, xfrac, rgn_h);
	if (maxdiff < diff)
		maxdiff = diff;
	new_curve(imp[region].lkt, imp[region].diff,
		imp[region].min, imp[region].max, rgn_h*xfrac);
	region++;
	tmpp += xfrac;
    }
}
if (yfrac){
    for (j=0; j<x_regions; j++, region++, tmpp+=rgn_w){
	diff=find_min_max(tmpp, imp+region, rgn_w, yfrac);
	if (maxdiff < diff)
		maxdiff = diff;
	new_curve(imp[region].lkt, imp[region].diff,
		imp[region].min, imp[region].max, yfrac*rgn_w);
    }
    if (xfrac){
	diff=find_min_max(tmpp, imp+region, xfrac, yfrac);
	if (maxdiff < diff)
		maxdiff = diff;
	new_curve(imp[region].lkt, imp[region].diff,
		imp[region].min, imp[region].max, yfrac*xfrac);
	tmpp += xfrac;
    }
}
}

{
register int	xd=x_regions + (xfrac!=0);

for (i=regions-(yfrac?xd:0); i--;)	/* difference interpolation */
    if (imp[i].diff<maxdiff && (!xfrac || (i+1)%xd)){
#ifdef	_DEBUG_
	message("lkt%d = %d\n", i, imp[i].diff);
#endif
	for (j=imp[i].diff; j<maxdiff; j++)
		imp[i].lkt[j] = imp[i].lkt[j-1];
    }
}
/* generate new histogram	*/
xinc = 1. / rgn_w;
yinc = 1. / rgn_h;
xfracinc = 1. / xfrac;
yfracinc = 1. / yfrac;
mx = x_regions + (xfrac != 0);
for (i=0; i<y_regions; i++){
    yrate = 1.;
    my = i * mx;
    if (i+1==y_regions)
	mx = -mx;
    imu = imp + my;
    imd = imu + mx;
    for (y=0; y<rgn_h; y++, yrate-=yinc){
	ycomp = 1. - yrate;
	for (j=0; j<x_regions; j++){
	    xrate = 1.;
	    mlu = imu + j;
	    mld = imd + j;
	    if (j+1==x_regions){
		mru = mlu - 1;
		mrd = mld - 1;
	    }
	    else{
		mru = mlu + 1;
		mrd = mld + 1;
	    }
	    for (x=0; x<rgn_w; x++, xrate-=xinc){
	    register int	A, B, C, D, where = *in++;
		A = where - mlu->min;
		B = where - mld->min;
		C = where - mru->min;
		D = where - mrd->min;
		if (A < 0)	A = 0;
		if (B < 0)	B = 0;
		if (C < 0)	C = 0;
		if (D < 0)	D = 0;
		xcomp = 1. - xrate;
		*out++ = xrate * (mlu->lkt[A] * yrate + mld->lkt[B] * ycomp) +
			xcomp * (mru->lkt[C] * yrate + mrd->lkt[D] * ycomp);
	    }
	}
	if (xfrac){
/*
	    xrate = 1.;
	    mlu = imu + j;
	    mld = imd + j;
	    mru = mlu - 1;
	    mrd = mld - 1;
	    for (x=0; x<xfrac; x++, xrate-=xfracinc){
	    register int	A, B, C, D, where = *in++;
		A = where - mlu->min;
		B = where - mld->min;
		C = where - mru->min;
		D = where - mrd->min;
		if (A < 0)	A = 0;
		if (B < 0)	B = 0;
		if (C < 0)	C = 0;
		if (D < 0)	D = 0;
		xcomp = 1. - xrate;
		*out++ = xrate * (mlu->lkt[A] * yrate + mld->lkt[B] * ycomp) +
			xcomp * (mru->lkt[C] * yrate + mrd->lkt[D] * ycomp);
	    }
*/
		for (x=xfrac; x--; *out++ = *in++);
	}
    }
}
if (edge)
if (yfrac){
    yrate = 1.;
	mx = x_regions + (xfrac != 0);
	my = i * mx;
	imu = imp + my;
	imd = imu - mx;
    for (y=0; y<yfrac; y++, yrate-=yfracinc){
	ycomp = 1. - yrate;
	for (j=0; j<x_regions; j++){
	    xrate = 1.;
	    mlu = imu + j;
	    mld = imd + j;
	    if (j+1==x_regions && !xfrac){
		mru = imu + j-1;
		mrd = imd + j-1;
	    }
	    else{
		mru = imu + j+1;
		mrd = imd + j+1;
	    }
	    for (x=0; x<rgn_w; x++, xrate-=xinc){
	    register int	A, B, C, D, where = *in++;
		A = where - mlu->min;
		B = where - mld->min;
		C = where - mru->min;
		D = where - mrd->min;
		if (A < 0)	A = 0;
		if (B < 0)	B = 0;
		if (C < 0)	C = 0;
		if (D < 0)	D = 0;
		xcomp = 1. - xrate;
		*out++ = xrate * (mlu->lkt[A] * yrate + mld->lkt[B] * ycomp) +
			xcomp * (mru->lkt[C] * yrate + mrd->lkt[D] * ycomp);
	    }
	}
	if (xfrac){
	    xrate = 1.;
	    mlu = imu + j;
	    mld = imd + j;
	    mru = imu + j-1;
	    mrd = imd + j-1;
	    for (x=0; x<xfrac; x++, xrate-=xfracinc){
	    register int	A, B, C, D, where = *in++;
		A = where - mlu->min;
		B = where - mld->min;
		C = where - mru->min;
		D = where - mrd->min;
		if (A < 0)	A = 0;
		if (B < 0)	B = 0;
		if (C < 0)	C = 0;
		if (D < 0)	D = 0;
		xcomp = 1. - xrate;
		*out++ = xrate * (mlu->lkt[A] * yrate + mld->lkt[B] * ycomp) +
			xcomp * (mru->lkt[C] * yrate + mrd->lkt[D] * ycomp);
	    }
	}
    }
}
else {
/*	if (xfrac){
	    xrate = 1.;
	    mlu = mru = imu + j-1;
	    mld = mrd = imd + j-1;
	    for (x=0; x<xfrac; x++, xrate-=xfracinc){
	    register int	where = *in++;
		xcomp = 1. - xrate;
		*out++ = xrate * (mlu->lkt[where - mlu->min] * yrate +
				mld->lkt[where - mld->min] * ycomp) +
			xcomp * (mru->lkt[where - mru->min] * yrate +
				mrd->lkt[where - mrd->min] * ycomp);
	    }
	}
    }
}
*/
if (yfrac)
	for (j=cols*yfrac; i--;)
		*out++ = *in++;


}
}
