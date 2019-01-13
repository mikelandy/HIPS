/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * cmapio.c - HIPS colormap file I/O
 *
 * The file format is:
 *
 *	number-of-entries
 *	r(0) g(0) b(0)
 *	r(1) g(1) b(1)
 *		.
 *		.
 *		.
 *	r(n-1) g(n-1) b(n-1)
 *
 * Michael Landy - 8/16/91
 */

#include <stdio.h>
#include <hipl_format.h>

int readcmap(filename,maxcount,count,r,g,b)

Filename filename;
int maxcount,*count;
byte *r,*g,*b;

{
	FILE *stream;
	int i,rr,gg,bb;

	if ((stream=fopen(filename,"r")) == (FILE *) NULL)
		return(perr(HE_OPEN,filename));
	if (fscanf(stream,"%d",count) < 1)
		return(perr(HE_READFILE,filename));
	if (*count > maxcount)
		return(perr(HE_COLOVF,"readcmap",filename));
	for (i=0;i<*count;i++) {
		if (fscanf(stream,"%d %d %d",&rr,&gg,&bb) < 3)
			return(perr(HE_READFILE,filename));
		r[i] = rr;
		g[i] = gg;
		b[i] = bb;
	}
	fclose(stream);
	return(HIPS_OK);
}
