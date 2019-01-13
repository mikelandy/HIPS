/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * view.c - generate a conic perspective of a 3D graph
 *
 * usage: view [-p dist]
 *
 * arguments: [dist] - The distance of the picture plane from the center
 *			of perspective.
 *		
 * defaults: dist=256. (half the width of the screen) .
 *
 * To load:	cc -o view view.c -lhips
 *
 * Yoav Cohen 17/11/82
 * HIPS 2 - msl - 7/24/91
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {
	{"p",{LASTFLAG},1,{{PTDOUBLE,"256.","dist"},LASTPARAMETER}},
	LASTFLAG};
int types[] = {PLOT3D,LASTTYPE};

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd;
	int nf,flags,inbytes,obytes,i;
	char buf[FBUFLIMIT],obuf[FBUFLIMIT];
	double sh_v[3],rot_m[3][3];
	double dist;
	Filename filename;
	FILE *fp;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&dist,FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_cpf(fp,&hd,types,filename);
	if (hd.num_frame <= 0)
		perr(HE_MSG,"no. of frames must be positive ");
	nf=hd.num_frame;
	write_headeru(&hd,argc,argv);
	for(i=0;i<nf;i++) {
		inbytes=read_frame(fp,buf,FBUFLIMIT,&flags,sh_v,rot_m,i,
			filename);
		trans_frame(buf,inbytes,sh_v,rot_m,&flags);
		obytes=view_frame(buf,inbytes,obuf,FBUFLIMIT,dist);
		write_frame(stdout,obuf,obytes,sh_v,rot_m,i);
	}
	return(0);
}
