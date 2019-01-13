/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * thin.c - thin white-on-black image.
 *
 * usage:	thin [-t] [-d] [-c] [-m] [-a] [-s] [-v] <in >out
 *
 * This program thins white-on-black images in two ways, and then categorizes
 * the points in the image.  The algorithms are derived from those of
 * Sakai, et. al. (Comp. Graph. and Image Proc. 1, 1972, 81-96).  The program
 * operates in several passes, any combination of which can be chosen with
 * switches:
 *
 *	Switch		Pass
 *	  -t		1) Thin the image by deleting points with 3 to 5
 *			   8-neighbors and 2 transitions.  This pass is repeated
 *			   until no further deletions occur unless the -s 
 *			   (single-pass) switch is given.
 *	  -d		2) Thin the image further, so that diagonal lines are
 *			   at most 1 pixel wide, but 8 connectivity is
 *			   preserved.  Delete pixels which have 2-6 8-neighbors
 *			   and exactly one 8-connected gap in the ring its
 *			   8-neighbors.
 *	  -c		3) Categorize pixels as Endpoints (020), Multiple branch
 *			   points, Isolated points (040), or Uninteresting
 *			   points (010).  Multiple branch points are
 *			   categorized as M's (0200) if 6 or more transitions
 *			   are found, otherwise as MM (0100).
 *	  -m		4) Multiple 8-neighbor MM point groups have an M point
 *			   replace the MM closest to the center of the group.
 *
 * The -a switch implies all four passes. The -v switch (verbose) prints the
 * number of deletions in pass 1, etc.  The -s switch keeps the first two passes
 * from being repeated if changes were made.  If no switches are given, the
 * default is -t -d.
 *
 * to load:	cc -o thin thin.c -lhipsh -lhips -lm
 *
 * Michael Landy - 10/22/82
 * HIPS 2 - msl - 8/4/91
 */

#include <stdio.h>
#include <hipl_format.h>

int types[] = {PFBYTE,LASTTYPE};
static Flag_Format flagfmt[] = {
	{"t",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"d",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"c",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"m",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"a",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"s",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"v",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	LASTFLAG};
h_boolean passflag[4] = {FALSE,FALSE,FALSE,FALSE};

int main(argc,argv)

int argc;
char **argv;

{
	int f,fr,method;
	struct header hd,hdp;
	Filename filename;
	FILE *fp;
	h_boolean tflag,dflag,cflag,mflag,aflag,sflag,vflag;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&tflag,&dflag,&cflag,&mflag,&aflag,
		&sflag,&vflag,FFONE,&filename);
	passflag[0] = tflag || aflag;
	passflag[1] = dflag || aflag;
	passflag[2] = cflag || aflag;
	passflag[3] = mflag || aflag;
	if (!tflag && !dflag && !cflag && !mflag && !aflag)
		passflag[0] = passflag[1] = TRUE;
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	method = fset_conversion(&hd,&hdp,types,filename);
	write_headeru2(&hd,&hdp,argc,argv,hips_convback);
	fr = hdp.num_frame;
	for (f=0;f<fr;f++) {
		fread_imagec(fp,&hd,&hdp,method,f,filename);
		h_thin(&hdp,passflag,sflag,vflag,f);
		write_imagec(&hd,&hdp,method,hips_convback,f);
	}
	return(0);
}
