/*	Copyright (c) 1993 Michael Grunkin, IMSOR 

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 *  cbdilate.c - This program performs a binary dilate operation
 *  with coefficient -c (see below). This means that a 
 *  black center pixel of a 3x3 mask is switched to white if more
 *  than c pixels are white. c=0 is classical dilation.
 
 *  Usage: cbdilate [-c coefficient] [-flip c1 c2] [-t times] < inseq >outseq
 *
 *  [-c]    Coefficient as described above.
 *  [-flip] Can only be used if times>1. If this is the case 
 *          the dilation switches between coefficient values of 
 *          c1 and c2.
 *  [-t]    Number of times to dilate.
 *
 *  to load: cc -o cbdilate cbdilate.c -lm -lhips
 *
 */

#define  Byte     unsigned char

#include <hipl_format.h>
#include <stdio.h>

#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <math.h>


#define   MIRROR(N,x) (((x)<0) ? 0 : (((x)>=N) ? N-1 : (x))) 

int              nrows, ncols, rr, cc, c1, c2, ttimes=1, flip;
int              ones, etyp;
struct header    hd, hdf;	
Byte             *ipic, *opic;

struct pixel {
		   int                   r, c, pix;
		   struct pixel          *next;
          };

struct pixel   *mask;


#define   IPIX(R,C)         *(ipic + (R)*ncols + (C))
#define   OPIX(R,C)         *(opic + (R)*ncols + (C))

struct pixel *insert(r,c,line)
int              r, c;
struct pixel  *line;
{
struct pixel           *help, *browse;



   help = (struct pixel *)malloc(sizeof(*help));
   help -> r    = r;
   help -> c    = c;
   help -> next = line;
   line         = help;
   
   return(line);
}


struct pixel *gen_mask(msize,mask)
int            msize;
struct pixel  *mask;
{
   int   r, c, l2;
   
   l2 = (int)((msize-1)/2.0 + 0.5);
   for(r=-l2;r<=l2;++r) for(c=-l2;c<=l2;++c) {
      if((r!=0)||(c!=0)) mask = insert(r,c,mask);
   }
   return(mask);
}
  
   
int ok_neighbors(r,c,color)
int  r, c, color;
{
   int  ok = 1;
   
   if((IPIX(r,c+1)>0) && (IPIX(r,c+1)!=color))     ok = 0;
   if((IPIX(r,c-1)>0) && (IPIX(r,c-1)!=color))     ok = 0;
   if((IPIX(r+1,c)>0) && (IPIX(r+1,c)!=color))     ok = 0;
   if((IPIX(r-1,c)>0) && (IPIX(r-1,c)!=color))     ok = 0;
   
   if((IPIX(r+1,c+1)>0) && (IPIX(r+1,c+1)!=color)) ok = 0;
   if((IPIX(r-1,c-1)>0) && (IPIX(r-1,c-1)!=color)) ok = 0;
   if((IPIX(r+1,c-1)>0) && (IPIX(r+1,c-1)!=color)) ok = 0;
   if((IPIX(r-1,c+1)>0) && (IPIX(r-1,c+1)!=color)) ok = 0;
   
   if((OPIX(r,c+1)>0) && (OPIX(r,c+1)!=color))     ok = 0;
   if((OPIX(r,c-1)>0) && (OPIX(r,c-1)!=color))     ok = 0;
   if((OPIX(r+1,c)>0) && (OPIX(r+1,c)!=color))     ok = 0;
   if((OPIX(r-1,c)>0) && (OPIX(r-1,c)!=color))     ok = 0;
   
   if((OPIX(r+1,c+1)>0) && (OPIX(r+1,c+1)!=color)) ok = 0;
   if((OPIX(r-1,c-1)>0) && (OPIX(r-1,c-1)!=color)) ok = 0;
   if((OPIX(r+1,c-1)>0) && (OPIX(r+1,c-1)!=color)) ok = 0;
   if((OPIX(r-1,c+1)>0) && (OPIX(r-1,c+1)!=color)) ok = 0;
   
   return(ok);
}
   

int main(argc,argv)
int argc;
char **argv;
{
	int             i,r,c,k,npix,nf,msize=3;
	int             newcolor, oldcolor, stop;
	int             ridx, cidx;
        struct pixel   *run;
	
	Progname = strsave(*argv);
       
	
	for(i=1;i<argc;i++){
           if (strncmp(argv[i],"-c",2)==0) {
	      c1 = c2 = atoi(argv[++i]);
	      continue;
	   }
	   if (strncmp(argv[i],"-flip",5)==0) {
	      c1 = atoi(argv[++i]);
	      c2 = atoi(argv[++i]);
	      continue;
	   }
	   if (strncmp(argv[i],"-t",2)==0) {
	      ttimes = atoi(argv[++i]);
	      continue;
	   }
	   if (strncmp(argv[i],"-m",2)==0) {
	      msize = atoi(argv[++i]);
	      continue;
	   }
	}
	
	read_header(&hd);
	if (hd.pixel_format != PFBYTE) 
	   perr(HE_MSG,"image must be in byte format");
  
	nrows = hd.orows;
	ncols = hd.ocols;
	npix = nrows*ncols;

        ipic =       (Byte *) halloc(npix,sizeof(Byte));
	opic =       (Byte *) halloc(npix,sizeof(Byte));
	

	hd.image=ipic;
	read_image(&hd,0);
		
	update_header(&hd,argc,argv);
	write_header(&hd);
	
	for(r=0;r<nrows;++r) for(c=0;c<ncols;++c) {
	   OPIX(r,c) = 0;
	}
	mask = NULL;
	mask = gen_mask(msize,mask); etyp = 0;
	for(k=1;k<=ttimes;++k){
	   if(etyp == 0) cc = c1;
	   else          cc = c2;
	   fprintf(stderr,"Dilation no. %d coeff. %d\n",k,cc);
	   for(r=0;r<nrows;++r){
  	     for(c=0;c<ncols;++c){
	        if(IPIX(r,c) == 0){
		   run = mask; ones = 0; stop=0;
		   while((run!=NULL) && (!stop)){
		      ridx = MIRROR(nrows,run->r+r);
		      cidx = MIRROR(ncols,run->c+c);
		      if((newcolor = IPIX(ridx,cidx)) > 0) {	 
		         ++ones;
			 if(ones == 1) oldcolor = newcolor;
			 if((ones>1) && (newcolor == oldcolor)) oldcolor = newcolor;
			 if((ones>1) && (newcolor != oldcolor)){
			    ++stop;
			    ones = -99;
			 }
		      }
		      run=run->next;
		   }
		   if(ones > cc){
		      if(ok_neighbors(r,c,oldcolor)) OPIX(r,c) = oldcolor;
		      else {
			 OPIX(r,c) = 0;
		      }
		   }
		   else {
		      if(ones == 0)OPIX(r,c) = IPIX(r,c);
		      if(ones <  0)OPIX(r,c) = 0;
		   }
	        }
	        else OPIX(r,c) = IPIX(r,c);
	     }
	  }
	  for(r=0;r<nrows;++r) for(c=0;c<ncols;++c) IPIX(r,c) = OPIX(r,c);
	  etyp = !etyp;
	}
	
	hd.image=opic;
	write_image(&hd,0);
	
}
