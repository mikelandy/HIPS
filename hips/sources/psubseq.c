/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * psubseq.c - extract a subsequence of frames from a PLOT3D file
 *
 * usage:	psubseq [-f fromframe [toframe [incr]]] < seq > subseq
 *
 * defaults:	fromframe=0 (first frame)
 *		toframe=fromframe
 *		incr(ement)=1
 *
 * to load:	cc -o psubseq psubseq.c -lhips
 *
 * Yoav Cohen 3/1/82
 * HIPS 2 - msl - 8/1/91
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {
	{"f",{LASTFLAG},1,{{PTINT,"0","from-frame"},{PTINT,"-1","to-frame"},
		{PTINT,"1","increment"},LASTPARAMETER}},
	LASTFLAG};
int types[] = {PLOT3D,LASTTYPE};

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd;
	int i,fromf,tof,increment,flags,oldfn,nbytes;
	char buf[FBUFLIMIT];
	double sh_v[3],rot_m[3][3];
	Filename filename;
	FILE *fp;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&fromf,&tof,&increment,FFONE,&filename);
	fp = hfopenr(filename);
	if (tof < 0)
		tof = fromf;
	if (fromf > tof)
		perr(HE_MSG,"to-frame must be >= from-frame");
	if (increment<1)
		perr(HE_MSG,"increment must be positive");
	fread_hdr_cpf(fp,&hd,types,filename);
	if (hd.num_frame < fromf )
		perr(HE_MSG,"subsequence not in sequence");
	if (hd.num_frame <= tof) {
		fprintf(stderr,
		  "%s: warning, some frames are not in input sequence\n",
			Progname);
		tof=hd.num_frame-1;
	}
	oldfn=hd.num_frame;
	hd.num_frame=(tof-fromf)/increment+1;
	write_headeru(&hd,argc,argv);
	for(i=0;i<oldfn;i++) {
		if (i>tof)
			break;
		if (i<fromf || (i-fromf)%increment!=0) 
			nbytes=read_frame(fp,buf,FBUFLIMIT,&flags,sh_v,rot_m,
				i,filename);
		else {
			nbytes=read_frame(fp,buf,FBUFLIMIT,&flags,sh_v,rot_m,
				i,filename);
			write_frame(stdout,buf,nbytes,sh_v,rot_m,i);
		}
	}
	return(0);
}
