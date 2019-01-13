/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * dispwbasis.c - Display the basis for Walsh transform for a 16X16 array
 *
 * usage:	dispwbasis
 *
 * The displayed basis arrays are reordered according to sequency.
 *
 * to load:	cc -o dispwbasis dispwbasis.c -lhipsh -lhips -lm
 *
 * Y.Cohen 2/24/82
 * HIPS 2 - msl - 8/11/91
 */

#include <hipl_format.h>

int walshindex();

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd;
	int i,j,k,l,ir,ic;
	int a[256];
	byte *v;
	Pixelval p;

	Progname = strsave(*argv);
	init_header(&hd,"","",1,"",328,328,PFBYTE,1,"");
	write_headeru(&hd,argc,argv);
	alloc_image(&hd);
	v = hd.image;
	p.v_byte = 127;
	h_setimage(&hd,&p);
	for(i=0;i<16;i++)
		for(j=0;j<16;j++) {
			for(k=0;k<256;k++)
				a[k]=0;
			a[i*16+j] = 1;
			h_fwt_i(a,8);
			ir = 20*walshindex(4,i)+4;
			ic = 20*walshindex(4,j)+4;
			for(k=0;k<16;k++)
			    for(l=0;l<16;l++)
				v[(ir+k)*328+ic+l] = a[l*16+k]&0377;
		}
	write_image(&hd,0);
	return(0);
}

/*
 * Given an index coefficient in a 2**loglen vector,
 * returns the index in the ordered vector
 */

int walshindex(loglen,index)

int loglen,index;

{
	int l,l2,l4;

	if (loglen<=1)
		return(index);
	l=1<<loglen;
	l2=l>>1;
	l4=l>>2;
	if (index<l2)
		return(walshindex(loglen-1,index));
	index-=l2;
	return(walshindex(loglen-1,index<l4?index+l4:index-l4)+l2);
}
