/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/* gcube.c - 	to generate a 3-D cube.
 *		If a file is redirected, add cube to last frame
 *
 *		The cube is generated so that its center is at (0,0,0)
 *		and its sides are 2.0 units long.
 *
 * usage:	gcube 
 *
 * To load:	cc -o gcube gcube.c  -lhips
 *
 * Yoav Cohen 11/15/82
 * HIPS 2 - msl - 7/24/91
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {LASTFLAG};
int types[] = {PLOT3D,LASTTYPE};

int main(argc,argv)

int argc;
char **argv;

{
	struct	header hd;
	int 	nf,i;
	char	buf[FBUFLIMIT];
	double	sh_v[3] , rot_m[3][3];
	int	inbytes , flags , j, isatty();
	Filename filename;
	FILE *fp;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,FFONE,&filename);
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
			if(i<nf-1)write_frame(stdout,buf,inbytes,sh_v,rot_m,i);
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
	
	inbytes=addvec(buf,inbytes,FBUFLIMIT,255.,-1.,-1.,-1.,-1.,1.,-1.);
	inbytes=addend(buf,inbytes,FBUFLIMIT,1.,1.,-1.);
	inbytes=addend(buf,inbytes,FBUFLIMIT,1.,-1.,-1.);
	inbytes=addend(buf,inbytes,FBUFLIMIT,-1.,-1.,-1.);
	inbytes=addend(buf,inbytes,FBUFLIMIT,-1.,-1.,1.);
	inbytes=addend(buf,inbytes,FBUFLIMIT,1.,-1.,1.);
	inbytes=addend(buf,inbytes,FBUFLIMIT,1.,-1.,-1.);
	inbytes=addvec(buf,inbytes,FBUFLIMIT,255.,-1.,1.,-1.,-1.,1.,1.);
	inbytes=addend(buf,inbytes,FBUFLIMIT,1.,1.,1.);
	inbytes=addend(buf,inbytes,FBUFLIMIT,1.,1.,-1.);
	inbytes=addvec(buf,inbytes,FBUFLIMIT,255.,1.,1.,1.,1.,-1.,1.);
	inbytes=addvec(buf,inbytes,FBUFLIMIT,255.,-1.,-1.,1.,-1.,1.,1.);

	write_frame(stdout,buf,inbytes,sh_v,rot_m,nf-1);

	return(0);
}
