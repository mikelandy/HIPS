static char *SccsId = "%W%      %G%";

/*	Copyright (c) 1987 Linda Gillespie 

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
ensure its reliability.   */

/* grey2disp - given a grey-level coded disparity image the
 *	program will generate a random dot stereogram interpretation.
 *	Copes with fractions of a pixel disparity.
 *		 
 * usage: grey2disp [-c] [-d LowestDisp HighestDisp] [-t]
 *
 * -c:          If set, pixel disparity is required. (corresponds to
 *              the original module "grey2disp". Otherwise sub-pixel
 *              disparity is required. (corresponds to the origanal
 *              module "grey2dispcnt".)
 * LowestDisp,
 * HighestDisp: For indicating the range of disparities. 
 * -t:          If TRUE, thresholded random-dot images are required.
 *
 * defaults: cnt = FALSE;
 *         : LowestDisp = 0;
 *	   : HighestDisp = 4;
 *	   : threshold = FALSE;
 *
 * to load: cc -o grey2disp grey2disp.c -lhipsa -lhips -lm
 *
 * Linda Gillespie August 1987.
 * Adapted to HIPS-2 Version by Jin Zhengping - 31 August 1991
 */

#include <hipl_format.h>
#include <stdio.h>
int types[] = {PFBYTE,LASTTYPE};
static Flag_Format flagfmt[] =
{
        {"c",
                {LASTFLAG},
                0,
                {{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"d",
                {LASTFLAG},
                2,
                {{PTINT,"0","lowestDisp"},{PTINT,"4","highestDisp"},LASTPARAMETER}},
        {"t",
                {LASTFLAG},
                0,
                {{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
        LASTFLAG
};

int main(argc,argv)

int	argc ;
char    **argv;

{	
	struct          header hd,hdp,hdo1,hdo2;
	int             method;
	Filename        filename;
	FILE            *fp;
	int		lowestDisp,highestDisp;
	h_boolean		cnt,threshold;
	extern int	picsize;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&cnt,&lowestDisp,&highestDisp,&threshold,FFONE,&filename);
	fp=hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	if ((hd.orows>picsize)||(hd.ocols>picsize))
	{
		char	msg[80];
		sprintf(msg,"max image size = %d",picsize);
		perr(HE_MSG,msg) ;
	}
	method=fset_conversion(&hd,&hdp,types,filename);
	if(hdp.num_frame!=1)
		perr(HE_MSG,"#frames must be equal to 1") ;
	dup_headern(&hdp,&hdo1);
	dup_headern(&hdp,&hdo2);
	alloc_image(&hdo1);
	alloc_image(&hdo2);
	hdo1.num_frame = 2;
	write_headeru2(&hd,&hdo1,argc,argv,hips_convback);
	fread_imagec(fp,&hd,hdp,method,0,filename);
	h_grey2disp(hdp,hdo1,hdo2,cnt,lowestDisp,highestDisp,threshold);
	write_imagec(&hd,&hdo1,method,hips_convback,0);
	write_imagec(&hd,&hdo2,method,hips_convback,1);
	return(0);
}
