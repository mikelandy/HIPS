/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * fcalccombp.c - main program for object code produced by fcalccomb.c
 *
 * usage:	fcalccomb [-d] [-o objectname] [-A n arg1 ... argn]
 *			 [-s "statements" | -F filename]
 *			 [-i "init-statements" | -I initfilename]
 *				iseq0 iseq1 ... > oseq
 *
 * pixel formats handled directly: FLOAT
 *
 * Michael Landy - 1/4/93
 * added depths - msl - 3/7/94
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {
	{"d",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"o",{LASTFLAG},1,{{PTFILENAME,"fcalccomb.local","executablefile"},
		LASTPARAMETER}},
	{"A",{LASTFLAG},1,{{PTLIST,"","numargs<integer> arg1 ... argn"},
		LASTPARAMETER}},
	{"s",{"F",LASTFLAG},1,{{PTBOOLEAN,"FALSE"},{PTSTRING,"","statements"},
		LASTPARAMETER}},
	{"F",{"s",LASTFLAG},1,{{PTBOOLEAN,"FALSE"},
		{PTFILENAME,"","statementsfile"},LASTPARAMETER}},
	{"i",{"I",LASTFLAG},1,{{PTBOOLEAN,"FALSE"},
		{PTSTRING,"","initstatements"},
		LASTPARAMETER}},
	{"I",{"i",LASTFLAG},1,{{PTBOOLEAN,"FALSE"},
		{PTFILENAME,"","initstatementsfile"},LASTPARAMETER}},
	LASTFLAG};
int types[] = {PFFLOAT,LASTTYPE};
int nargs;
float **ipixpt;
char **args;
void fcalccombsr();

int main(argc,argv)

int argc;
char **argv;

{
	struct header **hd,**hdp,hdo;
	int *method,i,*nex,numfiles,nrows,ncols,frames,numcolor,f,col,file;
	int depth,ndepth;
	Filename *filelist,objectname,progname,prognamei;
	h_boolean imagecopy,dflag,sflag,Fflag,iflag,Iflag,foundone=FALSE;
	char *savedesc,*savehist,*ccode,*ccodei;
	Listarg arguments;
	FILE **fp;
	float ***inrows,**outrows;
	struct hips_roi roi;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&dflag,&objectname,&arguments,&sflag,
		&ccode,&Fflag,&progname,&iflag,&ccodei,&Iflag,&prognamei,
		FFLIST,&numfiles,&filelist);
	fp = (FILE **) memalloc(numfiles,sizeof(FILE *));
	hd = (struct header **) memalloc(numfiles,sizeof(struct header *));
	hdp = (struct header **) memalloc(numfiles,sizeof(struct header *));
	method = (int *) memalloc(numfiles,sizeof(int));
	ipixpt = (float **) memalloc(numfiles,sizeof(float *));
	nex = (int *) memalloc(numfiles,sizeof(int));
	for (file=0;file<numfiles;file++) {
		hd[file] = (struct header *) memalloc(1,sizeof(struct header));
		hdp[file] = (struct header *) memalloc(1,sizeof(struct header));
	}
	nargs = arguments.argcount;
	args = arguments.args;

	/* First: read/merge headers, check consistency */

	for (file=0;file<numfiles;file++) {
		fp[file] = hfopenr(filelist[file]);
		if (file == 0)
			fread_hdr_a(fp[0],hd[0],filelist[0]);
		else
			fread_hdr_cca(fp[file],hd[file],hd[0],
				CM_ROWS|CM_COLS|CM_FRAMES|CM_NUMCOLOR3,
				filelist[file]);
		method[file] = fset_conversion(hd[file],hdp[file],types,
			filelist[file]);
		nex[file] = hdp[file]->ocols - hdp[file]->cols;
		if (hips_fullxpar && file>0)
			mergeparam(hdp[0],hdp[file]);
		if (hips_fulldesc) {
			if (hdp[file]->sizedesc > 1) {
				savedesc = hdp[file]->seq_desc;
				if (foundone)
				  desc_append2(hdp[0],HEP_SDS,
				    "****%s: sequence %d, file: %s****\n",
					Progname,file,filelist[file]);
				else
				  desc_set2(hdp[0],HEP_SDS,
				    "****%s: sequence %d, file: %s****\n",
					Progname,file,filelist[file]);
				foundone = TRUE;
				desc_indentadd(hdp[0],savedesc);
			}
		}
		if (hips_fullhist) {
			savehist = hdp[file]->seq_history;
			if (file == 0)
			    history_set(hdp[0],HEP_SDS,
				"****%s: sequence %d, file: %s****\n",Progname,
				file,filelist[file]);
			else
			    history_append(hdp[0],HEP_SDS,
				"****%s: sequence %d, file: %s****\n",Progname,
				file,filelist[file]);
			history_indentadd(hdp[0],savehist);
		}
	}

	/* set up output header, image, roi */

	dup_headern(hdp[0],&hdo);
	alloc_image(&hdo);
	if (hips_fullhist)
		write_headerun(&hdo,argc,argv);
	else
		write_headeru(&hdo,argc,argv);
	imagecopy =
		(hd[0]->rows != hd[0]->orows) || (hd[0]->cols != hd[0]->ocols);
	getroi(hdp[0],&roi);
	nrows = hd[0]->rows;
	ncols = hd[0]->cols;
	ndepth = hgetdepth(hd[0]);
	frames = hdp[0]->num_frame/(hdp[0]->numcolor * ndepth);
	numcolor = hdp[0]->numcolor;
	inrows = (float ***) memalloc(numfiles,sizeof(float **));
	for (file=0;file<numfiles;file++)
		inrows[file] = (float **) memalloc(nrows,sizeof(float *));
	outrows = (float **) memalloc(nrows,sizeof(float *));
	for(i=0;i<nrows;i++) {
		for (file=0;file<numfiles;file++)
			inrows[file][i] = ((float *) hdp[file]->firstpix) +
				i*hdp[file]->ocols;
		outrows[i] = ((float *) hdo.firstpix) + i*hdo.ocols;
	}
	for (f=0;f<frames;f++) {
	    for (depth=0;depth<ndepth;depth++) {
		for (col=0;col<numcolor;col++) {
			for (file=0;file<numfiles;file++) {
				fread_imagec(fp[file],hd[file],hdp[file],
					method[file],f*numcolor+col,
					filelist[file]);
			}
			if (imagecopy) {
				clearroi(hdp[0]);
				clearroi(&hdo);
				h_copy(hdp[0],&hdo);
				setroi2(hdp[0],&roi);
				setroi2(&hdo,&roi);
			}
			fcalccombsr(numfiles,f,frames,depth,ndepth,col,numcolor,
				nrows,ncols,inrows,outrows,nex,nex[0]);
			write_image(&hdo,f*numcolor+col);
		}
	    }
	}
	return(0);
}
