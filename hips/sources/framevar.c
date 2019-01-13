/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * framevar - basic statistics of an image
 *
 * usage:	framevar [-p] [-n] <iseq >oseq
 *
 * -p specifies that the input images should be sent to the output (the
 * statistics are reported on stderr).  -n specifies that zero-valued pixels
 * should be ignored in the calculations.  Note that the output images are
 * identically the input images, even though statistics may be computed on a
 * form of the image converted to byte or float format.
 *
 * pixel formats handled directly: BYTE, FLOAT
 *
 * to load:	cc -o framevar framevar.c -lhipsh -lhips
 *
 * Yoav Cohen 2/16/82
 * Hips 2 - msl - 7/5/91
 */

#include <stdio.h>
#include <hipl_format.h>
#include <math.h>

static Flag_Format flagfmt[] = {
	{"p",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"n",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	LASTFLAG};
int types[] = {PFBYTE,PFFLOAT,LASTTYPE};

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd,hdp;
	int method,fr,f,gnelem;
	Filename filename;
	FILE *fp;
	h_boolean pflag,nflag,notyet=TRUE;
	struct hips_stats stats;
	byte gminb,gmaxb;
	float gminf,gmaxf;
	double gsum,gssq,gmean,gvar,gstdev;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&pflag,&nflag,FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	method = fset_conversion(&hd,&hdp,types,filename);
	if (pflag)
		write_headeru(&hd,argc,argv);
	fr = hdp.num_frame;
	gnelem = gsum = gssq = 0;
	for (f=0;f<fr;f++) {
		fread_imagec(fp,&hd,&hdp,method,f,filename);
		h_stats(&hdp,&stats,nflag);
		fprintf(stderr,"%s: Frame %d\n",Progname,f);
		fprintf(stderr,"\tMean=%f SS=%f  Variance=%f StdDev=%f\n",
			stats.mean,stats.ssq,stats.var,stats.stdev);
		if (hdp.pixel_format == PFBYTE)
			fprintf(stderr,"\tMax=%d Min=%d Nelem=%d\n",
				(int) stats.statmax.v_byte,
				(int) stats.statmin.v_byte,stats.nelem);
		else
			fprintf(stderr,"\tMax=%f Min=%f Nelem=%d\n",
				stats.statmax.v_float,stats.statmin.v_float,
				stats.nelem);
		if (stats.nelem) {
			gsum += stats.sum;
			gssq += stats.ssq;
			gnelem += stats.nelem;
			if (notyet) {
				if (hdp.pixel_format == PFBYTE) {
					gminb = stats.statmin.v_byte;
					gmaxb = stats.statmax.v_byte;
				}
				else {
					gminf = stats.statmin.v_float;
					gmaxf = stats.statmax.v_float;
				}
				notyet = FALSE;
			}
			else  {
				if (hdp.pixel_format == PFBYTE) {
					if (gminb > stats.statmin.v_byte)
						gminb = stats.statmin.v_byte;
					if (gmaxb < stats.statmax.v_byte)
						gmaxb = stats.statmax.v_byte;
				}
				else {
					if (gminf > stats.statmin.v_float)
						gminf = stats.statmin.v_float;
					if (gmaxf < stats.statmax.v_float)
						gmaxf = stats.statmax.v_float;
				}
			}
		}
		if (pflag)
			write_image(&hd,f);
	}
	if (fr > 1) {
		if (gnelem) {
			gmean = gsum/gnelem;
			gvar = (gssq/gnelem) - (gmean*gmean);
			gstdev = sqrt(gvar);
		}
		else
			gmean = gvar = gstdev = 0;
		fprintf(stderr,"%s: Totals\n",Progname);
		fprintf(stderr,"\tMean=%f SS=%f  Variance=%f StdDev=%f\n",
			(float) gmean,(float) gssq,(float) gvar,(float) gstdev);
		if (hdp.pixel_format == PFBYTE)
			fprintf(stderr,"\tMax=%d Min=%d Nelem=%d\n",
				(int) gmaxb,(int) gminb,gnelem);
		else
			fprintf(stderr,"\tMax=%f Min=%f Nelem=%d\n",
				gmaxf,gminf,gnelem);
	}
	return(0);
}
