/*	Copyright (c) 1993 Michael Grunkin, IMSOR 

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 *  cberode.c - This program performs a binary erode operation
 *  with coefficient -c (see below). This means that a 
 *  white center pixel of a 3x3 mask is switched to zero if more
 *  than c pixels are black. c=0 is classical erosion.
 
 *  Usage: cberode [-c coefficient] [-flip c1 c2] [-t times] < inseq >outseq
 *
 *  [-c]    Coefficient as descrobed above.
 *  [-flip] Can only be used if times>1. If this is the case 
 *          the erosion switches between coefficient values of 
 *          c1 and c2.
 *  [-t]    Number of times to erode.
 *
 *  to load: cc -o cberode cberode.c -lm -lhips
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
int              zeroes, etyp;
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
  
   

int main(argc,argv)
int argc;
char **argv;
{
	int             i,r,c,k,npix,nf, msize=3;
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
	   fprintf(stderr,"Erosion no. %d coeff. %d\n",k,cc);
	   for(r=0;r<nrows;++r){
  	     for(c=0;c<ncols;++c){
	        if(IPIX(r,c) > 0){
		   run = mask; zeroes = 0;
		   while(run!=NULL){
		      if(IPIX(MIRROR(nrows,run->r+r),MIRROR(ncols,run->c+c)) == 0)
		      ++zeroes;
		      run=run->next;
		   }
		   if(zeroes > cc) {
		      OPIX(r,c) = 0;
		   }
		   else {
		      OPIX(r,c) = IPIX(r,c);
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
