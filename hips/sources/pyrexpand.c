/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * pyrexpand - apply the pyramid expand operation to an image or pyramid
 *
 * usage:	pyrexpand [-f filterfile] [-n nlevels] [-r rtype]
 *
 * to load:	cc -o pyrexpand pyrexpand.c -lhips
 *
 * Pyrexpand applies the pyramid expand operation to an image or pyramid.
 * The expand operation is performed `nlevels' times (which defaults to a
 * single expand operation).  The input may be either floating, integer,
 * floating pyramid, or integer pyramid, and the output is in the same format
 * as the input.  If the input is an image, the output will be the expanded
 * image (with the number of rows and columns multiplied by 2**nlevels).  If
 * the input is a pyramid, then the output will be a pyramid with `nlevels'
 * more levels below the first input level containing the result of
 * the expand operation applied to the lowest level.  The -f switch is used
 * to specify the file describing the filters.  The -r switch is used to
 * specify the reflection mode at the borders (the default is even reflection).
 *
 * input formats handled directly: INT, FLOAT, INTPYR, FLOATPYR
 *
 * Mike Landy - 3/6/89
 * Hips 2 - msl - 7/17/91
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {
    {"f",{LASTFLAG},1,{{PTBOOLEAN,"FALSE"},{PTFILENAME,"","filter"},
	LASTPARAMETER}},
    {"n",{LASTFLAG},1,{{PTINT,"1","nlevels"},LASTPARAMETER}},
    {"r",{LASTFLAG},1,{{PTINT,"1","rtype"},LASTPARAMETER}},
    LASTFLAG};
int types[] = {PFINT,PFFLOAT,PFINTPYR,PFFLOATPYR,LASTTYPE};

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd,hdp;
	int form,nr,nc,nfr,outlev,f,method,numlev,rtype,nlevels,one=1;
	h_boolean fflag,pyrflag=FALSE,floatflag=FALSE;
	FPYR fpyr;
	IPYR ipyr;
	FILTER rf,ef;
	Filename filterfile,filename;
	FILE *fp;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&fflag,&filterfile,&nlevels,&rtype,
		FFONE,&filename);
	if (!fflag)
		filterfile = (Filename) 0;
	getpyrfilters(filterfile,&rf,&ef);
	Image_border = ef.taps2 + 1;
	fp = hfopenr(filename);
	fread_header(fp,&hd,filename);
	method = fset_conversion(&hd,&hdp,types,filename);
	if (method != METH_IDENT)
		alloc_image(&hd);
	form = hdp.pixel_format;
	if (form == PFINTPYR || form == PFFLOATPYR) {
		pyrflag = TRUE;
		getparam(&hdp,"toplev",PFINT,&one,&numlev);
	}
	else
		numlev = 0;
	if (numlev + nlevels >= MAXLEV)
		perr(HE_MSG,"nlevels too large");
	if (form == PFFLOAT || form == PFFLOATPYR)
		floatflag = TRUE;
	nr = hd.orows;
	nc = hd.ocols;
	nfr = hdp.num_frame;
	if (floatflag) {
		def_fpyr(fpyr,nlevels,nr,nc);
		setsize(&hdp,fpyr[0].nr,fpyr[0].nc);
		alloc_fpyr(fpyr,0,numlev+nlevels);
	}
	else {
		def_ipyr(ipyr,nlevels,nr,nc);
		setsize(&hdp,ipyr[0].nr,ipyr[0].nc);
		alloc_ipyr(ipyr,0,numlev+nlevels);
	}
	if (pyrflag) {
		outlev = numlev+nlevels;
		setparam(&hdp,"toplev",PFINT,1,outlev);
	}
	else
		outlev = 0;
	write_headeru(&hdp,argc,argv);
	for (f=0;f<nfr;f++) {
		if (floatflag) {
			if (method == METH_IDENT)
				read_fpyr(fp,fpyr,nlevels,nlevels+numlev,
					f,filename);
			else {
				fread_imagec(fp,&hd,&hdp,method,f,filename);
				copy_ftoff(&hdp,fpyr[nlevels]);
			}
			fexpand(fpyr,0,nlevels,ef,0,rtype);
			write_fpyr(stdout,fpyr,0,outlev);
		}
		else {
			if (method == METH_IDENT)
				read_ipyr(fp,ipyr,nlevels,nlevels+numlev,
					f,filename);
			else {
				fread_imagec(fp,&hd,&hdp,method,f,filename);
				copy_itoii(&hdp,ipyr[nlevels]);
			}
			iexpand(ipyr,0,nlevels,ef,0,rtype);
			write_ipyr(stdout,ipyr,0,outlev);
		}
	}
	return(0);
}
