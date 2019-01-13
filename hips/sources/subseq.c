/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * subseq.c - extract a subsequence of frames or color planes
 *
 * Usage:	subseq [-f from [to [skip]]] [-c from [to [skip]]]
 *			[-d from [to [skip]]]
 *
 * defaults:
 * frames:	fromframe=0 (first frame)
 *		toframe=numframe-1 (last frame) if -f not specified
 *		toframe=fromframe               if -f specified
 *		increment=1
 *
 * color plane:	fromcolor=0 (first plane)
 *		tocolor=numcolor-1 (last plane) if -c not specified
 *		tocolor=fromcolor               if -c specified
 *		incrememt=1
 *
 * depth:	fromdepth=0 (first plane)
 *		todepth=numdepth-1 (last plane) if -d not specified
 *		todepth=fromdepth               if -d specified
 *		incrememt=1
 *
 * PFMIXED input files remain PFMIXED only if the output subsequence includes
 * more than one pixel format.  Color plane extraction is not possible with
 * 3color formats (PFRGB, etc.), and so one should first convert to byte using
 * htob.
 *
 * Load:	cc -o subseq subseq.c -lhips
 *
 * Y. Cohen 3/1/82
 * modified for lseek on disk files by Mike Landy 7/3/84
 * HIPS 2 - msl - 7/6/91
 * PFMIXED - msl - 11/12/92
 * RGB/RGBZ/etc. - msl - 5/24/93
 * -d flag - msl - 3/3/94
 */

#include <stdio.h>
#include <hipl_format.h>
#include <sys/types.h>
#include <sys/stat.h>

int types[] = {PFMSBF,PFLSBF,PFBYTE,PFSBYTE,PFSHORT,PFUSHORT,PFINT,PFUINT,
	PFFLOAT,PFDOUBLE,PFCOMPLEX,PFDBLCOM,PFINTPYR,PFFLOATPYR,PFMIXED,
	PFRGB,PFRGBZ,PFZRGB,PFBGR,PFBGRZ,PFZBGR,LASTTYPE};
static Flag_Format flagfmt[] = {
	{"f",{LASTFLAG},1,{{PTINT,"-1","from"},{PTINT,"-1","to"},
		{PTINT,"1","increment"},LASTPARAMETER}},
	{"c",{LASTFLAG},1,{{PTINT,"-1","from"},{PTINT,"-1","to"},
		{PTINT,"1","increment"},LASTPARAMETER}},
	{"d",{LASTFLAG},1,{{PTINT,"-1","from"},{PTINT,"-1","to"},
		{PTINT,"1","increment"},LASTPARAMETER}},
	LASTFLAG};

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd;
	Filename filename;
	FILE *fp;
	int ffrom,fto,finc,cfrom,cto,cinc,nf,nc,i,j,lastpos,currfr,noc,nof,k;
	int *nfmts,*fmts,fmtssize,ofr,dfrom,dto,dinc,nd,nod,jj;
	h_boolean piped=FALSE,mflag=FALSE,omixed;
	struct stat buf;
	hsize_t currsize;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&ffrom,&fto,&finc,&cfrom,&cto,&cinc,
		&dfrom,&dto,&dinc,FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_cpfa(fp,&hd,types,filename);
	nc = hd.numcolor;
	nd = hgetdepth(&hd);
	nf = hd.num_frame/(nc*nd);
	if (nf*nc*nd != hd.num_frame)
		perr(HE_IMSG,
			"num_frame not a multiple of numcolor*depth in header");
	if (ffrom<0) {
		ffrom = 0;
		fto = nf - 1;
	}
	else if (fto<0)
		fto = ffrom;
	if (ffrom > fto)
		perr(HE_MSG,"to-frame must not be less than from-frame");
	if (finc<1)
		perr(HE_MSG,"frame increment must be positive");
	if (cfrom<0) {
		cfrom = 0;
		cto = nc - 1;
	}
	else if (cto<0)
		cto = cfrom;
	if (cfrom > cto)
		perr(HE_MSG,"to-color must not be less than from-color");
	if ((cfrom != 0 || cto != 0) && type_is_col3(hd))
		perr(HE_MSG,
			"color plane extraction only in byte format, use htob");
	if (cinc<1)
		perr(HE_MSG,"color increment must be positive");
	if (dfrom<0) {
		dfrom = 0;
		dto = nd - 1;
	}
	else if (dto<0)
		dto = dfrom;
	if (dfrom > dto)
		perr(HE_MSG,"to-depth must not be less than from-depth");
	if (dinc<1)
		perr(HE_MSG,"depth increment must be positive");
	if (ffrom >= nf)
		perr(HE_MSG,"subsequence not in sequence");
	if (cfrom >= nc)
		perr(HE_MSG,"color subset not in color plane set");
	if (dfrom >= nd)
		perr(HE_MSG,"depth subset not in depth set");
	if (fto >= nf) {
		perr(HE_IMSG,"warning, some frames are not in input sequence");
		fto = nf-1;
	}
	if (cto >= nc) {
		perr(HE_IMSG,"warning, some colors are not in input sequence");
		cto = nc-1;
	}
	if (dto >= nd) {
		perr(HE_IMSG,"warning, some depths are not in input sequence");
		dto = nd-1;
	}
	if (fseek(fp,0L,1) < 0)
		piped = TRUE;
	nof = 1 + ((fto-ffrom)/finc);
	noc = 1 + ((cto-cfrom)/cinc);
	nod = 1 + ((dto-dfrom)/dinc);
	hd.num_frame = nof*noc*nod;
	hd.numcolor = noc;
	if (nd != nod)
		hsetdepth(&hd,nod);
#ifdef S_IFIFO
	if (!piped) {
		fstat(fileno(fp),&buf);
		if ((buf.st_mode & S_IFMT) == S_IFIFO)
			piped = TRUE;
	}
#endif
	if (hd.pixel_format == PFMIXED) {
		mflag = TRUE;
		fmtssize = nf*nc*nd;
		getparam(&hd,"formats",PFINT,&fmtssize,&fmts);
		if (fmtssize != nf*nc*nd)
			perr(HE_FMTSLEN,filename);
		currsize = 0;
		nfmts = (int *) hmalloc(nof*noc*nod*sizeof(int));
		ofr = 0;
		omixed = FALSE;
		for (i=0;i<nof;i++) {
		    for (jj=0;jj<nod;jj++) {
			for (j=0;j<noc;j++) {
				currfr = (ffrom+i*finc)*nc*nd +
					 (dfrom+jj*dinc)*nc + cfrom + j*cinc;
				nfmts[ofr++] = fmts[currfr];
				if (fmts[currfr] != nfmts[0])
					omixed = TRUE;
			}
		    }
		}
		hd.paramdealloc = FALSE;	/* don't dealloc fmts */
		if (omixed)
			setparam(&hd,"formats",PFINT,nof*nod*noc,nfmts);
		else {
			clearparam(&hd,"formats");
			setformat(&hd,nfmts[0]);
		}
	}
	write_headeru(&hd,argc,argv);
	lastpos = 0;
	ofr = 0;
	if (mflag) {
	    for (i=0;i<nof;i++) {
		for (jj=0;jj<nod;jj++) {
		    for (j=0;j<noc;j++) {
			currfr = (ffrom+i*finc)*nd*nc +
				(dfrom+jj*dinc)*nc + cfrom + j*cinc;
			if (currfr != lastpos) {
			    for (k=lastpos;k<currfr;k++) {
				setformat(&hd,fmts[lastpos]);
				if (piped || fseek(fp,hd.sizeimage,1) < 0) {
				    piped = TRUE;
				    if (hd.sizeimage > currsize) {
					free_image(&hd);
					alloc_image(&hd);
					currsize = hd.sizeimage;
				    }
				    fread_image(fp,&hd,k,filename);
				}
			    }
			}
			setformat(&hd,fmts[currfr]);
			if (hd.sizeimage > currsize) {
			    free_image(&hd);
			    alloc_image(&hd);
			    currsize = hd.sizeimage;
			}
			fread_image(fp,&hd,currfr,filename);
			write_image(&hd,ofr++);
			lastpos = currfr + 1;
		    }
		}
	    }
	}
	else {
	    for (i=0;i<nof;i++) {
		for (jj=0;jj<nod;jj++) {
		    for (j=0;j<noc;j++) {
			currfr = (ffrom+i*finc)*nd*nc +
				(dfrom+jj*dinc)*nc + cfrom + j*cinc;
			if (currfr != lastpos) {
			    if (piped ||
			      fseek(fp,(currfr-lastpos)*hd.sizeimage,1) < 0) {
				piped = TRUE;
				for (k=lastpos;k<currfr;k++)
					fread_image(fp,&hd,k,filename);
			    }
			}
			fread_image(fp,&hd,currfr,filename);
			write_image(&hd,ofr++);
			lastpos = currfr + 1;
		    }
		}
	    }
	}
	return(0);
}
