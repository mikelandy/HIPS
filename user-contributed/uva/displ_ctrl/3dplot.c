/*	Copyright (c) 1982 Michael Landy, Yoav Cohen, and George Sperling

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/* 3dplot.c
**
**
** Load with: (3d_code,cut_frame,read_frame,trans_frame,getplot,addplot) -lhips
**
**
** Yoav Cohen 11/15/82
** modified for itec and lex:  Charles Carman  4/24/87
*/
#include <stdio.h>
#include <hipl_format.h>
#include "device.h"

double cur_b, cur_x, cur_y;

main(argc,argv)
	int argc;
	char *argv[];
{
	struct header hd;
	double	shift_v[3], rot_m[3][3];
	int	nf, iframe, rows, cols;
	int 	flags, lex_flg;
	int	inbytes, nbytes;
	char	fbuf1[FBUFLIMIT], fbuf2[FBUFLIMIT];

	Progname = strsave(*argv);
	read_header(&hd);
	if (hd.pixel_format != PLOT3D)
		perror("input must be in PLOT3D format");

	rows = hd.orows; cols = hd.ocols;
	nf = hd.num_frame;

	switch (getdev()) {
	case 'I':
		lex_flg = 0;
		break;
	case 'L':
		lex_flg = 1;
		break;
	default:
		exit();
		break;
	}

	for (iframe=0; iframe<nf; iframe++)
	{
		inbytes = read_frame(stdin,fbuf1,FBUFLIMIT,&flags,shift_v,
			rot_m,iframe,"<stdin>");
		shift_v[0] += 256.0; shift_v[1] += 256.0;
		flags = flags | 02;
		trans_frame(fbuf1,inbytes,shift_v,rot_m,&flags);
		nbytes = cut_frame(fbuf1,inbytes,fbuf2,FBUFLIMIT,
		    256.0-cols/2.,256.0-rows/2.,255.0+cols/2.,255.0+rows/2.);
		shift_v[0] = shift_v[1] = 0.5;
		flags = 2;
		trans_frame(fbuf2,nbytes,shift_v,rot_m,&flags);
		if (iframe) {
			fprintf(stderr,"erase the screen here\n");
			if (lex_flg)
				cle_lex(0,0,0,0,0,1);
			else
				cle_itec(0,0,0,0,BW,0,1);
		}

		if (lex_flg)
			plot3d_lx(fbuf2,nbytes);
		else
			plot3d_itc(fbuf2,nbytes);
	}
}

perror(s)
	char *s;
{
	fprintf(stderr,"grplot: %s\n",s);
	exit(1);
}
