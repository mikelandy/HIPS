/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * pstrobe.c - collapse several frames of input into one frame of output
 *
 * usage: pstrobe [-b batch ] <in >out
 *
 * default: batch=length of sequence; i.e. the output file is a single frame.
 *
 * To load:	cc -o pstrobe pstrobe.c -lhips
 *
 * Yoav Cohen 17/11/82
 * HIPS 2 - msl - 8/1/91
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {
	{"b",{LASTFLAG},1,{{PTINT,"-1","batchlength"},LASTPARAMETER}},
	LASTFLAG};
int types[] = {PLOT3D,LASTTYPE};

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd;
	int nf,flags,inbytes,i,j,batch,index;
	char buf[FBUFLIMIT];
	double sh_v[3],rot_m[3][3];
	Filename filename;
	FILE *fp;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&batch,FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_cpf(fp,&hd,types,filename);
	if (hd.num_frame<=0)
		perr(HE_MSG,"no. of frames must be positive ");
	nf = hd.num_frame;
	if (batch < 0)
		batch = nf;
	if (batch<1)
		perr(HE_MSG,"unreasonable batch size");
	if (batch>nf) {
	  fprintf(stderr,
	    "%s: warning: batch size is too big, set to length of sequence.\n",
	    Progname);
	    batch = nf;
	}
	hd.num_frame = nf/batch+((nf%batch==0)?0:1);
	write_headeru(&hd,argc,argv);
	for(i=0;i<nf;i+=batch) {
		index=0;
		for (j=i;j<batch+i && j<nf;j++) {
			inbytes=read_frame(fp,buf+index,FBUFLIMIT,&flags,sh_v,
				rot_m,i,filename);
			trans_frame(buf+index,inbytes,sh_v,rot_m,&flags);
			index += inbytes;
		}
		write_frame(stdout,buf,index,sh_v,rot_m,i);
	}
	return(0);
}
