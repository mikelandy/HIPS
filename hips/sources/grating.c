/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * grating.c - create a spatiotemporal sinusoidal grating
 *
 * usage: grating [-s nr [nc] [-f nf] [-xf xfrq] [-yf yfrq] [-tf tfrq]
 *				[-p phase] [-a amplitude] > outseq
 *
 * Number of rows defaults to 64, number of columns to the number of rows, and
 * number of frames to 1.
 * Units:
 *	xf - horizontal frequency in cycles per frame width (default: 0)
 *	yf - vertical frequency in cycles per frame height (default: 0)
 *	tf - temporal frequency in cycles per frame height (default: 0)
 *	phase - degrees of phase angle (0 = cosine phase at the mean,
 *		-90 = sine phase at the mean), (default: 0)
 *	amplitude - multiplier (peak value if in cosine phase) (default: 1)
 *
 * to load:	cc -o grating grating.c -lhipsh -lhips -lm
 *
 * HIPS 2 - msl - 8/11/91
 */

#include <hipl_format.h>
#include <math.h>

static Flag_Format flagfmt[] = {
	{"s",{LASTFLAG},1,{{PTINT,"64","rows"},{PTINT,"-1","cols"},
		LASTPARAMETER}},
	{"f",{LASTFLAG},1,{{PTINT,"1","frames"},LASTPARAMETER}},
	{"xf",{LASTFLAG},1,{{PTDOUBLE,"0","xfreq"},LASTPARAMETER}},
	{"yf",{LASTFLAG},1,{{PTDOUBLE,"0","yfreq"},LASTPARAMETER}},
	{"tf",{LASTFLAG},1,{{PTDOUBLE,"0","tfreq"},LASTPARAMETER}},
	{"p",{LASTFLAG},1,{{PTDOUBLE,"0.","phase"},LASTPARAMETER}},
	{"a",{LASTFLAG},1,{{PTDOUBLE,"1.","amplitude"},LASTPARAMETER}},
	LASTFLAG};

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd;
	int nr,nc,nf,f;
	double xf,yf,tf,phase,amplitude;
	Pixelval p;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&nr,&nc,&nf,&xf,&yf,&tf,
		&phase,&amplitude,FFNONE);
	if (nc < 0)
		nc = nr;
	p.v_float = 0.;
	init_header(&hd,"","",nf,"",nr,nc,PFFLOAT,1,"");
	write_headeru(&hd,argc,argv);
	alloc_image(&hd);
	tf = 360.*tf/nf;
	for (f=0;f<nf;f++) {
		h_setimage(&hd,&p);
		h_addcos(&hd,&hd,xf,yf,phase+f*tf,amplitude);
		write_image(&hd,0);
	}
	return(0);
}
