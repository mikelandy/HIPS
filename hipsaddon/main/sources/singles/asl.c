static char *SccsId = "%W%      %G%";

/*      Copyright (c) 1991 The Turing Institute

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 * asl - an adaptive-surface-labeling smoothing filter
 *
 * usage:	asl [-a] [-d] [-o order | -w size] [-s sigma] [-v]
 *
 *
 * asl applies an adaptive-surface-labeling smoothing filter to each
 * image of the input sequence to suppress random noise.
 * The options available are described below.
 *
 *    -a        a flag which allows for outputting intermediate results 
 *              as well as the final processed image. It defaults to 
 *              outputting the final image only.
 *
 *    -d        a flag which specifies the averaging method used. 
 *              With -d "median value" method is used, otherwise "mean 
 *              value" method is used.
 *
 *    order     an alphanumeric string which specifies the order of 
 *              processing modules and defaults to fwfw-1...f3s3, where
 *              "w" is "size" and s3 contains the forcing phase. 
 *              The syntax of "order" is:
 *
 *                  order = AN[N]{AN[N]}*[t]
 *  
 *                   where:
 *                          A = z | f | s 
 *                          N = 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9
 * 
 *                   where z for zero order, f for first order and 
 *                   s for second order modules, and t stands for
 *                   termination which means the final phase is also
 *                   a forcing one.
 *
 *              Note: a theoretical order beginning from zero order 7x7
 *                    is z7z5f7z3f5s7s5f3s3.
 *
 *    sigma     a real number which specifies the standard deviation of 
 *              noise in the image and defaults to the standard deviation
 *              values in the extended parameter section of the header
 *              produced by "sigmaspat" or "sigmatemp" (see sigmaspat 
 *              or sigmatemp).
 *
 *    -v        a flag which allows for printing messages during
 *              processing.
 *
 *    size      an integer which specifies the initial width of the window 
 *              in which asl operates. It is required to be an odd positive
 *              number and defaults to 7, i.e., 7 x 7 window. If an even 
 *              number is specified, it will be reduced by 1 automatically.
 *
 * The module handles images of byte or floating-point pixel format.
 *
 * to load:	cc -o asl asl.c -lhipsa -lhips -lm
 *
 * Peter Mowforth & Jin Zhengping -16/1/85 
 * Rewritten -21/3/87 
 * Rewritten -21/4/87 
 * Rewritten by Jin Zhengping - 31 August 1991
 */

#include <stdio.h>
#include <math.h>
#include <hipl_format.h>

int types[] = {PFBYTE,PFFLOAT,LASTTYPE};
static Flag_Format flagfmt[] =
{
	{"a",
		{LASTFLAG},
		0,
		{{PTBOOLEAN, "FALSE"},LASTPARAMETER}},
	{"d",
		{LASTFLAG},
		0,
		{{PTBOOLEAN, "FALSE"},LASTPARAMETER}},
	{"o",
		{"w",LASTFLAG},
		1,
		{{PTSTRING,"f7f5f3s3","order"},LASTPARAMETER}},
	{"s",
		{LASTFLAG},
		1,
		{{PTDOUBLE,"-1.0","sigma"},LASTPARAMETER}},
	{"v",
		{LASTFLAG},
		0,
		{{PTBOOLEAN, "FALSE"},LASTPARAMETER}},
	{"w",
		{"o",LASTFLAG},
		1,
		{{PTINT,"7","size"},LASTPARAMETER}},
	LASTFLAG
};

#define  MAXMASK 99
/* MAXMASK:the largest window size this programme can handle. 
 *         It must be great than 8.
 */

int main(argc,argv)

int     argc;
char    **argv;

{
	struct          header hd,hdp,hdo;
	int             method,f,fr;
	Filename        filename;
	FILE            *fp;

	h_boolean		verbose, dumpall, med;
	double		sigma;
	float		*sigmas,*inits() ;
	char		*order="";
	int		ww=0;
	int		numstages,ordercheck();


	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,
		&dumpall,
		&med,
		&order,
		&sigma,
		&verbose,
		&ww,
		FFONE,&filename);

	fp=hfopenr(filename);

	numstages=ordercheck(&order,ww,verbose);
	/* note: the routine will allocate memory for "order" */

	fread_hdr_a(fp,&hd,filename);
	method=fset_conversion(&hd,&hdp,types,filename);
	dup_headern(&hdp,&hdo);
	fr = hdp.num_frame;
	if (dumpall) 
		hd.num_frame = hdo.num_frame = 2*(numstages-1)*fr;
	alloc_image(&hdo);
	sigmas=inits(sigma,&hd,fr);
	/* note: the routine will allocate memory for "sigmas" */
	write_headeru2(&hd,&hdo,argc,argv,hips_convback);


	if (verbose==TRUE) 
		fprintf(stderr,"\n   asl processing begins\n");

	for (f=0;f<fr;f++)
	{
		if (verbose==TRUE) 
			fprintf(stderr,"   frame #%d\n",f);

		fread_imagec(fp,&hd,&hdp,method,f,filename);
		h_asl(&hdp,&hdo,(*(sigmas+f)**(sigmas+f)),order,dumpall,med,verbose);
		write_imagec(&hd,&hdo,method,hips_convback,f);
	} /* loop "fr" */
	free((char *)order);
	free((char *)sigmas);
	if (verbose==TRUE) 
		fprintf(stderr,"   asl has finished successfully.\n");
	return(0) ;
}

float *
inits(sigma,hd,fr)
double		sigma;
struct header	*hd;
int		fr;
{
	int	f;
	float	*sigmas;
	if((sigmas=(float *)calloc((unsigned)fr,sizeof(*sigmas)))==0)
		perr(HE_ALLOC);
	if(sigma==-1.0)
	{
		if(findparam(hd,"sigma") == NULLPAR)
			for (f=0;f<fr;f++)
				*(sigmas+f) = 0.0;
		else
		{
			if(fr==1)
				getparam(hd,"sigma",PFFLOAT,fr,sigmas);
			else
				getparam(hd,"sigma",PFFLOAT,fr,&sigmas);
		}
	} else
		for (f=0;f<fr;f++)
			*(sigmas+f) = sigma;
	return(sigmas);
}

int
ordercheck(order,ww,verbose)
char	**order;
int	ww;
h_boolean	verbose;
{
	char	*orderp;
	int	i,strseg();
	int	numstages=0;
	if(ww>0)
	{
		if((ww%2)==0) ww--;
		if(ww<1)
			perr(HE_MSG,"mask size non-positive") ;
		i=((ww/2)*((int)log10((double)ww)+2)+4)*sizeof(**order);
		if((*order=(char *)malloc((unsigned)(i)))==0)
			perr(HE_ALLOC);
		orderp = *order;
		for(i=ww; i>2; i-=2)
		{
			sprintf(orderp, "f%d", i);
			orderp+=strlen(orderp);
		}
		strcat(orderp, "s3t");
		orderp = *order;
	} else
	{
		if((orderp=(char *)malloc((unsigned)strlen(*order)*sizeof(**order)))==0)
			perr(HE_ALLOC);
		strcpy(orderp, *order);
		*order=orderp;
	}

	if(verbose==TRUE)
		fprintf(stderr,"\nThe order of processing modules is:\n\n");
	while((*orderp)!='\0')
	{
		switch(*orderp++)
		{
		case 'f':
			if(verbose==TRUE)
				fprintf(stderr,"   first  order:  ");
			break ;
		case 's':
			if(verbose==TRUE)
				fprintf(stderr,"   second order:  ");
			break ;
		case 't':
			if(verbose==TRUE)
				fprintf(stderr,"   forcing.\n");
			break ;
		case 'z':
			if(verbose==TRUE)
				fprintf(stderr,"   zero   order:  ");
			break ;
		default:
			perr(HE_MSG, "unknown option for -o") ;
		}
		numstages++ ;
		if((*orderp)!='\0')
		{
			if((*orderp<'0')||(*orderp>'9'))
				perr(HE_MSG, "unknown option for -o");
			ww = strseg(&orderp) ;
			if(ww<1)
				perr(HE_MSG,"mask size non-positive") ;
			if(verbose==TRUE)
				fprintf(stderr,"      %dx%d\n", ww,ww);
		}
	}

	return(numstages);
}
