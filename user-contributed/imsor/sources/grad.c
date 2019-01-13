/***************************************************************************
 *      Copyright (c) 1992 Jorgen V. Beck 
 *
 *      Program til generering af et Gaussisk Gradient filter i x- eller
 *      y-retningen.
 *
 *      Options   -s storrelsen paa filtret.
 *                -w den positive centrale region af den 2. afleddet af G
 *                -x giver filter til beregning i x retningen
 *                -y giver filter til beregning i y retningen
 *
 *      Default   s = 11 og w =4   x sat.
 *
 *      Output  filter der kan bruges til programmet mask i HIPS
 *
 *	Modified for HIPS-2 Rasmus Larsen 18/1/93
 **************************************************************************/
#include <hipl_format.h>
#include <stdio.h>
#include <math.h>

static Flag_Format flagfmt[] = {
	{"x",{"y",LASTFLAG},0,
	{{PTBOOLEAN,"TRUE"},LASTPARAMETER}},

	{"y",{"x",LASTFLAG},0,
	{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},

	{"size",{LASTFLAG},1,
	{{PTINT,"11","size"},LASTPARAMETER}},

	{"w",{LASTFLAG},1,
	{{PTINT,"4","size"},LASTPARAMETER}},

	LASTFLAG
};

float  sqr();

int main(argc,argv)
int argc;
char *argv[];
{	
	h_boolean	xret,yret;
	int	i,j;
	int	w,size; 
	float	midt,r,x,y,sigma,konst,tmp;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,
		&xret,
		&yret,
		&size,
		&w,
		FFNONE);

	sigma=w/(2.0*sqrt(2.0));
	if (!(size%2)) perr(HE_MSG,"Size must be an odd number");
	size--;
	midt=0.5*size;
	printf("\"Gaussisk gradient filter w = %d\"\n",w);
        printf("1 4 3\n");
        printf("%d %d %d %d\n\n",size+1,size+1,size/2,size/2);

        for (i=0;i<=size;i++) {
		for (j=0;j<=size;j++) {
			y=i-midt;
			x=j-midt;
			tmp= -(sqr(x)+sqr(y))/(2*sqr(sigma));
              
			if (xret) tmp= x*exp(tmp)/(2*H_PI*sqr(sqr(sigma)));
			if (yret) tmp= y*exp(tmp)/(2*H_PI*sqr(sqr(sigma)));
	/*		if (tmp<0)	tmp -= 0.5;
			else		tmp += 0.5; */
			printf("%7.15f ",tmp);
		}
		printf("\n");
	}
}

float  sqr(x)
float x;
{ return (x*x); }
