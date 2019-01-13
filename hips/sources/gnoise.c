/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * gnoise.c - pipe sequence through a channel with Gaussian noise
 *
 * usage: gnoise [-p sd [random-seed]] [-l] [-f]
 *		[-c [rows [cols [frames [colors]]]]]
 *
 * For multiple frames, the normal operation of gnoise is to generate a
 * noise frame for the first frame, and randomly permute its rows and columns
 * for subsequent frames.  The -l switch specifies that a new noise frame
 * should be generated for every frame.  sd is the standard deviation of the
 * Gaussian distribution. -f specifies that a faster (and less accurate)
 * Gaussian deviate generator be used.  The normal operation is to add the
 * noise to the input frame.  If -c is specified, a sequence is generated
 * with mean set by -UL (which defaults to 0) in floating point format.
 *
 * defaults: sd=30.0, random-seed=1, rows=512, cols=rows, frames=1, colors=1
 *
 * pixel formats handled directly: BYTE, FLOAT
 *
 * to load:	cc -o gnoise gnoise.c -lhipsh -lhips -lm
 *
 * Yoav Cohen 3/15/82
 * modified for float images: Mike Landy 10/19/83
 * bug fixed for byte images: Mike Landy 11/4/87
 * Hips 2 - msl - 8/5/91
 * -c flag - msl - 11/13/92
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {
	{"p",{LASTFLAG},1,{{PTDOUBLE,"30.","sd"},{PTINT,"1","seed"},
		LASTPARAMETER}},
	{"l",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"f",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"c",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},{PTINT,"512","rows"},
		{PTINT,"-1","cols"},{PTINT,"1","frames"},{PTINT,"1","colors"},
		LASTPARAMETER}},
	LASTFLAG};
int types[] = {PFBYTE,PFFLOAT,LASTTYPE};

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd,hdp,hdn;
	int method,fr,f,seed,tlclip,thclip,nr,nc,ncol;
	Filename filename;
	FILE *fp;
	h_boolean fflag,lflag,cflag;
	double sd;
	Pixelval val,val2;
	char msg[100];

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&sd,&seed,&lflag,&fflag,
		&cflag,&nr,&nc,&fr,&ncol,FFONE,&filename);
	if (nc < 0)
		nc = nr;
	fr *= ncol;
	if (sd<=0.0)
		perr(HE_MSG,"sd must be >0");
	if (cflag) {
		init_header(&hdp,"","",fr,"",nr,nc,PFFLOAT,ncol,"");
		alloc_image(&hdp);
		write_headeru(&hdp,argc,argv);
		val2.v_float = hips_lchar;
		hips_convback = FALSE;
	}
	else {
		fp = hfopenr(filename);
		fread_hdr_a(fp,&hd,filename);
		method = fset_conversion(&hd,&hdp,types,filename);
		write_headeru2(&hd,&hdp,argc,argv,hips_convback);
		fr = hdp.num_frame;
	}
	if ((fr > 1 && !lflag) || hdp.pixel_format == PFBYTE) {
		dup_headern(&hdp,&hdn);
		if (hdp.pixel_format == PFBYTE)
			setformat(&hdn,PFSHORT);
		setsize(&hdn,hdn.rows,hdn.cols);
		alloc_image(&hdn);
	}
	if (hdp.pixel_format == PFBYTE)
		val.v_short = 0;
	else
		val.v_float = 0.;
	H__SRANDOM(seed);
	if (fflag)
		srand_g(seed);
	tlclip = thclip = 0;
	for (f=0;f<fr;f++) {
		if (cflag)
			h_setimage(&hdp,&val2);
		else
			fread_imagec(fp,&hd,&hdp,method,f,filename);
		hips_lclip = hips_hclip = 0;
		if ((fr == 1 || lflag) && hdp.pixel_format == PFFLOAT) {
			h_gnoise(&hdp,&hdp,sd,fflag);
		}
		else if (lflag || f == 0) {
			h_setimage(&hdn,&val);
			h_gnoise(&hdn,&hdn,sd,fflag);
			h_add(&hdp,&hdn,&hdp);
		}
		else
			h_shuffleadd(&hdp,&hdn,&hdp);
		thclip += hips_hclip;
		tlclip += hips_lclip;
		write_imagec(&hd,&hdp,method,hips_convback,f);
	}
	if (thclip || tlclip) {
		sprintf(msg,"total of %d underflows and %d overflows detected",
			tlclip,thclip);
		perr(HE_IMSG,msg);
	}
	return(0);
}
