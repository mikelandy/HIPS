static char *SccsId = "%W%      %G%";

/*      Copyright (c) 1991 The Turing Institute

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 * h_asl - apply an adaptive-surface-labeling smoothing filter to a sequence
 *
 * usage:       h_asl(hdi,hdo,sigma2,order,dumpall,median,verbose)
 *
 * where
 *        hdi,hdo -- headers of input and output images.
 *        sigma2  -- noise deviation of the image.
 *        order   -- an alphanumeric string which specifies the order of
 *                   processing modules.
 *        dumpall -- a flag which allows for outputting intermediate results.
 *        median  -- a flag which specifies the "median value method" is used
 *                   for averaging.
 *        verbose -- a flag which allows for printing messages during
 *                   processing. 
 *
 * It handles images of byte or floating-point pixel format.
 *
 * to load:	cc -c -o h_asl.o h_asl.c -lhips
 *
 * Peter Mowforth & Jin Zhengping -16/1/85 
 * Rewritten -21/3/87 
 * Rewritten -21/4/87 
 * Rewritten by Jin Zhengping - 31 August 1991
 */

#include <stdio.h>
#include <stdlib.h>
#include <hipl_format.h>

void padin(),padinf(),dump(),dumpf();
void fit0(),fit1(),fit2();
short median();
int strseg();
float exlp(),fl1(),fb();

int h_asl(hdi,hdo,sigma2,order,dumpall,med,verbose)
struct header	*hdi,*hdo;
float		sigma2;
char		*order;
h_boolean		dumpall,med,verbose;
{
	switch(hdi->pixel_format)
	{
	case PFBYTE:    return(h_asl_b(hdi,hdo,sigma2,order,dumpall,med,verbose));
	case PFFLOAT:    return(h_asl_f(hdi,hdo,sigma2,order,dumpall,med,verbose));
	default:        return(perr(HE_FMTSUBR,"h_asl",
				hformatname(hdi->pixel_format)));
	}
}

int h_asl_b(hdi,hdo,sigma2,order,dumpall,med,verbose)
struct header	*hdi,*hdo;
float		sigma2;
char		*order;
h_boolean		dumpall,med,verbose;
{
	return(h_asl_B(hdi->image,hdo->image,
		hdi->firstpix,hdo->firstpix,
		hdi->rows,hdi->cols,hdi->orows,hdi->ocols,
		sigma2,order,dumpall,med,verbose));
}

int h_asl_f(hdi,hdo,sigma2,order,dumpall,med,verbose)
struct header	*hdi,*hdo;
float		sigma2;
char		*order;
h_boolean		dumpall,med,verbose;
{
	return(h_asl_F((float *)hdi->image,(float *)hdo->image,
		(float *)hdi->firstpix,(float *)hdo->firstpix,
		hdi->rows,hdi->cols,hdi->orows,hdi->ocols,
		sigma2,order,dumpall,med,verbose));
}


#define	SE	"binomial extrapolation for window width"
#define	SL	"binomial  labelling  for  window  width"
#define	FE	"linear extrapolation for  window  width"
#define	FL	"linear   labelling   for  window  width"
#define	ZE	"flat  extrapolation  for  window  width"
#define	ZL	"flat   labelling   for   window   width"

#define clip(x)	(byte)(((x)<0)?(0):(((x)>255)?(255):((x))))

struct inter {
	float	*pic;        /* input image.                                  */
	float	*bop;
	float	*prt;        /* partial results                               */
	h_boolean	*mrk;        /* marks for pixes processed                     */
	h_boolean	*mrk2;       /* marks for pixes processed at the present level*/
	float	*al,*be,*ga; /* vector of surface linear fitting              */
	float	*re ;        /* residue of linear fitting                     */
	int	onr,onc;     /* rows and columns                              */
	int	nr,nc;       /* ROI rows and columns                          */
	int	size;        /* window size                                   */
	int	sizeh;       /* half window size                              */
	float	sigma2;      /* noise deviation                               */
	char	*order;      /* processing order                              */
	h_boolean	dumpall,med,verbose;
			     /* output partial results,
			      * use "median value method",
			      * print processing messages                     */
	byte	*imageo;
	float	*fimageo;
	byte	*firstpixo;
	float	*ffirstpixo;
};

struct im {
	double	a11,a12;
	double	a21,a22,a23;
	double	a33;
};
/* matrix.  */


struct pnt {
	int	i,j,p; /* p=i*nc+j */
	int	lp;
};
/* position  */


int h_asl_B(oimagei,oimageo,imagei,imageo,
	nr,nc,onr,onc,sigma2,order,dumpall,med,verbose)
byte    	*oimagei,*oimageo,*imagei,*imageo;
int     	nr,nc,onr,onc;
float		sigma2;
char		*order;
h_boolean		dumpall,med,verbose;
{
	struct inter	in;
	void		finit(),xtrct(),sl(),maxsize();
	void		lbl0(),lbl1(),lbl2();
	void		xtpl0(),xtpl1(),xtpl2();
	char            *memsetl(),*memcpyl();
	void		*calloc();

	unsigned	nrc=nr*nc;

	in.imageo=oimageo;
	in.firstpixo=imageo;
	in.sigma2=sigma2;
	in.order=order;
	in.dumpall=dumpall;
	in.med=med;
	in.verbose=verbose;
	in.onr=onr;
	in.onc=onc;
	in.nr=nr;
	in.nc=nc;

	if (med) 
	{
		maxsize(&in);
		if((in.bop=(float *)calloc((unsigned)in.size*in.size,sizeof(*in.bop)))==0)
			perr(HE_ALLOC);
	}

	if((in.pic=(float *)calloc(nrc,sizeof(*in.pic)))==0
	|| (in.prt=(float *)calloc(nrc,sizeof(*in.prt)))==0
	|| (in.mrk=(h_boolean *)calloc(nrc,sizeof(*in.mrk)))==0
	|| (in.mrk2=(h_boolean *)calloc(nrc,sizeof(*in.mrk2)))==0
	|| (in.al=(float *)calloc(nrc,sizeof(*in.al)))==0
	|| (in.be=(float *)calloc(nrc,sizeof(*in.be)))==0
	|| (in.ga=(float *)calloc(nrc,sizeof(*in.ga)))==0
	|| (in.re=(float *)calloc(nrc,sizeof(*in.re)))==0)
		perr(HE_ALLOC);

	finit(in.prt, 0.0, nrc);
	memsetl((char *)in.mrk, (int)0, (int)nrc*sizeof(*in.mrk));
	xtrct(imagei,&in);
	memcpyl((char *)oimageo,(char *)oimagei,(int)onr*onc*sizeof(*oimageo));

	/*  The processing begins. Its order is specified by 
	 *  ORDER which is contained in "order". 
	 */
	while(*in.order!='t')
	{
		switch(*in.order++)
		{
		case 'z': /* zero order processing */
			sl(&in,ZL,ZE,lbl0,xtpl0);
			break ;
		case 'f': /* first order processing */
			sl(&in,FL,FE,lbl1,xtpl1);
			break ;
		case 's': /* second order processing */
			sl(&in,SL,SE,lbl2,xtpl2);
			break ;
		}
	}
	if (med) 
		free((char *)in.bop);
	free((char *)in.pic);
	free((char *)in.prt);
	free((char *)in.mrk);
	free((char *)in.mrk2);
	free((char *)in.al);
	free((char *)in.be);
	free((char *)in.ga);
	free((char *)in.re);
	return(HIPS_OK);
}

int h_asl_F(oimagei,oimageo,imagei,imageo,
	nr,nc,onr,onc,sigma2,order,dumpall,med,verbose)
float    	*oimagei,*oimageo,*imagei,*imageo;
int     	nr,nc,onr,onc;
float		sigma2;
char		*order;
h_boolean		dumpall,med,verbose;
{
	struct inter	in;
	void		finit(),xtrctf(),slf(),maxsize();
	void		lbl0(),lbl1(),lbl2();
	void		xtpl0(),xtpl1(),xtpl2();
	char            *memsetl(),*memcpyl();
	void		*calloc();

	unsigned	nrc=nr*nc;

	in.fimageo = oimageo;
	in.ffirstpixo = imageo;
	in.sigma2=sigma2;
	in.order=order;
	in.dumpall=dumpall;
	in.med=med;
	in.verbose=verbose;
	in.onr=onr;
	in.onc=onc;
	in.nr=nr;
	in.nc=nc;

	if (med) 
	{
		maxsize(&in);
		if((in.bop=(float *)calloc((unsigned)in.size*in.size,sizeof(*in.bop)))==0)
			perr(HE_ALLOC);
	}

	if((in.pic=(float *)calloc(nrc,sizeof(*in.pic)))==0
	|| (in.prt=(float *)calloc(nrc,sizeof(*in.prt)))==0
	|| (in.mrk=(h_boolean *)calloc(nrc,sizeof(*in.mrk)))==0
	|| (in.mrk2=(h_boolean *)calloc(nrc,sizeof(*in.mrk2)))==0
	|| (in.al=(float *)calloc(nrc,sizeof(*in.al)))==0
	|| (in.be=(float *)calloc(nrc,sizeof(*in.be)))==0
	|| (in.ga=(float *)calloc(nrc,sizeof(*in.ga)))==0
	|| (in.re=(float *)calloc(nrc,sizeof(*in.re)))==0)
		perr(HE_ALLOC);

	finit(in.prt, 0.0, nrc);
	memsetl((char *)in.mrk, (int)0, (int)nrc*sizeof(*in.mrk));
	xtrctf(imagei,&in);
	memcpyl((char *)oimageo,(char *)oimagei,(int)onr*onc*sizeof(*oimageo));

	/*  The processing begins. Its order is specified by 
	 *  ORDER which is contained in "order". 
	 */
	while(*in.order!='t')
	{
		switch(*in.order++)
		{
		case 'z': /* zero order processing */
			slf(&in,ZL,ZE,lbl0,xtpl0);
			break ;
		case 'f': /* first order processing */
			slf(&in,FL,FE,lbl1,xtpl1);
			break ;
		case 's': /* second order processing */
			slf(&in,SL,SE,lbl2,xtpl2);
			break ;
		}
	}
	if (med) 
		free((char *)in.bop);
	free((char *)in.pic);
	free((char *)in.prt);
	free((char *)in.mrk);
	free((char *)in.mrk2);
	free((char *)in.al);
	free((char *)in.be);
	free((char *)in.ga);
	free((char *)in.re);
	return(HIPS_OK);
}

void
maxsize(in)
struct inter	*in;
{
	int	ww;
	char	*orderp=in->order;

	in->size=0;
        while((*orderp)!='t')
        {
                if((*orderp>='0')&&(*orderp<='9'))
                {
                        ww = strseg(&orderp) ;
			if(ww>in->size)
				in->size=ww;
                } else
			orderp++;
	}
 	
	return;
}

void
xtrct(imagei,in)
byte		*imagei;
struct inter	*in;
{
	int	i,j;
	byte	*ip = imagei;
	float	*picp=in->pic;
	int	nexi=in->onc-in->nc;
	for(i=0;i<in->nr;i++)
	{
		for(j=0;j<in->nc;j++)
			*picp++ = (float)(*ip++);
		ip += nexi;
	}
	return;
}

void
xtrctf(imagei,in)
float		*imagei;
struct inter	*in;
{
	int	i,j;
	float	*ip = imagei;
	float	*picp=in->pic;
	int	nexi=in->onc-in->nc;
	for(i=0;i<in->nr;i++)
	{
		for(j=0;j<in->nc;j++)
			*picp++ = *ip++;
		ip += nexi;
	}
	return;
}

int
strseg(point)
/* this routine takes in a string starting with numerical alphabets
 * (at least one character, otherwise error may occur.) transforms
 * the first segment consisting of only numerical alphabets up to but
 * excluding any character other than numerical alphabets into an
 * interger, and moves the pointer of the string to the first character
 * (could be the terminator) after the segment.
 */
char **point ;
{
	int	number;
	char	*sn,*snp;
	void	*malloc();

	if((sn=(char *)malloc((unsigned)(strlen(*point)*sizeof(**point))))==NULL)
		perr(HE_ALLOC);
	strcpy(sn,*point);
	snp=sn+1;
	while((*snp<='9')&&(*snp>='0'))
		snp++;
	*snp='\0';

	*point+=strlen(sn);
	number = atoi(sn) ;
	free((char *)sn);
	return((number%2==0)? (number-1): number);
}

void
padin(in)
struct inter	*in;
{
	int	i,j;
	int	nexo=in->onc-in->nc;
	float	*prtp=in->prt;
	byte	*opp=in->firstpixo;
	for(i=0;i<in->nr;i++)
	{
		for(j=0;j<in->nc;j++,prtp++)
			*opp++ = clip(*prtp);
		opp += nexo;
	}
	return;
}

void
padinf(in)
struct inter	*in;
{
	int	i,j;
	int	nexo=in->onc-in->nc;
	float	*prtp=in->prt,*opp=in->ffirstpixo;
	for(i=0;i<in->nr;i++)
	{
		for(j=0;j<in->nc;j++)
			*opp++ = *prtp++;
		opp += nexo;
	}
	return;
}

void
dump(in)
struct inter	*in;
{
	int	i,j;
	int	nexo=in->onc-in->nc;
	float	*prtp=in->prt;
	byte	*opp=in->firstpixo;
	for(i=0;i<in->nr;i++)
	{
		for(j=0;j<in->nc;j++,prtp++)
			*opp++ = clip(*prtp);
		opp += nexo;
	}
	if(fwrite((char *)in->imageo,
			sizeof(*in->imageo),
			in->onr*in->onc,
			stdout)
			!= (in->onr*in->onc))
	{
		fprintf(stderr,"write failed.\n");
		exit(1);
	}
	return;
}

void
dumpf(in)
struct inter	*in;
{
	int	i,j;
	int	nexo=in->onc-in->nc;
	float	*prtp=in->prt,*opp=in->ffirstpixo;
	for(i=0;i<in->nr;i++)
	{
		for(j=0;j<in->nc;j++)
			*opp++ = *prtp++;
		opp += nexo;
	}
	if(fwrite((char *)in->fimageo,
			sizeof(*in->fimageo),
			in->onr*in->onc,stdout)
			!= (in->onr*in->onc))
	{
		fprintf(stderr,"write failed.\n");
		exit(1);
	}
	return;
}

void
sl(in,sll,se,lbl,xtpl)
struct inter	*in;
char		*sll,*se;
void		(*lbl)(), (*xtpl)();
{
	in->size = strseg(&in->order);
	in->sizeh=in->size/2;
	if (in->verbose==TRUE) 
		fprintf(stderr,"   %s  %d\n",sll,in->size);
	lbl(in);
	if (in->dumpall)
		dump(in);

	if(*in->order=='t')
		in->sigma2 = (in->size*in->size)*(255*255) ;
	if (in->verbose==TRUE) 
		fprintf(stderr,"   %s  %d\n",se,in->size);
	xtpl(in) ;
	if (*in->order=='t')
		padin(in);
	else if(in->dumpall)
		dump(in);
	return;
}

void
slf(in,sl,se,lbl,xtpl)
struct inter	*in;
char		*sl,*se;
void		(*lbl)(), (*xtpl)();
{
	in->size = strseg(&in->order);
	in->sizeh=in->size/2;
	if (in->verbose==TRUE) 
		fprintf(stderr,"   %s  %d\n",sl,in->size);
	lbl(in);
	if (in->dumpall)
		dumpf(in);

	if(*in->order=='t')
		in->sigma2 = (in->size*in->size)*(255*255) ;
	if (in->verbose==TRUE) 
		fprintf(stderr,"   %s  %d\n",se,in->size);
	xtpl(in) ;
	if (*in->order=='t')
		padinf(in);
	else if(in->dumpall)
		dumpf(in);
	return;
}

float
exlp(in,pn1,pn2,invm,fit)
struct inter	*in;
struct pnt	*pn1,*pn2;
struct im	*invm;
void		(*fit)();
{
	int	i=pn1->i;
	int	j=pn1->j;
	int	pp=pn1->p;
	int	lp=pn1->lp;
	int	nc=in->nc;
	float	sigt = in->re[pn1->p];
	for(pn2->i=i-lp; pn2->i<=i+lp; pn2->i+=(lp*2))
		for(pn2->j=j-lp; pn2->j<=j+lp; pn2->j++) 
		{
			pn2->p=pn2->i*nc+pn2->j;
			if (!in->mrk2[pn2->p]) 
				fit(in,pn2,invm) ;
			if (in->re[pn2->p] < sigt)
			{
				sigt = in->re[pn2->p];
				pp = pn2->p;
			}
		}
	for(pn2->i=i-lp+1; pn2->i<=i+lp-1; pn2->i++)
		for(pn2->j=j-lp; pn2->j<=j+lp; pn2->j+=(lp*2)) 
		{
			pn2->p=pn2->i*nc+pn2->j;
			if (!in->mrk2[pn2->p]) 
				fit(in,pn2,invm) ;
			if (in->re[pn2->p] < sigt) 
			{
				sigt = in->re[pn2->p];
				pp = pn2->p;
			}
		}
	pn2->i=pp/nc;
	pn2->j=pp%nc;
	pn2->p=pp;
	return(sigt);
}


/*    This function does the actual flat fitting and 
 *    stores the parameters obtained
 *    in the vector (ga,re)(i,j).
 */

void
fit0(in,pn,invm)
struct inter	*in;
struct pnt	*pn;
struct im	*invm;
{
	int		nc=in->nc;
	int		wwh=in->sizeh;

	register float	*picj,*pici;
	register int	xi,yi ;
	register float	w1=0;
	float		ss=0.0,w ;

	for(pici=in->pic+(pn->i-wwh)*nc,xi = -wwh; xi<=wwh; xi++, pici+=nc)
		for(picj=pici+pn->j-wwh, yi = -wwh; yi<=wwh; yi++, picj++)
			w1 += *picj ;
	w1/=invm->a33;

	for(pici=in->pic+(pn->i-wwh)*nc,xi = -wwh; xi<=wwh; xi++, pici+=nc)
		for(picj=pici+pn->j-wwh, yi = -wwh; yi<=wwh; yi++, picj++)
		{
			w = w1 - *picj;
			ss += w * w;
		}
	in->ga[pn->p] = w1;
	in->re[pn->p] = ss/invm->a33;
	in->mrk2[pn->p] = 1;

	return ;
}

/*     This function returns the median value of pixel at (i,j)  
 *     of pic with window width 2*wwh+1.
 */
short
median(in,pn)
struct inter	*in;
struct pnt	*pn;
{
	int	ii,jj,i1;
	float	*p1;
	float	*pic=in->pic;
	int	nc=in->nc;
	int	wwh=in->sizeh;
	int	ww2 = in->size*in->size;
	int	i2 = ww2/2;

	for(p1=in->bop, ii=pn->i-wwh; ii<=pn->i+wwh; ii++)
		for(jj=pn->j-wwh; jj<=pn->j+wwh; jj++)
			*p1++ = pic[ii*nc+jj] ;
	for(ii=ww2-1; ii>=i2; ii--)
		for(p1=in->bop,jj=0; jj<ii; jj++,p1++)
			if(*p1 < *(p1+1))
			{
				i1 = *(p1+1),
				*(p1+1) = *p1,
				*p1 = i1;
			}
	return(*(in->bop+i2)) ;
}

/*     This function fits a flat polynomial facet of size w x w,
 *     where w = 2 x wwh + 1, and labels the pixel
 *     if it is fit, otherwise leaves it untouched.
 */
void
lbl0(in) 
struct inter	*in;
{
	struct im	invm;
	struct pnt	pn;
	int		wwh=in->sizeh;
	int		nr=in->nr;
	int		nc=in->nc;
	int		rup=nr-wwh;
	int		cup=nc-wwh;
	char		*memsetl();

	invm.a33 = in->size*in->size;

	memsetl((char *)in->mrk2,(int)0,nr*nc*sizeof(*in->mrk2));
	for(pn.i=wwh; pn.i<rup; pn.i++)
	    for(pn.p=pn.i*nc+wwh,pn.j=wwh; pn.j<cup; pn.j++,pn.p++)
		if (!in->mrk[pn.p])
		{
		    fit0(in,&pn,&invm) ;
		    if (in->re[pn.p] < in->sigma2)
		    {
			in->mrk[pn.p] = 1;
			in->prt[pn.p] = (in->med)? median(in,&pn): (in->ga[pn.p]);
		    }
		}
	return ;
}

/*    This function does the actual linear fitting and 
 *    stores the parameters obtained
 *    in the vector (al,be,ga,re)(i,j).
 */

void
fit1(in,pn,invm)
struct inter	*in;
struct pnt	*pn;
struct im	*invm;
{
	register float	*picj,*pici;
	register int	xi,yi ;
	register float	w1=0.0,w2=0.0,w3=0.0 ;
	float		ss=0.0;
	float		w;
	int		nc=in->nc;
	int		wwh=in->sizeh;


	for(pici=in->pic+(pn->i-wwh)*nc,xi = -wwh; xi<=wwh; xi++, pici+=nc)
		for(picj=pici+pn->j-wwh, yi = -wwh; yi<=wwh; yi++, picj++)
		{
			w1 += (*picj*xi);
			w2 += (*picj*yi);
			w3 += *picj;
		}
	w1/=invm->a11;
	w2/=invm->a11;
	w3/=invm->a33;

	for(pici=in->pic+(pn->i-wwh)*nc,xi = -wwh; xi<=wwh; xi++, pici+=nc)
		for(picj=pici+pn->j-wwh, yi = -wwh; yi<=wwh; yi++, picj++)
		{
			w = w1*xi + w2*yi + w3 - *picj;
			ss += (w*w);
		}
	in->al[pn->p] = w1;
	in->be[pn->p] = w2;
	in->ga[pn->p] = w3;
	in->re[pn->p] = ss/invm->a33;
	in->mrk2[pn->p] = 1;

	return ;
}

/*     This function fits a linear polynomial facet of size w x w,
 *     where w = 2 x wwh + 1, and labels the pixel
 *     if it is fit, otherwise leaves it untouched.
 */
void
lbl1(in)
struct inter	*in;
{
	struct im	invm;
	struct pnt	pn;
	int		wwh=in->sizeh;
	int		nr=in->nr;
	int		nc=in->nc;
	int		rup=nr-wwh;
	int		cup=nc-wwh;
	char		*memsetl();

	invm.a33 = in->size*in->size;
	invm.a11 = invm.a33 * wwh * (wwh+1) / 3 ;

	memsetl((char *)in->mrk2,(int)0,nr*nc*sizeof(*in->mrk2));
	for(pn.i=wwh; pn.i<rup; pn.i++)
	    for(pn.p=pn.i*nc+wwh,pn.j=wwh; pn.j<cup; pn.j++,pn.p++)
		if (!in->mrk[pn.p])
		{
		    fit1(in,&pn,&invm) ;
		    if (in->re[pn.p] < in->sigma2)
		    {
			in->mrk[pn.p] = 1;
			in->prt[pn.p] = (in->med)? median(in,&pn): (in->ga[pn.p]);
		    }
		}
	return ;
}

/*     This function does the actual binomial fitting of size 
 *     (wwh*2+1)(wwh*2+1), returns the residue and part of the
 *     parameter vector. 
*/
void
fit2(in,pn,invm)
struct inter	*in;
struct pnt	*pn;
struct im	*invm;
{
	register float	*picj,*pici;
	register int	i1;
	register int	xi,yi ;
	register float	w11=0.0,w12=0.0,w13=0.0;
	register float	w21=0.0,w22=0.0,w23=0.0;
	float		ss=0.0;
	float		w,v11, v12, v13;
	int		nc=in->nc;
	int		wwh=in->sizeh;


	for(pici=in->pic+(pn->i-wwh)*nc,xi = -wwh; xi<=wwh; xi++, pici+=nc)
		for(picj=pici+pn->j-wwh, yi = -wwh; yi<=wwh; yi++, picj++)
		{
			i1 = *picj * xi;
			w11 += i1 * xi, w21 += i1 * yi, w22 += i1;
			i1 = *picj * yi;
			w12 += i1 * yi, w23 += i1, w13 += *picj;
		}

	w21 /=invm->a11;
	w22 /=invm->a12;
	w23 /=invm->a12;

	v11 = invm->a21*w11                 + invm->a22*w13 ;
	v12 =                 invm->a21*w12 + invm->a22*w13 ;
	v13 = invm->a22*w11 + invm->a22*w12 + invm->a23*w13 ;
	w11=v11;
	w12=v12;
	w13=v13;

#ifdef	JINDEBUG
	fprintf(stderr, "parameters...\n");
	fprintf(stderr, "t%0.2fX*X + %0.2fX*Y + %0.2fY*Y\n",w11,w21,w12);
	fprintf(stderr, " + %0.2fX + %0.2fY + %0.2f\n",w22, w23, w13);
#endif

	for(pici=in->pic+(pn->i-wwh)*nc,xi = -wwh; xi<=wwh; xi++, pici+=nc)
		for(picj=pici+pn->j-wwh, yi = -wwh; yi<=wwh; yi++, picj++)
		{
			w = w11 * xi * xi + w12 * yi * yi + w21 * xi * yi 
			    + w22 * xi + w23 * yi + w13 - *picj;
			ss += w * w;
		}

	in->al[pn->p] = w11 ;
	in->be[pn->p] = w12 ;
	in->ga[pn->p] = w13 ;
	in->re[pn->p] = ss/invm->a33 ;
	in->mrk2[pn->p] = 1;

	return ;
}

/*     This function fits a binomial polynomial facet of size 3 x 3,
 *     and labels the pixel if it is fit, otherwise 
 *     leaves it untouched.
 */
void
lbl2(in) 
struct inter	*in;
{
	struct im	invm;
	struct pnt	pn;
	int		wwh=in->sizeh;
	int		i = in->size;
	int		j = wwh*(wwh+1) ;
	int		nr=in->nr;
	int		nc=in->nc;
	int		rup=nr-wwh;
	int		cup=nc-wwh;
	char		*memsetl();

	invm.a33 = i*i ;
	invm.a23 = invm.a33*(4*j-3) ;
	invm.a21 = 45.0/(j*invm.a23) ;
	invm.a22 = -15.0/invm.a23 ;
	invm.a23 = (14.0*j-3)/invm.a23;
	invm.a12 = i*j/3 ;
	invm.a11 = invm.a12*invm.a12 ;
	invm.a12 *= i ;

	memsetl((char *)in->mrk2,(int)0,nr*nc*sizeof(*in->mrk2));
	for(pn.i=wwh; pn.i<rup; pn.i++)
	    for(pn.p=pn.i*nc+wwh,pn.j=wwh; pn.j<cup; pn.j++,pn.p++)
		if(!in->mrk[pn.p])
		{
		    fit2(in,&pn,&invm) ;
		    if(in->re[pn.p] < in->sigma2)
		    {
			in->mrk[pn.p] = 1;
			in->prt[pn.p] = (in->med)? median(in,&pn): (in->ga[pn.p]);
		    }
		}
	return ;
}


/*     This function fits flat polynomial facets of size w x w,
 *     where w = 2 x wwh + 1, around a central pixel 
 *     and finds the best fit. It then extrapolates the central 
 *     pixel if it is regarded as fit, 
 *     otherwise leaves it untouched.
 */
void
xtpl0(in) 
struct inter	*in;
{

	int		cl,cr,ru,rl;
	struct im	invm;
	struct pnt	pn1,pn2;
	int		wwh=in->sizeh ;
	int		nr=in->nr;
	int		nc=in->nc;

	invm.a33 = in->size*in->size;
	for(pn1.lp=1; pn1.lp<=wwh; pn1.lp++)
	{
	    cl = ru = wwh + pn1.lp; 
	    cr = nc - cl; 
	    rl = nr - cl;
	    for(pn1.i=ru; pn1.i<rl; pn1.i++)
		for(pn1.p=pn1.i*nc+cl,pn1.j=cl; pn1.j<cr; pn1.j++,pn1.p++)
		    if ((!in->mrk[pn1.p]) &&
			(exlp(in,&pn1,&pn2,&invm,fit0)<in->sigma2))
		    {
			in->mrk[pn1.p] = 1;
			in->prt[pn1.p] = in->ga[pn2.p];
		    }
	} /* loop "lp" */
	return ;
}

float
fl1(in,pn1,pn2)
struct inter	*in;
struct pnt	*pn1,*pn2;
{
	return(in->al[pn2->p]*(pn1->i-pn2->i)+
		    in->be[pn2->p]*(pn1->j-pn2->j)+
		    in->ga[pn2->p]);
}

/*     This function fits linear polynomial facets of size w x w,
 *     where w = 2 x wwh + 1, around a central pixel 
 *     and finds the best fit. It then extrapolates the central 
 *     pixel if it is regarded as fit, 
 *     otherwise leaves it untouched.
 */
void
xtpl1(in) 
struct inter	*in;
{
	int		cl,cr,ru,rl;
	struct im	invm;
	struct pnt	pn1,pn2;
	int		wwh=in->sizeh;
	int		nr=in->nr;
	int		nc=in->nc;

	invm.a33 = in->size*in->size;
	invm.a11 = invm.a33 * wwh * (wwh+1)/3 ;

	for(pn1.lp=1; pn1.lp<=wwh; pn1.lp++)
	{
	    cl = ru = wwh + pn1.lp; 
	    cr = nc - cl; 
	    rl = nr - cl;
	    for(pn1.i=ru; pn1.i<rl; pn1.i++)
		for(pn1.p=pn1.i*nc+cl,pn1.j=cl; pn1.j<cr; pn1.j++,pn1.p++)
		    if ((!in->mrk[pn1.p]) &&
		        (exlp(in,&pn1,&pn2,&invm,fit1)<in->sigma2))
		    {
			in->mrk[pn1.p] = 1;
			in->prt[pn1.p] = fl1(in,&pn1,&pn2);
		    }
	} /* loop "lp" */
	return ;
}

/*     This function returns the extrapolation value of 
 *     binomial fitting.
*/
float
fb(in,pn1,pn2,invm) 
struct inter	*in;
struct pnt	*pn1,*pn2;
struct im	*invm;
{
	int		nc=in->nc;
	int		wwh=in->sizeh; 
	int		kk,ll ;

	register float	*picj,*pici;
	register int	xi, yi;
	register int	i1;
	register float	w21=0.0,w22=0.0,w23=0.0;
	register float	w11 = in->al[pn2->p] ; 
	register float  w12 = in->be[pn2->p] ;
	register float  w13 = in->ga[pn2->p] ;

	for(pici=in->pic+(pn2->i-wwh)*nc, xi = -wwh; xi<=wwh; xi++, pici+=nc)
		for(picj=pici+pn2->j-wwh, yi = -wwh; yi<=wwh; yi++, picj++)
		{
			i1 = *picj * xi;
			w21 += i1 * yi, w22 += i1;
			w23 += *picj * yi;
		}

	w21/=invm->a11;
	w22/=invm->a12;
	w23/=invm->a12;

	kk = pn1->i - pn2->i, ll = pn1->j - pn2->j;
	return(w11*kk*kk + w12*ll*ll + w21*kk*ll + w22*kk + w23*ll + w13);
}

/*     This function fits binomial polynomial facets of size 3 x 3
 *     around a central pixel and finds the best fit. 
 *     It then extrapolates the central pixel no matter whether it 
 *     is regarded as fit or not. 
 */
void
xtpl2(in) 
struct inter	*in;
{

	int		cl,cr,ru,rl;
	struct im	invm;
	struct pnt	pn1,pn2;
	int		wwh=in->sizeh;
	int		i = in->size;
	int		j = wwh*(wwh+1) ;
	int		nr=in->nr;
	int		nc=in->nc;

	invm.a33 = i * i ;
	invm.a23 = invm.a33 * (4 * j - 3) ;
	invm.a21 = 45.0 / (j * invm.a23) ;
	invm.a22 = -15.0 / invm.a23 ;
	invm.a23 = (14.0 * j - 3) / invm.a23;
	invm.a12 = i * j / 3 ;
	invm.a11 = invm.a12 * invm.a12 ;
	invm.a12 *= i ;

	for(pn1.lp=1; pn1.lp<=wwh; pn1.lp++)
	{
	    cl = ru = wwh + pn1.lp; 
	    cr = nc - cl; 
	    rl = nr - cl;
	    for(pn1.i=ru; pn1.i<rl; pn1.i++)
		for(pn1.j=cl,pn1.p=pn1.i*nc+cl; pn1.j<cr; pn1.j++,pn1.p++)
		    if ((!in->mrk[pn1.p]) && 
		        (exlp(in,&pn1,&pn2,&invm,fit2)<in->sigma2))
		    {
			in->mrk[pn1.p] = 1;
			in->prt[pn1.p] = fb(in,&pn1,&pn2,&invm) ; 
		    }
	} /* loop "lp" */
	return ;
}

void
finit(p, v, n)
float		*p,v;
unsigned	n;
{
	int	i;
	for(i=0;i<n;i++)
		*p++=v;
	return;
}

char *
memsetl(s,c,n)
char	*s;
int	c,n;
{
#ifdef	SUN
	return(memset(s,c,n));
#else
	char	*sp=s;
	while(n-- > 0)
		*sp++ = (char)c;
	return(s);
#endif
}

char *
memcpyl(s1,s2,n)
char	*s1,*s2;
int	n;
{
#ifdef	SUN
	return(memcpy(s1,s2,n));
#else
	char	*sp=s1;
	while(n-- > 0)
		*sp++ = *s2++;
	return(s1);
#endif
}
