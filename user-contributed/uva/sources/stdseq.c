/*	Copyright (c)

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 * stdseq.c -  calculates the standard deviation of an image sequence
 *             pixel by pixel and stores the result.
 *             The sequence and the results are either integer or
 *             float format.
 *
 * usage:	stdseq	iseq.ave < iseq >oseq
 *              [ iseq.ave is the mean image of the iseq. ]
 *
 * to load:     compile stdseq	
 *
 *              6/29/87
 */

#include <stdio.h>
#include <hipl_format.h>
#include <math.h>

int main (argc,argv)

int argc;
char **argv;

{
	struct header hdf,hd;
	int f,nrc,i,nbytes,nfr;
	FILE *fpp;
	float *fpic,*pic,*fvar,*fv,fdiv,*fp,*p;
	int *ifpic,*ipic,*ivar,*iv,idiv,*ifp,*ip;
	char tmp[100];

	Progname = strsave(*argv);
	if (argv[argc-1][0]=='-' && argv[argc-1][1]=='D') argc--;
	if (argc<2)
		perr(HE_MSG,"file argument missing");
	if ((fpp = fopen(argv[1],"r")) == NULL) {
		sprintf(tmp,"can't open frame file - %s",argv[1]);
		perr(HE_MSG,tmp);
	}
	read_header(&hd);
	fread_header(fpp,&hdf,argv[1]);
	if (hd.orows != hdf.orows || hd.ocols != hdf.ocols ||
	    hd.pixel_format != hdf.pixel_format)
		perr(HE_MSG,"frame file and input header mismatch");
	if (hd.pixel_format != PFFLOAT && hd.pixel_format != PFINT)
		perr(HE_MSG,"input must be float or integer");
	nrc = hd.orows*hd.ocols;
        nfr =  hd.num_frame;
	hd.num_frame = 1;
	update_header(&hd,argc,argv);
	write_header(&hd);
	switch (hd.pixel_format) {
        case PFFLOAT:
                nbytes = nrc*sizeof(float);
		pic = (float *) halloc(nrc,sizeof(float));
		fpic= (float *) halloc(nrc,sizeof(float));
                fvar = (float *) halloc(nrc,sizeof(float));
		if (fread(fpic,nrc*sizeof(float),1,fpp)!=1)
			perr(HE_MSG,"error during fixed frame read");
		for (f=0;f<nfr;f++) {
			if (fread(pic,nrc*sizeof(float),1,stdin)!=1)
				perr(HE_MSG,"error during read");
			p = pic;
			fp = fpic;
                        fv = fvar;
			for (i=0;i<nrc;i++,fp++,p++)
	                    *fv++ = (*fp - *p)*(*fp - *p); 
		}
		break;
	case PFINT:
                nbytes = nrc*sizeof(int);
		ipic = (int *) halloc(nrc,sizeof(int));
		ifpic= (int *) halloc(nrc,sizeof(int));
                ivar = (int *) halloc(nrc,sizeof(int));
		if (fread(ifpic,nrc*sizeof(int),1,fpp)!=1)
			perr(HE_MSG,"error during fixed frame read");
		for (f=0;f<nfr;f++) {
			if (fread(ipic,nrc*sizeof(int),1,stdin)!=1)
				perr(HE_MSG,"error during read");
			ip = ipic;
			ifp = ifpic;
                        iv = ivar;
			for (i=0;i<nrc;i++,ifp++,ip++)
                            *iv++ = (*ifp - *ip)*(*ifp - *ip);
		}
		break;
	}
	switch (hd.pixel_format) {
	case PFFLOAT:
                fv = fvar;
		fdiv = (float) nfr - 1;
		for (i=0;i<nrc;i++,fv++)
			*fv = sqrt( (double) *fv / fdiv);
	if (fwrite(fvar,nbytes,1,stdout) != 1)
		perr(HE_MSG,"error during write");
		break;
	case PFINT:
                iv = ivar;
		idiv = nfr - 1;
		for (i=0;i<nrc;i++,iv++)
			*iv = sqrt( (double) *iv / idiv);
	if (fwrite(ivar,nbytes,1,stdout) != 1)
		perr(HE_MSG,"error during write");
		break;
	}

	return(0);
}
