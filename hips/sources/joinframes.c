/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * joinframes.c - synchronize and abut the frames from 2 or more sequences
 *
 * usage: joinframes [-s rows [cols]] [-m margin] [-e]
 *			file1 file2 ... filen > joined-file
 *
 * Joinframes combines two or more sequences of frames into a single 
 * sequence.  Corresponding frames from each sequence are combined into a
 * single frame.  First, each frame is padded with the background grey-level
 * to the same shape, found by separately computing the maximum number of
 * rows and columns across all files.  Next, these padded frames are joined
 * into one combined frame.  The frames are added left-to-right, and then
 * row by row downward into a matrix of cells in the combined frame.  By
 * default the frames are combined in one horizontal row, but the number of
 * rows and columns in the matrix may be specified with the -s (shape) switch.
 * If only rows are specified, then cols defaults to the minimum number such
 * that rows*cols is sufficient to hold all the input files.
 * The background grey-level is specified, as usual, with the standard switch
 * -UL, and defaults to 0.  Finally, a border of pixels is interpolated between
 * each row and column.  Its width may be specified with the -m (margin) switch,
 * and defaults to no margin at all.  The number of files is arbitrary, but
 * must be less than rows*cols if the -s switch is specified.  The output
 * sequence will be as long as the longest input sequence.
 * For bit-packed output frames, the column width will be rounded up to
 * an even multiple of 8.  Only the regions-of-interest of the
 * input files are output.  However, if -e is specified, the input regions
 * of interest are reset to be the entire image, so that entire images are
 * joined.  The output region-of-interest is cleared to be the entire image.
 *
 * Load:	cc -o joinframes joinframes.c -lhipsh -lhips
 *
 * Michael Landy - 6/3/85
 * added float format - 7/2/89
 * HIPS 2 - Michael Landy - 7/21/91
 */

#include <stdio.h>
#include <hipl_format.h>

int types[] = {PFMSBF,PFLSBF,PFBYTE,PFSBYTE,PFSHORT,PFUSHORT,PFINT,PFUINT,
	PFFLOAT,PFDOUBLE,LASTTYPE};
int types2[] = {PFMSBF,PFLSBF,PFBYTE,PFSBYTE,PFSHORT,PFUSHORT,PFINT,PFUINT,
	PFFLOAT,PFDOUBLE,PFRGB,PFRGBZ,PFZRGB,PFBGR,PFBGRZ,PFZBGR,LASTTYPE};
static Flag_Format flagfmt[] = {
	{"s",{LASTFLAG},1,{{PTINT,"-1","rows"},{PTINT,"-1","cols"},
		LASTPARAMETER}},
	{"m",{LASTFLAG},1,{{PTINT,"0","margin"},LASTPARAMETER}},
	{"e",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	LASTFLAG};

int main(argc,argv)

int argc;
char **argv;

{
	struct header **hd,**hdp,hdo;
	FILE **fp;
	int *method,numfiles,outr,outc,*ir,*ic,*nr,*nc,*nfr,maxr,maxc,maxfr;
	int f,file,ofmt,maxinsize,maxprocsize;
	int margin,picperrow,picpercol,picrow,piccol;
	char *savedesc,*savehist;
	Filename *filelist;
	h_boolean foundone=FALSE,eflag;
	Pixelval val;
	byte *t1,*t2;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&picpercol,&picperrow,&margin,&eflag,
		FFLIST,&numfiles,&filelist);
	if (picpercol < 0) {
		picpercol = 1;
		picperrow = numfiles;
	}
	else if (picperrow < 0)
		picperrow = (numfiles + picpercol - 1)/picpercol;
	else if (picpercol*picperrow < numfiles)
		perr(HE_MSG,"not enough matrix positions to hold all files");
	if (numfiles < 2)
		perr(HE_MSG,"at least two files must be specified");
	fp = (FILE **) memalloc(numfiles,sizeof(FILE *));
	hd = (struct header **) memalloc(numfiles,sizeof(struct header *));
	hdp = (struct header **) memalloc(numfiles,sizeof(struct header *));
	method = (int *) memalloc(numfiles,sizeof(int));
	ir = (int *) memalloc(numfiles,sizeof(int));
	ic = (int *) memalloc(numfiles,sizeof(int));
	nr = (int *) memalloc(numfiles,sizeof(int));
	nc = (int *) memalloc(numfiles,sizeof(int));
	nfr = (int *) memalloc(numfiles,sizeof(int));
	for (file=0;file<numfiles;file++) {
		hd[file] = (struct header *) memalloc(1,sizeof(struct header));
		hdp[file] = (struct header *) memalloc(1,sizeof(struct header));
	}

	/* First: read/merge headers, check consistency, compute max format */

	for (file=0;file<numfiles;file++) {
		fp[file] = hfopenr(filelist[file]);
		if (file == 0) {
			fread_hdr_cpf(fp[0],hd[0],types2,filelist[file]);
			ofmt = type_is_col3(hd[0]) ? PFBYTE :
				hd[0]->pixel_format;
		}
		else {
			fread_hdr_cc(fp[file],hd[file],hd[0],
				CM_FRAMESC|CM_NUMCOLOR3,filelist[file]);
			ofmt = maxformat(ofmt,hd[file]->pixel_format,
				types,filelist[0],filelist[file]);
		}
		if (hips_fullxpar && file>0)
			mergeparam(hd[0],hd[file]);
		if (hips_fulldesc) {
			if (hd[file]->sizedesc > 1) {
				savedesc = hd[file]->seq_desc;
				if (foundone)
				  desc_append2(hd[0],HEP_SDS,
				    "****%s: sequence %d, file: %s****\n",
					Progname,file,filelist[file]);
				else
				  desc_set2(hd[0],HEP_SDS,
				    "****%s: sequence %d, file: %s****\n",
					Progname,file,filelist[file]);
				foundone = TRUE;
				desc_indentadd(hd[0],savedesc);
			}
		}
		if (hips_fullhist) {
			savehist = hd[file]->seq_history;
			if (file == 0)
			    history_set(hd[0],HEP_SDS,
				"****%s: sequence %d, file: %s****\n",Progname,
				file,filelist[file]);
			else
			    history_append(hd[0],HEP_SDS,
				"****%s: sequence %d, file: %s****\n",Progname,
				file,filelist[file]);
			history_indentadd(hd[0],savehist);
		}
	}

	/* compute frame size/number of frames */

	maxr = maxc = maxfr = 0;
	for (file=0;file<numfiles;file++) {
		if (eflag)
			clearroi(hd[file]);
		nr[file] = hd[file]->rows;
		nc[file] = hd[file]->cols;
		nfr[file] = hd[file]->num_frame;
		if (type_is_col3(hd[file]))
			nfr[file] *= 3;
		if (maxr < nr[file])
			maxr = nr[file];
		if (maxc < nc[file])
			maxc = nc[file];
		if (maxfr < nfr[file])
			maxfr = nfr[file];
	}
	if ((ofmt == PFMSBF || ofmt == PFLSBF) && ((maxc+margin)%8 != 0))
		maxc += 8 - ((maxc+margin)%8);
	outr = (maxr + margin)*picpercol - margin;
	outc = (maxc + margin)*picperrow - margin;

	/* set up output header, image, roi */

	dup_headern(hd[0],&hdo);
	hdo.num_frame = maxfr;
	if (type_is_col3(hd[0]))
		hdo.numcolor = 3;
	setformat(&hdo,ofmt);
	setsize(&hdo,outr,outc);
	alloc_image(&hdo);
	clearroi(&hdo);
	if (hips_fullhist)
		write_headerun(&hdo,argc,argv);
	else
		write_headeru(&hdo,argc,argv);

	/* set up roi's for copying, calculate methods */

	maxinsize = maxprocsize = 0;
	for (file=0;file<numfiles;file++) {
		picrow = file/picperrow;
		piccol = file % picperrow;
#ifdef ULORIG
		ir[file] = picrow*(maxr + margin);
#else
		ir[file] = (picpercol - picrow - 1)*(maxr + margin);
#endif
		ic[file] = piccol*(maxc + margin);
		method[file] = ffind_method(hd[file]->pixel_format,ofmt,
			filelist[file]);
		dup_header(hd[file],hdp[file]);
		setformat(hdp[file],ofmt);
		if (hdp[file]->sizeimage > maxprocsize)
			maxprocsize = hdp[file]->sizeimage;
		if (method[file] != METH_IDENT &&
			hd[file]->sizeimage > maxinsize &&
			(!type_is_col3(hd[file])))
				maxinsize = hd[file]->sizeimage;
		if (method[file] < 0)
			perr(HE_CONVI,
				hformatname_f(hd[file]->pixel_format,ofmt),
				hformatname_t(hd[file]->pixel_format,ofmt),
				filelist[file]);
		else if (method[file] != METH_IDENT)
			perr(HE_CONV,
				hformatname_f(hd[file]->pixel_format,ofmt),
				hformatname_t(hd[file]->pixel_format,ofmt),
				filelist[file]);
	}

	/* allocate input image and converted image */

	t1 = (byte *) memalloc(maxprocsize,sizeof(byte));
	if (maxinsize > 0)
		t2 = (byte *) memalloc(maxinsize,sizeof(byte));
	for (file=0;file<numfiles;file++) {
		hdp[file]-> image = t1;
		setroi(hdp[file],hdp[file]->frow,hdp[file]->fcol,
			hdp[file]->rows,hdp[file]->cols);
		if (type_is_col3(hd[file]))
			alloc_image(hd[file]);
		else if (method[file] == METH_IDENT)
			hd[file]->image = t1;
		else
			hd[file]->image = t2;
		setroi(hd[file],hd[file]->frow,hd[file]->fcol,
			hd[file]->rows,hd[file]->cols);
	}

	/* set up background value */

	switch (ofmt) {
	case PFMSBF:
	case PFLSBF:	val.v_byte = hips_lchar ? 255 : 0; break;
	case PFBYTE:	val.v_byte = hips_lchar; break;
	case PFSBYTE:	val.v_sbyte = hips_lchar; break;
	case PFSHORT:	val.v_short = hips_lchar; break;
	case PFUSHORT:	val.v_ushort = hips_lchar; break;
	case PFINT:	val.v_int = hips_lchar; break;
	case PFUINT:	val.v_uint = hips_lchar; break;
	case PFFLOAT:	val.v_float = hips_lchar; break;
	case PFDOUBLE:	val.v_double = hips_lchar; break;
	}

	/* do the work */

	clearroi(&hdo);
	h_setimage(&hdo,&val);
	for (f=0;f<maxfr;f++) {
		for (file=0;file<numfiles;file++) {
			if (f < nfr[file]) {
				fread_imagec(fp[file],hd[file],hdp[file],
					method[file],f,filelist[file]);
				setroi(&hdo,ir[file],ic[file],nr[file],
					nc[file]);
				h_copy(hdp[file],&hdo);
			}
			else if (f == nfr[file]) {
				setroi(&hdo,ir[file],ic[file],nr[file],
					nc[file]);
				h_setimage(&hdo,&val);
			}
		}
		write_image(&hdo,f);
	}
	return(0);
}
