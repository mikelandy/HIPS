/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * gabor.c - create a Gabor packet in x,y,t
 *
 * usage: gabor [-s nr [nc] [-f nf] [-xf xfrq] [-yf yfrq] [-tf tfrq]
 *	[-xm xmean] [-ym ymean] [-tm tmean]
 *	[-xs xsigma] [-ys ysigma] [-ts tsigma]
 *	[-p phase] [-a amplitude] > outseq
 *
 * Number of rows defaults to 64, number of columns to the number of rows, and
 * number of frames to 4.
 * Units:
 *	xf - horizontal frequency in cycles per frame width (default: 0)
 *	yf - vertical frequency in cycles per frame height (default: 0)
 *	tf - temporal frequency in cycles per frame height (default: 0)
 *	xm - horizontal mean pixel position (default: centered)
 *	ym - vertical mean pixel position (default: centered)
 *	tm - temporal mean pixel position (default: centered)
 *	xs - horizontal standard deviation in pixels (default: 10)
 *	ys - vertical standard deviation in pixels (default: 10)
 *	ts - temporal standard deviation in pixels (default: 2)
 *	phase - degrees of phase angle (0 = cosine phase at the mean,
 *		-90 = sine phase at the mean), (default: 0)
 *	amplitude - multiplier (peak value if in cosine phase) (default: 1)
 *
 * to load:	cc -o gabor gabor.c -lhipsh -lhips -lm
 *
 * Charlie Chubb 11/6/86
 * HIPS 2 - msl - 8/11/91
 */

#include <hipl_format.h>
#include	<math.h>

static Flag_Format flagfmt[] = {
	{"s",{LASTFLAG},1,{{PTINT,"64","rows"},{PTINT,"-1","cols"},
		LASTPARAMETER}},
	{"f",{LASTFLAG},1,{{PTINT,"4","frames"},LASTPARAMETER}},
	{"xf",{LASTFLAG},1,{{PTDOUBLE,"0","xfreq"},LASTPARAMETER}},
	{"yf",{LASTFLAG},1,{{PTDOUBLE,"0","yfreq"},LASTPARAMETER}},
	{"tf",{LASTFLAG},1,{{PTDOUBLE,"0","tfreq"},LASTPARAMETER}},
	{"xm",{LASTFLAG},1,{{PTDOUBLE,"-999.","xmean"},LASTPARAMETER}},
	{"ym",{LASTFLAG},1,{{PTDOUBLE,"-999.","ymean"},LASTPARAMETER}},
	{"tm",{LASTFLAG},1,{{PTDOUBLE,"-999.","tmean"},LASTPARAMETER}},
	{"xs",{LASTFLAG},1,{{PTDOUBLE,"10.","xsigma"},LASTPARAMETER}},
	{"ys",{LASTFLAG},1,{{PTDOUBLE,"10.","ysigma"},LASTPARAMETER}},
	{"ts",{LASTFLAG},1,{{PTDOUBLE,"10.","tsigma"},LASTPARAMETER}},
	{"p",{LASTFLAG},1,{{PTDOUBLE,"0.","phase"},LASTPARAMETER}},
	{"a",{LASTFLAG},1,{{PTDOUBLE,"1.","amplitude"},LASTPARAMETER}},
	LASTFLAG};

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd;
	int nr,nc,nf,f;
	double xf,yf,tf,xm,ym,tm,xs,ys,ts,phase,amplitude,t,p2,a2,tssq;
	Pixelval p;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&nr,&nc,&nf,&xf,&yf,&tf,&xm,&ym,&tm,
		&xs,&ys,&ts,&phase,&amplitude,FFNONE);
	if (nc < 0)
		nc = nr;
	if (xm == -999.)
		xm = nc/2.;
	if (ym == -999.)
		ym = nr/2.;
	if (tm == -999.)
		tm = nf/2.;
	p.v_float = 0.;
	init_header(&hd,"","",nf,"",nr,nc,PFFLOAT,1,"");
	write_headeru(&hd,argc,argv);
	alloc_image(&hd);
	tf = 360.*tf/nf;
	tssq = 2.*ts*ts;
	for (f=0;f<nf;f++) {
		h_setimage(&hd,&p);
		t = f - tm;
		p2 = phase + t*tf;
		a2 = amplitude*exp(-t*t/tssq);
		h_addgabor(&hd,&hd,xm,ym,xf,yf,xs,ys,p2,a2);
		write_image(&hd,0);
	}
	return(0);
}
