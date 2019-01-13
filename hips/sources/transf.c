/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * transf.c - transform a sequence
 *
 * All the entries are transformed according to the shift-vector and the
 * rotation-matrix which are set to zero and identity and are not written out.
 *
 * To load:	cc -o transf transf.c -lhips
 *
 * Yoav Cohen 17/11/82
 * HIPS2 - msl - 8/1/91
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {LASTFLAG};
int types[] = {PLOT3D,LASTTYPE};

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd;
	int nf,flags,inbytes,i;
	char buf[FBUFLIMIT];
	double sh_v[3],rot_m[3][3];
	Filename filename;
	FILE *fp;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_cpf(fp,&hd,types,filename);
	if (hd.num_frame<=0)
		perr(HE_MSG,"no of images must be positive");
	nf = hd.num_frame;
	write_headeru(&hd,argc,argv);
	for(i=0;i<nf;i++) {
		inbytes = read_frame(fp,buf,FBUFLIMIT,&flags,sh_v,rot_m,
			i,filename);
		trans_frame(buf,inbytes,sh_v,rot_m,&flags);
		write_frame(stdout,buf,inbytes,sh_v,rot_m,i);
	}
	return(0);
}
