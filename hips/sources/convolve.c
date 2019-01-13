/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * convolve.c - 3D convolution
 *
 * usage: convolve kernel < iseq > oseq
 *
 * The output is a full spatio-temporal convolution of the two input
 * sequences, and hence has a size which is the sum of the two input sizes
 * minus 1.  For large sequences it can run into space problems.  It does its
 * best by only allocating enough space for the entirety of the smaller of the
 * two input sequences (two copies).
 *
 * To load: cc -o convolve convolve.c  -lhipsh -lhips -lm
 *
 * HIPS 2 - msl - 8/10/91
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {LASTFLAG};
int types[] = {PFINT,PFFLOAT,LASTTYPE};

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd,hdp,hdf,hdfp,hdo;
	int method1,method2,f,ofmt,savesize,*pi;
	int nri,nci,nfi,nrk,nck,nfk,nfo,currfr,firstk,firsti,numf,minf,maxf;
	char *savehist,*savedesc;
	Filename filename1,filename2;
	FILE *fp1,*fp2;
	h_boolean bigkernel;
	float *pf;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,FFTWO,&filename1,&filename2);
	fp1 = hfopenr(filename1);
	fp2 = hfopenr(filename2);
	fread_header(fp2,&hd,filename2);
	fread_header(fp1,&hdf,filename1);
	clearroi(&hd);
	clearroi(&hdf);
	ofmt = maxformat(hd.pixel_format,hdf.pixel_format,types,
		filename2,filename1);
	method2 = pset_conversion(&hd,&hdp,ofmt,filename2);
	method1 = pset_conversion(&hdf,&hdfp,ofmt,filename1);
	if (hdp.numcolor > 1 || hdfp.numcolor > 1)
		perr(HE_MSG,"number of color planes must be 1");
	nri = hd.rows;
	nci = hd.cols;
	nfi = hdp.num_frame;
	nrk = hdf.rows;
	nck = hdf.cols;
	nfk = hdfp.num_frame;
	nfo = nfi + nfk - 1;
	minf = (nfk < nfi) ? nfk : nfi;
	maxf = (nfk < nfi) ? nfi : nfk;
	bigkernel = nfk > nfi;
	if (bigkernel) {
		setsize(&hdp,nri*nfi,nci);	/* fake the image allocation */
		alloc_image(&hdp);
		setsize(&hdp,nri,nci);
		if (method2 == METH_IDENT)
			hd.image = hd.firstpix = hdp.image;
		else
			alloc_image(&hd);
		setsize(&hdfp,nrk*nfi,nck);	/* fake the image allocation */
		alloc_image(&hdfp);
		setsize(&hdfp,nrk,nck);
		if (method1 == METH_IDENT)
			hdf.image = hdf.firstpix = hdfp.image;
		else
			alloc_image(&hdf);
	}
	else {
		setsize(&hdfp,nrk*nfk,nck);	/* fake the image allocation */
		alloc_image(&hdfp);
		setsize(&hdfp,nrk,nck);
		if (method1 == METH_IDENT)
			hdf.image = hdf.firstpix = hdfp.image;
		else
			alloc_image(&hdf);
		setsize(&hdp,nri*nfk,nci);	/* fake the image allocation */
		alloc_image(&hdp);
		setsize(&hdp,nri,nci);
		if (method2 == METH_IDENT)
			hd.image = hd.firstpix = hdp.image;
		else
			alloc_image(&hd);
	}
	dup_headern(&hdp,&hdo);
	setsize(&hdo,hd.rows+hdf.rows-1,hd.cols+hdf.cols-1);
	hdo.num_frame = nfo;
	alloc_image(&hdo);
	if (hips_fullxpar)
		mergeparam(&hdo,&hdfp);
	if (hips_fulldesc) {
		savedesc = hdp.seq_desc;
		savesize = hdp.sizedesc;
		if (hdfp.sizedesc > 1) {
			desc_set2(&hdo,HEP_SS,
				"****%s: kernel sequence, file: %s****\n",
				Progname,filename1);
			desc_indentadd(&hdo,hdfp.seq_desc);
			if (savesize > 1) {
				desc_append2(&hdo,HEP_SS,
				    "****%s: input sequence, file: %s****\n",
				    Progname,filename2);
				desc_indentadd(&hdo,savedesc);
			}
		}
		else if (hdp.sizedesc > 1) {
			desc_set2(&hdo,HEP_SS,
				"****%s: input sequence, file: %s****\n",
				Progname,filename2);
			desc_indentadd(&hdo,savedesc);
		}
	}
	if (hips_fullhist) {
		savehist = hdp.seq_history;
		history_set(&hdo,HEP_SS,
			"****%s: kernel sequence, file: %s****\n",Progname,
			filename1);
		history_indentadd(&hdo,hdfp.seq_history);
		history_append(&hdo,HEP_SS,
			"****%s: input sequence, file: %s****\n",Progname,
			filename2);
		history_indentadd(&hdo,savehist);
		write_headerun(&hdo,argc,argv);
	}
	else
		write_headeru(&hdo,argc,argv);
	if (bigkernel) {
		if (ofmt == PFINT) {
			pi = (int *) hdp.image;
			for (f=0;f<nfi;f++) {
				hdp.image = (byte *) (pi + f*nri*nci);
				fread_imagec(fp2,&hd,&hdp,method2,f,filename2);
			}
			hdp.image = (byte *) pi;
		}
		else {
			pf = (float *) hdp.image;
			for (f=0;f<nfi;f++) {
				hdp.image = (byte *) (pf + f*nri*nci);
				fread_imagec(fp2,&hd,&hdp,method2,f,filename2);
			}
			hdp.image = (byte *) pf;
		}
	}
	else {
		if (ofmt == PFINT) {
			pi = (int *) hdfp.image;
			for (f=0;f<nfk;f++) {
				hdfp.image = (byte *) (pi + f*nrk*nck);
				fread_imagec(fp1,&hdf,&hdfp,method1,f,
					filename1);
			}
			hdfp.image = (byte *) pi;
		}
		else {
			pf = (float *) hdfp.image;
			for (f=0;f<nfk;f++) {
				hdfp.image = (byte *) (pf + f*nrk*nck);
				fread_imagec(fp1,&hdf,&hdfp,method1,f,
					filename1);
			}
			hdfp.image = (byte *) pf;
		}
	}
	currfr = 0;
	for (f=0;f<nfo;f++) {
		if (bigkernel) {
			if (ofmt == PFINT) {
				pi = (int *) hdfp.image;
				hdfp.image = (byte *) (pi + currfr*nrk*nck);
				if (f < nfk)
					fread_imagec(fp1,&hdf,&hdfp,method1,f,
						filename1);
				hdfp.image = (byte *) pi;
			}
			else {
				pf = (float *) hdfp.image;
				hdfp.image = (byte *) (pf + currfr*nrk*nck);
				if (f < nfk)
					fread_imagec(fp1,&hdf,&hdfp,method1,f,
						filename1);
				hdfp.image = (byte *) pf;
			}
			currfr = (currfr + 1) % nfi;
		}
		else {
			if (ofmt == PFINT) {
				pi = (int *) hdp.image;
				hdp.image = (byte *) (pi + currfr*nri*nci);
				if (f < nfi)
					fread_imagec(fp2,&hd,&hdp,method2,f,
						filename2);
				hdp.image = (byte *) pi;
			}
			else {
				pf = (float *) hdp.image;
				hdp.image = (byte *) (pf + currfr*nri*nci);
				if (f < nfi)
					fread_imagec(fp2,&hd,&hdp,method2,f,
						filename2);
				hdp.image = (byte *) pf;
			}
			currfr = (currfr + 1) % nfk;
		}
		numf = (f < minf) ? (f + 1) :
			((f < maxf) ? minf : (nfo - f));
		firstk = (nfk + f - nfo > 0) ? (nfk + f - nfo) : 0;
		firsti = (nfi + f - nfo > 0) ? (nfi + f - nfo) : 0;
		if (bigkernel) {
			firstk %= nfi;
			h_convolve(&hdfp,&hdp,&hdo,firstk,firsti,numf);
		}
		else {
			firsti %= nfk;
			h_convolve(&hdp,&hdfp,&hdo,firsti,firstk,numf);
		}
		write_image(&hdo,f);
	}
	return(0);
}
