/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * calcwarpp.c  - main program for object code produced by calcwarp.c
 *
 * usage:	calcwarp [-d] [-o objectname] [-s rows [cols]]
 *			[-A n arg1 ... argn]
 *			[-W "warpcode" | -w warpcodefile]
 *			[-i "init-statements" | -I initfilename]
 *				< iseq > oseq
 *
 * pixel formats handled directly: BYTE
 *
 * Mike Landy - 12/29/88
 * Hips 2 - msl - 8/12/91
 * added -i/-I - msl - 1/7/93
 * added depths - msl - 3/7/94
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {
	{"d",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"o",{LASTFLAG},1,{{PTFILENAME,"calcwarp.local","executablefile"},
		LASTPARAMETER}},
	{"s",{LASTFLAG},1,{{PTBOOLEAN,"FALSE"},{PTINT,"-1","rows"},
		{PTINT,"-1","cols"},LASTPARAMETER}},
	{"A",{LASTFLAG},1,{{PTLIST,"","numargs<integer> arg1 ... argn"},
		LASTPARAMETER}},
	{"W",{"w",LASTFLAG},1,{{PTBOOLEAN,"FALSE"},{PTSTRING,"","warpcode"},
		LASTPARAMETER}},
	{"w",{"W",LASTFLAG},1,{{PTBOOLEAN,"FALSE"},
		{PTFILENAME,"","warpcodefile"},LASTPARAMETER}},
	{"i",{"I",LASTFLAG},1,{{PTBOOLEAN,"FALSE"},
		{PTSTRING,"","initstatements"},LASTPARAMETER}},
	{"I",{"i",LASTFLAG},1,{{PTBOOLEAN,"FALSE"},
		{PTFILENAME,"","initstatementsfile"},LASTPARAMETER}},
	LASTFLAG};
int types[] = {PFBYTE,LASTTYPE};
int nargs;
char **args;
void calcwarpsr();

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd,hdp,hdo,hdcb;
	int method,i,nex,nr,nc,nor,noc,frames,numcolor,f,col,depth,ndepth;
	Filename filename,objectname,progname,prognamei;
	h_boolean dflag,sflag,Wflag,wflag,iflag,Iflag;
	char *ccode,*ccodei;
	Listarg arguments;
	FILE *fp;
	byte **inrows,**outrows;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&dflag,&objectname,&sflag,&nor,&noc,
		&arguments,&Wflag,&ccode,&wflag,&progname,
		&iflag,&ccodei,&Iflag,&prognamei,FFONE,&filename);
	if (noc < 0)
		noc = nor;
	nargs = arguments.argcount;
	args = arguments.args;
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	nr = hd.rows;
	nc = hd.cols;
	if (nor < 0) {
		nor = nr;
		noc = nc;
	}
	method = fset_conversion(&hd,&hdp,types,filename);
	ndepth = hgetdepth(&hdp);
	frames = hdp.num_frame/(hdp.numcolor * ndepth);
	numcolor = hdp.numcolor;
	dup_headern(&hdp,&hdo);
	setsize(&hdo,nor,noc);
	alloc_image(&hdo);
	if (hips_convback)
		setupconvback(&hd,&hdo,&hdcb);
	write_headeru2(&hdcb,&hdo,argc,argv,hips_convback);

	inrows = (byte **) memalloc(nr,sizeof(byte *));
	outrows = (byte **) memalloc(nor,sizeof(byte *));
	
	for(i=0;i<nr;i++)
		inrows[i] = hdp.firstpix + i*hdp.ocols;
	for(i=0;i<nor;i++)
		outrows[i] = hdo.firstpix + i*hdo.ocols;
	nex = hdo.ocols - hdo.cols;
	for (f=0;f<frames;f++) {
	    for (depth=0;depth<ndepth;depth++) {
		for (col=0;col<numcolor;col++) {
			fread_imagec(fp,&hd,&hdp,method,f*numcolor+col,
				filename);
			calcwarpsr(f,frames,depth,ndepth,col,numcolor,nr,nc,
				nor,noc,inrows,outrows,hd.ocols,nex);
			write_imagec(&hdcb,&hdo,method,hips_convback,
				f*numcolor+col);
		}
	    }
	}
	return(0);
}
