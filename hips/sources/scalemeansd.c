/*
 * scalemeansd - scale an image to have specified mean and sd
 *
 * usage:	scalemeansd [-m mean] [-s sd] <iseq >oseq
 *
 * This program scales each frame in an image sequence separately so as to
 * have the specified mean and standard deviation.  The mean defaults to 0
 * and sd to 1.
 *
 * pixel formats handled directly: FLOAT
 *
 * to load:	cc -o scalemeansd scalemeansd.c -lhipsh -lhips -lm
 *
 * msl - 7/21/94
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {
        {"m",{LASTFLAG},1,{{PTDOUBLE,"0","mean"},LASTPARAMETER}},
        {"s",{LASTFLAG},1,{{PTDOUBLE,"1","std-dev"},LASTPARAMETER}},
	LASTFLAG};
int types[] = {PFFLOAT,LASTTYPE};

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd,hdp;
	int method,fr,f;
	Filename filename;
	FILE *fp;
	double mean,sd,currmean,currsd;
	struct hips_stats stats;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&mean,&sd,FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	method = fset_conversion(&hd,&hdp,types,filename);
	write_headeru2(&hd,&hdp,argc,argv,hips_convback);
	fr = hdp.num_frame;
	for (f=0;f<fr;f++) {
		fread_imagec(fp,&hd,&hdp,method,f,filename);
		h_stats(&hdp,&stats,FALSE);
		currmean = stats.mean;
		currsd = stats.stdev;
		h_linscale(&hdp,&hdp,sd/currsd,mean-(currmean*sd/currsd));
		write_imagec(&hd,&hdp,method,hips_convback,f);
	}
	return(0);
}
