/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * colorkey - perform a `color keying' operation for merging several images
 *
 * usage:	colorkey [-b] ctrlseq file1 ... filen > oseq
 *
 * Colorkey merges several images to a single image as specified by an input
 * control file.  The control file and the file arguments must all have the
 * same number of rows and columns.  Each pixel in the output comes from file1
 * if that pixel in the control file has value `1', from file2 if the control
 * file pixel has value `2', etc.  If the -b switch is specified, then control
 * file pixels which are not in the range from 1 to n result in output pixels
 * with the background value (set by -UL).  Otherwise, control file pixels with
 * value `0' or lower result in pixels from file1, and control file pixels with
 * values greater than `n' result in pixels from filen.  The image files file1
 * through filen must all have the same number of frames and are converted to
 * the same pixel format, and that will be either byte, integer, float,
 * integer pyramid, or floating pyramid.  The control sequence will be
 * converted to byte format or integer format (for byte, integer and float
 * input images), or integer pyramid format (for pyramid input files), and
 * must either have the same number of frames as the
 * image files, or only a single frame, in which case that frame is used to
 * control the keying operation for all image frames.
 *
 * Load:	cc -o colorkey colorkey.c -lhipsh -lhips -lm
 *
 * Mike Landy - 3/11/89
 * HIPS 2 - Michael Landy - 8/13/91
 */

#include <stdio.h>
#include <hipl_format.h>

int typesc[] = {PFBYTE,PFINT,PFINTPYR,LASTTYPE};
int types[] = {PFBYTE,PFINT,PFFLOAT,LASTTYPE};
int typesp[] = {PFINTPYR,PFFLOATPYR,LASTTYPE};
static Flag_Format flagfmt[] = {
	{"b",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	LASTFLAG};
char fileusage[] = "ctrlseq image0 image1 ... imagen";

int main(argc,argv)

int argc;
char **argv;

{
	struct header **hdi,**hdip,hdc,hdcp;
	FILE *fpc,**fp;
	int methodc,*method,numfiles,nimage,file,f,nf,savesize,ofmt,maxinsize;
	char *savedesc,*savehist;
	h_boolean bflag,foundone=FALSE,pyrflag;
	Filename *filelist;
	byte *tmp;

	Progname = strsave(*argv);
	parseargsu(argc,argv,flagfmt,&bflag,FFLIST,&numfiles,&filelist,
		fileusage);
	nimage = numfiles - 1;
	if (nimage < 2)
		print_usage();
	hdi = (struct header **) memalloc(nimage,sizeof(struct header *));
	hdip = (struct header **) memalloc(nimage,sizeof(struct header *));
	for (file=0;file<nimage;file++) {
		hdi[file] = (struct header *) memalloc(1,sizeof(struct header));
		hdip[file] =
			(struct header *) memalloc(1,sizeof(struct header));
	}
	fp = (FILE **) memalloc(nimage,sizeof(FILE *));
	method = (int *) memalloc(nimage,sizeof(int));

	fpc = hfopenr(filelist[0]);	/* control image */
	fread_hdr_a(fpc,&hdc,filelist[0]);
	methodc = fset_conversion(&hdc,&hdcp,typesc,filelist[0]);
	pyrflag = (hdcp.pixel_format == PFINTPYR);
	for (file=0;file<nimage;file++) {
		fp[file] = hfopenr(filelist[file+1]);	/* input image */
		if (file == 0) {
			fread_hdr_cc(fp[0],hdi[0],&hdc,
				CM_ROWS|CM_COLS|CM_NUMLEV,filelist[1]);
			ofmt = hdi[0]->pixel_format;
			ofmt = maxformat(ofmt,ofmt,
				pyrflag ? typesp : types,filelist[1],
					filelist[1]);
		}
		else {
			fread_hdr_cc(fp[file],hdi[file],hdi[0],
			    CM_ROWS|CM_COLS|CM_FRAMES|CM_NUMCOLOR3|CM_NUMLEV,
			    filelist[file+1]);
			if (file == 1)
				ofmt = maxformat(hdi[0]->pixel_format,
					hdi[1]->pixel_format,
					pyrflag ? typesp : types,filelist[1],
					filelist[2]);
			else
				ofmt = maxformat(ofmt,hdi[file]->pixel_format,
					pyrflag ? typesp : types,filelist[1],
					filelist[file+1]);
		}
	}
	maxinsize = 0;
	for (file=0;file<nimage;file++) {
		method[file] = ffind_method(hdi[file]->pixel_format,ofmt,
			filelist[file+1]);
		dup_header(hdi[file],hdip[file]);
		setformat(hdip[file],ofmt);
		if (type_is_col3(hdi[file])) {
			hdip[file]->num_frame *= 3;
			hdip[file]->numcolor = 3;
		}
		if (method[file] < 0)
			perr(HE_CONVI,
				hformatname_f(hdi[file]->pixel_format,ofmt),
				hformatname_t(hdi[file]->pixel_format,ofmt),
				filelist[file+1]);
		else if (method[file] != METH_IDENT)
			perr(HE_CONV,
				hformatname_f(hdi[file]->pixel_format,ofmt),
				hformatname_t(hdi[file]->pixel_format,ofmt),
				filelist[file+1]);
		if (method[file] != METH_IDENT &&
			hdi[file]->sizeimage > maxinsize &&
			(!type_is_col3(hdi[file])))
				maxinsize = hdi[file]->sizeimage;
	}
	if (maxinsize > 0)
		tmp = (byte *) memalloc(maxinsize,sizeof(byte));
	for (file=0;file<nimage;file++) {
		alloc_image(hdip[file]);
		if (type_is_col3(hdi[file]))
			alloc_image(hdi[file]);
		else if (method[file] != METH_IDENT)
			hdi[file]->image = tmp;
		setroi(hdi[file],hdi[file]->frow,hdi[file]->fcol,
			hdi[file]->rows,hdi[file]->cols);
	}
	if (hdcp.num_frame != 1 && (hdcp.num_frame != hdip[0]->num_frame ||
		hdcp.numcolor != hdip[0]->numcolor))
			perr(HE_MSG,
			  "control sequence number of frames/colors mismatch");
	savedesc = hdi[0]->seq_desc;
	savesize = hdi[0]->sizedesc;
	savehist = hdi[0]->seq_history;
	if (hips_fullxpar)
		mergeparam(hdip[0],&hdc);
	if (hips_fulldesc) {
		if (hdc.sizedesc > 1) {
			desc_set2(hdip[0],HEP_SS,
			    "****%s: control sequence, file: %s****\n",
				Progname,filelist[0]);
			foundone = TRUE;
			desc_indentadd(hdip[0],hdc.seq_desc);
		}
	}
	if (hips_fullhist) {
		history_set(hdip[0],HEP_SS,
			"****%s: control sequence, file: %s****\n",Progname,
			filelist[0]);
		history_indentadd(hdip[0],hdc.seq_history);
	}
	for (file=0;file<nimage;file++) {
		if (hips_fullxpar && file != 0)
			mergeparam(hdip[0],hdip[file]);
		if (hips_fulldesc) {
			if (((file == 0) ? savesize : hdip[file]->sizedesc)
			    > 1) {
				if (foundone)
				  desc_append2(hdip[0],HEP_SDS,
				    "****%s: input sequence %d, file: %s****\n",
					Progname,file,filelist[file+1]);
				else
				  desc_set2(hdip[0],HEP_SDS,
				    "****%s: input sequence %d, file: %s****\n",
					Progname,file,filelist[file+1]);
				foundone = TRUE;
				desc_indentadd(hdip[0],
					(file == 0) ? savedesc :
					hdip[file]->seq_desc);
			}
		}
		if (hips_fullhist) {
			history_append(hdip[0],HEP_SDS,
				"****%s: input sequence %d, file: %s****\n",
				Progname,file,filelist[file+1]);
			history_indentadd(hdip[0],
				(file == 0) ? savehist :
				hdip[file]->seq_history);
		}
	}
	if (hips_fullhist)
		write_headerun(hdip[0],argc,argv);
	else
		write_headeru(hdip[0],argc,argv);
	nf = hdip[0]->num_frame;
	for (f=0;f<nf;f++) {
		if (f < hdcp.num_frame)
			fread_imagec(fpc,&hdc,&hdcp,methodc,f,filelist[0]);
		for (file=0;file<nimage;file++)
			fread_imagec(fp[file],hdi[file],hdip[file],
				method[file],f,filelist[file+1]);
		h_colorkey(&hdcp,nimage,hdip,hdip[0],bflag);
		write_image(hdip[0],f);
	}
	return(0);
}
