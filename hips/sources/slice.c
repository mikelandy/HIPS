/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * slice - display slices through an image as a graph
 *
 * usage:	slice [-h] [-v] [-p row-or-column] < seq > graphseq
 *
 * The -h flag specifies a horizontal slice through the image (which is the
 * default), and -v specifies a vertical slice.  The grey values in the row
 * or column specified in the command are displayed as a bar graph.
 * For horizontal slices, the output number of columns is 2 wider than the
 * input region-of-interest, and there are 257 rows (one for each possible
 * nonzero grey level plus 2 for a border).  For vertical slices, the same
 * thing is reflected.  The graph border value is 128, and the foreground
 * and background values may be specified with the standard switches -UL
 * and -UH, which default to 0 and 255 as usual.
 * 
 * to load:	cc -o slice slice.c -lhipsh -lhips -lm
 *
 * Michael Landy - 8/4/87
 * HIPS 2 - msl - 6/29/91
 */

#include <stdio.h>
#include <hipl_format.h>
int types[] = {PFBYTE,LASTTYPE};
static Flag_Format flagfmt[] = {
    {"h",{"v",LASTFLAG},0,{{PTBOOLEAN,"TRUE"},LASTPARAMETER}},
    {"v",{"h",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
    {"p",{LASTFLAG},1,{{PTINT,"0","row-or-column-position"},LASTPARAMETER}},
    LASTFLAG};

int main(argc,argv)

int argc;
char **argv;

{
	h_boolean vflag,hflag;
	int posn,f,fr,method;
	struct header hd,hdp,hdo;
	Filename filename;
	FILE *fp;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&hflag,&vflag,&posn,FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	method = fset_conversion(&hd,&hdp,types,filename);
	dup_headern(&hdp,&hdo);
	if (vflag)
		setsize(&hdo,hd.rows+2,257);
	else
		setsize(&hdo,257,hd.cols+2);
	alloc_image(&hdo);
	write_headeru(&hdo,argc,argv);
	fr = hdp.num_frame;
	for (f=0;f<fr;f++) {
		fread_imagec(fp,&hd,&hdp,method,f,filename);
		h_slice(&hdp,&hdo,posn,vflag);
		write_image(&hdo,f);
	}
	return(0);
}
