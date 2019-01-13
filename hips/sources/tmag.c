/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * tmag.c - scale a 3-D graph
 * 
 * usage:	tmag -p t x [y [z]]
 * 
 * defaults:	y,z=1.0
 *
 * t is the number of time units over which magnification
 * is to be performed. The last frame of the input sequence
 * is pushed as it is, but thereafter t frames are added,
 * each magnified by the factor-arguments relative to the
 * preceding one.
 *
 * To load:	cc -o tmag tmag.c -lhips
 *
 * Yoav Cohen 11/16/82
 * HIPS 2 - msl - 7/24/91
 */

#include <stdio.h>
#include <hipl_format.h>

double xf,yf,zf;

static Flag_Format flagfmt[] = {
	{"p",{LASTFLAG},1,{{PTINT,"-1","t"},{PTDOUBLE,"0.","x"},
		{PTDOUBLE,"0.","y"},{PTDOUBLE,"0.","z"},LASTPARAMETER}},
	LASTFLAG};
int types[] = {PLOT3D,LASTTYPE};

char buf[FBUFLIMIT];
int nbuf,isatty();

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd;
	int nf,nt,i,j,flags;
	double rot_m[3][3],sh_v[3];
	Filename filename;
	FILE *fp;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&nt,&xf,&yf,&zf,FFONE,&filename);
	if (nt <= 0)
		perr(HE_MSG,"-p must be specified and t must be > 0");
	fp = hfopenr(filename);

	/* is there a file input ? */
	if (!isatty(fileno(fp))) {
		fread_hdr_cpf(fp,&hd,types,filename);
		if (hd.num_frame <= 0)
			perr(HE_MSG,"no. of frames must be positive ");
		nf=hd.num_frame;
		hd.num_frame=nf+nt;
		write_headeru(&hd,argc,argv);
		for (i=0;i<nf;i++) {
			nbuf=read_frame(fp,buf,FBUFLIMIT,&flags,sh_v,rot_m,i,
				filename);
			write_frame(stdout,buf,nbuf,sh_v,rot_m,i);
		}
		for(i=0;i<nt;i++) {
			for(j=0;j<3;j++) {
				rot_m[0][j]*=xf;
				rot_m[1][j]*=yf;
				rot_m[2][j]*=zf;
			}
			sh_v[0]*=xf; sh_v[1]*=yf; sh_v[2]*=zf;
			write_frame(stdout,buf,nbuf,sh_v,rot_m,nf+i);
		}
	}
	else
		perr(HE_MSG,"no input file");
	return(0);
}
