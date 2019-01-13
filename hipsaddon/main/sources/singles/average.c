static char *SccsId = "%W%      %G%";

/*      Copyright (c) 1991 The Turing Institute

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 * average - average sub-sequence of frames pixel-by-pixel without normalization
 *
 * usage:	average [-f from] [-t to]
 *
 * where "from" and "to" is the first and "last" frame respectively of the
 * sub-sequence to be averaged and defaults to 0 and the last frame of the
 * input sequence.
 *
 * to load:	cc -o average average.c -lhipsa -lhips
 *
 * Peter Mowforth & Jin Zhengping - 8/5/85
 * Rewritten by Jin Zhengping - 31 August 1991
 */

#include <stdio.h>
#include <hipl_format.h>
int types[] = {PFBYTE,LASTTYPE};
static Flag_Format flagfmt[] =
{
	{"f",
		{LASTFLAG},
		1,{{PTINT,"0","from"},LASTPARAMETER}},
	{"t",
		{LASTFLAG},
		1,{{PTINT,"-1","to"},LASTPARAMETER}},
	LASTFLAG
};


int main(argc,argv)

int     argc;
char    **argv;

{
	struct          header hd,hdp,hdo;
	int             method,f,fr;
	Filename        filename;
	FILE            *fp;
	int		from,to;
	byte		*imagep;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&from,&to,FFONE,&filename);
	fp=hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	method=fset_conversion(&hd,&hdp,types,filename);
	fr = hdp.num_frame;
	if (to == -1)
		to=fr-1;
	if(from<0)
		from=0;
	else if(from>=fr)
		from=fr-1;
	if(to<from)
		to=from;
	else if(to>=fr)
		to=fr-1;
	hdp.num_frame = to-from+1;
	free_image(&hdp);
	if((hdp.image=(byte *)malloc(hdp.sizeimage*hdp.num_frame))==0)
		perr(HE_ALLOC);
	hdp.firstpix=hdp.image+((hdp.ocols * hdp.frow) + hdp.fcol) * hdp.sizepix;
	hdp.imdealloc = TRUE;
	dup_headern(&hdp,&hdo);
	hd.num_frame = hdo.num_frame = 1;
	alloc_image(&hdo);
	write_headeru2(&hd,&hdo,argc,argv,hips_convback);
	for (f=0;f<from;f++)
		fread_image(fp,&hd,f,filename);
	imagep=hdp.image;
	for(imagep=hdp.image,f=from;f<=to;f++,hdp.image+=hdp.sizeimage)
		fread_imagec(fp,&hd,&hdp,method,f,filename);
	hdp.image=imagep;
	h_average(&hdp,&hdo);
	write_imagec(&hd,&hdo,method,hips_convback,f);
	return(0);
}
