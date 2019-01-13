/*	Copyright (c) 1993 Michael Grunkin, IMSOR 

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 *  hole_fill.c - Filling small holes embedded in holes.
 *                The filling takes place iff the holes are 
 *                smaller than a given limit and the excentricity 
 *                below (another) given limit.
 *
 *  Usage: hole_fill [-ls limit] [-le limit] <inseq >outseq
 *
 *  [-ls] option sets the limit size (in pixels) for the holes that
 *        should be filles.
 *
 *  [-le] option sets the limit excentricity (in float) for the
 *        holes that should be filled
 *
 *  to load: cc -o hole_fill hole_fill.c -lhips 
 *
 *  Input sequence must be in byte format.
 */

#define  Byte     unsigned char

#include <hipl_format.h>
#include <stdio.h>

#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <math.h>

int              nrows, ncols, rr, cc, current_hole, 
                 size_limit = 75, size_count;
struct header    hd, hdf;	
Byte            *ipic, *opic;
float            ex_limit = 0.0;

struct point {
   int           r, c;
   struct point *next;
};



#define   IPIX(R,C)       *(ipic + (R)*ncols + (C))
#define   OPIX(R,C)       *(opic + (R)*ncols + (C))


float excentric(hlist)
struct point  *hlist;
{
struct point  *help;
float          mr, mc, sr, sc, src, l1, l2;

mr = mc = 0.0;
help = hlist;
while(help != NULL){
   mr  += (float)(help->r);
   mc  += (float)(help->c);
   help = help->next;
}
mr  /= (float) size_count;
mc  /= (float) size_count;
sr = sc = src = 0.0;
help = hlist;
while(help != NULL){
   sr  += (float)(help->r - mr)*(help->r - mr);
   sc  += (float)(help->c - mc)*(help->c - mc);
   src += (float)(help->r - mr)*(help->c - mc);
   help = help->next;
}
sr  /= (float) size_count;
sc  /= (float) size_count;
src /= (float) size_count;
l1 = sr + sc + sqrt((sr - sc)*(sr - sc) + 4.0*src*src);
l2 = sr + sc - sqrt((sr - sc)*(sr - sc) + 4.0*src*src);
if((l1 == 0.0) && (l2 == 0)) return(1.0);
else                         return(l2/l1);
}


void rem_hole(ftype, hlist)
int            ftype;
struct point  *hlist;
{
  int           r, c, filled=0, nlegal, sum;
  struct point *help;

  help = hlist; 
  while(help != NULL) {
     OPIX(help->r,help->c) = ftype;
     help = help->next;
  }
}



int ok_pix(r,c)
int  r, c;
{ 
  if((r>0) && (c>0) && (r < nrows) && (c < ncols)){
    if(IPIX(r,c) == 0)                                       return(1);
    else                                                     return(0);
  }
  else return(0);
}


struct point *mark_hole(r,c, hlist)
int           r,c;
struct point *hlist;
{
  struct point  *help;
  int            dummy; 
  
  if(ok_pix(r,c)){
     if((r==52) && (c==118)){
	dummy = 0;
     }
     help = (struct point *)malloc(sizeof(struct point));
     help->r     = r;
     help->c     = c;
     help->next  = hlist;
     hlist       = help;
     ++size_count;
     IPIX(r,c) = 255;
     OPIX(r,c) = 6;
     hlist = mark_hole(r+1,c,hlist);
     hlist = mark_hole(r-1,c,hlist);
     hlist = mark_hole(r,c+1,hlist);
     hlist = mark_hole(r,c-1,hlist);
     /*
     hlist = mark_hole(r-1,c+1,hlist);
     hlist = mark_hole(r+1,c-1,hlist);
     hlist = mark_hole(r-1,c-1,hlist);
     hlist = mark_hole(r+1,c+1,hlist);
     */
  }
  return(hlist);
}

int main(argc,argv)
int argc;
char **argv;
{
	int            i,r,c,k,npix,nf;
        struct point   *hlist;
	float          exc;

	Progname = strsave(*argv);
       
	
	for(i=1;i<argc;i++){
	   if (strncmp(argv[i],"-ls",3)==0) {
	      size_limit = atoi(argv[++i]);
	      continue;
	   }
           if (strncmp(argv[i],"-le",3)==0) {
	      ex_limit = atof(argv[++i]);
	      continue;
	   }
	}
	
	read_header(&hd);
	if (hd.pixel_format != PFBYTE) 
	   perr(HE_MSG,"image must be in byte format");

	update_header(&hd,argc,argv);
	write_header(&hd);
	nrows = hd.orows;
	ncols = hd.ocols;
	npix = nrows*ncols;



	ipic = (Byte *) halloc(npix,sizeof(Byte));
	opic = (Byte *) halloc(npix,sizeof(Byte));
	hd.image=ipic;

	read_image(&hd,0);

	
        current_hole = 1;
        for(r=0;r<nrows;++r) {
           for(c=0;c<ncols;++c) {
		 if(ok_pix(r,c)) {
		   size_count = 0; hlist = NULL;
		   hlist = mark_hole(r,c,hlist);
		   if(size_count < size_limit) {
		     exc = excentric(hlist);
		     fprintf(stderr,"%d  %e\n",size_count,exc);
		     if ((exc > 0.0 ) && (exc < ex_limit)) rem_hole(0,hlist);
		     else                                  rem_hole(1,hlist);
		   }
		   else rem_hole(0,hlist);
		 }
		 else if((IPIX(r,c) != 255) && IPIX(r,c) != 0) 
		      OPIX(r,c) = IPIX(r,c);
	    }
	}             
	for(r=0;r<nrows;++r) for(c=0;c<ncols;++c) {
	   if(OPIX(r,c) == 6) OPIX(r,c) = 0;
	}
	hd.image=opic;
	write_image(&hd,0);
	
}
