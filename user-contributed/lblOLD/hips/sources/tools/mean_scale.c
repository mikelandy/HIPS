/*
% MEAN_SCALE.C - scale the image to entire mean of background mean or
%		weighted forground mean value or certain given value.
%
%	Copyright (c)	Jin Guojun
%
% compile:
%	cc -O -o mean_scale mean_scale.c -lscs3 -lccs -lhips -lrle -ltiff -lm
%
% AUTHOR:	Jin Guojun - LBL	1/11/91				*/

char	serbuf[256], usage[]="options\n\
-a	adjust float input automatically.\n\
-A x y [w [h]]	use given Area (center at x..y with size w,h [16x16]) as\n\
	mean-scale reference.	\n\
-B[#]	mean_scale to weight Balanced brightness.\n\
-b[#]	make all background have same value. If further value given	\n\
		immediately, then make background to that value.	\n\
-C \"start_postion square_size\"	\n\
	choose 4 corners, mean them as background value.	\n\
-c	mean_scale to ceiling. default is mean_scale to floor.	\n\
-e[#]	enhanced mode. # is enhance factor for trying on the worst case.\n\
	No necessary to use it at regular time.	\n\
-E #	End frames special. # < 0, 0..#; # > 0, n-#..n	\n\
-F #	overflow value handle Factor. Default=1. If F=1.5, then when	\n\
	overflow happened, that pixel value = max / F. It is used for	\n\
	avoiding too bright in a certain case.		\n\
-f[#]	foreground threshold. A value won't be changed if the value is	\n\
	lower then this threshold.	\n\
-h[#]	height percentage from the top of a hill. It'd better work with	\n\
	-S option. It is defaulted to 50%.	\n\
-l #	\n\
-m #	\n\
	thresholds -- see message at begin of running.	\n\
	The -m is main threshold.	\n\
	-l means that any value lower than -l value # are treated as 0.	\n\
	It is used to build narrow hills.\n\
-n # [#s]	num frames, start frame	\n\
-S[#]	smoothing search. An enhanced method for precisely process.	\n\
	The good range is 1-3. Default=2 (also for enhanced mode	\n\
	without smooth option or smooth factor greater than 3.		\n\
-s #	step (sample width). Default=3. This is a key parameter in	\n\
		MEAN_SCALE processing. The width 2 or 4, sometimes,	\n\
		works better than 3 for BYTE format image. So if result	\n\
		is not desired, try -s 2 or 4 with/without -e options	\n\
		before trying other options.	\n\
-T	Testing for single frame.		\n\
-t	count top values, otherwise omit.	\n\
-V #	valley position. The first valley in main chart is used if -v	\n\
	without -V.\n\
-v[#]	dig valley with width #. Default=samples width.	\n\
-W #	min hill foot width. default=2.5 sample width.	\n\
-w #	min weight of choosing hill.\n\
-z[#]	evaluate zeros. BUT if # given, then not evaluate all value below #\n\
-G	send all information to a file which name is input file name	\n\
	plusing all options and suffix \".rpt\".\n\
-M	print all message on screen.		\n\
-o outfile_name					\n\
	thie is used for PC kind computer which requires binary output	\n\
	file mode.	\n\
[<] in_filename [> out_file]\n";

#include "header.def"
#include "imagedef.h"
#include <math.h>

U_IMAGE	uimg;

#define	ibuf	uimg.src
#define	obuf	uimg.dest
#define	cln	uimg.width
#define	row	uimg.height
#define	pxl_bytes	uimg.pxl_in

#ifdef	IBMPC
#define	MinReset	32767
#else
#define	MinReset	131072
#endif
#ifndef	SampleWidth
#define	SampleWidth	3
#endif
#ifndef	MOSTHILLS
#define	MOSTHILLS	16
#endif
#ifndef	THRESHOLD
#define	THRESHOLD	32
#endif
#ifndef	VISIBLE_LEVEL
#define	VISIBLE_LEVEL	80
#endif

#define	Mean_Scale	2.5
#define	Rate_Width	3
#define	BasicSize	16

typedef	struct	{
	int	lower_p, upper_p,
		peak_p, peak_v,
		weight;
	} W_Spectrum;

typedef	struct	{
	int	nsp, objects,
		hillfoot_w, min_weight, height_rate,
		l_u, lp, rp, weight,
		slope[Rate_Width<<1], sample_w;
		/*	lower, upper, width, p;	*/
	W_Spectrum	*s_array;
	} Info;

typedef	struct	{	/* u is used different from mainpeak	*/
	int	l, m, u,/* any value lower than u will not be changed	*/
		overf;
	} Threshold;

#ifdef	_DEBUG_
extern	int	debug;
#endif
Info	info;
Threshold	threshold={0};
bool	adj, corner, smooth, eEn,
	Msg, Bflag, Zf=16, Test, Top, idpn, errff;
MType	align, frmp, fsize;
int	*hist, maxhisv=256, slope_sets=Rate_Width, Valley, VP,
	Ay, Ax, Ah, Aw=BasicSize, frm, bfrm;
float	*mean, sum, top, enh, ovff=1;
#define	Mean	*mean

#define	GValue()	arget(argc, argv, &i, &frmp)


main(argc, argv)
int	argc;
char **	argv;
{
int	i;

format_init(&uimg, IMAGE_INIT_TYPE, HIPS, -1, *argv, "S20-1");

for (i=1; i<argc; i++)
    if (*argv[i] == '-')	{
	frmp=1;
	switch(argv[i][frmp++])
	{
	case 'a':	adj++;	break;
	case 'A':	Ax = GValue();	Ay = GValue();
			Aw = GValue();	Ah = GValue();
		if (!Aw)	Aw = BasicSize;
		if (!Ah)	Ah = Aw;	break;
	case 'B':	Bflag++;
	case 'b':	align = GValue();
		if (!align)	align--;	break;
	case 'c':	top = .475;	break;
	case 'C':
		if (GValue())
			sscanf(argv[i], "%d %d", &corner, &Aw);
		else	corner = Aw = BasicSize;
		break;
#ifdef	_DEBUG_
	case 'D':	debug++;	break;
#endif
	case 'e':	enh = GValue();
			if (!enh)	enh=Mean_Scale;	break;
	case 'h':	info.height_rate = 100 - GValue();
			if (info.height_rate<5 || info.height_rate>95)
				info.height_rate = 50;	break;
	case 'E':	eEn = GValue();	break;
	case 'F':	ovff = GValue();	break;
	case 'G':	errff++;
	case 'M':	Msg = GValue();
			if (!Msg)	Msg--;	break;
	case 'R':	slope_sets = GValue();	break;
	case 'I':	idpn++;
	case 'S':	smooth = GValue();
			if (smooth<1 || smooth>15) smooth=2;	break;
	case 's':	info.sample_w = GValue();	break;
	case 'T':	Test++;	break;
	case 't':	Top++;	break;
	case 'l':	threshold.l = GValue();	break;
	case 'm':	threshold.m = GValue();	break;
	case 'n':	frm = GValue();
		if (isdigit(argv[i][frmp]))	i++,	bfrm = GValue();
			break;
	case 'f':	threshold.u = GValue();
			if (!threshold.u)	threshold.u=32;	break;
	case 'v':	Valley = GValue() - 1;	break;
	case 'V':	VP = GValue() - 1;	break;
	case 'z':	Zf = GValue() - 1;	break;
	case 'W':	info.hillfoot_w = GValue();	break;
	case 'w':	info.min_weight = GValue();	break;
	case 'o':if (avset(argc, argv, &i, &frmp, 1) &&
			freopen(argv[i]+frmp, "wb", stdout))	break;
		message("output file -- %s", argv[i]);
	default:
errout:		usage_n_options(usage, i, argv[i]);
	}
    }
    else{	if ((in_fp=freopen(argv[i], "rb", stdin))==NULL)
			syserr("input file -- %s", argv[i]);
		strcpy(serbuf, argv[i]);
	}

io_test(fileno(in_fp), goto	errout);

(*uimg.header_handle)(HEADER_READ, &uimg, 0, 0);
uimg.o_form = uimg.in_form;
uimg.pxl_out = uimg.pxl_in;
if (frm < 1)	frm = uimg.frames;
if (bfrm < 0)	bfrm = 0;
fsize = (MType)cln * row;

if (frm<2 && !Test)	syserr("Not a 3D image");
if (errff) {
	if (!strlen(serbuf))
		strcpy(serbuf, "mean_tmp");
	for (i=1; i<argc; i++)
		if (*argv[i]=='-')	strcat(serbuf, argv[i]);
	strcat(serbuf, ".rpt");
	if (!freopen(serbuf, "w", stderr))
		syserr("can't reopen %s for stderr", serbuf);
}

if (uimg.in_form==IFMT_SHORT)
	maxhisv <<= 8;
else if(uimg.in_form)	/*	other non-byte	*/
	maxhisv <<= 10;	/*	1 MByte memory	*/

if (!info.sample_w)	info.sample_w = SampleWidth;
if (!info.hillfoot_w)	info.hillfoot_w = slope_sets*SampleWidth*Mean_Scale;
if (!threshold.m)
	threshold.m = fsize / ((row + cln)*pxl_bytes);
while (threshold.m <= threshold.l)
	threshold.m += threshold.l/pxl_bytes;
if (enh)	{
	threshold.m /= enh;
	if (smooth>>2)	smooth = 2;
}
if (!threshold.l)	threshold.l = threshold.m>>2;
if (Valley<0 || VP<0)	Valley = MAX(info.sample_w * slope_sets, 12);

ibuf = nzalloc(fsize*uimg.frames, pxl_bytes, "ibuf");
obuf = nzalloc(fsize, pxl_bytes, "obuf");
mean = (float*)zalloc(frm+1L, (MType)sizeof(*mean), "mean");
hist = (int*)zalloc(frm+1L, (MType)maxhisv*sizeof(*hist), "hist");

(*uimg.std_swif)(FI_LOAD_FILE, &uimg, uimg.load_all=uimg.frames, No);
#ifdef	_DEBUG_
message("bg=%d, crnr=%d, csz=%d, ceil=%f\n", align, corner, Aw, top);
#endif

{
char	mesgbuf[1024];
sprintf(mesgbuf, "L_threshold=%d, M_threshold=%d, fg=%d, max_v=%d, Hr=%d%%,\
 MinWt=%d\nHill_W=%d, Step_W=%d, Smooth=%d, Enhanced=%.2f\n",
	threshold.l, threshold.m, threshold.u, maxhisv, info.height_rate,
	info.min_weight, info.hillfoot_w, info.sample_w, smooth, enh);
msg("%s", mesgbuf);
(*uimg.header_handle)(ADD_DESC, &uimg, mesgbuf);
}
(*uimg.header_handle)(HEADER_WRITE, &uimg, argc, argv, True);

if (bfrm)	fwrite(ibuf, bfrm * pxl_bytes, fsize, out_fp);
i = bfrm * pxl_bytes * fsize;
#define	i_buf	(char*)ibuf + i
switch(uimg.in_form)
{	case IFMT_BYTE:
		b_get_mean_hist(i_buf);
		b_align(i_buf, obuf);
		break;
	case IFMT_SHORT:
		s_get_mean_hist(i_buf);
		s_align(i_buf, obuf);
		break;
	case IFMT_LONG:
		i_get_mean_hist(i_buf);
		i_align(i_buf, obuf);
		break;
	case IFMT_FLOAT:
		f_get_mean_hist(i_buf);
		f_align(i_buf, obuf);
		break;
	default:syserr("unaccept format");
}
if (bfrm+frm < uimg.frames)
	i = (bfrm+frm) * pxl_bytes,
	fwrite((char*)ibuf + i * fsize, uimg.frames-i, fsize, out_fp);
message("%s to %f\n", *argv, Mean);
#ifdef	_DEBUG_
if (debug)
	for (i=bfrm, frm+=i; i++ < frm;)	{
		message("mean(%2d) = %-f	", i, mean[i]);
		if (!(i % 3))	mesg("\n");
	}
#endif
exit(Mean);
}


/*==============================================*
*	calculate entire & each frame mean	*
*==============================================*/
b_get_mean_hist(buf)
register byte*	buf;
{
register int	i, *hp0, *hp;
hp = hp0 = hist;
if (corner || Ax || Ay)	{
	if (Ax || Ay)
		message("Area = %d %d (%d x %d)\n", Ax, Ay, Aw, Ah);
	for (i=sum=0; i<frm; i++, hp+=maxhisv, buf += fsize)
	    if (corner)
		sum += (mean[i+1] = corner_bg_sample(buf, hp, corner, Aw));
	    else {
		int	vp=0, min, max;
		register int	r, c;
		register byte*	bp = buf + Ay*cln + Ax;
			for (r=Ah; r--; bp+=cln)
			    for (c=Aw; c--;)
				hp[bp[c]]++;
			find_mostvp(hp, &vp, &min, &max, maxhisv);
			sum += (mean[i+1] = vp);
		}
	Mean = sum / i;
	return;
}
for (frmp=sum=0; frmp < frm; frmp++)
   {	register int	itmp;
	hp += maxhisv;
	for (i=itmp=0; i<fsize; i++)	{
		hp0[*buf]++;	hp[*buf]++;
		itmp += *buf++;
	}
	sum += (mean[frmp+1] = (float)itmp / fsize);
   }
Mean = sum / frmp;
if (align)	get_sample(THRESHOLD);
}

s_get_mean_hist(buf)
register short*	buf;
{
register int	i, itmp, *hp0, *hp;
hp = hp0 = hist;
for (frmp=sum=0; frmp < frm; frmp++)
   {	hp += maxhisv;
	for (i=itmp=0; i<fsize; i++)	{
		hp0[*buf]++;	hp[*buf]++;
		itmp += *buf++;
	}
	sum += (mean[frmp+1] = (float)itmp / fsize);
   }
Mean = sum / frmp;
if (align)	get_sample(THRESHOLD);
}

i_get_mean_hist(buf)
register int*	buf;
{
register int	i, itmp, *hp0, *hp;
hp = hp0 = hist;
for (frmp=sum=0; frmp < frm; frmp++)
   {	hp += maxhisv;
	for (i=itmp=0; i<fsize; i++){
		hp0[*buf]++;	hp[*buf]++;
		itmp += *buf++;
	}
	sum += (mean[frmp+1] = (float)itmp / fsize);
   }
Mean = sum / frmp;
if (align)	get_sample(THRESHOLD);
}

f_get_mean_hist(buf)
register float*	buf;
{
register int	i;
register float	ftmp;
if (align)
   msg("warning: No others applied for FP. Using scale to integer before %d\n",
	Progname);
for (frmp=sum=0; frmp < frm; frmp++)
   {	for (i=ftmp=0; i<fsize; i++)	ftmp += *buf++;
	sum += (mean[frmp+1] = ftmp / fsize);
   }
Mean = sum / frmp;
}

/*==============================================*
*	make all frames have same mean value	*
*	or peak position or center of gravity	*
*==============================================*/
b_align(ibp, bbuf)
register byte	*ibp;
byte	*bbuf;
{
register int	i, itmp, diffmean;
register byte	*obp;
for (frmp=0; frmp < frm; frmp++)
    {	obp = bbuf;
	diffmean = Mean - mean[frmp+1] + top;
	if (!diffmean){
		obp = ibp;	ibp += fsize;
	if(Msg)	message("no change for frame %-3d[%d]\n", frmp+1, diffmean);
	}
	else {
	if(Msg)	message("mean difference(%d) = %d\n", frmp+1, diffmean);
		for (i=0; i<fsize; i++)
		{
		    if (*ibp < threshold.u)	{
			*obp++ = *ibp++;
			continue;
			}
		    else{
			itmp = diffmean + *ibp++;
			if (itmp > 255)	*obp = 255/ovff;
			else if (itmp < 0)	*obp = 0;
			else	*obp = itmp;
			obp++;
			}
		}
		obp =bbuf;
	}
	i = fwrite(obp, pxl_bytes, fsize, out_fp);
	if (i != fsize)	syserr("write mean data %d", i);
    }
}

s_align(ibp, sbuf)
register unsigned short	*ibp;
unsigned short	*sbuf;
{
register MType	i, itmp, diffmean;
register unsigned short	*obp;
for (frmp=0; frmp < frm; frmp++)
    {	obp = sbuf;
	diffmean = Mean - mean[frmp+1] + top;
	if (!diffmean)	{
		obp = ibp;	ibp += fsize;
	if(Msg)	message("no change for frame %d\n", frmp+1);
	}
	else {	for (i=0; i<fsize; i++)
		{
		    if (*ibp < threshold.u)	{
			*obp++ = *ibp++;
			continue;
			}
		    else{
			itmp = diffmean + *ibp++;
			if (itmp > 65535)	*obp = 65535/ovff;
			else if (itmp < 0)	*obp = 0;
			else	*obp = itmp;
			obp++;
			}
		}
		obp = sbuf;
	}
	if (fwrite(obp, pxl_bytes, fsize, out_fp) != fsize)
		syserr("write short");
    }
}

i_align(ibp, buf)
register int	*ibp;
int	*buf;
{
register int	i, diffmean, *obp;
for (frmp=0; frmp < frm; frmp++)
    {	diffmean = Mean - mean[frmp+1] + top;
	obp = buf;
	if (!diffmean)	{
		obp = ibp;	ibp += fsize;
	if(Msg)	message("no change for frame %d\n", frmp+1);
	}
	else{	for (i=0; i<fsize; i++)
			*obp++ = diffmean + *ibp++;
		obp = buf;
	}
	if (fwrite(buf, pxl_bytes, fsize, out_fp) != fsize)
		syserr("write integer");
    }
}

f_align(ibp, fbuf)
register float	*ibp;
float	*fbuf;
{
register int	i;
register float	diffmean, *obp;
for (frmp=0; frmp < frm; frmp++)
    {	obp = fbuf;
	diffmean = Mean - mean[frmp+1] + top;
	if (!diffmean)	{
		obp = ibp;	ibp += fsize;
	if(Msg)	message("no change for frame %d\n", frmp+1);
	}
	else{	for (i=0; i<fsize; i++)
			*obp++ = diffmean + *ibp++;
		obp = fbuf;
	}
	if (fwrite(obp, pxl_bytes, fsize, out_fp) != fsize)
		syserr("write float");
    }
}

/*======================================*
%	find the most frequency value h	%
%	and its position (*mostvp), and	%
%	effective hill feet (min, max)	%
*======================================*/
find_mostvp(hp, mostvp, min, max, n)
register int	*hp, *mostvp;
int	*min, *max, n;
{
register int	h, i = *mostvp;

while (!hp[i])	i++;
h = hp[i],	*min = i;
do if (hp[i] > h)
	h = hp[*mostvp = i];
while(++i < n);

while(!hp[--i]);
*max = i;
#ifdef	_DEBUG_
if(Msg)	message("min=%d, max=%d, vp=%d, v=%d\n", *min, *max, *mostvp, h);
#endif
return	h;
}

get_sample(T)
{
int	*hp=hist, f;
    if (Bflag)	{
	int	t;

	info.s_array = zalloc((MType)sizeof(*info.s_array), (MType)MOSTHILLS);
	for (f=0; f<=frm; f++)	{
	register int	i;
		if (f)	{
			if (Valley)
			    for (i=0; i<Valley; i++)
				hp[VP+i] = 0;
			hills(hp, maxhisv, (MType)MOSTHILLS, &info, &threshold, smooth, f);
		}
		else	{
		int	s = (smooth!=0) + (frm<8);
#ifdef	NO_STRUCT_AUTOINIT
		Threshold	TH;
			TH.l = threshold.l * frm;
			TH.m = threshold.m * frm;
#else
		Threshold TH = threshold;
			TH.l *= frm;
			TH.m *= frm;
#endif
			hills(hp, maxhisv, (MType)MOSTHILLS, &info, &TH, s, f);
			if(!VP)	VP = info.s_array[0].upper_p - (Valley<<1)/3;
			if(Valley) message("Dig valley(%d) at %d\n",Valley,VP);
		}
		if (info.nsp)	{
		register int	Width=0, Weight=0, weight=0;
		/*	find a hill which satisfies hill foot width	*/
			for (i=info.nsp; i-- &&
			    (Width<info.hillfoot_w || Weight<info.min_weight);){
				Weight = info.s_array[i].weight;
				info.rp = info.s_array[i].upper_p;
				info.lp = info.s_array[i].lower_p;
				Width = info.rp - info.lp;
			}
			t = i+1;
			if (info.height_rate)	{
				i=info.s_array[i].peak_v*info.height_rate/100;
				while (hp[info.lp]<i)
					info.lp++;
				while (hp[info.rp]<i)
					info.rp--;
				Weight=0;	/* recalc total weight	*/
				for (i=info.lp; i<=info.rp; i++)
					Weight += hp[i];
			}
		/*	calc center of weight	*/
			Weight >>= 1;	weight = 0;
			for (i=info.lp; weight < Weight; i++)
				weight += hp[i];
			mean[f] = --i;
		if(Msg)	message("WtCnt(%d)=%d, hw=%d, lp=%d, wp=%d, up=%d, hp=%u\n\n",
			t, Weight, weight, info.lp, i, info.rp, hp);
		}
		else	msg("no hill found in frame %d\n", f);
		hp += maxhisv;
	}
	if (align > VISIBLE_LEVEL)
		Mean = align;
    }else{
	int	min, max, mostvp=T, mp;

	/*	get entire background value	*/
	find_mostvp(hp, &mostvp, &min, &max, maxhisv);

	/*	get bg value for each frame	*/
	for(f=0; f<frm; f++)
	   {	hp += maxhisv;
		mp = T;
		find_mostvp(hp, &mp, &min, &max, maxhisv);
		mean[f+1] = mp;
	   }

	if (align > 0)	mostvp = align;
	align=Mean;	Mean = mostvp;
	if (mostvp > 192 || abs(mostvp-align) > 32){
	msg("warning: may not properly scale to background\nentire BG = %d",
		mostvp);
	msg("	mean=%d : try using -C \"margin sample_size\" or -w\n",align);
		for (f=0; f<frm;){
			message("f%-4dBG = %-8.3f", f+1, mean[f+1]);
			if (!(++f % 4))	mesg("\n");
		}
		return	0;
	}
#	ifdef	_DEBUG_
	if (Msg){
		msg("entire most value is %d\n", mostvp);
		for (f=0; f<frm;){
			msg("frame %-3d bg=%-04.0f", f+1, mean[f+1]);
			if (!(++f % 4))	mesg("\n");
		}
		mesg("\n");
	}
#	endif
	return	min;
    }
}

/*	collect given area pixel and compute mean as background value	*/
corner_bg_sample(buf, hp, margin, sam_sz)
register byte*	buf;
register int	*hp;
int	margin, sam_sz;
{
int	i, j;

    buf += (cln+1) * margin;
    for (i=0; i<sam_sz; i++) {
	for (j=0; j<sam_sz; j++)
		hp[*buf++]++;
	buf += cln - ((margin + sam_sz) << 1);
	for (j=0; j<sam_sz; j++)
		hp[*buf++]++;
	buf += margin << 1;
    }
    buf += (row - ((margin + sam_sz) << 1)) * cln;
    for (i=0; i<sam_sz; i++) {
	for (j=0; j<sam_sz; j++)
		hp[*buf++]++;
	buf += cln - margin - sam_sz;
	for (j=0; j<sam_sz; j++)
		hp[*buf++]++;
	buf += margin << 1;
    }
    margin=0;
    find_mostvp(hp, &margin, &i, &j, maxhisv);
return	margin;
}

/*======================================================*
*	find group of hills & their weight in a frame	*
*	peak_v < pixel_number/((r+c) * bytes_per_pixel)	*
*	valley => 5 is suggested (range 1 - ...)	*
*======================================================*/
get_slope(ra_p)
int	*ra_p;
{
register int	i, sum;
for (i=sum=0; i<slope_sets; i++, ra_p++)
	sum += Sign(*ra_p);
return	sum;
}

hills(hbuf, bwidth, sets, ip, trshold, smooth, f)
int	*hbuf, bwidth;	/*	histo buffer width	*/
MType	sets;	/*	total spectrums required	*/
Info*	ip;
Threshold	*trshold;
{
register int	*hp=hbuf;
int	leftdir, lslope, predir=1, rightdir=0, rslope=0, ap=0, rp=slope_sets,
	*minp0, *minp1, *maxp, samples=0,
	main_threshold = trshold->m, lower_t = trshold->l;
unsigned	min=0, max=0;
bool	peakflag=0;

    if (f+eEn < 0 || f+eEn > frm){
	main_threshold /= 2.5;
	lower_t >>= 1;
    }
    if (Zf>0){
	register int	i;
	for (i=0; i<Zf; i++)
	hp[i] = 0;	/* reset zero's value	*/
    }
    else	msg("zeros = %d\n", *hp);
    if (!Top)
	for(lslope=3; lslope; lslope--)
		hp[bwidth - lslope] = 0;
    if (smooth){	/*	SMOOTH HISTOGRAM	*/
	register int	i, j, s, n = (smooth << 1) + 1;
	if (idpn)
	    for (i=smooth; i<bwidth-smooth; i++){
		for (s=0, j = -smooth; j<=smooth; j++)
			s += hp[i+j];
		hp[i] = s / n;
	    }
	else	{
	int *tbuf = (int*)zalloc((MType)bwidth, (MType)sizeof(*tbuf), "tbuf");
	register int*	thp = tbuf + smooth;
	    for (i=smooth; i<bwidth-smooth; i++){
		for (s=0, j = -smooth; j<=smooth; j++)
			s += hp[i+j];
		*thp++ = s / n;
	    }
	    memcpy(hp, tbuf, bwidth*sizeof(*hp));
	    free(tbuf);
	}
    }
    while (!*hp)	hp++;	/*	pass non value zone	*/
    minp1 = hp-1;
    {	register int	i;	/* set beingning slope	*/
	for (i=0; i<slope_sets; i++){
		ip->slope[i] = *(hp+ip->sample_w) - *hp;
		if (*hp > max)	/* get approximately	*/
		{	max = *hp;	maxp = hp;	}
		hp += ip->sample_w;
	}
    }
    rp = slope_sets;
    leftdir = get_slope(ip->slope);
    if (leftdir<0)	goto	peakset;/* if down hill, set having a peak	*/
    for (; hp<bwidth+hbuf && ap < sets; hp++){
	if (min>*hp && *hp>lower_t)
	{	min = *hp;	minp0 = hp;
		if (peakflag)	minp1 = minp0;
	}
	else if (*hp<lower_t && !rslope)	minp1 = hp;
	else if (*hp>max && rslope>=0)
	{	max = *hp;	maxp = hp;	}
	if (++samples < ip->sample_w)
		continue;	/* not sample enough for slope	*/
	ip->slope[rp++] = *hp - *(hp-samples);	/* interpolate slope	*/
	samples=0;
	if (rp < (slope_sets << 1))
		continue;	/* not sample enough for a group slope	*/
	rp = slope_sets;
	memcpy(ip->slope, ip->slope+rp, rp*sizeof(*ip->slope));
	rightdir = get_slope(ip->slope+rp);	/* calc current slope	*/
	lslope = Sign(leftdir);	rslope = Sign(rightdir);
	if (lslope == rslope)	continue;
	if (rslope)
	if (lslope || !lslope && Sign(predir)!=rslope){
		if (rightdir > leftdir){	/* valley	*/
ahill:			ip->s_array[ap].upper_p = minp0 - hbuf;
			max=0;	/*	change valley	*/
		   if (peakflag){
			ip->s_array[ap].weight = 0;
			for (maxp=ip->s_array[ap].lower_p+hbuf;maxp<=minp0;)
				ip->s_array[ap].weight += *maxp++;
			ap++;
			peakflag=0;
		   }
		}
		else if (max > main_threshold){	/*	reach top	*/
peakset:		peakflag++;
			ip->s_array[ap].peak_v = max;
			ip->s_array[ap].peak_p = maxp - hbuf;
			ip->s_array[ap].lower_p = minp1 - hbuf;
			min = *hp;	minp0 = minp1 = hp;
		}
	}	/* end if lslope	*/
	else	if (rslope > 0)
			max = 0;
		else	min = *hp;
	if (leftdir)	predir = leftdir;/* common exit after rslope	*/
	leftdir = rightdir;
    }
    if (peakflag)	goto	ahill;
    hills_dump(ip->s_array, ap, Msg);
    if(Msg){
	if (hp < bwidth+hbuf)
		msg("There are more than %d", ap);
	else	msg("Total %d Distinguishable", ap);
	msg(" Peaks in (%d)\n", f);
    }
    ip->nsp = ap;
}

hills_dump(array, nsp, nMsg)
W_Spectrum*	array;
unsigned	nsp, nMsg;
{
register unsigned	i, j=MIN(nsp, nMsg);
for (i=0; i<j; i++, array++)
	message("PEAK%02d: lp->%-4d: rp->%-4d|=| pp->%-4d: pv->%-4d, w=%d\n", i+1,
		array->lower_p, array->upper_p, array->peak_p, array->peak_v,
		array->weight);
}
