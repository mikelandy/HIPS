/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * pyrtoimg - reconstruct an image from a Laplacian pyramid
 *
 * usage:	pyrtoimg [-f filter] [-r rtype] < ipyrseq > oseq
 *
 * to load:	cc -o pyrtoimg pyrtoimg.c -lhips
 *
 * Pyrtoimg reconstructs an image from its Laplacian pyramid representation.
 * The default filters for reduction and expansion are the Gaussian-like set
 * proposed by Burt (.05/.25/.4/.25/.05), but any symmetric filter with an
 * odd number of taps may be specified using the -f switch.  The -r switch is
 * used to specify the reflection mode at the borders (the default is even
 * reflection).
 *
 * Mike Landy - 3/5/89
 * Hips 2 - msl - 7/18/91
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {
    {"f",{LASTFLAG},1,{{PTBOOLEAN,"FALSE"},{PTFILENAME,"","filter"},
	LASTPARAMETER}},
    {"r",{LASTFLAG},1,{{PTINT,"1","rtype"},LASTPARAMETER}},
    LASTFLAG};
int types[] = {PFINTPYR,PFFLOATPYR,LASTTYPE};

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd;
	int f,fr,nr,nc,i,toplev,rtype,one=1;
	FPYR fpyr;
	IPYR ipyr;
	FILTER rf,ef;
	Filename filterfile,filename;
	FILE *fp;
	h_boolean fflag;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&fflag,&filterfile,&rtype,FFONE,&filename);
	if (!fflag)
		filterfile = (Filename) 0;
	getpyrfilters(filterfile,&rf,&ef);
	Image_border = ef.taps2 + 1;
	fp = hfopenr(filename);
	fread_hdr_cpf(fp,&hd,types,filename);
	getparam(&hd,"toplev",PFINT,&one,&toplev);
	nr = hd.orows; nc = hd.ocols; fr = hd.num_frame;
	hd.pixel_format = (hd.pixel_format == PFINTPYR) ? PFINT : PFFLOAT;
	if (hd.pixel_format == PFINT) {
		i = def_ipyr(ipyr,0,nr,nc);
		if (toplev > i)
			perr(HE_MSG,"input toplev setting is too high!!!");
		alloc_ipyr(ipyr,0,toplev);
	}
	else {
		i = def_fpyr(fpyr,0,nr,nc);
		if (toplev > i)
			perr(HE_MSG,"input toplev setting is too high!!!");
		alloc_fpyr(fpyr,0,toplev);
	}
	write_headeru(&hd,argc,argv);
	if (hd.pixel_format == PFINT) {
		for (f=0;f<fr;f++) {
			read_ipyr(fp,ipyr,0,toplev,f,filename);
			iexpand(ipyr,0,toplev,ef,1,rtype);
			write_iimage(stdout,ipyr[0],f);
		}
	}
	else {
		for (f=0;f<fr;f++) {
			read_fpyr(fp,fpyr,0,toplev,f,filename);
			fexpand(fpyr,0,toplev,ef,1,rtype);
			write_fimage(stdout,fpyr[0],f);
		}
	}
	return(0);
}
