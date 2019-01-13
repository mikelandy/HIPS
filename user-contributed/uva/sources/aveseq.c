/*	Copyright (c) 1982 Michael Landy, Yoav Cohen, and George Sperling

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 * aveseq.c - compute the average image from a sequence in
 *	   byte, short, integer or float format.
 *
 * usage:	aveseq	<iseq >oseq
 *
 * to load:	cc -o aveseq aveseq.c -lhips
 *
 * Michael Landy 10/27/83
 * modified - msl, 4/18/85
 * modified - csc, 6/1/87
 */

#include <stdio.h>
#include <hipl_format.h>

int main (argc,argv)
	int argc;
	char **argv;
{
	struct header hd;
	register float fdiv;
	int f,nfr,nrc,nbytes;
	register int i, idiv;
	float *fpic,*fp,*fsum,*fsp;
	int *ipic,*ip,*isum,*isp;
	short *spic, *sp;
	unsigned char *cpic, *cp;
	byte *rwbuf;

	Progname = strsave(*argv);
	read_header(&hd);
	if (hd.pixel_format != PFFLOAT && hd.pixel_format != PFINT &&
		hd.pixel_format != PFSHORT && hd.pixel_format != PFBYTE)
		perr(HE_MSG,"input must be byte, short, integer, or float");
	nrc = hd.orows*hd.ocols;
	nfr = hd.num_frame;
	hd.num_frame = 1;
	update_header(&hd,argc,argv);
	write_header(&hd);
	switch (hd.pixel_format) {
	case PFFLOAT:
		nbytes = nrc * sizeof(float);
		rwbuf = halloc(nrc,sizeof(float));
		fsum = (float *) halloc(nrc,sizeof(float));
		fpic = (float *) rwbuf;
		break;
	case PFINT:
		nbytes = nrc * sizeof(int);
		rwbuf = halloc(nrc,sizeof(int));
		isum = (int *) halloc(nrc,sizeof(int));
		ipic = (int *) rwbuf;
		break;
	case PFSHORT:
		nbytes = nrc * sizeof(short);
		rwbuf = halloc(nrc,sizeof(short));
		isum= (int *) halloc(nrc,sizeof(int));
		spic = (short *) rwbuf;
		break;
	case PFBYTE:
		nbytes = nrc * sizeof(char);
		rwbuf = halloc(nrc,sizeof(char));
		isum = (int *) halloc(nrc,sizeof(int));
		cpic = (unsigned char *) rwbuf;
		break;
	}
	for (f=0;f<nfr;f++) {
		if (fread(rwbuf,nbytes,1,stdin) != 1)
			perr(HE_MSG,"error during read");
		switch (hd.pixel_format) {
		case PFFLOAT:
			fp = fpic;
			fsp = fsum;
			for (i=0;i<nrc;i++,fsp++,fp++)
				*fsp += *fp;
			break;
		case PFINT:
			ip = ipic;
			isp = isum;
			for (i=0;i<nrc;i++,isp++,ip++)
				*isp += *ip;
			break;
		case PFSHORT:
			sp = spic;
			isp = isum;
			for (i=0;i<nrc;i++,isp++,sp++)
				*isp += *sp;
			break;
		case PFBYTE:
			cp = cpic;
			isp = isum;
			for (i=0;i<nrc;i++,isp++,cp++)
				*isp += *cp;
			break;
		}
	}
	switch (hd.pixel_format) {
	case PFFLOAT:
		fp = fpic;
		fsp = fsum;
		fdiv = (float) nfr;
		for (i=0;i<nrc;i++,fsp++,fp++)
			*fp = *fsp / fdiv;
		break;
	case PFINT:
		ip = ipic;
		isp = isum;
		idiv = nfr;
		for (i=0;i<nrc;i++,isp++,ip++)
			*ip = *isp / idiv;
		break;
	case PFSHORT:
		sp = spic;
		isp = isum;
		idiv = nfr;
		for (i=0;i<nrc;i++,isp++,sp++)
			*sp = *isp / idiv;
		break;
	case PFBYTE:
		cp = cpic;
		isp = isum;
		idiv = nfr;
		for (i=0;i<nrc;i++,isp++,cp++)
			*cp = *isp / idiv;
		break;
	}
	if (fwrite(rwbuf,nbytes,1,stdout) != 1)
		perr(HE_MSG,"error during write");
	return(0);
}
