static char *SccsId = "%W%      %G%";

/*      Copyright (c) 1991 The Turing Institute

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 * greylevel - generate an image with grey-levels 0 through 255
 *
 * usage:	greylevel
 *
 * to load:	cc -o greylevel greylevel.c -lhips
 *
 * Jin Zhengping -1/7/85 
 * Rewritten by Jin Zhengping - 31 August 1991
 */

#include <stdio.h>
#include <hipl_format.h>
#define	N	512 
int types[] = {PFBYTE,LASTTYPE};

int main(argc,argv)

int     argc;
char    **argv;

{
	struct          header hd;
	int		i,j,k,count=0;

	Progname = strsave(*argv);
	init_header(&hd,"","graylevels",1,
		"",N,N,PFBYTE,1,"graylevel from 0 through 255.");
	alloc_image(&hd);
	write_headeru(&hd,argc,argv);

	for(i=0; i<N; i+=4,count++)
		for(j=0; j<4; j++)
			for(k=0; k<N/2; k++)
				hd.image[k*N+i+j] = count;
	for(i=0; i<N; i+=4,count++)
		for(j=0; j<4; j++)
			for(k=N/2; k<N; k++)
				hd.image[k*N+i+j] = count;
	write_image(&hd);
	return(0) ;
}
