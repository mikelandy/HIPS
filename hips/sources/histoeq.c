/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * histoeq - histogram equalization
 *
 * usage:	histoeq [-n] <iseq >oseq
 *
 * pixel formats handled directly: BYTE
 *
 * to load:	cc -o histoeq histoeq.c -lhipsh -lhips
 *
 * Hips 2 - msl - 8/7/91
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {
	{"n",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	LASTFLAG};
int types[] = {PFBYTE,LASTTYPE};

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd,hdp;
	int method,fr,f,count;
	Filename filename;
	FILE *fp;
	h_boolean nzflag;
	Pixelval min,max;
	struct hips_histo histo;
	byte map[256];

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&nzflag,FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	method = fset_conversion(&hd,&hdp,types,filename);
	write_headeru2(&hd,&hdp,argc,argv,hips_convback);
	min.v_byte = 0;
	max.v_byte = 255;
	histo.histodealloc = FALSE;
	alloc_histo(&histo,&min,&max,256,PFBYTE);
	fr = hdp.num_frame;
	for (f=0;f<fr;f++) {
		fread_imagec(fp,&hd,&hdp,method,f,filename);
		h_clearhisto(&histo);
		h_histo(&hdp,&histo,nzflag,&count);
		h_histoeq(&histo,count,map);
		h_pixmap(&hdp,&hdp,map);
		write_imagec(&hd,&hdp,method,hips_convback,f);
	}
	return(0);
}
