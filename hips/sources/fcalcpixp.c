/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * fcalcpixp.c - main program for object code produced by fcalcpix.c
 *
 * usage:	fcalcpix [-d] [-o objectname] [-c [rows [cols]] [-f frames]
 *			[-nd numdepth] [-nc numcolor] [-A n arg1 ... argn]
 *			[-s "statements" | -F filename]
 *			[-i "init-statements" | -I initfilename]
 *				< iseq > oseq
 *
 * pixel formats handled directly: FLOAT
 *
 * Yoav Cohen 8/9/82
 * Added -g switch - msl - 7/29/85
 * Hips 2 - msl - 7/11/91
 * added -i/-I - msl - 1/7/93
 * added depths - msl - 3/3/94
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {
	{"d",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"o",{LASTFLAG},1,{{PTFILENAME,"fcalcpix.local","executablefile"},
		LASTPARAMETER}},
	{"c",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},{PTINT,"512","rows"},
		{PTINT,"-1","cols"},LASTPARAMETER}},
	{"f",{LASTFLAG},1,{{PTINT,"1","frames"},LASTPARAMETER}},
	{"nd",{LASTFLAG},1,{{PTINT,"1","numdepth"},LASTPARAMETER}},
	{"nc",{LASTFLAG},1,{{PTINT,"1","numcolor"},LASTPARAMETER}},
	{"A",{LASTFLAG},1,{{PTLIST,"","numargs<integer> arg1 ... argn"},
		LASTPARAMETER}},
	{"s",{"F",LASTFLAG},1,{{PTBOOLEAN,"FALSE"},{PTSTRING,"","statements"},
		LASTPARAMETER}},
	{"F",{"s",LASTFLAG},1,{{PTBOOLEAN,"FALSE"},
		{PTFILENAME,"","statementsfile"},LASTPARAMETER}},
	{"i",{"I",LASTFLAG},1,{{PTBOOLEAN,"FALSE"},
		{PTSTRING,"","initstatements"},LASTPARAMETER}},
	{"I",{"i",LASTFLAG},1,{{PTBOOLEAN,"FALSE"},
		{PTFILENAME,"","initstatementsfile"},LASTPARAMETER}},
	LASTFLAG};
int types[] = {PFFLOAT,LASTTYPE};
int nargs;
char **args;
void fcalcpixsr();

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd,hdp,hdo;
	int method,i,nex;
	Filename filename,objectname,progname,prognamei;
	h_boolean dflag,cflag,sflag,Fflag,iflag,Iflag;
	int rows,cols,frames,numcolor,numdepth,depth,f,col;
	char *ccode,*ccodei;
	Listarg arguments;
	FILE *fp;
	float **inrows,**outrows;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&dflag,&objectname,&cflag,&rows,&cols,
		&frames,&numdepth,&numcolor,&arguments,&sflag,&ccode,&Fflag,
		&progname,&iflag,&ccodei,&Iflag,&prognamei,FFONE,&filename);
	if (cols < 0)
		cols = rows;
	nargs = arguments.argcount;
	args = arguments.args;
	if (cflag) {
		init_header(&hd,"","",frames*numdepth*numcolor,"",rows,cols,
			PFFLOAT,numcolor,"");
		if (numdepth > 1)
			hsetdepth(&hd,numdepth);
		alloc_image(&hd);
	}
	else {
		fp = hfopenr(filename);
		fread_hdr_a(fp,&hd,filename);
		rows = hd.rows;
		cols = hd.cols;
	}
	method = fset_conversion(&hd,&hdp,types,filename);
	numdepth = hgetdepth(&hdp);
	numcolor = hdp.numcolor;
	frames = hdp.num_frame/(numdepth*numcolor);
	write_headeru2(&hd,&hdp,argc,argv,hips_convback);
	dup_headern(&hdp,&hdo);
	alloc_image(&hdo);

	inrows = (float **) memalloc(rows,sizeof(float *));
	outrows = (float **) memalloc(rows,sizeof(float *));
	
	for(i=0;i<rows;i++) {
		inrows[i] = ((float *) hdp.firstpix) + i*hdp.ocols;
		outrows[i] = ((float *) hdo.firstpix) + i*hdo.ocols;
	}
	nex = hd.ocols - hd.cols;
	for (f=0;f<frames;f++) {
	    for (depth=0;depth<numdepth;depth++) {
		for (col=0;col<numcolor;col++) {
			if (!cflag)
				fread_imagec(fp,&hd,&hdp,method,
					f*numdepth*numcolor+depth*numcolor+col,
					filename);
			fcalcpixsr(f,frames,depth,numdepth,col,numcolor,rows,
				cols,inrows,outrows,nex,nex);
			write_imagec(&hd,&hdo,method,hips_convback,
				f*numdepth*numcolor+depth*numcolor+col);
		}
	    }
	}
	return(0);
}
