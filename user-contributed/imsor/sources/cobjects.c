/*	Copyright (c) 1993 Michael Grunkin, IMSOR 

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 *  cobjects.c -  Counting separate objects, giving each of these a
 *                unique (integer) label. All objects less than a given
 *                size ls (see below) is removed.
 *
 *  Usage: cobjects [-ls limit]  <inseq >outseq
 *
 *  [-ls] option sets the limit size (in pixels) for the holes that
 *        should be filles.
 *
 *  to load: cc -o cobjects cobjects.c -lhips 
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

int              nrows, ncols, rr, cc, current_object, 
                 size_limit =10, size_count;
struct header    hd, hdf;	
Byte            *ipic, *opic;




#define   IPIX(R,C)       *(ipic + (R)*ncols + (C))
#define   OPIX(R,C)       *(opic + (R)*ncols + (C))





void rem_object(col)
int col;
{
  int   r,c;

  for(r=0;r<nrows;++r) for(c=0;c<ncols;++c) {
    if(OPIX(r,c)==col) OPIX(r,c) = 0;
  }
}

int ok_pix(r,c)
int  r, c;
{ 
  if((r < nrows) && (c < ncols)){
    if(((IPIX(r,c)>=1) && (IPIX(r,c)<=5))) return(1);
    else                                                     return(0);
  }
  else return(0);
}

void mark_object(current_object,r,c)
int current_object,r,c;
{
  if(ok_pix(r,c)){
     ++size_count;
     IPIX(r,c) = 0;
     OPIX(r,c) = current_object;
     mark_object(current_object,r+1,c);
     mark_object(current_object,r-1,c);
     mark_object(current_object,r,c+1);
     mark_object(current_object,r,c-1);
     mark_object(current_object,r-1,c+1);
     mark_object(current_object,r+1,c-1);
     mark_object(current_object,r+1,c+1);
     mark_object(current_object,r-1,c-1);
  }
}

int main(argc,argv)
int argc;
char **argv;
{
	int            i,r,c,k,npix,nf;
        struct point   *hlist;

	Progname = strsave(*argv);
       
	read_header(&hd);
	if (hd.pixel_format != PFBYTE) 
	   perr(HE_MSG,"image must be in byte format");

	for(i=1;i<argc;i++){
	   if (strncmp(argv[i],"-ls",3)==0) {
	      size_limit = atoi(argv[++i]);
	      continue;
	   }
	}
	
	
	update_header(&hd,argc,argv);
	write_header(&hd);
	nrows = hd.orows;
	ncols = hd.ocols;
	npix = nrows*ncols;



	ipic = (Byte *) halloc(npix,sizeof(Byte));
	opic = (Byte *) halloc(npix,sizeof(Byte));
	hd.image=ipic;

	read_image(&hd,0);

	
        current_object = 1;
        for(r=0;r<nrows;++r) {
           for(c=0;c<ncols;++c) {
		 if(ok_pix(r,c)) {
		   size_count = 0; rr=r; cc=c;
		   mark_object(current_object,r,c);
		   if(size_count > size_limit) {
		     fprintf(stderr,"%d  %d\n",current_object,size_count);
		     ++current_object;
		    }
		   else rem_object(current_object);
		 }
	    }
	}             
	hd.image=opic;
	write_image(&hd,0);
	
}
