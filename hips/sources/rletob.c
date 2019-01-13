/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * rletoh - convert from run-length encoded format to byte format
 *
 * usage:	rletoh <iseq >oseq
 *
 * rletoh converts from from RLE (run-length encoded) format to byte.  The
 * RLE format encodes each image row separately.  Runs are encoded as three
 * bytes:  <255><length><byte-value>.  Bytes which are not in a run other
 * than <255> are encoded as themselves.  <255>'s are encoded as a pair of
 * <255>'s.  Thus, a run of two <255>'s will be coded as a run, but the run
 * length must be at least 4 to bother coding any other value.  The length
 * byte is the run-length minus 2 for runs of <255> and the run-length minus 4
 * for runs of any other value.  The largest value of <length> allowed is 254.
 * Each frame is preceded by the byte count for that frame.  If that count
 * is as much or more than the number of bytes in the original input frame,
 * then that frame is unencoded.
 *
 * pixel formats handled directly: RLE
 * output format: byte
 *
 * to load:	cc -o rletoh rletoh.c -lhipsh -lhips
 *
 * Mike Landy - 8/12/94
 */

#include <stdio.h>
#include <hipl_format.h>

int types[] = {PFRLE,LASTTYPE};
static Flag_Format flagfmt[] = {LASTFLAG};

int main(argc,argv)

int argc;
char **argv;

{
	int f,fr,i,size,r,c,storelen;
	struct header hd,hdo;
	Filename filename;
	FILE *fp;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,FFONE,&filename);
	fp = hfopenr(filename);
	fread_header(fp,&hd,filename);
	dup_headern(&hd,&hdo);
	setformat(&hdo,PFBYTE);
	alloc_image(&hdo);
	r = hd.orows;
	c = hd.ocols;
	size = r*c;
	i = 1.5*size + 1;	/* worst case size */
	hd.image = hmalloc((int) i);
	write_headeru(&hdo,argc,argv);
	clearroi(&hdo);
	fr = hd.num_frame;
	for (f=0;f<fr;f++) {
		fread(&storelen,4,1,fp);
		if (storelen > i) {
			fprintf(stderr,"input buffer overflow\n");
			exit(1);
		}
		if (storelen >= size)
			fread_image(fp,&hdo,f,filename);
		else {
			hd.sizeimage = storelen;
			fread_image(fp,&hd,f,filename);
			h_rletob(&hd,&hdo,storelen);
		}
		write_image(&hdo,f);
	}
	return(0);
}
