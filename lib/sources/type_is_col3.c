/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * type_is_col3.c - check for 3-color format
 *
 * Michael Landy - 5/20/93
 */

#include <stdio.h>
#include <hipl_format.h>

h_boolean type_is_col3(hd)

struct header *hd;

{
	int pfmt;

	pfmt = hd->pixel_format;
	if (pfmt == PFRGB ||
	    pfmt == PFRGBZ ||
	    pfmt == PFZRGB ||
	    pfmt == PFBGR ||
	    pfmt == PFBGRZ ||
	    pfmt == PFZBGR)
		return(TRUE);
	else
		return(FALSE);
}

h_boolean ptype_is_col3(pfmt)

int pfmt;

{
	if (pfmt == PFRGB ||
	    pfmt == PFRGBZ ||
	    pfmt == PFZRGB ||
	    pfmt == PFBGR ||
	    pfmt == PFBGRZ ||
	    pfmt == PFZBGR)
		return(TRUE);
	else
		return(FALSE);
}
