/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * pyrreduce - apply the pyramid reduce operation to an image or pyramid
 *
 * usage:	pyrreduce [-f filterfile] [-n nlevels] [-r rtype] [-l]
 *
 * to load:	cc -o pyrreduce pyrreduce.c -lhips
 *
 * Pyrreduce applies the pyramid reduce operation to an image or pyramid.
 * The reduce operation is performed `nlevels' times (which defaults to a
 * single reduce operation).  The input may be either floating, integer,
 * floating pyramid, or integer pyramid, and the output is in the same format
 * as the input.  If the input is an image, the output will be the reduced
 * image (with the number of rows and columns divided by 2**nlevels).  If the
 * input is a pyramid, then the output will be a pyramid with `nlevels' more
 * levels above the top input level containing the result of the reduce
 * operation applied to the highest level.  The -f switch is used to specify
 * the file describing the filters.  The -r switch is used to specify the
 * reflection mode at the borders (default to even reflection). For pyramids,
 * the -l switch performs the reduce in the manner of a Laplacian pyramid
 * (first reducing, then subtracting the expansion of that reduced image from
 * the image on the previous level).  Thus, pyrreduce may be used to add
 * additional levels to a Laplacian pyramid created with imgtopyr.
 *
 * input formats handled directly: INT, FLOAT, INTPYR, FLOATPYR
 *
 * Mike Landy - 3/6/89
 * Hips 2 - msl - 7/18/91
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {
    {"f",{LASTFLAG},1,{{PTBOOLEAN,"FALSE"},{PTFILENAME,"","filter"},
	LASTPARAMETER}},
    {"n",{LASTFLAG},1,{{PTINT,"1","nlevels"},LASTPARAMETER}},
    {"r",{LASTFLAG},1,{{PTINT,"1","rtype"},LASTPARAMETER}},
    {"l",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
    LASTFLAG};
int types[] = {PFINT,PFFLOAT,PFINTPYR,PFFLOATPYR,LASTTYPE};

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd,hdp;
	int form,nr,nc,nfr,outlev,f,method,numlev,rtype,nlevels,one=1;
	h_boolean fflag,pyrflag=FALSE,floatflag=FALSE,lflag;
	FPYR fpyr;
	IPYR ipyr;
	FILTER rf,ef;
	Filename filterfile,filename;
	FILE *fp;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&fflag,&filterfile,&nlevels,&rtype,&lflag,
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
		outlev = 0;
	}
	else {
		outlev = nlevels;
		numlev = 0;
	}
	if (numlev + nlevels >= MAXLEV)
		perr(HE_MSG,"nlevels too large");
	if (form == PFFLOAT || form == PFFLOATPYR)
		floatflag = TRUE;
	nr = hd.orows;
	nc = hd.ocols;
	nfr = hdp.num_frame;
	if (floatflag) {
		def_fpyr(fpyr,0,nr,nc);
		if (fpyr[numlev+nlevels].nr == 0)
			perr(HE_MSG,"input can not be reduced that much");
		alloc_fpyr(fpyr,0,numlev+nlevels);
	}
	else {
		def_ipyr(ipyr,0,nr,nc);
		if (ipyr[numlev+nlevels].nr == 0)
			perr(HE_MSG,"input can not be reduced that much");
		alloc_ipyr(ipyr,0,numlev+nlevels);
	}
	if (!pyrflag) {
		if (floatflag)
			setsize(&hdp,fpyr[nlevels].nr,fpyr[nlevels].nc);
		else
			setsize(&hdp,ipyr[nlevels].nr,ipyr[nlevels].nc);
	}
	else
		setparam(&hdp,"toplev",PFINT,1,numlev+nlevels);
	write_headeru(&hdp,argc,argv);
	for (f=0;f<nfr;f++) {
		if (floatflag) {
			if (method == METH_IDENT)
				read_fpyr(fp,fpyr,0,numlev,f,filename);
			else {
				fread_imagec(fp,&hd,&hdp,method,f,filename);
				copy_ftoff(&hdp,fpyr[0]);
			}
			freduce(fpyr,numlev,numlev+nlevels,rf,rtype);
			if (lflag)
				fexpand(fpyr,numlev,numlev+nlevels,ef,-1,rtype);
			write_fpyr(stdout,fpyr,outlev,numlev+nlevels);
		}
		else {
			if (method == METH_IDENT)
				read_ipyr(fp,ipyr,0,numlev,f,filename);
			else {
				fread_imagec(fp,&hd,&hdp,method,f,filename);
				copy_itoii(&hdp,ipyr[0]);
			}
			ireduce(ipyr,numlev,numlev+nlevels,rf,rtype);
			if (lflag)
				iexpand(ipyr,numlev,numlev+nlevels,ef,-1,rtype);
			write_ipyr(stdout,ipyr,outlev,numlev+nlevels);
		}
	}
	return(0);
}
