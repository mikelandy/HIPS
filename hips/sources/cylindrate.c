/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * cylindrate.c - project frames onto a semicylinder
 *
 * usage:	cylindrate <iseq >oseq
 *
 * Project each frame of inseq onto a semicylinder.  The number of
 * cols in outseq is 2/PI the number in inseq.
 *
 * pixel formats handled directly: BYTE
 *
 * to load:	cc -o cylindrate cylindrate.c -lhipsh -lhips
 *
 * Charlie Chubb - 2/26/87
 * Hips 2 - msl - 8/11/91
 */

#include <stdio.h>
#include <hipl_format.h>
#include <math.h>

static Flag_Format flagfmt[] = {LASTFLAG};
int types[] = {PFFLOAT,LASTTYPE};

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd,hdp,hdo;
	Filename filename;
	FILE *fp;
	struct gap {
		double s_weight,e_weight;
		int start,end,count;
	} *G;
	register struct gap *gp;
	register float *ip,*op;
	register int x,y;
	double *C,tmp;
	double cum,start,end,radius,phi,pixsz,psz_o_radius,arg;
	int frames,rows,ocols,icols;
	int t,method,ipix,iocols;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	method = fset_conversion(&hd,&hdp,types,filename);
	frames = hdp.num_frame;
	rows = hd.rows;
	icols = hd.cols;
	iocols = hd.ocols;
	ocols = 2*(int)(radius = ((double)hd.cols / H_PI));
	dup_headern(&hdp,&hdo);
	setsize(&hdo,rows,ocols);
	alloc_image(&hdo);
	pixsz =  2.*radius/(double)ocols;
	write_headeru(&hdo,argc,argv);
	C = (double *) memalloc(icols,sizeof(double));
	G = (struct gap *) memalloc(ocols,sizeof(struct gap));
	phi = H_PI/(double)icols;
	for (ipix=0;ipix<icols;ipix++) {
		tmp = cos((double)(ipix+1) * phi) - cos((double)ipix * phi);
		C[ipix] = ABS(tmp);
	}
	for (psz_o_radius=pixsz/radius,gp=G,x=0;x<ocols;x++,gp++) {
		start = acos(1. - (double)x * psz_o_radius);
		arg = 1. - (double)(x+1) * psz_o_radius;
		if (arg < -1.)
			arg = -1.;
		end = acos(arg);
		for (ipix = 0; (double)ipix * phi < start; ipix++)
			;
		gp->start = ipix-1;
		tmp = cos((double)ipix * phi) - cos(start);
		gp->s_weight = ABS(tmp);
		for (--ipix; (double)ipix * phi < end; ipix++)
			;
		gp->end = ipix;
		tmp = cos(end) - cos((double)(ipix - 1) * phi);
		gp->e_weight = ABS(tmp);
		gp->count = gp->end - gp->start + 1;
	}
	for (t=0;t<frames;t++) {
		fread_imagec(fp,&hd,&hdp,method,t,filename);
		op = (float *) hdo.firstpix;
		ip = (float *) hdp.firstpix;
		for (y=rows;y;y--,ip+=iocols)
		    for (gp=G,x=0;x<ocols;x++,op++,gp++) {
			cum = gp->s_weight * ip[gp->start];
			for (ipix=gp->start+1;ipix<gp->end;ipix++)
				cum += C[ipix] * ip[ipix];
			cum += gp->e_weight * ip[gp->end];
			*op = cum/gp->count;
		}
		write_image(&hdo,t);
	}
	return(0);
}
