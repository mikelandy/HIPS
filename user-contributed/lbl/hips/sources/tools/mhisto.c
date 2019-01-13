/*
% mhisto.c - compute grey-level histograms for all formatted images.
%
%	Copyright (c)	1991	Jin, Guojun
%
% usage:
%	mhisto [-l(#logbins)] [-c] [-m #] [-b # -n #] [-r] [-f -t -z] [-s #]
%		[-a #] [-S [#]] [-v [#]] [-M] [<] image [> [-o] outhist]
% where log(bins) is the log2 of the number of bins. Default bins is 256.	*/
char	usage[]="options\n\
-c causes multiple frame sequences to collapse to a single histogram,\n\
   instead of a separate histogram being generated for each input frame.\n\
	It can be used to compute 3D image histogram.\n\
-l #	bin # in log2 (default=8)\n\
-m #	specify the max output value. The maximum integer input is 2^31.\n\
-1 -2 -a #	any file (1=short, 2=int) with size #\n\
-b #	begin process from #th frame.\n\
-n #	to process # frames.\n\
-f -t	will eliminate the frequent top or zero value count.	\n\
-r	recalculate maximum count for each frame and -R option will be used\n\
	in mdisphist to Retrieve these conuts.\n\
-s #	set display scale (maximum count) for entire histogram.	\n\
-v [#]	dig valley at given position [#], which defaulted at 32, and the valley\n\
	is digged to 2/3 at position left and to 1/3 at its right.\n\
-V #	Valley width. Default = 9.\n\
-z	count zeros. Default is eliminate zero value.	\n\
-M	allow to display some important message.	\n\
-S [#]	smooth the histogram. The smaller following number, the rougher of\n\
	smoothing.	\n\
[<] image [> histogram]	\n";
/*
% compile:	cc -O -o mhisto mhisto.c -lscs3 -lccs -lhips -lrle -ltiff -lm
% note:	there are 2 integers (bin_width and bins) between header and data and
%	1 integer (maxcnt) before each frame.
%
% AUTHOR:	Jin Guojun - LBL	1/8/91
*/

#include "header.def"
#include "imagedef.h"
#include <math.h>

bool	any_pure, Msg, cflag, zflag, recalc, topf, freqf, set_f, smooth;
U_IMAGE	uimg;

#define	inbuf	uimg.src
#define	frame	uimg.frames

#ifndef	MaxBin
#define	MaxBin  131072
#endif
#ifndef	SMOOTH
#define	SMOOTH	17
#endif

struct	{
	MType	max, frm, bin;
	}rec;

struct	{
	int	pos, w;
	}valley={0, 9};

#define	GValue()	arget(ac, av, &f, &fr)


main(ac, av)
int	ac;
char**	av;
{
MType	f, fr, nfrms=0, bgnf=0, MaxVal=256,
	maxcnt=0, secmax, maxpos, secmaxp, numbin=0,
	binwidth,/* reduce resolution. i.e. if binwidth=3, then 1,2,3
			will go to same bin and 4,5,6 will go to same bin */
	fsize, simu_size=0;
register MType	*hist;
float	scale;

format_init(&uimg, IMAGE_INIT_TYPE, HIPS, -1, *av, "S20-1");

for (f=1; f<ac; f++)
    if (*av[f] == '-')	{
	fr = 1;	
	switch(av[f][fr++])	{
	case 'D':debug++;	break;
	case 'M':Msg++;	break;
	case 'S':
		smooth = GValue();
		if (smooth<1 || smooth>SMOOTH)	smooth=5;
		break;
	case '1': case '2':	simu_size = av[f][fr-1] - '0';
	case 'a':	any_pure = GValue();	break;
	case 'b':
		bgnf = GValue() - 1;	break;
	case 'c':
		cflag++;	break;
	case 'f':	freqf++;	break;
	case 'l':
		numbin = 1 << (int)GValue();	break;
	case 'm':
		MaxVal = GValue();	break;
	case 'n':
		nfrms = GValue();	break;
	case 'r':
		recalc++;	break;
	case 's':
		set_f = maxcnt = GValue();	break;
	case 't':
		topf++;	break;
	case 'v':
		valley.pos = GValue();
		if (!valley.pos)	valley.pos = 32;
		break;
	case 'V':
		valley.w = GValue();	break;
	case 'z':
		zflag++;	break;
	case 'o':
		if (out_fp=fopen(av[f]+2, "wb"))	break;
	default:
info:		usage_n_options(usage, f, av[f]);
	}
    }
    else if (!(in_fp=fopen(uimg.name=av[f], "rb")))
		syserr("%s - not found", av[f]);

io_test(fileno(in_fp), goto	info);

if (any_pure > 0)	{
	fsize = uimg.width = any_pure>>simu_size;
	f = frame = uimg.height = 1;
	uimg.pxl_in = 1 << (uimg.in_form=simu_size);
	(*uimg.std_swif)(FI_INIT_NAME, &uimg, uimg.name, 0);
}
else	{
	(*uimg.header_handle)(HEADER_READ, &uimg, 0, 0);
	fsize = uimg.width * uimg.height;
	f = frame;
	if (bgnf<0)	bgnf=0;
	else if(bgnf>=frame)	bgnf = frame-1;
	if (bgnf)	fr = fseek(in_fp, fsize*bgnf, 1);
	if (fr==EOF)	syserr("passing %d first %d frames", fr, bgnf);
	if (Msg)	msg("fp at %d\n", fr);
	if (nfrms<1 || nfrms > frame-bgnf)	nfrms=frame-bgnf;
	if (nfrms)	f = frame = nfrms;/*	display less frames	*/
}
if (uimg.in_form != IFMT_BYTE)
	MaxVal = 65536;
if (!numbin)
	numbin = 256;
else if (numbin > MaxVal)
	numbin = MaxVal;

if (uimg.in_form > IFMT_FLOAT)
	syserr("image format must be lower than float point");

hist = zalloc(numbin, (MType)sizeof(*hist), "hist");

uimg.o_form = IFMT_HIST;
uimg.pxl_out = sizeof(int);

if (cflag)	frame = 1;

binwidth = MaxVal / numbin;
message("maxv=%ld, numbin=%ld, bin_w=%ld, fs=%ld",MaxVal,numbin,binwidth,fsize);
if (valley.pos)
	message(", valley->%d(width=%d)", valley.pos-(valley.w<<1)/3, valley.w);
mesg("\n");

(*uimg.header_handle)(HEADER_WRITE, &uimg, ac, av, True);

fwrite(&binwidth, 1, sizeof(binwidth), out_fp);
fwrite(&numbin, 1, sizeof(numbin), out_fp);

for(fr=0; fr<f; fr++) {
register MType	i;
	(*uimg.std_swif)(FI_LOAD_FILE, &uimg, uimg.load_all=0, No);
	if (recalc)	maxcnt = 1;
	switch(uimg.in_form)
	{
	case IFMT_BYTE:{
		register byte* bp=inbuf;
		register MType	j,k;
			for (i=0; i<fsize; i++) {
				j = *bp++ & 0xFF;
				if (!(j | zflag))	continue;
				k = ++hist[j/binwidth];
				if (k>maxcnt && k>set_f){
				   secmaxp = maxpos;	maxpos = j;
				   if (set_f){
					 secmax = set_f;	set_f = k;
				   }
				   else{ secmax = maxcnt;	maxcnt = k; }
				}
			}
		}
		break;
	case IFMT_LONG: {
		register int*	ip=inbuf, min=16777276, max=0;
		register unsigned short	*bp=inbuf;
			for (i=0; i<fsize; i++, ip++)
				if (*ip < min)	min = *ip;
				else if (*ip > max)	max = *ip;
			i = abs(max - min);
			scale = (float)MaxVal / i;
	if(Msg)	message("min=%d, max=%d, scale=%f\n", min, max, scale);
			ip = inbuf;
			for (i=0; i<fsize; i++)
				*bp++ = (*ip++ - min) * scale + .5;
		}
		goto	Hshort;
	case IFMT_FLOAT:{
		register float*	fp=inbuf, min=1e38, max = -1e38;
		register unsigned short	*bp=inbuf;
			for (i=0; i<fsize; i++, fp++)
				if (*fp < min)	min = *fp;
				else if (*fp > max)	max = *fp;
			i = max - min;
			scale = (float)MaxVal / i;
	if(Msg)	message("min=%f, max=%f, scale=%f\n", min, max, scale);
			fp = inbuf;
			for (i=0; i<fsize; i++)
				*bp++ = (*fp++ - min) * scale + .5;
		}
	case IFMT_SHORT:
Hshort:		{
		register unsigned short	*sp=inbuf;
		register MType	j,k;
			for (i=0; i<fsize; i++) {
				j = *sp++;
				if (!(j | zflag))	continue;
				k = ++hist[j/binwidth];
				if (k>maxcnt && k>set_f){
				   secmaxp = maxpos;	maxpos = j;
				   if (set_f){
					 secmax = set_f;	set_f = k;
				   }
				   else{ secmax = maxcnt;	maxcnt = k; }
				}
			}
		}
		break;
	}
	if (smooth){
	register int	j, s, n = (smooth << 1) + 1;
		for (i=smooth; i<numbin-smooth; i++){
			for (s=0, j = -smooth; j<=smooth; j++)
				s += hist[i+j];
			hist[i] = s / n;
		}
	}
	if (valley.pos){
	register int	i, p=valley.pos - (valley.w<<1)/3;
		for (i=valley.w; i; i--)
			hist[p++] = 0;
	}
	if (topf)	hist[numbin-1] = 0;
	if (freqf && maxpos && maxpos != numbin-1){
		hist[maxpos]=0;
		if (set_f)	set_f=secmax;
		else	maxcnt = secmax;
		maxpos = secmaxp;
	}
	if (set_f > rec.max){
		rec.max = set_f;
		rec.frm = fr;	rec.bin = maxpos;
	}
	else if (!set_f && maxcnt > rec.max){
		rec.max = maxcnt;
		rec.frm = fr;
		rec.bin = maxpos;
	}
	if(Msg)	{
	register int	maxc = set_f ? set_f : maxcnt;
		message("%s:(%03d) max count = %d at postion %d\n",
			*av, fr+1, maxc, maxpos);
	}
	if (!cflag) {
		if (fwrite(&maxcnt, 1, sizeof(maxcnt), out_fp) != sizeof(maxcnt))
			syserr("can not write max count");
		if (fwrite(hist, sizeof(numbin), numbin, out_fp) != numbin)
				syserr("error during write");
		for (i=0; i<numbin; i++)	hist[i] = 0;
	}
    }
if (cflag){
	if (fwrite(&maxcnt, 1, sizeof(maxcnt), out_fp) != sizeof(maxcnt))
		syserr("can not write max count");
	if (fwrite(hist, sizeof(*hist), numbin, out_fp) != numbin)
		syserr("error during write");
}
message("%s: max count = %ld at frame %ld, bin %ld\n",
	*av, rec.max, rec.frm, rec.bin);
}
