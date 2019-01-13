static char *SccsId = "%W%      %G%";

/*      Copyright (c) 1991 The Turing Institute

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 * sigmatemp - estimate the standard deviations of random noise of each
 * image in a sequence base on each successive two images.
 *
 * usage:	sigmatemp [-p] [-s]
 *
 * The double-floating estimated values are displayed and put in the
 * extended parameter section of the header.
 * The pixel format of the images is byte.
 *
 * -p is for the pipeline application.
 * -s enables to deal with the situation where one image is a "pure"
 * without any noise while the other is corrupted with noise. In this case
 * the number of frames must be 2. Any frames after the first two will
 * either be ignored or passed through if -p is set.
 *
 * to load:	cc -o sigmatemp sigmatemp.c -lhipsa -lhips -lm 
 *
 * Peter Mowforth & Jin Zhengping -7/5/85 
 * Modified by Jin Zhengping -5/1/90 
 *      1)  lift of the maximum image size limit;
 *      2)  change of window-width default value from 9 to 1.
 * Rewritten by Jin Zhengping - 31 August 1991
 */

#include <stdio.h>
#include <math.h>
#include <hipl_format.h>
int types[] = {PFBYTE,LASTTYPE};
static Flag_Format flagfmt[] =
{
	{"p",
		{LASTFLAG},
		0,
		{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"s",
		{LASTFLAG},
		0,
		{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	LASTFLAG
};

#define N_min 16

int main(argc,argv)

int     argc;
char    **argv;

{
	struct          header hd,hdp1,hdp2,*hd1,*hd2,*hdp;
	int             method,f,fr;
	Filename        filename;
	FILE            *fp;
	h_boolean		pass,pure;

	float		*sigmas, *sigmasp;
	byte		*buff;
	int		nrc;
	char		*memcpyl();

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&pass,&pure,FFONE,&filename);
	fp=hfopenr(filename);

	fread_hdr_a(fp,&hd,filename) ;
	method=fset_conversion(&hd,&hdp1,types,filename);
	dup_headern(&hdp1,&hdp2);
	alloc_image(&hdp2);
	hd1 = &hdp1;
	hd2 = &hdp2;

	nrc = hd.orows*hd.ocols;
	if(hdp1.num_frame<2)
		perr(HE_MSG, "number of frames must be greater than 1.");
	if((sigmas=(float *)malloc((unsigned)hdp1.num_frame*sizeof(*sigmas)))==0)
		perr(HE_ALLOC);
	fr = hdp1.num_frame;
	if(pass==TRUE)
		if((buff=(byte *)malloc((unsigned)fr*nrc*sizeof(*buff)))==0)
			perr(HE_ALLOC);

	fread_imagec(fp,&hd,hd2,method,0,filename);
	if(pass==TRUE)
		memcpyl((char *)(buff), (char *)hd2->image, nrc*sizeof(*buff));
	for (sigmasp=sigmas, f=1;f<fr;f++, sigmasp++)
	{
		if(f>1 && pure==TRUE)
		{
			if(pass==TRUE)
			{
				fread_imagec(fp,&hd,hd2,method,f,filename);
				memcpyl((char *)(buff+f*nrc), (char *)hd2->image, nrc*sizeof(*buff));
			}
		} else
		{
			hdp = hd2;
			hd2 = hd1;
			hd1 = hdp;
			fread_imagec(fp,&hd,hd2,method,f,filename);
			if(pass==TRUE)
				memcpyl((char *)(buff+f*nrc), (char *)hd2->image, nrc*sizeof(*buff));
			h_sigmatemp(hd1,hd2,pure,(double *)sigmasp);
		}
	}
	if(pass==TRUE)
	{
		*(sigmas+fr-1) = *(sigmas+fr-2);
		setparam(&hdp1,"sigma",PFFLOAT,fr,(float *)sigmas);
		write_headeru(&hdp1,argc,argv);
		if(fwrite((char *)buff, sizeof(*buff), fr*nrc, stdout)!=fr*nrc)
			perr(HE_HDRWRT);
	}
	free((char *)buff);
	return(0) ;
}
