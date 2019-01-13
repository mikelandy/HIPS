/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * plot3topix.c - convert from plot3d format to pixels
 * 
 * usage:	plot3topix < plot3dfile > imagefile
 * 
 * To load:	cc -o plot3topix plot3topix.c -lhips
 *
 * Mike Landy 10/19/83
 * HIPS 2 - msl - 8/1/91
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {LASTFLAG};
int types[] = {PLOT3D,LASTTYPE};

int main(argc,argv)

int argc;
char **argv;

{
	int nf,iframe,flags,inbytes,nbytes,rows,cols;
	double shift_v[3],rot_m[3][3];
	struct header hd;
	char fbuf1[FBUFLIMIT],fbuf2[FBUFLIMIT];
	Filename filename;
	FILE *fp;
	Pixelval val;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_cpf(fp,&hd,types,filename);
	setformat(&hd,PFBYTE);
	clearroi(&hd);
	alloc_image(&hd);
	write_headeru(&hd,argc,argv);
	rows=hd.rows; cols=hd.cols;
	nf=hd.num_frame;
	val.v_byte = 0;
	
	for(iframe=0;iframe<nf;iframe++) {
		h_setimage(&hd,&val);
		inbytes = read_frame(fp,fbuf1,FBUFLIMIT,&flags,shift_v,rot_m,
			iframe,filename);
		shift_v[0]+= 256.; shift_v[1]+= 256.; flags = flags | 02;
		trans_frame(fbuf1,inbytes,shift_v,rot_m,&flags);
		nbytes=cut_frame(fbuf1,inbytes,fbuf2,FBUFLIMIT,256.-cols/2.,
			256.-rows/2.,255.+cols/2.,255.+rows/2.);
		shift_v[0]=shift_v[1]=.5; flags=2;
		trans_frame(fbuf2,nbytes,shift_v,rot_m,&flags);
		pix_code(fbuf2,nbytes,hd.firstpix,rows,hd.ocols);
		write_image(&hd,iframe);
	}
	return(0);
}
