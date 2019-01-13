/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * cutf.c - to "cut" a sequence; all the entries are transformed
 *		and "cut" by cut_frame.
 *
 * arguments: [-p x0 [y0 [xn [yn ]]]] - the coordinates (lower-left and
 *		upper-right) of the window.
 *		
 * defaults: xn|yn = 512 - x0|y0
 *		y0 = x0
 *		x0 = 0
 *
 * To load:	cc -o cutf cutf.c -lhips
 *
 * Yoav Cohen 17/11/82
 * HIPS 2 - msl - 7/24/91
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {
	{"p",{LASTFLAG},1,{{PTDOUBLE,"0.","x0"},{PTDOUBLE,"0.","y0"},
		{PTDOUBLE,"-9999.","xn"},{PTDOUBLE,"-9999.","yn"},
		LASTPARAMETER}},
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
	double x0,y0,xn,yn;
	Filename filename;
	FILE *fp;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&x0,&y0,&xn,&yn,FFONE,&filename);
	if (xn == -9999.)
		xn = 512.-x0;
	if (yn == -9999.)
		yn = 512.-y0;
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
		obytes=cut_frame(buf,inbytes,obuf,FBUFLIMIT,x0,y0,xn,yn);
		write_frame(stdout,obuf,obytes,sh_v,rot_m,i);
	}
	return(0);
}
