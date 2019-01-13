/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * pyrextract - extract a subset of pyramid levels
 *
 * usage:	pyrextract [-l botlev [toplev]] < ipyrseq > oseq
 *
 * to load:	cc -o pyrextract pyrextract.c -lhips
 *
 * Pyrextract extracts a subset of the levels of an image pyramid.  The levels
 * from botlev to toplev are extracted, where toplev defaults to the value of
 * botlev, and botlev defaults to zero.  The input must be in either integer
 * or floating pyramid format.  If only a single level is extracted, the
 * output will be in integer or floating point format, otherwise it will have
 * the same pyramid format as the input.
 *
 * input formats handled directly: INTPYR, FLOATPYR
 *
 * Mike Landy - 3/5/89
 * Hips 2 - msl - 7/19/91
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {
    {"l",{LASTFLAG},1,{{PTINT,"0","botlev"},{PTINT,"-1","toplev"},
	LASTPARAMETER}},
    LASTFLAG};
int types[] = {PFINTPYR,PFFLOATPYR,LASTTYPE};

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd;
	int botlev,toplev,intoplev,r,c,fr,f,one=1;
	FPYR fpyr;
	IPYR ipyr;
	Filename filename;
	FILE *fp;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&botlev,&toplev,FFONE,&filename);
	if (toplev == -1)
		toplev = botlev;
	Image_border = 0;
	fp = hfopenr(filename);
	fread_hdr_cpf(fp,&hd,types,filename);
	getparam(&hd,"toplev",PFINT,&one,&intoplev);
	r = hd.rows; c = hd.cols;
	fr = hd.num_frame;
	if (botlev < 0 || toplev < botlev || toplev > intoplev)
		perr(HE_MSG,"invalid range of pyramid levels");
	if (botlev == toplev) {
		setformat(&hd,(hd.pixel_format == PFINTPYR) ? PFINT : PFFLOAT);
		clearparam(&hd,"toplev");
	}
	else
		setparam(&hd,"toplev",PFINT,1,toplev-botlev);
	if (hd.pixel_format == PFINT || hd.pixel_format == PFINTPYR) {
		def_ipyr(ipyr,0,r,c);
		setsize(&hd,ipyr[botlev].nr,ipyr[botlev].nc);
		alloc_ipyr(ipyr,0,intoplev);
	}
	else {
		def_fpyr(fpyr,0,r,c);
		setsize(&hd,fpyr[botlev].nr,fpyr[botlev].nc);
		alloc_fpyr(fpyr,0,intoplev);
	}
	write_headeru(&hd,argc,argv);
	if (hd.pixel_format == PFINT || hd.pixel_format == PFINTPYR) {
		for (f=0;f<fr;f++) {
			read_ipyr(fp,ipyr,0,intoplev,f,filename);
			write_ipyr(stdout,ipyr,botlev,toplev,f);
		}
	}
	else {
		for (f=0;f<fr;f++) {
			read_fpyr(fp,fpyr,0,intoplev,f,filename);
			write_fpyr(stdout,fpyr,botlev,toplev,f);
		}
	}
	return(0);
}
