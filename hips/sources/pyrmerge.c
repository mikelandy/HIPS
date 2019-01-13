/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * pyrmerge - merge several images or pyramids into a single pyramid
 *
 * usage:	pyrmerge [-i | -f] file1 ... filen
 *
 * to load:	cc -o pyrmerge pyrmerge.c -lhips
 *
 * Pyrmerge merges two or more images or pyramids into a single image pyramid.
 * All input files must be either combinations of raster images and
 * floating point pyramids, or raster images and integer pyramids.  All must
 * have the same number of frames, and must have row and column dimensions
 * which are consistent with a single image pyramid, and with each other if
 * at the same level.  Effectively, a single image pyramid is generated
 * which has a bottom level with dimensions of the largest input image, and
 * a top level with dimensions of the smallest input image.  This pyramid is
 * cleared to zeros, and then for each frame, the next image or pyramid is
 * read into the appropriate level or levels, beginning with file1 and ending
 * with filen.  Thus, if a later file reads to the same level as a previous
 * file, that information will be replaced.  Pyrmerge first examines the first
 * image header to determine the output format.  If it sees a pyramid first,
 * then it knows what the output format will be (the same as the type of that
 * pyramid).  If it sees a raster image first, then it will determine the
 * output format based on the raster format and the usual rules for format
 * conversion (float, double, complex and double complex result in a float
 * pyramid, and all others in an integer pyramid).  Once the output format is
 * set, all later images must either be raster (and will be converted to the
 * appropriate format: float for float pyramid output, and integer for integer
 * pyramid output) or must be of the same type of pyramid as the chosen
 * output format.  The chosen output format may be forced to be integer (with
 * -i) or floating point (with -f).
 *
 * input formats handled directly: INT, FLOAT, INTPYR, FLOATPYR
 *
 * Mike Landy - 3/6/89
 * Hips 2 - msl - 7/19/91
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {
    {"f",{"i",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
    {"i",{"f",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
    LASTFLAG};
int types[] = {PFINT,PFFLOAT,PFINTPYR,PFFLOATPYR,LASTTYPE};
int typesf[] = {PFFLOAT,PFFLOATPYR,LASTTYPE};
int typesi[] = {PFINT,PFINTPYR,LASTTYPE};

int main(argc,argv)

int argc;
char **argv;

{
	struct header **hd,hdt,hdo;
	FILE **fp;
	int file,numfiles,f,*method,*botlev,nr[2*MAXLEV],one=1;
	int nc[2*MAXLEV],ofmt,r,c,botl,topl,rr,cc,nfr,i,*numlev,j,maxr,maxc;
	char *savedesc,*savehist;
	Filename *filelist;
	h_boolean foundone=FALSE,iflag,fflag;
	FPYR fpyr;
	IPYR ipyr;
	char msg[100];

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&fflag,&iflag,FFLIST,&numfiles,&filelist);
	if (numfiles < 2)
		perr(HE_MSG,"number of files must be at least two");
	Image_border = 0;
	fp = (FILE **) memalloc(numfiles,sizeof(FILE *));
	hd = (struct header **) memalloc(numfiles,sizeof(struct header *));
	method = (int *) memalloc(numfiles,sizeof(int));
	botlev = (int *) memalloc(numfiles,sizeof(int));
	numlev = (int *) memalloc(numfiles,sizeof(int));
	for (i=0;i<numfiles;i++)
		hd[i] = (struct header *) memalloc(1,sizeof(struct header));

	/* First: read headers, compute pyramid levels, check consistency */

	for (file=0;file<numfiles;file++) {
		fp[file] = hfopenr(filelist[file]);
		if (file == 0)
			fread_header(fp[0],hd[0],filelist[file]);
		else
			fread_hdr_cc(fp[file],hd[file],hd[0],
				CM_FRAMES|CM_NUMCOLOR3,filelist[file]);
		i = hd[file]->pixel_format;
		if (i == PFINTPYR || i == PFFLOATPYR)
			getparam(hd[file],"toplev",PFINT,&one,&(numlev[file]));
		else
			numlev[file] = 0;
		ofmt = ffind_closest(hd[file],
			fflag ? typesf : (iflag ? typesi : types),
			filelist[file]);
		if (!fflag && !iflag && file == 0) {
			if (ofmt == PFFLOAT || ofmt == PFFLOATPYR)
				fflag = TRUE;
			else
				iflag = TRUE;
		}
		method[file] = ffind_method(hd[file]->pixel_format,ofmt,
			filelist[file]);
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
		if (file == 0) {
			nr[MAXLEV] = hd[0]->rows;
			nc[MAXLEV] = hd[0]->cols;
			for (i=MAXLEV+1;i<2*MAXLEV;i++) {
			    nr[i] = (nr[i-1] + 1)/2;
			    nc[i] = (nc[i-1] + 1)/2;
			}
			botlev[0] = botl = MAXLEV;
			topl = botl + numlev[0];
		}
		else {
			r = hd[file]->rows;
			c = hd[file]->cols;
			if (r <= nr[botl]) {
				for (i=botl;i<2*MAXLEV;i++) {
					if (nr[i] == r && nc[i] == c)
					    break;
				}
				if (i >= 2*MAXLEV) {
					sprintf(msg,"size mismatch, file: %s",
						filelist[file]);
					perr(HE_MSG,msg);
				}
				botlev[file] = i;
				topl = MAX((i + numlev[file]),topl);
			}
			else {
				rr = r;
				cc = c;
				for (i=1;i<=botl;i++) {
					rr = (rr + 1)/2;
					cc = (cc + 1)/2;
					if (rr < nr[botl]) {
						sprintf(msg,
						    "size mismatch, file: %s",
						    filelist[file]);
						perr(HE_MSG,msg);
					}
					if (rr == nr[botl] && cc != nc[botl]) {
						sprintf(msg,
						    "size mismatch, file: %s",
						    filelist[file]);
						perr(HE_MSG,msg);
					}
					if (rr == nr[botl] && cc == nc[botl])
					    break;
				}
				if (i > botl)
					perr(HE_MSG,"can't match size!!!????");
				nr[botl-i] = r;
				nc[botl-i] = c;
				for (j=botl-i+1;j<botl;j++) {
					nr[j] = (nr[j-1] + 1)/2;
					nc[j] = (nc[j-1] + 1)/2;
				}
				botlev[file] = botl = botl-i;
			}
		}
	}
	if (topl - botl >= MAXLEV)
		perr(HE_MSG,"total number of levels is too large");

	/* Allocate and set up pyramid */

	if (fflag) {
		def_fpyr(fpyr,0,nr[botl],nc[botl]);
		alloc_fpyr(fpyr,0,topl-botl);
	}
	else {
		def_ipyr(ipyr,0,nr[botl],nc[botl]);
		alloc_ipyr(ipyr,0,topl-botl);
	}

	/* allocate images that require conversion */

	maxr = maxc = 0;

	for (file=0;file<numfiles;file++) {
		if (method[file] == METH_IDENT)
			continue;
		alloc_image(hd[file]);
		if (hd[file]->rows > maxr)
			maxr = hd[file]->rows;
		if (hd[file]->cols > maxc)
			maxc = hd[file]->cols;
	}

	/* allocate one image to convert into of sufficient size for all */

	if (maxr > 0) {
		dup_headern(hd[0],&hdt);
		setformat(&hdt,fflag ? PFFLOAT : PFINT);
		setsize(&hdt,maxr,maxc);
		alloc_image(&hdt);
	}

	/* Merge and output header */

	dup_headern(hd[0],&hdo);
	if (type_is_col3(&hdo)) {
		hdo.num_frame *= 3;
		hdo.numcolor = 3;
	}
	setsize(&hdo,nr[botl],nc[botl]);
	setpyrformat(&hdo,fflag ? PFFLOATPYR : PFINTPYR,topl-botl);
	if (hips_fullxpar)
		for (i=1;i<numfiles;i++)
			mergeparam(&hdo,hd[i]);
	if (hips_fulldesc) {
		for (i=0;i<numfiles;i++) {
			if (hd[i]->sizedesc > 1) {
				savedesc = hd[i]->seq_desc;
				if (foundone)
				  desc_append2(&hdo,HEP_SDS,
				    "****%s: sequence %d, file: %s****\n",
					Progname,i,filelist[i]);
				else
				  desc_set2(&hdo,HEP_SDS,
				    "****%s: sequence %d, file: %s****\n",
					Progname,i,filelist[i]);
				foundone = TRUE;
				desc_indentadd(&hdo,savedesc);
			}
		}
	}
	if (hips_fullhist) {
		savehist = hd[0]->seq_history;
		history_set(&hdo,HEP_SDS,
			"****%s: sequence %d, file: %s****\n",Progname,
			0,filelist[0]);
		history_indentadd(&hdo,savehist);
		for (i=1;i<numfiles;i++) {
			history_append(&hdo,HEP_SDS,
			    "****%s: sequence %d, file: %s****\n",Progname,i,
			    filelist[i]);
			history_indentadd(&hdo,hd[i]->seq_history);
		}
		write_headerun(&hdo,argc,argv);
	}
	else
		write_headeru(&hdo,argc,argv);

	/* Read, merge and output pyramids */

	nfr = hdo.num_frame;
	for (f=0;f<nfr;f++) {
		for (file=0;file<numfiles;file++) {
		    if (method[file] == METH_IDENT) {
			if (fflag)
			    read_fpyr(fp[file],fpyr,botlev[file]-botl,
				botlev[file]+numlev[file]-botl,f,
				filelist[file]);
			else
			    read_ipyr(fp[file],ipyr,botlev[file]-botl,
				botlev[file]+numlev[file]-botl,f,
				filelist[file]);
		    }
		    else {
			setsize(&hdt,hd[file]->rows,hd[file]->cols);
			fread_imagec(fp[file],hd[file],&hdt,method[file],f,
				filelist[file]);
			if (fflag)
				copy_ftoff(&hdt,fpyr[botlev[file]-botl]);
			else
				copy_itoii(&hdt,ipyr[botlev[file]-botl]);
		    }
		}
		if (fflag)
			write_fpyr(stdout,fpyr,0,topl-botl,f);
		else
			write_ipyr(stdout,ipyr,0,topl-botl,f);
	}
	return(0);
}
