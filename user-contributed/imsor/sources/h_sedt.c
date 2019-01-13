/***************************************************************
*  File       :   h_sedt.c			Date: 20/03-92
*  Author     :   Ulrik P. V. Skands 
*  Purpose    :
*  Description:   Signed Euclidean Distance Transform 
*  Features   :
*  Options    :
***************************************************************/
#include <stdio.h>
#include <math.h>		/* for abs() */
#include <hipl_format.h>

#ifndef HNOVALUESH
#include <values.h>
#else
#define MAXINT  (0x7fffffff)
#endif

#define	 SQR(X)		(X)*(X)
#define	 MINIMUM(A,B)	((A)<(B))?(A):(B)


int h_sedt(hdi, hdo, d1, d2)

struct header	*hdi, *hdo;
double 		d1, d2;

{
   register	int		r, c, k, kmin, nrows, ncols, n2cols;
   register	float		lmin;
   float	rev[5], imv[5], l[5];
   register	unsigned char	*ifr, *ip;
   register	float		*ofr, *op;

   ifr=(unsigned char *)hdi->firstpix;
   ofr=(float *)hdo->firstpix;
   nrows=hdi->orows;
   ncols=hdi->ocols;
   n2cols=2*ncols;

     for(ip=ifr, op=ofr, r=0;r<nrows;r++)
       for(c=0;c<ncols;c++, op++, ip++) {
	 if(*ip) {
	   *op=MAXINT;
	   *(++op)=MAXINT;
	   }
	 else {
	   *op=0;
	   *(++op)=0;
	   }
       }

     /* Forward Superpass */
     for(op=ofr, r=0;r<nrows;r++){
       op=&op[n2cols-2];
       for(c=(ncols-1);c>=0;c--, op-=2)
         if(*op || op[1]){
           if((r==0)||(c==0)) rev[0]=imv[0]=MAXINT;
           else{
             rev[0]=op[-n2cols-2]-d1;
             imv[0]=op[-n2cols-1]+d2;
             }
           if(r==0) rev[1]=imv[1]=MAXINT;
           else{
             rev[1]=op[-n2cols];
             imv[1]=op[-n2cols+1]+d2;
             }
           if((r==0)||(c==(ncols-1))) rev[2]=imv[2]=MAXINT;
           else{
             rev[2]=op[-n2cols+2]+d1;
             imv[2]=op[-n2cols+3]+d2;
             }
           if(c==(ncols-1)) rev[3]=imv[3]=MAXINT;
           else{
             rev[3]=op[2]+d1;
             imv[3]=op[3];
             }
           rev[4]= *op;
           imv[4]=op[1];

           lmin=MAXINT;
           kmin=0;
           for(k=0;k<5;k++){
             l[k]=SQR(rev[k])+SQR(imv[k]);
             if(l[k]<lmin){
               lmin=l[k];
               kmin=k;
               }
             }
           *op=rev[kmin];
           op[1]=imv[kmin];
         }
       op+=2;
       for(c=0;c<ncols;c++,op+=2)
         if(*op || op[1]){
           if(c==0) rev[0]=imv[0]=MAXINT;
           else{
             rev[0]=op[-2]-d1;
             imv[0]=op[-1];
             }
           rev[1]= *op;
           imv[1]= op[1];

           lmin=MAXINT;
           kmin=0;
           for(k=0;k<2;k++){
             l[k]=SQR(rev[k])+SQR(imv[k]);
             if(l[k]<lmin){
               lmin=l[k];
               kmin=k;
               }
             }
           *op=rev[kmin];
           op[1]=imv[kmin];
         }
     } 

     /* Backward Superpass */
     for(r=(nrows-1);r>=0;r--){
       op=&op[-n2cols];
       for(c=0;c<ncols;c++,op+=2)
         if(*op || op[1]) {
           if(c==0) rev[0]=imv[0]=MAXINT;
           else{
             rev[0]=op[-2]-d1;
             imv[0]=op[-1];
             }
           if((r==(nrows-1))||(c==0)) rev[1]=imv[1]=MAXINT;
           else{
             rev[1]=op[n2cols-2]-d1;
             imv[1]=op[n2cols-1]-d2;
             }
           if(r==(nrows-1)) rev[2]=imv[2]=MAXINT;
           else{
             rev[2]=op[n2cols];
             imv[2]=op[n2cols+1]-d2;
             }
           if((r==(nrows-1))||(c==(ncols-1))) rev[3]=imv[3]=MAXINT;
           else{
             rev[3]=op[n2cols+2]+d1;
             imv[3]=op[n2cols+3]-d2;
             }
           rev[4]= *op;
           imv[4]= op[1];

           lmin=MAXINT;
           kmin=0;
           for(k=0;k<5;k++){
             l[k]=SQR(rev[k])+SQR(imv[k]);
             if(l[k]<lmin){
               lmin=l[k];
               kmin=k;
               }
             }
           *op=rev[kmin];
           op[1]=imv[kmin];
         }
       op-=2;
       for(c=(ncols-1);c>=0;c--,op-=2)
         if(*op || op[1]){
           if(c==(ncols-1)) rev[0]=imv[0]=MAXINT;
           else{
             rev[0]=op[2]+d1;
             imv[0]=op[3];
             }
           rev[1]= *op;
           imv[1]=op[1];

           lmin=MAXINT;
           kmin=0;
           for(k=0;k<2;k++){
             l[k]=SQR(rev[k])+SQR(imv[k]);
             if(l[k]<lmin){
               lmin=l[k];
               kmin=k;
               }
             }
           *op=rev[kmin];
           op[1]=imv[kmin];
         }
       op+=2;
     }
  return(HIPS_OK);
}

