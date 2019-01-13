/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 CHANGES
 
 rld - added include of stdlib.h for malloc
 */

#include <stdlib.h>
#include <hipl_format.h>
#include <string.h>

extern char *memalloc();

char *strsave(s)

char *s;

{
	char *news;
	if ((news = memalloc(strlen(s)+1, sizeof(char))) == (char *) HIPS_ERROR)
		return((char *) HIPS_ERROR);
	return(strcpy(news,s));
}
