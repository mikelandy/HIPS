/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * hdepth - get/set the depth parameter (the number of depths per frame)
 */

#include <hipl_format.h>

int hgetdepth(hd)

struct header *hd;

{
	int one=1,depth;

	if (findparam(hd,"depth") != NULLPAR) {
		if (getparam(hd,"depth",PFINT,&one,&depth) == HIPS_ERROR)
			return(HIPS_ERROR);
	}
	else
		depth = 1;
	return(depth);
}

int hsetdepth(hd,depth)

struct header *hd;
int depth;

{
	if (depth < 1)
		return(perr(HE_BADDEPTH));
	return(setparam(hd,"depth",PFINT,1,depth));
}
