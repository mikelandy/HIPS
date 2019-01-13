/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * gpoly.c -  generate a 2-D polygon.
 *
 * If a file is redirected, add polygon to last frame
 *
 * The polygon is generated so that its center is at (0,0,0) and its radius
 * is 1.0.
 *
 * usage: gpoly [-n nedges]
 *
 * if nedges=0 (default) an empty frame is generated.
 *	 =1 a point at (0,0,0)
 *	 =2 a horizontal line
 *	 =3 a triangle
 *		.
 *		.
 *		.
 *
 * To load:	cc -o gpoly gpoly.c -lhips -lm
 *
 * Yoav Cohen 11/15/82
 * HIPS 2 - msl - 7/24/91
 */

#include <stdio.h>
#include <hipl_format.h>
#include <math.h>

static Flag_Format flagfmt[] = {
	{"n",{LASTFLAG},1,{{PTINT,"0","nedges"},LASTPARAMETER}},
	LASTFLAG};
int types[] = {PLOT3D,LASTTYPE};
int isatty();

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd;
	int  nedges,nf,i;
	double alpha,a0,a1,x1,x2,y1,y2,x,y,a,d;
	char buf[FBUFLIMIT];
	double sh_v[3],rot_m[3][3];
	int inbytes,flags,j;
	Filename filename;
	FILE *fp;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&nedges,FFONE,&filename);
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
			if (i<nf-1)
				write_frame(stdout,buf,inbytes,sh_v,rot_m,i);
		}
		if(flags)
			trans_frame(buf,inbytes,sh_v,rot_m,&flags);
	}
	else {
		init_header(&hd,"","",1,"",512,512,PLOT3D,1,"");
		inbytes=0;
		for(i=0;i<3;i++) {
			sh_v[i]=0;
			for(j=0;j<3;j++)
				rot_m[i][j]=(i==j)?1.0:0.0;
		}
		write_headeru(&hd,argc,argv);
	}
	
	switch(nedges) {
	case 0:	
		break;
	case -1:
	case 1:
			inbytes=addpoint(buf,inbytes,FBUFLIMIT,255.,0.,0.,0.);
			break;
	default:
		alpha=H_PI/nedges;
		d=1.5*H_PI;
		a0=d-alpha; a1=d+alpha;
		x1=cos(a0); y1=sin(a0);
		x2=cos(a1); y2=sin(a1);
		inbytes=addvec(buf,inbytes,FBUFLIMIT,255.,x1,y1,0.0,x2,y2,0.0);
		if(nedges>2)for(i=1;i<nedges;i++) {
			a=d+(i+i+1)*alpha;
			x=cos(a); y=sin(a);
			inbytes=addend(buf,inbytes,FBUFLIMIT,x,y,0.0);
		}
		break;
	}
	write_frame(stdout,buf,inbytes,sh_v,rot_m,nf-1);

	return(0);
}
