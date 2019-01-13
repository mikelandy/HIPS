/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * repframe - repeat or interpolate frames by different methods.
 *
 * usage:	repframe [-b rate] [-n | -a | -r | -i | -u]
 *
 * Repframe repeats frames by various methods.  Rate is an integer that
 * determines the size of the input-batches. For example, rate=2 means that
 * every second frame is repeated. rate=3 means that 2 out of every three
 * frames are repeats. With the -n switch, rate=2 means that every frame
 * is repeated twice, resulting in more frames in the output sequence.
 *
 *	  The flags: n,r,u,i,a refer to the method for interpolation.
 *
 *		n - new frames are added
 *		r - repetition
 *		u - bit-wise union (or)
 *	 	i - bit-wise intersection (and)
 *		a - weighted averaging
 *
 * pixel formats handled directly:
 * -n/-r:	all raster and pyramid formats
 * -a:		BYTE, SHORT, INT, FLOAT
 * -i/-u:	MSBF, LSBF, BYTE
 *
 * Defaults: rate=2, method=r
 *
 * to load:	cc -o repframe repframe.c -lhipsh -lhips
 *
 * Yoav Cohen 6/8/82
 * modified by Mike Landy 10/18/83
 * Hips 2 - msl - 7/8/91
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {
	{"b",{LASTFLAG},1,{{PTINT,"2","rate"},LASTPARAMETER}},
	{"n",{"r","u","i","a",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"r",{"n","u","i","a",LASTFLAG},0,{{PTBOOLEAN,"TRUE"},LASTPARAMETER}},
	{"u",{"n","r","i","a",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"i",{"n","r","u","a",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"a",{"n","r","u","i",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	LASTFLAG};
int typesnr[] = {PFMSBF,PFLSBF,PFBYTE,PFSBYTE,PFSHORT,PFUSHORT,PFINT,PFUINT,
	PFFLOAT,PFDOUBLE,PFCOMPLEX,PFDBLCOM,PFINTPYR,PFFLOATPYR,LASTTYPE};
int typesa[] = {PFBYTE,PFSHORT,PFINT,PFFLOAT,LASTTYPE};
int typesiu[] = {PFMSBF,PFLSBF,PFBYTE,LASTTYPE};

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd,hdp1,hdp2,hdp3,*first,*last,*temp;
	int method,fr,f,fi,fo,rate,i,nbatch,j;
	Filename filename;
	FILE *fp;
	h_boolean nflag,rflag,uflag,iflag,aflag;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&rate,&nflag,&rflag,&uflag,&iflag,&aflag,
		FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	if (rate < 2)
		perr(HE_MSG,"rate must be at least two");
	if (aflag)
		method = fset_conversion(&hd,&hdp1,typesa,filename);
	else if (iflag || uflag)
		method = fset_conversion(&hd,&hdp1,typesiu,filename);
	else
		method = fset_conversion(&hd,&hdp1,typesnr,filename);
	if (hdp1.numcolor != 1)
		perr(HE_MSG,"Number of color planes must be 1, use subseq");
	if (hgetdepth(&hdp1) != 1)
		perr(HE_MSG,"Number of depth planes must be 1, use subseq");
	fr = hdp1.num_frame;
	if (nflag) {
		hd.num_frame *= rate;
		hdp1.num_frame *= rate;
	}
	else if (fr < rate+1)
		perr(HE_MSG,"sequence too short");
	write_headeru2(&hd,&hdp1,argc,argv,hips_convback);
	clearroi(&hdp1);
	if (nflag) {
		for (f=0;f<fr;f++) {
			fread_imagec(fp,&hd,&hdp1,method,f,filename);
			for (i=0;i<rate;i++)
				write_imagec(&hd,&hdp1,method,hips_convback,f);
		}
	}
	else if (aflag) {
		nbatch = 1 + ((fr-1)/rate);
		dup_headern(&hdp1,&hdp2);
		alloc_image(&hdp2);
		dup_headern(&hdp1,&hdp3);
		alloc_image(&hdp3);
		first = &hdp1;
		last = &hdp3;
		fi = fo = 0;
		fread_imagec(fp,&hd,&hdp1,method,fi++,filename);
		for (i=0;i<nbatch;i++) {
			if (i == nbatch-1)
				rate = fr - (nbatch-1)*rate;
			for (j=1;j<rate;j++)
				fread_imagec(fp,&hd,&hdp2,method,fi++,filename);
			write_imagec(&hd,first,method,hips_convback,fo++);
			if (i != nbatch-1)
				fread_imagec(fp,&hd,last,method,fo++,filename);
			for (j=1;j<rate;j++) {
				if (i == nbatch-1)
					write_imagec(&hd,first,method,
						hips_convback,fo++);
				else {
					h_avg(first,last,&hdp2,(float) rate-j,
						(float) j);
					write_imagec(&hd,&hdp2,method,
						hips_convback,fo++);
				}
			}
			temp = last; last = first; first = temp;
		}
	}
	else if (rflag) {
		dup_headern(&hdp1,&hdp2);
		alloc_image(&hdp2);
		for (f=0;f<fr;f++) {
			if ((f%rate) == 0)
				fread_imagec(fp,&hd,&hdp1,method,f,filename);
			else
				fread_imagec(fp,&hd,&hdp2,method,f,filename);
			write_imagec(&hd,&hdp1,method,hips_convback,f);
		}
	}
	else {	/* -i or -u */
		nbatch = 1 + ((fr-1)/rate);
		dup_headern(&hdp1,&hdp2);
		alloc_image(&hdp2);
		first = &hdp1;
		last = &hdp2;
		fread_imagec(fp,&hd,&hdp1,method,0,filename);
		fi = fo = 0;
		for (i=0;i<nbatch;i++) {
			if (i == nbatch-1)
				rate = fr - (nbatch-1)*rate;
			for (j=1;j<rate;j++)
				fread_imagec(fp,&hd,last,method,fi++,filename);
			write_imagec(&hd,first,method,hips_convback,fo++);
			if (i != nbatch-1) {
				fread_imagec(fp,&hd,last,method,fi++,filename);
				if (iflag)
					h_and(last,first,first);
				else
					h_or(last,first,first);
			}
			for (j=1;j<rate;j++)
			    write_imagec(&hd,first,method,hips_convback,fo++);
			temp = last; last = first; first = temp;
		}
	}
	return(0);
}
