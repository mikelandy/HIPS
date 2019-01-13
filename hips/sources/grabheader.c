/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * grabheader - grab an image header
 *
 * usage:	grabheader <iseq >oheader
 *
 * to load:	cc -o grabheader grabheader.c -lhips
 *
 * Michael Landy - 5/5/82
 * Hips 2 - msl - 7/5/91
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {LASTFLAG};

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd;
	Filename filename;
	FILE *fp;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,FFONE,&filename);
	fp = hfopenr(filename);
	fread_header(fp,&hd,filename);
	write_header(&hd);
	return(0);
}
