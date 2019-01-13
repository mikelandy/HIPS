/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * htoi - convert to integer pixel format
 *
 * usage:	htoi <iseq >oseq
 *
 * Convert from any pixel format to integer format.  Also convert from
 * floating point pyramid format to integer pyramid format.
 *
 * to load:	cc -o htoi htoi.c -lhips -lm
 *
 * Mike Landy - 1/8/91
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {LASTFLAG};
int types[] = {PFINT,LASTTYPE};

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd,hdp;
	int method,fr,f,one=1,toplev;
	Filename filename;
	FILE *fp;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	if (hd.pixel_format == PFFLOATPYR) {
		fprintf(stderr,
			"%s: converting from float to integer pyramid\n",
			Progname);
		dup_headern(&hd,&hdp);
		getparam(&hd,"toplev",PFINT,&one,&toplev);
		setpyrformat(&hdp,PFINTPYR,toplev);
		method = METH_INT;
		alloc_image(&hdp);
	}
	else
		method = fset_conversion(&hd,&hdp,types,filename);
	write_headeru(&hdp,argc,argv);
	fr = hdp.num_frame;
	for (f=0;f<fr;f++) {
		fread_imagec(fp,&hd,&hdp,method,f,filename);
		write_image(&hdp,f);
	}
	return(0);
}
