/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * read_image.c - read an image frame
 *
 * Michael Landy - 1/4/91
 */

#include <stdio.h>
#include <hipl_format.h>

int read_image(hd,fr)

struct header *hd;
int fr;

{
	return(fread_image(stdin,hd,fr,"<stdin>"));
}

int fread_image(fp,hd,fr,fname)

FILE *fp;
struct header *hd;
int fr;
Filename fname;

{
	if (fread(hd->image,hd->sizeimage,1,fp) != 1)
		return(perr(HE_READFRFILE,fr,fname));
	return(HIPS_OK);
}
