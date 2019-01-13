/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * disphist - display histogram files as a bar graph
 *
 * usage:	disphist [-p percent | -m maxcnt] [-w barwidth] [-h barheight]
 *			 [-b borderwidth] [-g bordergreylevel] <hist >graph
 *
 * The -m flag specifies an initial maximum bincount for use in scaling the
 * displays.  The -p flag specifies a percentage of the total histogram area
 * (not including underflow/overflow bins) which should be filled with bin
 * data.  The default is -p 30.  If the number of counts per bar specified by
 * -m or -p is insufficient, the counts per bar is set to the maximum number
 * of counts in a given frame's histogram (and a message is printed to that
 * effect).  Each histogram bar is barwidth pixels wide and barheight pixels
 * tall.  Barwidth defaults to 1 and barheight defaults to 256.  The
 * entire histogram has a border of width borderwidth (which defaults to 1
 * pixel wide) with grey level bordergreylevel (default: 128).  A strip of
 * width borderwidth and grey level bordergreylevel also separates the main
 * histogram from the underflow and overflow bins.  The main histogram bars
 * have greylevel hips_lchar on a background of hips_hchar.  The underflow and
 * overflow bars have a background of bordergreylevel; they disappear into the
 * background if empty.  The output image size is thus barheight+2*borderwidth
 * rows and (nbins+2)*barwidth+4*borderwidth columns.  The background and
 * foreground greylevels may be set using -UH and -UL, which default to 255
 * and 0 as usual.
 * 
 * to load:	cc -o disphist disphist.c -lhipsh -lhips -lm
 *
 * Michael Landy - 12/15/82
 * HIPS 2 - msl - 7/2/91
 * Added -p/-b/-g etc. - msl - 1/20/92
 */

#include <stdio.h>
#include <hipl_format.h>
int types[] = {PFHIST,LASTTYPE};
static Flag_Format flagfmt[] = {
    {"m",{"p",LASTFLAG},1,{{PTBOOLEAN,"FALSE"},
	{PTINT,"0","maxcount"},LASTPARAMETER}},
    {"p",{"m",LASTFLAG},1,{{PTBOOLEAN,"TRUE"},
	{PTDOUBLE,"30","percentarea"},LASTPARAMETER}},
    {"w",{LASTFLAG},1,{{PTINT,"1","barwidth"},LASTPARAMETER}},
    {"h",{LASTFLAG},1,{{PTINT,"256","barheight"},LASTPARAMETER}},
    {"b",{LASTFLAG},1,{{PTINT,"1","borderwidth"},LASTPARAMETER}},
    {"g",{LASTFLAG},1,{{PTINT,"128","bordergrey"},LASTPARAMETER}},
    LASTFLAG};

int main(argc,argv)

int argc;
char **argv;

{
	int f,fr,maxcnt,barwidth,barheight,currmax,i1,i2;
	int borderw,borderg,i,j;
	float f1,f2;
	double percent;
	struct header hd,hdo;
	struct hips_histo histo;
	Filename filename;
	FILE *fp;
	char msg[150];
	h_boolean mflag,pflag;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&mflag,&maxcnt,&pflag,&percent,
		&barwidth,&barheight,&borderw,&borderg,FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_cpfa(fp,&hd,types,filename);
	dup_headern(&hd,&hdo);
	setformat(&hdo,PFBYTE);
	hdr_to_histo(&hd,&histo);
	alloc_histobins(&histo);
	setsize(&hdo,barheight+2*borderw,(histo.nbins+2)*barwidth+4*borderw);
	alloc_image(&hdo);
	write_headeru(&hdo,argc,argv);
	fr = hd.num_frame;
	switch (histo.pixel_format) {
	case PFBYTE:	i1 = histo.minbin.v_byte;
			i2 = histo.binwidth.v_byte;
			break;
	case PFSBYTE:	i1 = histo.minbin.v_sbyte;
			i2 = histo.binwidth.v_sbyte;
			break;
	case PFSHORT:	i1 = histo.minbin.v_short;
			i2 = histo.binwidth.v_short;
			break;
	case PFUSHORT:	i1 = histo.minbin.v_ushort;
			i2 = histo.binwidth.v_ushort;
			break;
	case PFINT:	i1 = histo.minbin.v_int;
			i2 = histo.binwidth.v_int;
			break;
	case PFUINT:	i1 = histo.minbin.v_uint;
			i2 = histo.binwidth.v_uint;
			break;
	case PFFLOAT:
	case PFCOMPLEX:	f1 = histo.minbin.v_float;
			f2 = histo.binwidth.v_float;
			break;
	case PFDOUBLE:
	case PFDBLCOM:	f1 = histo.minbin.v_double;
			f2 = histo.binwidth.v_double;
			break;
	default:	perr(HE_FMT,hformatname(histo.pixel_format));
	}
	sprintf(msg,"region size was %d x %d\n",hd.rows,hd.cols);
	perr(HE_IMSG,msg);
	if (histo.pixel_format == PFFLOAT ||
	    histo.pixel_format == PFCOMPLEX ||
	    histo.pixel_format == PFDOUBLE ||
	    histo.pixel_format == PFDBLCOM)
		sprintf(msg,"left edge: %f; bin width: %f; number of bins: %d",
			f1,f2,histo.nbins);
	else
		sprintf(msg,"left edge: %d; bin width: %d; number of bins: %d",
			i1,i2,histo.nbins);
	perr(HE_IMSG,msg);
	for (f=0;f<fr;f++) {
		fread_histo(fp,&histo,f,filename);
		if (f == 0 && pflag) {
			i = 0;
			for (j=0;j<=histo.nbins;j++)
				i += histo.histo[j];
			if (histo.nbins == 0)
				maxcnt = 1;
			else
				maxcnt = ((i*100./percent)/histo.nbins) + .5;
		}
		currmax = h_maxhisto(&histo);
		if (currmax > maxcnt) {
			maxcnt = currmax;
			sprintf(msg,"frame %d, maxcnt increased to %d",fr,
				maxcnt);
			perr(HE_IMSG,msg);
		}
		h_disphist(&histo,&hdo,barwidth,barheight,maxcnt,borderw,
			borderg);
		write_image(&hdo,f);
	}
	return(0);
}
