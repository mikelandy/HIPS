/***************************************************************************
 *      Copyright (c) 1992 Jorgen V. Beck 
 *
 *      Program til generering af et Gaussisk Gradient filter i  
 *      t-retningen.
 *
 *      Options   -s storrelsen paa filtret.
 *                -w den positive centrale region af den 2. afleddet af G
 *
 *      Default   s = 11 og w =4  
 *
 *      Output  filter der kan bruges til programmet mask i HIPS
 *
 *	Modified for HIPS-2 Rasmus Larsen 18/1/93
 **************************************************************************/
#include <hipl_format.h>
#include <stdio.h>
#include <math.h>

static Flag_Format flagfmt[] = {
	{"size",{LASTFLAG},1,
	{{PTINT,"11","size"},LASTPARAMETER}},

	{"w",{LASTFLAG},1,
	{{PTINT,"4","size"},LASTPARAMETER}},

	LASTFLAG
};

float  sqr(x) float x; { return (x*x); }

int main(argc,argv)
int argc;
char *argv[];
{	
	int i,j,w,size; 
	float midt,x,y,sigma,tmp,factor;
	float sum;
	float  **filter;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,
		&size,
		&w,
		FFNONE);

	if (!(size%2)) perr(HE_MSG,"Size must be an odd number");
	sigma=w/(2.0*sqrt(2.0));
	factor = 1.0 / (sqrt(2*H_PI*sigma));
	size--;
	midt=0.5*size;

	filter = (float **)calloc(size,sizeof(float *));
	for (i=0;i<size;i++) {
        	filter[i]= (float *)calloc(size,sizeof(float));
        }

	printf("\"Gaussisk filter w = %d\"\n",w);
        printf("1 4 3\n");
        printf("%d %d %d %d\n\n",size+1,size+1,size/2,size/2);

	sum=0;
	for (i=0;i<=size;i++) {
		y=i-midt;
		for (j=0;j<=size;j++) {
			x=j-midt;
			tmp= -(x*x+y*y)/(2*sqr(sigma));
			tmp=factor * exp(tmp); 
			filter[i][j]=tmp;
/*			sum=sum+tmp; */
		}
	}

	for (i=0;i<=size;i++) {
		for (j=0;j<=size;j++) {
	/*		printf(" %7.5f ",filter[i][j]/sum); */
			printf(" %7.15f ",filter[i][j]);
		}
		printf("\n");
	}
	printf("\n");

	return(0);
}
