static char *SccsId = "%W%      %G%";

/*      Copyright (c) 1991 The Turing Institute

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 * sigmaspat - estimate the standard deviations of random noise of each
 * image in a sequence base on each individual images.
 *
 * usage:	sigmaspat [-p]
 *
 * The double-floating estimated values are displayed and put in the
 * extended parameter section of the header.
 * The pixel format of the images is byte. The longer ROI dimension
 * of the image must be larger than 15 to ensure a reliable
 * estimation. The shorter ROI dimenion must be greater than 1.
 * -p is for the pipeline application.
 *
 * to load:	cc -o sigmaspat sigmaspat.c -lhipsa -lhips -lm 
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
	LASTFLAG
};

#define MINN 16

int main(argc,argv)

int     argc;
char    **argv;

{
	struct          header hd,hdp;
	int             method,f,fr;
	Filename        filename;
	FILE            *fp;
	h_boolean		pass;

	float		*sigmas, *sigmasp;
	byte		*buff;
	int		nrc;
	char		*memcpyl();

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&pass,FFONE,&filename);
	fp=hfopenr(filename);

	fread_hdr_a(fp,&hd,filename) ;
	method=fset_conversion(&hd,&hdp,types,filename);

	nrc = hd.orows*hd.ocols;
	if(((hd.rows<hd.cols) && (hd.cols < MINN)) ||
	   ((hd.cols<=hd.rows) && (hd.rows < MINN)))
	{
		fprintf(stderr, "%s: the maximum ROI dimension of the image is less than %d.\n",
				 Progname, (int)MINN) ;
		fprintf(stderr, "the estimation may not be accurate.\n") ;
	}
	if(hd.rows<3 && hd.cols<3)
		perr(HE_MSG, "the minimal ROI dimension of the image must be larger than 2.") ;

	if((sigmas=(float *)malloc((unsigned)hdp.num_frame*sizeof(*sigmas)))==0)
		perr(HE_ALLOC);
	fr = hdp.num_frame;
	if(pass==TRUE)
		if((buff=(byte *)malloc((unsigned)fr*nrc*sizeof(*buff)))==0)
			perr(HE_ALLOC);

	for (sigmasp=sigmas, f=0;f<fr;f++, sigmasp++)
	{
		fread_imagec(fp,&hd,&hdp,method,f,filename);
		if(pass==TRUE)
			memcpyl((char *)(buff+f*nrc), (char *)hdp.image, nrc*sizeof(*buff));
		h_sigmaspat(&hdp,(double *)sigmasp);
	}
	if(pass==TRUE)
	{
		if(fr==1)
			setparam(&hdp,"sigma",PFFLOAT,fr,*sigmas);
		else
			setparam(&hdp,"sigma",PFFLOAT,fr,(float *)sigmas);
		write_headeru(&hdp,argc,argv);
		if(fwrite((char *)buff, sizeof(*buff), fr*nrc, stdout)!=fr*nrc)
			perr(HE_HDRWRT);
	}
	free((char *)buff);
	return(0) ;
}
