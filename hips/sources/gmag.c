/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * gmag.c - scale a 3-D graph
 * 
 * usage:	gmag [-p x [y [z]]]
 * 
 * defaults:	y,z...=1.0
 *
 * To load:	cc -o gmag gmag.c -lhips
 *
 * Yoav Cohen 11/15/82
 * HIPS 2 - msl - 7/24/91
 */

#include <stdio.h>
#include <hipl_format.h>

double xf,yf,zf;

static Flag_Format flagfmt[] = {
	{"p",{LASTFLAG},1,{{PTDOUBLE,"1.","x"},{PTDOUBLE,"1.","y"},
		{PTDOUBLE,"1.","z"},LASTPARAMETER}},
	LASTFLAG};
int types[] = {PLOT3D,LASTTYPE};
int isatty();

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd;
	int nf,i,j,flags,inbytes;
	char buf[FBUFLIMIT];
	double sh_v[3],rot_m[3][3];
	Filename filename;
	FILE *fp;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&xf,&yf,&zf,FFONE,&filename);
	fp = hfopenr(filename);

	/* is there a file input ? */
	if (!isatty(fileno(fp))) {
		fread_hdr_cpf(fp,&hd,types,filename);
		if (hd.num_frame <= 0)
			perr(HE_MSG,"no. of frames must be positive ");
		nf=hd.num_frame;
		write_headeru(&hd,argc,argv);
		for(i=0;i<nf;i++) {
			inbytes=read_frame(fp,buf,FBUFLIMIT,&flags,sh_v,rot_m,
				i,filename);
			for(j=0;j<3;j++) {
				rot_m[0][j]*=xf;
				rot_m[1][j]*=yf;
				rot_m[2][j]*=zf;
			}
			sh_v[0]*=xf; sh_v[1]*=yf; sh_v[2]*=zf;
			write_frame(stdout,buf,inbytes,sh_v,rot_m,i);
		}
	}
	else
		perr(HE_MSG,"no input file");

	return(0);
}
