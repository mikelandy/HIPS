/*
Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the author.*/ 

/*      deriche.c   apply a recursiv canny-edge-detector to an image.
 *      usage :   deriche [alpha] [omega] <iseq >oseq 
 *      or        deriche [alpha] <iseq >oseq
 *
 *      defaults: 
 *                alpha = 10
 *                omega = alpha/1000
 *
 *      For more information see:
 *              "A Computational Approach to Edge Detection"
 *              - John Canny
 *              - IEEE Trans. on Patt.Anal.&Machine Intell.
 *              - Vol PAMI 8. NO. 6. NOV. 86
 *              
 *              "Using Canny's criteria to derive a recusively
 *              implem. optimal edge detektor."
 *              -Rachid Deriche.
 *              -Int.J.of comp.vision 167-187 1987.
 *
 *      Author: Arni G. Sigurdsson 
 *
 *      Modified by Carsten K. Olsson, 15/3-91
 *
 */

#include <stdio.h>
#include <math.h>
/* #include <stdlib.h> */

#ifndef HNOVALUESH
#include <values.h>
#ifndef LN_MAXDOUBLE
#define LN_MAXDOUBLE    (M_LN2 * DMAXEXP)
#endif
#endif

#include <hipl_format.h>

#define Sqr(x)  ((x)*(x))

double fltmax,lfltmax;
void filter(),f_filter(),h_filter2();
int make_complx();

int main(argc,argv)

char    *argv[];

{
        struct  header hd;

        float           *ofr,*opf,*ipf;
        int             r,c,f,fr;
        double          alpha,omega;

        Progname = strsave(*argv);

#ifdef HNOVALUESH
	fltmax = MAXFLOAT;
	lfltmax = log(MAXFLOAT);
#else
	fltmax = MAXDOUBLE;
	lfltmax = LN_MAXDOUBLE; 
#endif
        read_header (&hd);
        if(hd.pixel_format != PFFLOAT)
                perr(HE_MSG,"pixel format must be float");
        r = hd.orows; c = hd.ocols;
        fr = hd.num_frame;
	setformat(&hd,PFCOMPLEX);
        update_header (&hd,argc,argv);
        write_header (&hd);
        switch (argc) {
                case 2:
                        alpha = (double)atof(argv[1]);
                        omega = alpha / 1000;
                        break;
                case 3:
                        alpha = (double)atof(argv[1]);
                        omega = (double)atof(argv[2]);
                        break;
                default:
                        alpha = 10.0;
                        omega = alpha / 1000;
                        break;
        }
        if (alpha<=0 || omega<=0)
                perr(HE_MSG,"Arguments must be > 0");
        if (omega>M_PI/2)
                perr(HE_MSG,"Unreasonable value of omega (second argument), if \
                        omega unspecified then please specify omega");
        ipf= (float *) malloc(r*c*sizeof (float));
        ofr = (float *) halloc(2*r*c,sizeof (float));
        for (f=0;f<fr;f++) {
                if (fread(ipf,r*c*sizeof(float),1,stdin) != 1)
                        perr(HE_MSG,"error during read");
                opf = ofr;
                filter(r,c,alpha,omega,ipf,opf);
                if (fwrite(ofr,2*r*c*sizeof(float),1,stdout) != 1)
                        perr(HE_MSG,"error during write");
        }
        return(0);
}



void filter(row,col,alpha,omega,inp_buf,outp_buf)
/*---------                                        */
int     row,col;
float   alpha,omega;
float   *inp_buf,*outp_buf;
{
        double   a,a0,a1,a2,a3,b1,b2,c1,c2,k,exp1,exp2,com,som;
        float   *buf1,*buf2;
        int     hor_indx(),ver_indx();

        buf1 = (float *)halloc(row*col,sizeof(float));
        buf2 = (float *)halloc(row*col,sizeof(float));

        if (-alpha<-lfltmax)
                exp1 = 0;
        else{
                if (-alpha>lfltmax)
                        exp1 = fltmax;
                else
                        exp1 = exp(-alpha);
        }
        if (-2*alpha<-lfltmax)
                exp2 = 0;
        else{
                if (-2*alpha>lfltmax)
                        exp2 = fltmax;
                else
                        exp2 = exp(-2*alpha);
        }
        com = cos(omega);
        som = sin(omega);

        k = Sqr(1-exp1)*Sqr(alpha)/(1+2*alpha*exp1-exp2);
        a = -Sqr(1-exp1)*som;
        b1 = -2*exp1*com;
        b2 = exp2;
        c2 = (alpha*alpha+omega*omega);
        c1 = k*alpha/c2;
        c2 = k*omega/c2;
        a0 = c2;
        a1 = (-c2*com + c1*som)*exp1;
        a2 = a1 - c2*b1;
        a3 = -c2*b2;
        f_filter(a,b1,b2,row-1,col-1,hor_indx,col,inp_buf,buf1);
        h_filter2(a0,a1,a2,a3,b1,b2,col-1,row-1,ver_indx,col,buf1,buf1);
        f_filter(a,b1,b2,col-1,row-1,ver_indx,col,inp_buf,buf2);
        h_filter2(a0,a1,a2,a3,b1,b2,row-1,col-1,hor_indx,col,buf2,buf2);
        make_complx(row,col,buf1,buf2,outp_buf);
        free(buf2);
        free(buf1);

}

int     make_complx(row,col,real,imag,out)
/*---------                                        */
/*      make_complx creates a complex image out of 
        two real images.
        row *col is the actual size of the image 
*/
int     row,col;
float   *real,*imag,*out;
{
        int     i;
        float   *dum;

        dum = out;
        for(i=1;i<=row*col;i++){
                *dum++ = *real++;
                *dum++ = *imag++;
        }
        return(0);
}



void f_filter(a,b1,b2,fim,sim,index,length,inp,outp)
/*---------                                        */
/*  f_filter :  Recursive filter approximation to optimal
                Canny filter.:  
                     f(x) = -c * EXP(-alfa * x)* SIN(omega * x)
                For more info. see art. :
                        Using Canny's criteria to derive a recusively
                        implem. optimal edge detektor.
                        -Rachid Deriche.
                        -Int.J.of comp.vision 167-187 1987.
*/
/* Usage:
        a..b2 are filter constants.
        fim <first index maximum> the loop outer maximum or number of
        arrays to filter minus one. 
        sim <sec. index maximum> the inner loop maximum or the length 
        of the arrays to filter minus one.
        index is pointer to INDEX function defined elsewhere.
        length is a help parameter for function INDEX.
        inp,outp are pointers to input and output buffers
*/
int     fim,sim,length;
float   *inp,*outp;
int     (* index)();
double  a,b1,b2;
{
        
        int     fi,si;
        double  *y_plus,*y_min;
        double  i,plus1_old,plus2_old,min1_old,min2_old;

        y_plus = (double *)halloc(sim+1,sizeof(double));
        y_min  = (double *)halloc(sim+1,sizeof(double));
        for(fi=0;fi<=fim;fi++){
                if (1+b1+b2==0){
                        /* Will this ever happen ?? */
                        plus1_old = 0;
                        min1_old = 0;
                }
                else{
                        plus1_old = inp[index(fi,0,length)]/(1+b1+b2);
                        min1_old  = inp[index(fi,sim,length)]/(1+b1+b2);
                }
                plus2_old = plus1_old;
                min2_old  = min1_old;
                for(si=0;si<=sim;si++){
                        i = (si == 0)   ? (double)inp[index(fi,0,length)] 
                                        : (double)inp[index(fi,si-1,length)];
                        y_plus[si] = i - b1*plus1_old - b2*plus2_old;
                        plus2_old = plus1_old;
                        plus1_old = y_plus[si];
                        i = (si == 0) ? (double)inp[index(fi,sim,length)] 
                                      : (double)inp[index(fi,sim-si+1,length)];
                        y_min[sim-si] = i - b1*min1_old - b2*min2_old;
                        min2_old = min1_old;
                        min1_old = y_min[sim-si];
                }
                for(si=0;si<=sim;si++)
                    outp[index(fi,si,length)] =(float)(a * (y_plus[si] - y_min[si]));
        }
        free(y_min);
        free(y_plus);
}

void h_filter2(a0,a1,a2,a3,b1,b2,fim,sim,index,length,inp,outp)
/*---------                                        */
/* h_filter2 :   Recursive smoothing filter approximation to optimal
                Canny filter.:  
                        h(x) = INTEGRAL(f(x),dx)
                For more info. see art. :
                        Using Canny's criteria to derive a recusively
                        implem. optimal edge detektor.
                        -Rachid Deriche.
                        -Int.J.of comp.vision 167-187 1987.
*/
/* Usage:
        a0..b2 are filter constants.
        fim <first index maximum> the loop outer maximum or number of
        arrays to filter minus one. 
        sim <sec. index maximum> the inner loop maximum or the length 
        of the arrays to filter minus one.
        index is pointer to INDEX function defined elsewhere.
        length is a help parameter for function INDEX 
        inp,outp are pointers to input and output buffers
*/
double  a0,a1,a2,a3,b1,b2;
int     fim,sim,(* index)(),length;
float   *inp,*outp;
{
        double  i,ii,plus1_old,plus2_old,min1_old,min2_old;
        double  *h_plus,*h_min;
        int     fi,si;

        h_plus = (double *) halloc(sim+1,sizeof(double));
        h_min  = (double *) halloc(sim+1,sizeof(double));
        for(fi=0;fi<=fim;fi++){
                if (1+b1+b2==0){
                        plus1_old = 0;
                        min1_old  = 0;
                }
                else{
                        plus1_old = (a0+a1)*inp[index(fi,0,length)]/(1+b1+b2);
                        min1_old  = (a0+a1)*inp[index(fi,sim,length)]/(1+b1+b2);
                }
                plus2_old = plus1_old;
                min2_old  = min1_old;
                for(si=0;si<=sim;si++){
                        i = (si == 0) ? (double)inp[index(fi,0,length)] 
                                        : (double)inp[index(fi,si-1,length)];
                        h_plus[si]  = a0*inp[index(fi,si,length)] + a1*i 
                                        - b1*plus1_old - b2*plus2_old;
                        plus2_old   = plus1_old;
                        plus1_old   = h_plus[si];
                        i  = (si == 0)  ? inp[index(fi,sim,length)] 
                                        : inp[index(fi,sim-si+1,length)];
                        ii = (si < 2)   ? i 
                                        : inp[index(fi,sim-si+2,length)]; 
                        h_min[sim-si]  = a2*i + a3*ii 
                                        - b1*min1_old - b2*min2_old;
                        min2_old = min1_old;
                        min1_old = h_min[sim-si];
                }
                for(si=0;si<=sim;si++)
                        outp[index(fi,si,length)] = (float)(h_plus[si] - h_min[si]);
        }
        free(h_min);
        free(h_plus);
}

/* help functions for f_filter and h_filter2 to calc. indexes in 
a matrix 
*/

int     hor_indx(i,j,length)
int     i,j,length;
{       
        int     dummy;
        dummy = i*length + j;
        return (dummy);
}

int     ver_indx(i,j,length)
int     i,j,length;
{
        int     dummy;
        dummy = i + j*length;
        return(dummy);
}
