/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * fastcosum.c - Compute a sum of spatiotemporal sinewave gratings (quickly)
 *
 * usage:	fastcosum < file > outseq
 *
 * File contains N+1 lines directing the construction of a sum of N
 * cosine gratings.  The first line is formatted
 * 
 * 	"%d %d %d %d", N, nf, nr, nc
 * 
 * for N the number of gratings being summed, nc the number of columns, nr the
 * number of rows, and nf the number of frames.  Each number must be greater
 * than zero and an integral divisor of 512.
 *
 * Each of the remaining N lines is formatted as follows:
 * 
 * "%d %d %d %lf %lf", tf[i], yf[i], xf[i], ph[i], am[i]
 * 
 * These represent, for the i'th grating:
 *
 *	xf - horizontal frequency in cycles per frame width
 *	yf - vertical frequency in cycles per frame height
 *	tf - temporal frequency in cycles per frame height
 *	phase - degrees of phase angle (0 = cosine phase at the mean,
 *		-90 = sine phase at the mean)
 *	amplitude - multiplier (peak value if in cosine phase)
 *
 * to load:	cc -o fastcosum fastcosum.c -lhipsh -lhips -lm
 *
 * Charlie Chubb 12/6/85
 * HIPS 2 - msl - 8/12/91
 */

#include <hipl_format.h>

static Flag_Format flagfmt[] = {LASTFLAG};

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd;
	int i,n,nr,nc,nf,f,*xf,*yf,itf;
	double *tf,*phase,*amplitude;
	Filename filename;
	Pixelval p;
	FILE *fp;
	char buf[200];

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,FFONE,&filename);
	fp = hfopenr(filename);
/*
 * Read input file
 */
	if (fscanf(fp,"%d %d %d %d",&n,&nf,&nr,&nc) != 4)
		perr(HE_READFILE,filename);
	if (n <= 0 || nc <= 0 || nr <= 0 || nf <= 0)
		perr(HE_MSG,"invalid number of gratings or image size");
	if (512%nf || 512%nr || 512%nc)
		perr(HE_MSG,HE_POW2);
	init_header(&hd,"","",nf,"",nr,nc,PFFLOAT,1,"");
	xf = (int *) memalloc(n,sizeof(int));
	yf = (int *) memalloc(n,sizeof(int));
	tf = (double *) memalloc(n,sizeof(double));
	phase = (double *) memalloc(n,sizeof(double));
	amplitude = (double *) memalloc(n,sizeof(double));

	sprintf(buf,"%s: sum of following %d gratings:\n\n",Progname,n);
	desc_set(&hd,strsave(buf));
	for (i=0;i<n;i++) {
		if (fscanf(fp,"%d %d %d %lf %lf",&itf,yf+i,xf+i,phase+i,
			amplitude+i) != 5)
				perr(HE_READFILE,filename);
		sprintf(buf,
		"     %lf * cosine(2PI(%d x/%d + %d y/%d + %d t/%d + %lf/360.))\n",
			amplitude[i],xf[i],nc,yf[i],nr,itf,nf,phase[i]);
		desc_append(&hd,buf);
		tf[i] = 360.*itf/nf;
	}
	p.v_float = 0.;
	write_headeru(&hd,argc,argv);
	alloc_image(&hd);
	for (f=0;f<nf;f++) {
		h_setimage(&hd,&p);
		for (i=0;i<n;i++)
			h_fastaddcos(&hd,&hd,xf[i],yf[i],phase[i]+f*tf[i],
				amplitude[i]);
		write_image(&hd,0);
	}
	return(0);
}
