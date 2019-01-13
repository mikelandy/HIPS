/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * checkers - make a checkerboard
 *
 * usage:	checkers [-s rows [cols]] [-f frames] >oseq
 *
 * Creates a checkerboard sequence beginning with hips_lchar in the 
 * upper-left corner, alternating hips_lchar with hips_hchar.
 * -s and -f are used to specify the size and number of frames.  The defaults
 * are:  rows=512, cols=rows, frames=1.
 *
 * to load:	cc -o checkers checkers.c -lhipsh -lhips
 *
 * Michael Landy/Yoav Cohen - 5/5/82
 * HIPS 2 - msl - 7/5/91
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {
    {"s",{LASTFLAG},1,{{PTINT,"512","rows"},{PTINT,"-1","cols"},LASTPARAMETER}},
    {"f",{LASTFLAG},1,{{PTINT,"1","frames"},LASTPARAMETER}},
    LASTFLAG};

int main(argc,argv)

int argc;
char **argv;

{
	int rows,cols,frames,f;
	h_boolean highflag;
	struct header hd;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&rows,&cols,&frames,FFNONE);
	if (cols == -1)
		cols = rows;
	init_header(&hd,"","",frames,"",rows,cols,PFBYTE,1,"");
	write_headeru(&hd,argc,argv);
	alloc_image(&hd);
#ifdef ULORIG
	highflag = TRUE;
#else
	highflag = (rows % 2) ? TRUE : FALSE;
#endif
	for (f=0;f<frames;f++) {
		h_checkers(&hd,highflag);
		write_image(&hd);
		highflag = !highflag;
	}
	return(0);
}
