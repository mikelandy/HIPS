/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * imgtopyr - compute a Laplacian or Gaussian pyramid
 *
 * usage:	imgtopyr [-f filter] [-g] [-l toplev] [-r rtype] <iseq >opyrseq
 *
 * to load:	cc -o imgtopyr imgtopyr.c -lhips
 *
 * Imgtopyr computes a Gaussian or Laplacian pyramid from a single image.
 * The default is to compute a Laplacian pyramid, but a Gaussian pyramid may
 * be requested by using the -g switch.  The default filters for reduction
 * and expansion are the Gaussian-like set proposed by Burt
 * (.05/.25/.4/.25/.05), but any symmetric filter with an odd number of taps
 * may be specified using the -f switch.  Finally, the default is to build
 * a complete pyramid (up to a level where at least one dimension is of
 * length one).  The user may specify the index of the top level with the
 * -l switch (the bottom level has index zero).  The -r switch is used to
 * specify the reflection mode at the borders (the default is even reflection).
 *
 * input formats handled directly: INT, FLOAT
 *
 * Mike Landy - 3/4/89
 * Hips 2 - msl - 1/14/91
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {
    {"f",{LASTFLAG},1,{{PTBOOLEAN,"FALSE"},{PTFILENAME,"","filter"},
	LASTPARAMETER}},
    {"g",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
    {"l",{LASTFLAG},1,{{PTINT,"-1","toplev"},LASTPARAMETER}},
    {"r",{LASTFLAG},1,{{PTINT,"1","rtype"},LASTPARAMETER}},
    LASTFLAG};
int types[] = {PFINT,PFFLOAT,LASTTYPE};

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd,hdp;
	int i,r,c,fr,f,method,toplev,rtype,gaussian;
	h_boolean fflag;
	FPYR fpyr;
	IPYR ipyr;
	FILTER rf,ef;
	Filename filterfile,filename;
	FILE *fp;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&fflag,&filterfile,&gaussian,&toplev,
		&rtype,FFONE,&filename);
	if (!fflag)
		filterfile = (Filename) 0;
	fp = hfopenr(filename);
	fread_header(fp,&hd,filename);
	method = fset_conversion(&hd,&hdp,types,filename);
	if (method != METH_IDENT)
		alloc_image(&hd);
	r = hd.orows; c = hd.ocols; fr = hdp.num_frame;
	hdp.pixel_format = (hdp.pixel_format == PFINT) ? PFINTPYR : PFFLOATPYR;
	if (hdp.pixel_format == PFINTPYR) {
		if (toplev < 0)
			toplev = def_ipyr(ipyr,0,r,c);
		else {
			i = def_ipyr(ipyr,0,r,c);
			if (toplev > i)
				perr(HE_MSG,"toplev setting is too high");
		}
	}
	else {
		if (toplev < 0)
			toplev = def_fpyr(fpyr,0,r,c);
		else {
			i = def_fpyr(fpyr,0,r,c);
			if (toplev > i)
				perr(HE_MSG,"toplev setting is too high");
		}
	}
	setparam(&hdp,"toplev",PFINT,1,toplev);
	write_headeru(&hdp,argc,argv);
	getpyrfilters(filterfile,&rf,&ef);
	Image_border = rf.taps2 + 1;
	if (hdp.pixel_format == PFINTPYR) {
		alloc_ipyr(ipyr,0,toplev);
		for (f=0;f<fr;f++) {
			if (method == METH_IDENT)
				read_iimage(fp,ipyr[0],f,filename);
			else {
				fread_imagec(fp,&hd,&hdp,method,f,filename);
				copy_itoii(&hdp,ipyr[0]);
			}
			ireduce(ipyr,0,toplev,rf,rtype);
			if (!gaussian)
				iexpand(ipyr,0,toplev,ef,-1,rtype);
			write_ipyr(stdout,ipyr,0,toplev,f);
		}
	}
	else {
		alloc_fpyr(fpyr,0,toplev);
		for (f=0;f<fr;f++) {
			if (method == METH_IDENT)
				read_fimage(fp,fpyr[0],f,filename);
			else {
				fread_imagec(fp,&hd,&hdp,method,f,filename);
				copy_ftoff(&hdp,fpyr[0]);
			}
			freduce(fpyr,0,toplev,rf,rtype);
			if (!gaussian)
				fexpand(fpyr,0,toplev,ef,-1,rtype);
			write_fpyr(stdout,fpyr,0,toplev,f);
		}
	}
	return(0);
}
