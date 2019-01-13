/*	Copyright (c) 1982 Michael Landy, Yoav Cohen, and George Sperling

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 * divseq.c - divide each frame in a sequence by the corresponding frame
 *	   of another sequence pixel by pixel. If the file sequence consists
 *	   of only a single frame, then that fixed frame is divided by each
 *	   frame in the input sequence, otherwise corresponding frames are used.
 *	   The -z option specifies the zero threshold for preventing divides
 *	   by numbers close to zero.
 *	   The first sequence must be float and the standard in
 *	   sequence must be float.
 *
 * usage:	divseq	file [-z thresh]  <iseq >oseq
 *
 * to load:	cc -o divseq divseq.c -lhips
 *
 * Ann Adams 2/26/87
 * Tom Riedl 5/14/84, derived from
 * Michael Landy 5/27/82
 */

#include <stdio.h>
#include <hipl_format.h>

int main (argc,argv)
	int argc;
	char **argv;
{
	struct header hdf,hd;
	int r,c,f,NOZERO;
	FILE *fpp;
	double atof();
	register float z_thrsh;
	float *fpic,*pic,*fp,*p;
	char filenm[40],tmp[100];

	Progname = strsave(*argv);
	if (argc<2) {
		sprintf(tmp,"Syntax:	%s file [-z thresh]\n",argv[0]);
		perr(HE_MSG,tmp);
	}
	z_thrsh = 1.0;

	for (c=1; c<argc; c++) {
		if (argv[c][0] == '-') {
			switch (argv[c][1]) {
			case 'D':
				break;
			case 'z':
				z_thrsh = atof(argv[++c]);
				break;
			default:
				perr(HE_MSG,"Unknown argument");
			}
		}
		else
			strcpy(filenm,argv[c]);
	}

	if ((fpp = fopen(filenm,"r")) == NULL) {
		sprintf(tmp,"can't open frame file - %s",filenm);
		perr(HE_MSG,tmp);
	}
	read_header(&hd);
	fread_header(fpp,&hdf,filenm);
	if (hd.pixel_format != PFFLOAT || hdf.pixel_format != PFFLOAT)
		perr(HE_MSG,"files must be in float format");
	if (hd.orows != hdf.orows || hd.ocols != hdf.ocols)
		perr(HE_MSG,"frame file and input header mismatch");
	if((hdf.num_frame != 1) && (hd.num_frame != hdf.num_frame))
		perr(HE_MSG,"frame file and input header mismatch");

	pic = (float *) halloc(hd.orows*hd.ocols,sizeof(float));
	fpic= (float *) halloc(hd.orows*hd.ocols,sizeof(float));

	update_header(&hd,argc,argv);
	write_header(&hd);

	if(hdf.num_frame == 1) {
	    if (fread(fpic,hd.orows*hd.ocols*sizeof(float),1,fpp) != 1)
		    perr(HE_MSG,"error during fixed frame read");
	}
	for (f=0;f<hd.num_frame;f++) {
	    if(hdf.num_frame != 1) {
		if (fread(fpic,hd.orows*hd.ocols*sizeof(float),1,fpp) != 1)
			perr(HE_MSG,"error during fixed sequence read");
	    }
	    if (fread(pic,hd.orows*hd.ocols*sizeof(float),1,stdin) != 1)
		perr(HE_MSG,"error during read");
	    p = pic;
	    fp = fpic;
	    NOZERO = 0;
	    for (r=0;r<hd.orows;r++) {
		for (c=0;c<hd.ocols;c++,p++) {
			if (*fp < z_thrsh ) {
				*p = 0.0;
				NOZERO++;
				fp++;
			}
			else
				*p = *p / *fp++;
		}
	    }
	    if (fwrite(pic,hd.orows*hd.ocols*sizeof(float),1,stdout) != 1)
			perr(HE_MSG,"error during write");
	}
	fprintf(stderr,"number of divide by zeroes = %d\n",NOZERO);
	return(0);
}
