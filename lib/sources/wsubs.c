/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * wsubs.c - HIPS image header write header subroutines
 *
 * Michael Landy - 2/1/82
 * modified to use read/write - 4/26/82
 * modified to return #chars - msl - 9/21/90
 * modified for HIPS 2 - msl - 1/3/91
 */

#include <stdio.h>
#include <hipl_format.h>

int wnocr(fp,s)

char *s;
FILE *fp;

{
	char *t;
	int i;

	t = s;
	i = 0;
	while (*t != '\n' && *t != '\0') {
		putc(*t++,fp);
		i++;
	}
	putc('\n',fp);
	return(i+1);
}

int dfprintf(fp,i,fname)

FILE *fp;
int i;
Filename fname;

{
	char s[30];
	int j;

	sprintf(s,"%d\n",i);
	j = strlen(s);
	if (fwrite(s,j,1,fp) != 1)
		return(perr(HE_HDRWRT,fname));
	return(j);
}
