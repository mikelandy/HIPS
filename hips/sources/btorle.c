/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * btorle - convert to run-length encoded format for byte images
 *
 * usage:	btorle [-v | -V] <iseq >oseq
 *
 * btorle converts from byte format to RLE (run-length encoded) format.  The
 * RLE format encodes each image row separately.  Runs are encoded as three
 * bytes:  <255><length><byte-value>.  Bytes which are not in a run other
 * than <255> are encoded as themselves.  <255>'s are encoded as a pair of
 * <255>'s.  Thus, a run of two <255>'s will be coded as a run, but the run
 * length must be at least 4 to bother coding any other value.  The length
 * byte is the run-length minus 2 for runs of <255> and the run-length minus 4
 * for runs of any other value.  The largest value of <length> allowed is 254.
 * Each frame is preceded by the byte count for that frame.  If that count
 * is as much or more than the number of bytes in the original input frame,
 * then that frame is left unencoded.  The -v flag prints statistics for
 * every frame and -V prints summary statistics only.
 *
 * pixel formats handled directly: BYTE
 * output format: PFRLE
 *
 * to load:	cc -o btorle btorle.c -lhipsh -lhips
 *
 * Mike Landy - 8/12/94
 */

#define	PFRLE	405	/* temporary hack */

#include <stdio.h>
#include <hipl_format.h>

int types[] = {PFBYTE,LASTTYPE};
static Flag_Format flagfmt[] = {
	{"v",{"V",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"V",{"v",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	LASTFLAG};

int main(argc,argv)

int argc;
char **argv;

{
	int f,fr,i,method,size,r,c,storelen,totlen;
	struct header hd,hdp,hdo;
	Filename filename;
	FILE *fp;
	h_boolean vflag,Vflag;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&vflag,&Vflag,FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	method = fset_conversion(&hd,&hdp,types,filename);
	dup_headern(&hdp,&hdo);
	setformat(&hdo,PFRLE);
	r = hd.orows;
	c = hd.ocols;
	size = r*c;
	i = 1.5*size + 1;	/* worst case size */
	hdo.image = hmalloc((int) i);
	write_headeru(&hdo,argc,argv);
	clearroi(&hdp);
	fr = hdp.num_frame;
	totlen = 0;
	for (f=0;f<fr;f++) {
		fread_imagec(fp,&hd,&hdp,method,f,filename);
		h_btorle(&hdp,&hdo,i,&storelen);
		if (storelen > size)
			storelen = size;
		if (vflag)
			fprintf(stderr,"%s: Frame %d, %d bytes, ratio=%.4f%s\n",
				Progname,f,storelen,
				((float) storelen + sizeof(int))/size,
				(storelen == size) ? ", unencoded" : "");
		totlen += storelen + sizeof(int);
		fwrite(&storelen,sizeof(int),1,stdout);
		if (storelen == size)
			write_image(&hdp,f);
		else {
			hdo.sizeimage = storelen;
			write_image(&hdo,f);
		}
	}
	if (Vflag || (vflag && fr>1))
		fprintf(stderr,"%s: Total of %d bytes, ratio=%.4f\n",
			Progname,totlen,((float) totlen)/(fr*size));
	return(0);
}
