/*
@	MAINPEAK.C -- find main image objects.
@
@	Copyright (c) 1991	Jin, Guojun
@
@ Usage:
@	mainpeak [-lmuw#] [-LnU#] [-M [#]] [-O#] [-p#] [-z] [-F] [-a] [-c] [-e]
@		[-fu] [<] infilename [> [-o] filename]				*/

char	usage[]="Options\n\
-F	force same format of output as input. Default output BYTE format.\n\
	Inputs are in BYTE, SHORT, LONG(INT), and FLOAT formats\n\
-M	Display all important information.\n\
-g|d #	to reset good or bad range.\n\
-a	do automatical adjustment.\n\
-c[#]	clip top value down to 0 or [#] for non_BYTE format;\n\
	if a number [#] given, it must follow -c without space.\n\
-e	for preprocessed or enhanced image to adjust good and bad range.\n\
-fu	FITS unix type\n\
-L|U #	shift to #th Lower or Upper object.\n\
-n #	output # peaks from lower one.\n\
-O #	Offset for both ends. This convenient way to increase image range. \n\
-p #	maximum peaks for one frame. Default is 48 for BYTE; 192 for SHORT &\n\
	768 for LONG. When message \"There are more than xxx peaks in frame #\"\n\
	given and picture is not satisfied, use this option to increase the\n\
	peak bins to get more peaks.	\n\
-lmu #	lower, middle & upper threshold value for frequency domain.	\n\
-w #	hill foot width. default is 0 -- find widest one. Good value is > 256.\n\
-z	count zero value.	\n\
-o	option is used for PC kind computer which requires binary output\n\
	file modes. No space allowed between switch and filename.	\n\
[<] input [> output]\n", Dsrp[384], mbuf[128];
/*
@ compile: cc -O -o mainpeak mainpeak.c -lscs3 -lccs -lhips -lrle -ltiff -lm
@
@ AUTHOR:	Guojun Jin - Lawrence Berkeley Laboratory	1/22/91
*/

#include "header.def"
#include "imagedef.h"
#include <math.h>

U_IMAGE	uimg;

#define	ipxl_bytes	uimg.pxl_in
#define	cln	uimg.width
#define	row	uimg.height
#define	frm	uimg.frames

#define	GValue(frp)	arget(argc, argv, &i, &frp)

#ifndef	GoodRange	/* used for find correct object spatial	domain	*/
#define	GoodRange	512
#endif
#ifndef	BadRange
#define	BadRange	48
#endif
#ifndef	MOSTPEAKS	/* most peaks = MOSTPEAKS * pixel_bytes ^ 2	*/
#define	MOSTPEAKS	48
#endif

typedef	struct	{	/* to store each peak's information	*/
	int	lower_p, upper_p,
		peak_p, peak_v;	/* pos & value */
	} Peak;

typedef	struct	{	/* system control block	*/
	int	nclip, nsp, objects,
		hillfoot_w, doffset,
		l_u, lp, rp,
		lower, upper, width, p;
	Peak*	s_array;
	} Info;

typedef	struct	{
	int	l, m, u,
		overf;
	} Threshold;

#ifdef	_DEBUG_
extern	int	debug;
#endif

MType	frp, fsize,
	goodrange=GoodRange,
	badrange = BadRange;
float	scale=1., fmin, fscale();
Info	info = {255, 0}; /* stupid SUN cc compiler can not initialize in main */
Threshold threshold = {5, 0};
bool	Msg=0, Zf=0, adj=0, pre_enh=0;


main(argc, argv)
int	argc;
char **	argv;
{
int	i, MostObjs=0;
MType	*hist, maxhisv=256;
VType	*buf;

format_init(&uimg, IMAGE_INIT_TYPE, HIPS, -1, *argv, "S20-1");

for (i=1; i<argc; i++)
    if (*argv[i] == '-') {
	frp = 1;
	switch(argv[i][frp++])
	{
	case 'F':	uimg.o_form=-1;	break;
#ifdef	_DEBUG_
	case 'D':	debug++;	break;
#endif
	case 'a':	adj++;	break;
	case 'b':	badrange = GValue(frp);	break;
	case 'c':	info.nclip=GValue(frp);	break;
	case 'e':	pre_enh++;	break;
#ifdef	FITS_IMAGE
	case 'f':{
extern	int	FTy;	FTy = 'u';	break;
	}
#endif
	case 'g':	goodrange = GValue(frp);
		break;
	case 'M':	if (argv[i][frp])
				Msg = atoi(argv[i]+frp);
			else	Msg--;	break;
	case 'L':	info.l_u = -GValue(frp);	break;
	case 'U':	info.l_u =  GValue(frp);	break;
	case 'n':	info.objects = GValue(frp);	break;
	case 'O':	info.doffset = GValue(frp);	break;
	case 'l':	threshold.l = GValue(frp);	break;
	case 'm':	threshold.m = GValue(frp);	break;
	case 'u':	threshold.u = GValue(frp);	break;
	case 'p':	MostObjs = GValue(frp) - 1;	break;
	case 'w':	info.hillfoot_w = GValue(frp);	break;
	case 'z':	Zf++;	break;
	case 'o':if (freopen(argv[i]+2, "wb", stdout))	break;
		message("output file -- %s", argv[i]);
	default:
errout:		usage_n_options(usage, i, argv[i]);
	}
    }
    else if (freopen(argv[i], "rb", stdin) != stdin)
		syserr("input file -- %s", argv[i]);
io_test(stdin_fd, goto	errout);

if ((*uimg.header_handle)(HEADER_READ, &uimg, 0, 0))
	syserr("unknown image type");
fsize = cln * row;
if (ipxl_bytes == 2)
	maxhisv <<= 8;
else if(ipxl_bytes == 4)
	maxhisv <<= 10;	/* may use 12 - 16 for some reason */
if (uimg.o_form < 0)
	uimg.o_form = uimg.in_form,
	uimg.pxl_out = uimg.pxl_in;
else	uimg.o_form = IFMT_BYTE,
	uimg.pxl_out = 1;

if (!MostObjs)	MostObjs = MOSTPEAKS * ipxl_bytes * ipxl_bytes;
if (!threshold.m)
	threshold.m = fsize / ((row + cln)*ipxl_bytes);
while (threshold.m <= threshold.l)
	threshold.m += threshold.l / ipxl_bytes;
if (!threshold.u)
	threshold.u = (1 << 31) - 1;
else if (threshold.u < threshold.m)
	threshold.u += threshold.m;
if (pre_enh){
	goodrange <<= ipxl_bytes;
	badrange <<= ipxl_bytes;
	threshold.m >>= 1;
}

if(Msg)	message("l_threshold=%d, m_threshold=%d, u_threshold=%d, maxhv=%d\n",
		threshold.l, threshold.m, threshold.u, maxhisv);
if (uimg.in_type != HIPS){
register int	mo_form = uimg.o_form, mo_pxl = uimg.pxl_out;
	if (uimg.in_type == FITS)
		uimg.o_form = uimg.in_form,	uimg.pxl_out = uimg.pxl_in;
	(*uimg.std_swif)(FI_LOAD_FILE, &uimg, uimg.load_all=uimg.frames, No);
	uimg.o_form = mo_form,	uimg.pxl_out = mo_pxl;
}
else	buf = uimg.src = nzalloc(fsize, ipxl_bytes, "buf");

hist = zalloc(maxhisv, (MType)sizeof(*hist), "hist");
uimg.dest = uimg.src;

for (frp=0; frp<frm; frp++){
	if (uimg.in_type != HIPS)
		buf = (byte *) uimg.src + frp * fsize;
	else	(*uimg.std_swif)(FI_LOAD_FILE, &uimg, uimg.load_all=0, No);
	switch(uimg.in_form){
	case IFMT_BYTE:
		b_get_hist(buf, hist);
		spectrum(hist, maxhisv, MostObjs, &threshold, &info);
		i = widest_sp(&info, maxhisv);
		if (i)	b_clip(buf, info.lp, info.rp);
		break;
	case IFMT_SHORT:
		s_get_hist(buf, hist);
		spectrum(hist, maxhisv, MostObjs, &threshold, &info);
		i = widest_sp(&info, maxhisv);
		if (i)	s_clip(buf, info.lp, info.rp, info.nclip);
		break;
	case IFMT_LONG:
		i_get_hist(buf, hist);
		spectrum(hist, maxhisv, MostObjs, &threshold, &info);
		i = widest_sp(&info, maxhisv);
		if (i)	i_clip(buf, info.lp, info.rp, info.nclip);
		break;
	case IFMT_FLOAT:
		scale = 1. / fscale(buf, maxhisv, &fmin);
		i_get_hist(buf, hist);
		spectrum(hist, maxhisv, MostObjs, &threshold, &info);
		i = widest_sp(&info, maxhisv);
		if (i)	i_clip(buf, info.lp, info.rp, info.nclip);
		break;
	default:syserr("unaccept format %d", uimg.in_form);
	}
	if (!i)	msg("no object found in frame(%d)", frp);
	if (uimg.in_form != uimg.o_form)
		tobyte(buf, ipxl_bytes);
	else if (uimg.o_form == IFMT_FLOAT)
		itof(buf);
	if (frp==0){
		(*uimg.header_handle)(ADD_DESC, &uimg, Dsrp);
		(*uimg.header_handle)(HEADER_WRITE, &uimg, argc, argv, True);
	}
	(*uimg.std_swif)(FI_SAVE_FILE, &uimg, buf, uimg.save_all=0);
	for (i=0; i<maxhisv; i++)	hist[i]=0;
}
exit(0);
}

/*======================================*
*	find histgram for each frame	*
*======================================*/
b_get_hist(ibp, hstp)
register byte*	ibp;
register int*	hstp;
{
register int	i;
for (i=0; i<fsize; i++)	hstp[*ibp++]++;
}

s_get_hist(ibp, hstp)
register unsigned short	*ibp;
register int*	hstp;
{
register int	i;
for (i=0; i<fsize; i++)	hstp[*ibp++]++;
}

i_get_hist(ibp, hstp)
register int*	ibp, *hstp;
{
register int	i;
for (i=0; i<fsize; i++)	hstp[*ibp++]++;
}

/*======================================*
*	discard unwanted objects	*
*======================================*/
b_clip(bp, lp, rp)
register byte*	bp;
register int	lp, rp;
{
register int	i;

for (i=0; i<fsize; i++, bp++)
	if (*bp < lp)	*bp = lp;
	else if (*bp > rp)	*bp = rp;
}

s_clip(bp, lp, rp, top)
register unsigned short	*bp;
register int	lp, rp, top;
{
register int	i;

for (i=0; i<fsize; i++, bp++)
	if (*bp < lp)	*bp = lp;
	else if (*bp > rp)	*bp = top;
}

i_clip(bp, lp, rp, top)
register unsigned*	bp;
register int	lp, rp, top;
{
register int	i;

for (i=0; i<fsize; i++, bp++)
	if (*bp < lp)	*bp = lp;
	else if (*bp > rp)	*bp = top;
}


/*======================================================*
*	find group of hills in a frame			*
*	peak_v < pixel_number/((r+c) * bytes_per_pixel)	*
*	valley => 5 is suggested (range 1 - ...)	*
*======================================================*/
spectrum(buf, bwidth, sets, tp, ip)
MType	*buf, bwidth;
int	sets;
Threshold*	tp;
Info*	ip;
{
register MType	hp = -1, ap;
Peak*	array=zalloc(sizeof(*array), sets);

if (!Zf)	*buf = 0;	/* reset zero's value	*/
else	msg("zeros = %d\n", *buf);
while (!buf[++hp]);	/* find lowest bound */
if (scale==1. && !fmin)
	sprintf(Dsrp, "%d -- ", hp);
else	sprintf(Dsrp, "%f -- ", hp*scale+fmin);

for (ap=0; hp<bwidth && ap < sets; hp++){
register int	hval;
	hval = buf[hp];
	/*	find widest unthreshold body for adjusting	*/
	if (hval && !ip->lower)	ip->lower = hp;
	else if (!hval && ip->lower){
		ip->upper = hp;
		if (ip->upper - ip->lower > ip->width)
			ip->width = ip->upper - ip->lower;
		ip->lower = 0;
	}
	/* CALC spectrums.	If over lower threshold	*/
	if (hval > tp->l){
		if (!array[ap].lower_p)
			array[ap].lower_p = hp;	/* get entrance	*/
		if (hval > tp->u)
			tp->overf++;
		if (hval > tp->m && hval > array[ap].peak_v){
			array[ap].peak_p = hp;	/* get peak point	*/
			array[ap].peak_v = hval;
		}
	}/*	ELSE if lower than threshold	*/
	else if (array[ap].lower_p)
		if (array[ap].peak_v && !tp->overf)	/* if passed or had peak	*/
			array[ap++].upper_p = hp;	/* get exit	*/
		else	/* reset entrance	*/
			array[ap].lower_p=array[ap].peak_v=tp->overf=0;
}
if (array[ap].peak_v && !array[ap].upper_p)
	array[ap++].upper_p = hp;
if(ap){
	ip->s_array = zalloc((MType)ap, (MType)sizeof(*array));
	memcpy(ip->s_array, array, ap*sizeof(*array));
}
free(array);
if (scale==1. && !fmin)
	sprintf(mbuf, "%d: ", ip->upper);
else	sprintf(mbuf, "%f: ", ip->upper*scale+fmin);
strcat(Dsrp, mbuf);
ip->lower = ip->upper - ip->width;
spectrum_dump(ip->s_array, ap, Msg);
if (hp == bwidth)
	sprintf(mbuf, "Total %d Distinguishable", ap);
else	sprintf(mbuf, "There are more than %d", ap);
strcat(Dsrp, mbuf);
sprintf(mbuf, " Peak(s) in frame %d\n", frp+1);
strcat(Dsrp, mbuf);
mesg(Dsrp);
ip->nsp = ap;
}

spectrum_dump(array, nsp, nMsg)
Peak*	array;
unsigned	nsp, nMsg;
{
unsigned	i, j=MIN(nsp,nMsg);
for (i=0; i<j; i++, array++)
	message("PEAK%02d: lp->%d : up->%d |=| pp->%d : pv->%d\n", i+1,
		array->lower_p, array->upper_p, array->peak_p, array->peak_v);
}


/*======================================================*
*	find peaks which satisfy the conditions set	*
*	in infomation structure. If the peak bottom	*
*	smaller than condition & no more objects	*
*	specified, this program will try to find a	*
*	good range to display the main image.		*
*======================================================*/
bool	widest_sp(IFp, Top)
register Info*	IFp;
MType	Top;
{
register Peak*	spp = IFp->s_array;
register int	i, tmp, maxw=spp->upper_p - spp->lower_p;
int	sp=0;

for (i=1; i<IFp->nsp; i++){
	spp++;
	tmp = spp->upper_p - spp->lower_p;
	if (maxw < tmp){
		maxw = tmp;	sp=i;
	}
}
if (!IFp->hillfoot_w && maxw < badrange && !IFp->objects){
	if (Msg) {
		sprintf(mbuf, "parameter automtically adjusted: brange=%d, grange=%d\n",
			badrange, goodrange);
		strcat(Dsrp, mbuf);
		mesg(mbuf);
	}
	if (adj && IFp->width > maxw)
		sp = IFp->lower;
	sp += IFp->l_u;
	if (sp < 0)	sp = 0;
	else if (sp > IFp->nsp)
		sp = IFp->nsp-1;
	if (maxw==1 && !sp)	/* for large spread, start from middle */
		sp = IFp->nsp >> 1;
	IFp->lp = IFp->s_array[tmp=sp].lower_p;

	if (scale==1. && !fmin)
	    sprintf(mbuf, "Base: width = %d(%d -- %d), sp=%d\n", maxw,
		IFp->s_array[tmp].lower_p, IFp->s_array[tmp].upper_p, sp);
	else sprintf(mbuf, "Base: width = %.4f(%.4f -- %.4f), sp=%d\n",
		maxw*scale, IFp->s_array[tmp].lower_p*scale+fmin,
		IFp->s_array[tmp].upper_p*scale+fmin, sp);
	strcat(Dsrp, mbuf);
	mesg(mbuf);

	{
	register int	offset=0;
	    while (maxw < goodrange && (tmp || sp < IFp->nsp)){
		if(tmp)	{	/* lower bound check */
			tmp--;
			if (pre_enh && IFp->lp - IFp->s_array[tmp].upper_p > badrange)
#ifdef	SOMEHOW
				offset += IFp->lp-IFp->s_array[tmp].upper_p+badrange;
#else
				offset += badrange;
#endif
			IFp->lp = IFp->s_array[tmp].lower_p;
		}
		if(sp<IFp->nsp){/* upper bound check */
		    if (pre_enh && IFp->rp)
			if (IFp->s_array[sp].lower_p - IFp->rp > badrange)
#ifdef	SOMEHOW
				offset += IFp->s_array[sp].lower_p-IFp->rp+badrange;
#else
				offset += badrange;
#endif
			IFp->rp = IFp->s_array[sp++].upper_p;
		}
		maxw = IFp->rp - IFp->lp - offset;
	    }
	}
	IFp->objects = sp-- - ++tmp;
}
else{
	tmp = sp + IFp->l_u;
	if (tmp + IFp->objects >= IFp->nsp)
		tmp = IFp->nsp-1-IFp->objects;
	if (tmp < 0){	tmp = 0;	IFp->objects = sp;	}
	else if(tmp >= IFp->nsp)
		{	tmp = IFp->nsp-1;	IFp->objects=1;	}
	IFp->lp = IFp->s_array[tmp].lower_p;
	IFp->rp = IFp->s_array[tmp++ + IFp->objects].upper_p;
}
IFp->lp -= IFp->doffset;
if (IFp->lp < 0)	IFp->lp = 0;
IFp->rp += IFp->doffset;
if (IFp->rp > Top)	IFp->rp = Top;
if (IFp->nclip)	IFp->nclip=IFp->rp;
if (scale==1. && !fmin)
    sprintf(mbuf, "Output from #%d to #%d bandwidth %d--%d\n",
	tmp, IFp->objects+tmp, IFp->lp, IFp->rp);
else sprintf(mbuf, "Output from #%d to #%d bandwidth %.4f--%.4f\n",
	tmp, IFp->objects+tmp, IFp->lp*scale+fmin, IFp->rp*scale+fmin);
strcat(Dsrp, mbuf);
mesg(mbuf);
return	(maxw >= IFp->hillfoot_w);
}


/*======================================*
*	find the min & max for float	*
*	& scale float to integer	*
*======================================*/
float	fscale(buf, maxfv, fmin)
void	*buf;
float	*fmin;
{
register int	i;
register float	*bp=buf;
register float	max = -3.4e38,
		min = 3.4e38;
for (i=0; i<fsize; i++, bp++)
	if (*bp > max)	max = *bp;
	else if (*bp < min)	min = *bp;
#ifdef	_DEBUG_
	message("Min=%f, Max=%f, ", min, max);
#endif
   {
	register int*	ibp=buf;
	max = maxfv / (max - min); /* around 1 MBYTE */
#ifdef	_DEBUG_
	msg("Scale=%f\n", max);	/* use max register as scale register */
#endif
	for (i=0, bp=buf; i<fsize; i++)
		*ibp++ = (*bp++ - min) * max + 0.5;
   }
*fmin = min;
return	max;	/* scale */
}

tobyte(buf, ibytes)
void	*buf;
{
register int	i;

if (ibytes == 2){	/*	short => integer convert	*/
register unsigned short	*sbp = buf, min=65535;
   {	register unsigned short	max=0;
	for (i=0; i<fsize; i++, sbp++)
		if (*sbp > max)	max = *sbp;
		else if (*sbp < min)	min = *sbp;
	i = max - min;
	if(Msg)	message("SHORT to BYTE (%d -- %d)\n", min, max);
   }{
	register float	scale = 255. / i;
	register byte*	bp = buf;
	for (i=0, sbp=buf; i<fsize; i++)
		*bp++ = (*sbp++ - min) * scale + 0.5;
   }
}
else{
register int*	ibp = buf, min=1048575;
   {	register int	max = -32768;
	for (i=0; i<fsize; i++, ibp++)
		if (*ibp > max)	max = *ibp;
		else if (*ibp < min)	min = *ibp;
	i = max - min;
	if(Msg)	message("long to BYTE (%d -- %d)\n", min, max);
   }{
	register float	scale = 255. / i;
	register byte*	bp = buf;
	for (i=0, ibp=buf; i<fsize; i++)
		*bp++ = (*ibp++ - min) * scale + 0.5;
   }
}
}

itof(buf)
void	*buf;
{
register int	*ip=buf, i;
register float	*op=buf;

for (i=fsize; i--;)
	*op++ = *ip++;
}
