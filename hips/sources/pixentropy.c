/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * pixentropy - compute the entropy of pixels
 *
 * usage:	pixentropy [-p] <iseq
 *
 * If -p is specified, entropy is computed across pairs of pixels
 * (horizontal neighbors), and if the number of columns is odd, the last
 * pixel in each column is ignored.  If -p is not specified, entropy is 
 * computed over isolated pixels.
 *
 * pixel formats handled directly: BYTE
 *
 * to load:	cc -o pixentropy pixentropy.c -lhipsh -lhips
 *
 * Yoav Cohen - 9/20/82
 * HIPS 2 - Michael Landy - 7/5/91
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {
	{"p",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	LASTFLAG};
int types[] = {PFBYTE,LASTTYPE};

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd,hdp;
	int method,fr,f,*table,count;
	Filename filename;
	FILE *fp;
	h_boolean pflag;
	double entropy,h_entropy();

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&pflag,FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	method = fset_conversion(&hd,&hdp,types,filename);
	table = (int *) halloc(pflag ? (256*256) : 256,sizeof(int));
	fr = hdp.num_frame;
	for (f=0;f<fr;f++) {
		fread_imagec(fp,&hd,&hdp,method,f,filename);
		h_entropycnt(&hdp,table,pflag);
	}
	count = pflag ? fr*hdp.rows*(hdp.cols - (hdp.cols % 2))/2
		: fr*hdp.rows*hdp.cols;
	entropy = h_entropy(table,count,pflag);
	fprintf(stderr,"%s: in %d blocks, entropy= %f\n",Progname,count,
		(float) entropy);
	if (pflag)
		fprintf(stderr,"%s: entropy per pixel = %f\n",Progname,
			(float) entropy/2);
	return(0);
}
