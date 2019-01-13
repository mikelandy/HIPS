/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * interlace.c - interlace the frames of several files
 *
 * usage:	interlace [-s scriptfile] file1 [file2 ... ] > newfile
 *
 * Interlace combines several sequences into one by interlacing the frames of
 * each.  By default, the program outputs one frame from each input file, 
 * rotating among the input files (1 to n, 1 to n, ....) until all input files
 * are exhausted.  Alternatively, the user may specify a scriptfile (using the
 * -s option).  This allows an arbitrary combination of arbitrarily chosen
 * frames from each of the input files.  The format of the scriptfile is:
 *
 *	n (= number-of-output-frames)
 *	file-number(1)	frame-number-from-file(1)
 *	file-number(2)	frame-number-from-file(2)
 *	file-number(3)	frame-number-from-file(3)
 *				.
 *				.
 *				.
 *	file-number(n)	frame-number-from-file(n)
 *
 * Note that all input frames must have the same number of rows and columns.
 * The region of interest is taken from file1.  A `frame' consists of all
 * color planes and depths for a single frame, and all input sequences must
 * have the same number of colors and depths.  Also, if any input sequence
 * has a 3-color format (RGB, RGBZ, ZRGB, BGR, BGRZ or ZBGR), then all the
 * input sequences must be in some 3-color format.
 *
 * Load:	cc -o interlace interlace.c -lhips
 *
 * Michael Landy - 3/14/86
 * HIPS 2 - Michael Landy - 8/12/91
 * added depth - msl - 3/3/94
 */

#include <stdio.h>
#include <hipl_format.h>

int types[] = {PFMSBF,PFLSBF,PFBYTE,PFSBYTE,PFSHORT,PFUSHORT,PFINT,PFUINT,
	PFFLOAT,PFDOUBLE,PFCOMPLEX,PFDBLCOM,PFINTPYR,PFFLOATPYR,PFRGB,PFRGBZ,
	PFZRGB,PFBGR,PFBGRZ,PFZBGR,LASTTYPE};
int typesn[] = {PFMSBF,PFLSBF,PFBYTE,PFSBYTE,PFSHORT,PFUSHORT,PFINT,PFUINT,
	PFFLOAT,PFDOUBLE,PFCOMPLEX,PFDBLCOM,PFINTPYR,PFFLOATPYR,LASTTYPE};
static Flag_Format flagfmt[] = {
	{"s",{LASTFLAG},1,{{PTBOOLEAN,"FALSE"},{PTFILENAME,"","scriptfile"},
		LASTPARAMETER}},
	LASTFLAG};

int main(argc,argv)

int argc;
char **argv;

{
	struct header **hd,hdp;
	FILE **fp,*sfile;
	int numfiles,i,j,f,ofmt,*method,nf,nc,nd,*pos,sumframes;
	int maxframes,currfile,currframe;
	char *savedesc,*savehist,msg[100];
	Filename *filelist,scriptfile;
	h_boolean foundone,sflag,is3col;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&sflag,&scriptfile,FFLIST,
		&numfiles,&filelist);
	if (sflag)
		sfile = ffopen(scriptfile,"r");
	fp = (FILE **) memalloc(numfiles,sizeof(FILE *));
	hd = (struct header **) memalloc(numfiles,sizeof(struct header *));
	pos = (int *) memalloc(numfiles,sizeof(int));
	method = (int *) memalloc(numfiles,sizeof(int));
	for (i=0;i<numfiles;i++)
		hd[i] = (struct header *) memalloc(1,sizeof(struct header));
	fp[0] = hfopenr(filelist[0]);
	fread_hdr_cpfa(fp[0],hd[0],types,filelist[0]);
	is3col = type_is_col3(hd[0]);
	nc = hd[0]->numcolor;
	nd = hgetdepth(hd[0]);
	ofmt = hd[0]->pixel_format;
	pos[0] = 0;
	maxframes = sumframes = hd[0]->num_frame / (nc*nd);
	for (i=1;i<numfiles;i++) {
		pos[i] = 0;
		fp[i] = hfopenr(filelist[i]);
		fread_hdr_cc(fp[i],hd[i],hd[0],
			CM_OROWS|CM_OCOLS|CM_NUMLEV|CM_NUMCOLOR|CM_DEPTH,
			filelist[i]);
		if (type_is_col3(hd[i]) != is3col) {
			sprintf(msg,
				"all files must be either 1-color or 3-color format, file: %s",
				filelist[i]);
			perr(HE_MSG,msg);
		}
		if (!is3col)
			ofmt = maxformat(ofmt,hd[i]->pixel_format,typesn,
				filelist[0],filelist[i]);
		sumframes += hd[i]->num_frame / (nc*nd);
		if (hd[i]->num_frame > maxframes)
			maxframes = hd[i]->num_frame;
	}
	method[0] = pset_conversion(hd[0],&hdp,ofmt,filelist[0]);
	for (i=1;i<numfiles;i++) {
		method[i] = ffind_method(hd[i]->pixel_format,ofmt,filelist[i]);
		if (method[i] < 0)
			perr(HE_CONVI,hformatname_f(hd[i]->pixel_format,ofmt),
				hformatname_t(hd[i]->pixel_format,ofmt),
				filelist[i]);
		else if (method[i] != METH_IDENT)
			perr(HE_CONV,hformatname_f(hd[i]->pixel_format,ofmt),
				hformatname_t(hd[i]->pixel_format,ofmt),
				filelist[i]);
		if (method[i] == METH_IDENT) {
			hd[i]->sizeimage = hdp.sizeimage;
			hd[i]->image = hdp.image;
			hd[i]->firstpix = hdp.firstpix;
			hd[i]->imdealloc = FALSE;
		}
		else if (hd[i]->pixel_format == hd[0]->pixel_format) {
			hd[i]->sizeimage = hd[0]->sizeimage;
			hd[i]->image = hd[0]->image;
			hd[i]->firstpix = hd[0]->firstpix;
			hd[i]->imdealloc = FALSE;
		}
		else
			alloc_image(hd[i]);
	}
	if (sflag)
		fscanf(sfile,"%d",&nf);
	else
		nf = sumframes;
	hdp.num_frame = nf*nc*nd;
	if (hips_fullxpar)
		for (i=1;i<numfiles;i++)
			mergeparam(&hdp,hd[i]);
	if (hips_fulldesc) {
		foundone = FALSE;
		for (i=0;i<numfiles;i++) {
			if (hd[i]->sizedesc > 1) {
				savedesc = hd[i]->seq_desc;
				if (foundone)
				  desc_append2(&hdp,HEP_SDS,
				    "****%s: sequence %d, file: %s****\n",
					Progname,i,filelist[i]);
				else
				  desc_set2(&hdp,HEP_SDS,
				    "****%s: sequence %d, file: %s****\n",
					Progname,i,filelist[i]);
				foundone = TRUE;
				desc_indentadd(&hdp,savedesc);
			}
		}
	}
	if (hips_fullhist) {
		savehist = hd[0]->seq_history;
		history_set(&hdp,HEP_SDS,
			"****%s: sequence %d, file: %s****\n",Progname,
			0,filelist[0]);
		history_indentadd(&hdp,savehist);
		for (i=1;i<numfiles;i++) {
			history_append(&hdp,HEP_SDS,
			    "****%s: sequence %d, file: %s****\n",Progname,i,
			    filelist[i]);
			history_indentadd(&hdp,hd[i]->seq_history);
		}
		write_headerun(&hdp,argc,argv);
	}
	else
		write_headeru(&hdp,argc,argv);
	currfile = -1;
	currframe = 0;
	for (f=0;f<nf;f++) {
		if (sflag) {
			fscanf(sfile,"%d %d",&currfile,&currframe);
			if (currfile < 0 || currfile >= numfiles)
				perr(HE_MSG,
				    "input script file number out of range");
			if (currframe < 0 || currframe >=
			    ((hd[currfile]->num_frame)/(nc*nd)))
				perr(HE_MSG,
				    "input script frame number out of range");
		}
		else {
			for (i=currframe;i<maxframes;i++) {
				for (j=currfile+1;j<numfiles;j++) {
					if (i<((hd[j]->num_frame)/(nc*nd)))
						goto foundit;
				}
				currfile = -1;
			}
			sprintf(msg,"can't find frame %d!!!???",f);
			perr(HE_MSG,msg);
		foundit:
			currframe=i;
			currfile=j;
		}
		if (currframe != pos[currfile]) {
			if (fseek(fp[currfile],
			    hd[currfile]->sizeimage * nc * nd *
			    (currframe-pos[currfile]),1) < 0)
				perr(HE_MSG,"can't do seek");
			pos[currfile] = currframe+1;
		}
		for (i=0;i<nc*nd;i++) {
			fread_imagec(fp[currfile],hd[currfile],&hdp,
				method[currfile],currframe*nc*nd+i,
				filelist[currfile]);
			pos[currfile] = currframe+1;
			write_image(&hdp,f*nc*nd+i);
		}
	}
	return(0);
}
