/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * intercolor.c - interleave color planes
 *
 * Usage:	intercolor file1 file2 ...
 *
 * The region of interest is taken from file1.
 *
 * Load:	cc -o intercolor intercolor.c -lhips
 *
 * HIPS 2 - Michael Landy - 7/6/91
 */

#include <stdio.h>
#include <hipl_format.h>

int types[] = {PFMSBF,PFLSBF,PFBYTE,PFSBYTE,PFSHORT,PFUSHORT,PFINT,PFUINT,
	PFFLOAT,PFDOUBLE,PFCOMPLEX,PFDBLCOM,PFINTPYR,PFFLOATPYR,PFRGB,
	PFRGBZ,PFZRGB,PFBGR,PFBGRZ,PFZBGR,LASTTYPE};
int types2[] = {PFMSBF,PFLSBF,PFBYTE,PFSBYTE,PFSHORT,PFUSHORT,PFINT,PFUINT,
	PFFLOAT,PFDOUBLE,PFCOMPLEX,PFDBLCOM,PFINTPYR,PFFLOATPYR,LASTTYPE};
static Flag_Format flagfmt[] = {LASTFLAG};

int main(argc,argv)

int argc;
char **argv;

{
	struct header **hd,hdp;
	FILE **fp;
	int numfiles,i,j,k,f,ofmt,*method,nf,noc,nc;
	char *savedesc,*savehist;
	Filename *filelist;
	h_boolean foundone;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,FFLIST,&numfiles,&filelist);
	if (numfiles < 2)
		perr(HE_MSG,"number of files must be at least two");
	fp = (FILE **) memalloc(numfiles,sizeof(FILE *));
	hd = (struct header **) memalloc(numfiles,sizeof(struct header *));
	method = (int *) memalloc(numfiles,sizeof(int));
	for (i=0;i<numfiles;i++)
		hd[i] = (struct header *) memalloc(1,sizeof(struct header));
	fp[0] = hfopenr(filelist[0]);
	fread_hdr_cpfa(fp[0],hd[0],types,filelist[0]);
	nf = hd[0]->num_frame/hd[0]->numcolor;
	noc = type_is_col3(hd[0]) ? 3 : hd[0]->numcolor;
	for (i=1;i<numfiles;i++) {
		fp[i] = hfopenr(filelist[i]);
		fread_hdr_cc(fp[i],hd[i],hd[0],
			CM_OROWS|CM_OCOLS|CM_FRAMES|CM_NUMLEV,filelist[i]);
		noc += type_is_col3(hd[i]) ? 3 : hd[i]->numcolor;
		if (i==1)
			ofmt = maxformat(hd[0]->pixel_format,
				hd[1]->pixel_format,types2,filelist[0],
				filelist[1]);
		else
			ofmt = maxformat(ofmt,hd[i]->pixel_format,types2,
				filelist[0],filelist[i]);
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
		else if (type_is_col3(hd[i]))
			alloc_image(hd[i]);
		else if (hd[i]->pixel_format == hd[0]->pixel_format) {
			hd[i]->sizeimage = hd[0]->sizeimage;
			hd[i]->image = hd[0]->image;
			hd[i]->firstpix = hd[0]->firstpix;
			hd[i]->imdealloc = FALSE;
		}
		else
			alloc_image(hd[i]);
	}
	hdp.numcolor = noc;
	hdp.num_frame = nf*noc;
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
	f = 0;
	for (i=0;i<nf;i++) {
	    for (j=0;j<numfiles;j++) {
		nc = type_is_col3(hd[j]) ? 3 : hd[j]->numcolor;
		for (k=0;k<nc;k++) {
			f++;
			fread_imagec(fp[j],hd[j],&hdp,method[j],(i*nc)+k,
				filelist[j]);
			write_image(&hdp,f);
		}
	    }
	}
	return(0);
}
