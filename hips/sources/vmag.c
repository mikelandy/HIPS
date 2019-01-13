/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * vmag.c - scale in time a world around an observer in a 3-D graph
 * 
 * usage:	vmag -p from to [x [y [z]]]
 * 
 * defaults:	x,y,z=1.0
 *
 * Frame "from" and all the preceeding frames are  
 * pushed as intact, but therafter the coordinates
 * in each frame are multiplied by x,y,z relative to the
 * preceeding frame. 
 * All frames after frame "to" are scaled by the same constant;
 * e.g. the x factor for frame "to" and all the following
 * frames is x*(to-from).
 * Frame numbering starts at zero.
 *
 * To load:	cc -o vmag vmag.c -lhips
 *
 * Yoav Cohen 2/1/83
 * HIPS 2 - msl - 7/24/91
 */

#include <stdio.h>
#include <hipl_format.h>

double xf,yf,zf;
double txf,tyf,tzf;

static Flag_Format flagfmt[] = {
	{"p",{LASTFLAG},2,{{PTINT,"-1","from"},{PTINT,"-1","to"},
		{PTDOUBLE,"0.","x"},{PTDOUBLE,"0.","y"},{PTDOUBLE,"0.","z"},
		LASTPARAMETER}},
	LASTFLAG};
int types[] = {PLOT3D,LASTTYPE};

char buf[FBUFLIMIT];
int nbuf,isatty();

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd;
	int nf,nfrom,nto,i,j,flags;
	double rot_m[3][3],sh_v[3];
	Filename filename;
	FILE *fp;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&nfrom,&nto,&xf,&yf,&zf,FFONE,&filename);
	if (nfrom < 0)
		perr(HE_MSG,"-p must be specified and nfrom must be >= 0");
	if (nto-nfrom<=0)
		perr(HE_MSG,"error in frame arguments");
	fp = hfopenr(filename);

	/* is there a file input ? */
	if (!isatty(fileno(fp))) {
		fread_hdr_cpf(fp,&hd,types,filename);
		if (hd.num_frame <= 0)
			perr(HE_MSG,"no. of frames must be positive ");
		nf=hd.num_frame;
		if (nfrom>=nf) {
			fprintf(stderr,
				"%s: warning: sequence is shorter than %d\n",
				Progname,nfrom+1);
			nfrom=nf-1;
		}
		write_headeru(&hd,argc,argv);
		for(i=0;i<nfrom;i++) {
			nbuf=read_frame(fp,buf,FBUFLIMIT,&flags,sh_v,rot_m,i,
				filename);
			write_frame(stdout,buf,nbuf,sh_v,rot_m,i);
		}
		if (nto>=nf) {
			fprintf(stderr,
				"%s: warning: sequence is shorter than %d\n",
				Progname,nto+1);
			nto=nf-1;
		}
	/* inefficient but more legible */
		txf=tyf=tzf=1.0;
		for(i=nfrom;i<=nto;i++) {
			nbuf=read_frame(fp,buf,FBUFLIMIT,&flags,sh_v,rot_m,i,
				filename);
			txf*=xf; tyf*=yf; tzf*=zf;
				sh_v[0]*= txf;
				sh_v[1]*= tyf;
				sh_v[2]*= tzf;
			for(j=0;j<3;j++) {
				rot_m[0][j]*=txf;
				rot_m[1][j]*=tyf;
				rot_m[2][j]*=tzf;
			}
			write_frame(stdout,buf,nbuf,sh_v,rot_m,i);
		}
		for(i=nto+1;i<nf;i++) {
			nbuf=read_frame(fp,buf,FBUFLIMIT,&flags,sh_v,rot_m,i,
				filename);
			sh_v[0]*= txf;
			sh_v[1]*= tyf;
			sh_v[2]*= tzf;
			for(j=0;j<3;j++) {
				rot_m[0][j]*= txf;
				rot_m[1][j]*= tyf;
				rot_m[2][j]*= tzf;
			}
			write_frame(stdout,buf,nbuf,sh_v,rot_m,i);
		}
	}
	else
		perr(HE_MSG,"no input file");

	return(0);
}
