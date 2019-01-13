/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * halloc.c - HIPL Picture Format core allocation.
 *
 * Michael Landy 1/3/91
 */

#include <hipl_format.h>

byte *halloc(i,j)

int i,j;

{
	byte *k;

	k = (byte *) calloc(i,j);
	if (k == (byte *) 0)
		return((byte *) perr(HE_ALLOC));
	return(k);
}

byte *hmalloc(i)

hsize_t i;

{
	byte *k;

	k = (byte *) malloc(i);
	if (k == (byte *) 0)
		return((byte *) perr(HE_ALLOC));
	return(k);
}

char *memalloc(nelem,elsize)

int nelem;
hsize_t elsize;

{
	char *address = (char *) malloc((unsigned)(nelem*elsize));

	if (address == (char *) 0)
		return((char *) perr(HE_ALLOC));
	return(address);
}
