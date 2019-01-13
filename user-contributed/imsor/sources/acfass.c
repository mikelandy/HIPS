/*
 * Copyright (c) 1991
 *
 *      Rasmus Larsen
 *      IMSOR/TUD
 *      The Institute of Mathematical Statististics and Operations Research
 *      The Technical University of Denmark
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 *
 */
			      
#include <hipl_format.h>
#include <stdio.h>
#include <stdlib.h>


int     types[] = {PFFLOAT,LASTTYPE};


float	SQR();

int main(argc,argv)
int argc;
char *argv[];
{
struct header	hd;
FILE		*fp;
Filename	filename = "<stdin>";

int		nrows,ncols,nbands,i,j,k;
int		nsize;

float		*image,*imagep;
float		*outp,*out;

float		peak;
int		rflag = 0;

for (i=1;i<argc;i++) {
	if (strncmp(argv[i], "-r", 2) == 0) {
		rflag = 1;
		continue;
	}
	if (strncmp(argv[i],"-",1) == 0) {
		fprintf(stderr,"%s: unknown option '%s'\n",Progname,argv[i]);
		fprintf(stderr,"Usage: acf [-r]\n");
		exit(1);
	}
}

Progname = strsave(*argv); /* save program name to be used by perr */
fp = hfopenr(filename);
fread_hdr_cpf(fp,&hd,types,filename);

nsize =  hd.orows * hd.ocols;
nrows = hd.orows;
ncols = hd.ocols;
nbands = hd.num_frame;
image = (float *) malloc(nsize*sizeof(float));
out = (float *) malloc(nsize*sizeof(float));
write_headeru(&hd,argc,argv);
for (k=0;k<nbands;k++) {
	outp = out;
	imagep = image;
	if(fread(image,sizeof(float),nsize,fp) != nsize) {
		fprintf(stderr,"%s: Error during read",Progname);
		exit(1);
	}
	if (rflag) {
		peak = image[0];
		for (i=0;i<nrows;i++) for (j=0;j<ncols;j++)
			*outp++ = SQR ( (*imagep++)/peak);
	}
	else {
		for (i=0;i<nrows;i++) for (j=0;j<ncols;j++)
			*outp++ = SQR (*imagep++);
	}
	if (fwrite(out,sizeof(float),nsize,stdout) != nsize) {
		fprintf(stderr,"%s: Error during write",Progname);
		exit(1);
	}
}
return 0;
}

float	SQR(A)
float	A;
{ return A*A; } 

