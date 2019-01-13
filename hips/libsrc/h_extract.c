/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_extract.c - subroutines to extract a subimage
 *
 * pixel formats: same as h_copy
 *
 * Michael Landy - 1/16/91
 */

#include <hipl_format.h>

int h_extract(hdi,hdo,frow,fcol,nrows,ncols)

struct header *hdi,*hdo;
int frow,fcol,nrows,ncols;

{
	struct hips_roi iroi,oroi;

	getroi(hdi,&iroi);
	if (setroi(hdi,frow,fcol,nrows,ncols) == HIPS_ERROR)
		return(HIPS_ERROR);
	getroi(hdo,&oroi);
	clearroi(hdo);
	if (h_copy(hdi,hdo) == HIPS_ERROR)
		return(HIPS_ERROR);
	if (setroi2(hdi,&iroi) == HIPS_ERROR)
		return(HIPS_ERROR);
	if (setroi2(hdo,&oroi) == HIPS_ERROR)
		return(HIPS_ERROR);
	return(HIPS_OK);
}
