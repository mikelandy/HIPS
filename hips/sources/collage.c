/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * collage.c - combine 2 or more sequences into a single collage sequence
 *
 * usage: collage [-s rows [cols]] [-f] [-e]
 *			file1 ir1 ic1
 *			file2 ir2 ic2
 *				.
 *				.
 *				.
 *			filen irn icn > collage-file
 *
 * Collage combines two or more sequences of frames into a single 
 * sequence.  Corresponding frames from each sequence are combined into a
 * single frame.  The output frame is initially set to the background color, 
 * which is specified with the standard -UL switch and defaults to 0.  Then,
 * file1's frame is placed with its upper-left corner at (ir1,ic1).  Then,
 * file2's frame is placed with its upper-left corner at (ir2,ic2), possibly
 * overlaying part of file1's frame.  This process continues until all n files'
 * frames have been read, and then the frame is output.  The process repeats
 * for each input frame.  The size of the output frame may be set with the -s
 * switch (and cols defaults to rows).  Otherwise it defaults to the minimum
 * size needed to contain all of the input frames (except that the output
 * frames upper-left corner is always at position (0,0)).  Input frames will be
 * clipped at the edges of the output frame and need not fit entirely within
 * the output frame.  The output sequence will have as many frames as the
 * longest input sequence.  If any input sequence is shorter than the maximum,
 * then either it will simply no longer take part (the default), or the last
 * frame will be repeatedly used (a "freeze-frame", specified with the -f
 * switch).  For bit-packed output frames, the initial column positions will be
 * clipped to an even multiple of 8.  Only the regions-of-interest of the
 * input files are collaged.  However, if -e is specified, the input regions
 * of interest are reset to be the entire image, so that entire images are
 * collaged.  The output region-of-interest is cleared to be the entire image.
 *
 * Load:	cc -o collage collage.c -lhipsh -lhips
 *
 * Michael Landy - 11/14/86
 * HIPS 2 - Michael Landy - 7/20/91
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
	{"f",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"e",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	LASTFLAG};
char fileusage[] = "file1 ir1 ic1 ... filen irn icn";
h_boolean isinteger();
void print_usage();

int main(argc,argv)

int argc;
char **argv;

{
	struct header **hd,**hdp,hdo;
	FILE **fp;
	int *method,numfiles,outr,outc,*ir,*ic,*nr,*nc,*nfr,maxr,maxc,maxfr;
	int f,iir,iic,nnr,nnc,file,ofmt,ffrow,ffcol,maxinsize,maxprocsize;
	char *savedesc,*savehist,msg[100];
	Filename *filelist;
	h_boolean foundone=FALSE,fflag,*nocopy,eflag,clearflag;
	Pixelval val;
	byte *t1,*t2;

	Progname = strsave(*argv);
	parseargsu(argc,argv,flagfmt,&outr,&outc,&fflag,&eflag,
		FFLIST,&numfiles,&filelist,fileusage);
	if (outc < 0)
		outc = outr;
	if (numfiles%3 != 0)
		print_usage();
	numfiles /= 3;
	fp = (FILE **) memalloc(numfiles,sizeof(FILE *));
	hd = (struct header **) memalloc(numfiles,sizeof(struct header *));
	hdp = (struct header **) memalloc(numfiles,sizeof(struct header *));
	method = (int *) memalloc(numfiles,sizeof(int));
	ir = (int *) memalloc(numfiles,sizeof(int));
	ic = (int *) memalloc(numfiles,sizeof(int));
	nc = (int *) memalloc(numfiles,sizeof(int));
	nr = (int *) memalloc(numfiles,sizeof(int));
	nfr = (int *) memalloc(numfiles,sizeof(int));
	nocopy = (h_boolean *) memalloc(numfiles,sizeof(h_boolean));
	for (file=0;file<numfiles;file++) {
		hd[file] = (struct header *) memalloc(1,sizeof(struct header));
		hdp[file] = (struct header *) memalloc(1,sizeof(struct header));
	}

	/* First: read/merge headers, check consistency, compute max format */

	for (file=0;file<numfiles;file++) {
		fp[file] = hfopenr(filelist[3*file]);
		if (isinteger(filelist[3*file+1]))
			ir[file] = atoi(filelist[3*file+1]);
		else
			print_usage();
		if (isinteger(filelist[3*file+2]))
			ic[file] = atoi(filelist[3*file+2]);
		else
			print_usage();
		if (file == 0) {
			fread_hdr_cpf(fp[0],hd[0],types2,filelist[3*file]);
			ofmt = type_is_col3(hd[0]) ? PFBYTE :
				hd[0]-> pixel_format;
		}
		else {
			fread_hdr_cc(fp[file],hd[file],hd[0],
				CM_DEPTH|CM_NUMCOLOR3,filelist[3*file]);
			ofmt = maxformat(ofmt,hd[file]->pixel_format,
				types,filelist[0],filelist[file*3]);
		}
		if (hips_fullxpar && file>0)
			mergeparam(hd[0],hd[file]);
		if (hips_fulldesc) {
			if (hd[file]->sizedesc > 1) {
				savedesc = hd[file]->seq_desc;
				if (foundone)
				  desc_append2(hd[0],HEP_SDS,
				    "****%s: sequence %d, file: %s****\n",
					Progname,file,filelist[3*file]);
				else
				  desc_set2(hd[0],HEP_SDS,
				    "****%s: sequence %d, file: %s****\n",
					Progname,file,filelist[3*file]);
				foundone = TRUE;
				desc_indentadd(hd[0],savedesc);
			}
		}
		if (hips_fullhist) {
			savehist = hd[file]->seq_history;
			if (file == 0)
			    history_set(hd[0],HEP_SDS,
				"****%s: sequence %d, file: %s****\n",Progname,
				file,filelist[3*file]);
			else
			    history_append(hd[0],HEP_SDS,
				"****%s: sequence %d, file: %s****\n",Progname,
				file,filelist[3*file]);
			history_indentadd(hd[0],savehist);
		}
	}

	/* compute frame size/number of frames */

	maxr = maxc = maxfr = 0;
	for (file=0;file<numfiles;file++) {
		if (eflag)
			clearroi(hd[file]);
		if ((ofmt == PFMSBF || ofmt == PFLSBF) && (ic[file]%8 != 0)) {
			ic[file] = (ic[file]/8)*8;
			sprintf(msg,
			 "binary output, column position reset to %d, file: %s",
			 ic[file],filelist[3*file]);
			perr(HE_IMSG,msg);
		}
		nr[file] = hd[file]->rows;
		nc[file] = hd[file]->cols;
		nfr[file] = hd[file]->num_frame;
		if (type_is_col3(hd[file]))
			nfr[file] *= 3;
		if (maxr < nr[file]+ir[file])
			maxr = nr[file]+ir[file];
		if (maxc < nc[file]+ic[file])
			maxc = nc[file]+ic[file];
		if (maxfr < nfr[file])
			maxfr = nfr[file];
	}
	if (outr < 0) {
		outr = maxr;
		outc = maxc;
	}
	if (outr <= 0 || outc <= 0)
		perr(HE_MSG,"no files overlap output image");

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

	/* set up from/to roi's for copying, calculate methods */

	maxinsize = maxprocsize = 0;
	for (file=0;file<numfiles;file++) {
		iir = ir[file];
		iic = ic[file];
		nnr = nr[file];
		nnc = nc[file];
		ffrow = hd[file]->frow;
		ffcol = hd[file]->fcol;
		if (iir < 0) {
			ffrow -= iir;
			nnr += iir;
			iir = 0;
		}
		if (iir + nnr > outr)
			nnr = outr - iir;
		if (iic < 0) {
			ffcol -= iic;
			nnc += iic;
			iic = 0;
		}
		if (iic + nnc > outc)
			nnc = outc - iic;
		if (nnr <= 0 || nnc <= 0) {
			nocopy[file] = TRUE;
			sprintf(msg,
				"file %s skipped, no overlap with output image",
				filelist[3*file]);
			perr(HE_IMSG,msg);
		}
		else {
			setroi(hd[file],ffrow,ffcol,nnr,nnc);
			nocopy[file] = FALSE;
			ir[file] = iir;
			ic[file] = iic;
			nr[file] = nnr;
			nc[file] = nnc;
		}
		method[file] = ffind_method(hd[file]->pixel_format,ofmt,
			filelist[3*file]);
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
				filelist[3*file]);
		else if (method[file] != METH_IDENT)
			perr(HE_CONV,
				hformatname_f(hd[file]->pixel_format,ofmt),
				hformatname_t(hd[file]->pixel_format,ofmt),
				filelist[3*file]);
	}

	/* allocate input image and converted image */

	if (maxprocsize == 0)
		perr(HE_MSG,"no files overlap output image");
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

	clearflag = TRUE;

	for (f=0;f<maxfr;f++) {
		if (clearflag) {
			clearroi(&hdo);
			h_setimage(&hdo,&val);
			clearflag = FALSE;
		}
		for (file=0;file<numfiles;file++) {
			if (nocopy[file])
				continue;
			if (f < nfr[file])
				fread_imagec(fp[file],hd[file],hdp[file],
					method[file],f,filelist[3*file]);
			if (f < nfr[file]) {
				setroi(&hdo,ir[file],ic[file],nr[file],
					nc[file]);
				h_copy(hdp[file],&hdo);
			}
			if ((f == nfr[file]-1) && !fflag)
				clearflag = TRUE;
		}
		write_image(&hdo,f);
	}
	return(0);
}
