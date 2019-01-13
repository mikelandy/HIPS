/*	Copyright (c) 1993 Michael Grunkin, IMSOR 

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 *  isolate.c - This program performs a binary erode operation
 *  with coefficient -c (see below). This means that a 
 *  white center pixel of a 3x3 mask is switched to zero if more
 *  than c pixels are black. c=0 is classical erosion.
 *  The erosion is continued until only isolated points are left in the 
 *  scene.
 *
 *  Usage: isolate [-c coefficient] [-flip c1 c2]  < inseq >outseq
 *
 *  [-c]    Coefficient as descrobed above.
 *  [-flip] If this switch is the used the erosion switches between
 *          coefficient values of c1 and c2.
 *  [-el]   Eliminate spots that emerges up till el iterations.
 *
 *  to load: cc -o isolate isolate.c -lm -lhips
 *
 */

#define  Byte     unsigned char

#include <hipl_format.h>
#include <stdio.h>

#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <math.h>


#define   OUTSIDE(N,x) (((x)<0) ? 1 : (((x)>=N) ? 1 : (0))) 

int              nrows, ncols, rr, cc, c1, c2, times, flip;
int              pixsum, etyp, object_size, e_objects, eliminate=0;
struct header    hd, hdf;	
Byte             *ipic, *opic;

struct pixel {
		   int                   r, c, pix;
		   struct pixel          *next;
          };

struct pixel   *mask, *object;


#define   IPIX(R,C)         *(ipic + (R)*ncols + (C))
#define   OPIX(R,C)         *(opic + (R)*ncols + (C))


int in_object(r,c,object)
int            r, c;
struct pixel  *object;
{
   struct pixel *run;
   int           found=0;
   
   run=object;
   while((run!=NULL)&&(!found)){
      if((run->r == r) && (run->c == c)) ++found;
      run=run->next;
   }
   return(found);
}

int still_exist(cc,object,mask)
int           cc;
struct pixel *object, *mask;
{
   struct pixel *o_run, *m_run;
   int           se=0, pixsum;
   
   o_run = object;
   while((o_run != NULL) && (!se)){
      m_run = mask; pixsum=0;
      while(m_run!=NULL){
	 if(!in_object(o_run->r+m_run->r,o_run->c+m_run->c,object)) ++pixsum;
	 m_run = m_run->next;
      }
      if(pixsum <= cc) ++se;
      o_run = o_run -> next;
   }
   return(se);
}
       
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
   ++object_size;
   return(line);
}


struct pixel *get_object(r,c,object)
int             r, c;
struct pixel   *object;
{
   if((r>0) && (c>0) && (r < nrows) && (c < ncols)){
     object =insert(r,c,object); 
     OPIX(r,c) = 0;
     if(OPIX(r+1,c) > 0) object = get_object(r+1,c,object);
     if(OPIX(r-1,c) > 0) object = get_object(r-1,c,object);
     if(OPIX(r,c-1) > 0) object = get_object(r,c-1,object);
     if(OPIX(r,c+1) > 0) object = get_object(r,c+1,object);
     if(OPIX(r+1,c+1) > 0) object = get_object(r+1,c+1,object);
     if(OPIX(r+1,c-1) > 0) object = get_object(r+1,c-1,object);
     if(OPIX(r-1,c+1) > 0) object = get_object(r-1,c+1,object);
     if(OPIX(r-1,c-1) > 0) object = get_object(r-1,c-1,object);
   }
   return(object);
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
	int             i,r,c,k,npix,nf,msize=3, mass_r, mass_c;
        struct pixel   *run;
	
	Progname = strsave(*argv);
       
	
	for(i=1;i<argc;i++){
           if (strncmp(argv[i],"-c",2)==0) {
	      c1 = atoi(argv[++i]);
	      continue;
	   }
	   if (strncmp(argv[i],"-flip",5)==0) {
	      c1 = atoi(argv[++i]);
	      c2 = atoi(argv[++i]);
	      continue;
	   }
	   if (strncmp(argv[i],"-t",2)==0) {
	      times = atoi(argv[++i]);
	      continue;
	   }
	    if (strncmp(argv[i],"-el",3)==0) {
	      eliminate = atoi(argv[++i]);
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
	
	for(r=0;r<nrows;++r) for(c=0;c<ncols;++c) OPIX(r,c) = 0;
	mask = NULL;
	mask = gen_mask(msize,mask);  etyp = 0;
	for(k=1;k<=times;++k){
	   if(k==25){
	     k=25;
	   }
	   if(etyp == 0) cc = c1;
	   else          cc = c2;
	   e_objects = 0;
	   fprintf(stderr,"%d ",k);
	   for(r=0;r<nrows;++r){
  	     for(c=0;c<ncols;++c){
	        if(IPIX(r,c) > 0){
		   run = mask; pixsum = 0;
		   while(run!=NULL){
		      if((OUTSIDE(nrows,run->r+r) || OUTSIDE(ncols,run->c+c))) ++pixsum;
		      else {
		         if(IPIX(run->r+r,run->c+c) == 0) ++pixsum;
		      }
		      run=run->next;
		   }
		   if(pixsum > cc) {
		      if(pixsum == (msize*msize-1)) OPIX(r,c) = IPIX(r,c);
		      else                          OPIX(r,c) = 0;
		   }
		   else                             OPIX(r,c) = IPIX(r,c);
	        }
	        else OPIX(r,c) = IPIX(r,c);
	     }
	   }
	  for(r=0;r<nrows;++r) for(c=0;c<ncols;++c) IPIX(r,c) = 0;
	  for(r=0;r<nrows;++r){
	     for(c=0;c<ncols;++c){
		if(OPIX(r,c) > 0){
		   object_size = 0; object = NULL;
		   object = get_object(r,c,object);
		   if((!still_exist(c1,object,mask))&& (object_size > 1 )){
		      run = object; mass_r = 0; mass_c = 0;
		      while(run!=NULL){
		        mass_r += run->r;
			mass_c += run->c;
			run = run->next;
		      }
		      mass_r = (int)(mass_r/object_size + 0.5);
		      mass_c = (int)(mass_c/object_size + 0.5);
		      if (k>eliminate) IPIX(mass_r,mass_c) = 255;
		      ++e_objects;
		   }
		   else {
		     run = object;
		     while(run!=NULL){
			IPIX(run->r,run->c) = 255;
			run = run->next;
		     }
		   }
		}
	     }
	  }
	  etyp = !etyp;
	  fprintf(stderr,"%d\n",e_objects);
	}
	hd.image=ipic;
	write_image(&hd,0);
	
}
