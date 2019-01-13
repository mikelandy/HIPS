/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * greybar - generate a ramp of grey bars
 *
 * usage:	greybar [-s barheight [barwidth]] [-n numbar] [-f frames]
 *			[-nc numcolor]
 *			[-g mingrey maxgrey | -i mingrey step] >oseq
 *
 * Creates a sequence consisting of a ramp of vertical grey bars.  -s
 * specifies the size of each bar (barheight defaults to 512 and barwidth to
 * 16).  -n specifies the number of bars (the default is enough to make the
 * width of the image be 512 or nearly so).  The frame will be output the
 * number of times specified by -f times that specified by -nc.
 * The leftmost bar has greylevel mingrey.
 * The greylevels either step by the amount specified by -i, or so as to
 * reach the maximum level specified by -g.  The default is `-g 0 255'.
 *
 * to load:	cc -o greybar greybar.c -hipsh -lhips
 *
 * HIPS 2 - Michael Landy - 8/16/91
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {
    {"s",{LASTFLAG},1,{{PTINT,"512","barheightr"},
	{PTINT,"16","barwidth"},LASTPARAMETER}},
    {"n",{LASTFLAG},1,{{PTINT,"-1","numbars"},LASTPARAMETER}},
    {"f",{LASTFLAG},1,{{PTINT,"1","frames"},LASTPARAMETER}},
    {"nc",{LASTFLAG},1,{{PTINT,"1","numcolor"},LASTPARAMETER}},
    {"g",{"i",LASTFLAG},2,{{PTBOOLEAN,"TRUE"},{PTINT,"0","mingrey"},
	{PTINT,"255","maxgrey"},LASTPARAMETER}},
    {"i",{"g",LASTFLAG},2,{{PTINT,"-1","mingrey"},{PTINT,"-1","step"},
	LASTPARAMETER}},
    LASTFLAG};

int main(argc,argv)

int argc;
char **argv;

{
	h_boolean gflag;
	int frames,nc,f,bh,bw,nb,gmin,gmax,imin,istep;
	double min,step;
	struct header hd;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&bh,&bw,&nb,&frames,&nc,&gflag,&gmin,&gmax,
		&imin,&istep,FFNONE);
	if (nb < 0)
		nb = 512 / bw;
	if (nb <= 0)
		nb = 1;
	init_header(&hd,"","",frames*nc,"",bh,nb*bw,PFBYTE,nc,"");
	write_headeru(&hd,argc,argv);
	alloc_image(&hd);
	if (gflag) {
		min = gmin;
		step = ((float) gmax - gmin) / (nb - 1);
	}
	else {
		min = imin;
		step = istep;
	}
	h_greybar(&hd,bw,min,step);
	frames *= nc;
	for (f=0;f<frames;f++)
		write_image(&hd);
	return(0);
}
