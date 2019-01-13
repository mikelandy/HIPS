/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * interdepth.c - interleave depth planes
 *
 * Usage:	interdepth file1 file2 ...
 *
 * The region of interest is taken from file1.
 *
 * Load:	cc -o interdepth interdepth.c -lhips
 *
 * Michael Landy - 3/2/94
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
	int numfiles,i,j,k,l,f,ofmt,*method,nf,nc,nd,nod,saved1;
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
	nod = hgetdepth(hd[0]);
	nf = hd[0]->num_frame/(hd[0]->numcolor * nod);
	nc = type_is_col3(hd[0]) ? 3 : hd[0]->numcolor;
	for (i=1;i<numfiles;i++) {
		fp[i] = hfopenr(filelist[i]);
		fread_hdr_cc(fp[i],hd[i],hd[0],
			CM_OROWS|CM_OCOLS|CM_NUMCOLOR3|CM_NUMLEV,filelist[i]);
		nd = hgetdepth(hd[i]);
		if ((hd[i]->num_frame / (hd[i]->numcolor * nd)) != nf)
			perr(HE_C_FRM,filelist[i]);
		nod += nd;
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
	saved1 = hgetdepth(&hdp);	/* save in case hdp = hd[0] */
	hsetdepth(&hdp,nod);
	hdp.num_frame = nf*nod*nc;
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
	hsetdepth(&hdp,saved1);		/* restore in case hdp = hd[0] */
	f = 0;
	nc = hdp.numcolor;
	for (i=0;i<nf;i++) {
	    for (j=0;j<numfiles;j++) {
		nd = hgetdepth(hd[j]);
		for (k=0;k<nd;k++) {
		    for (l=0;l<nc;l++) {
			f++;
			fread_imagec(fp[j],hd[j],&hdp,method[j],i*nc*nd+k*nc+l,
				filelist[j]);
			write_image(&hdp,f);
		    }
		}
	    }
	}
	return(0);
}
