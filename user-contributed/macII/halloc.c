/*	Copyright (c) 1982 Michael Landy, Yoav Cohen, and George Sperling

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 * halloc.c - HIPL Picture Format core allocation.
 *
 * Michael Landy 2/1/82
 */

#include "hipl_format.h"

char *halloc(i,j)

int i,j;

{
	char *k;

	k = (char *) calloc(i,j);
	if(k == NULL) {
		fprintf(stderr,"Not enough core available.\n");
		exit(1);
	}
	return(k);
}
