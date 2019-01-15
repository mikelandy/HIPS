/*
 *	AHECLIP.C : Modify the histogram for a region of the image
 *	in order to control the mapping function.
 *
 *	Corrected a bug from the integer operations
 *					--- Shie Jue Lee
 *						Nov. 13, 1987
 *	Modified by Shie-Jue Lee on invalid clip checking.
 *		N is changed from N = xdim * ydim to
 */

#include <stdio.h>
#include <math.h>
/*#include <image.h>*/
#include "ahe.h"

aheclip(histo, maxlevels, cliplimit, xdim, ydim, ptemp, p)
     int        *histo,		/* histogram array */
	 maxlevels,		/* number of grey levels in image */
	 xdim,			/* region size in x */
	 ydim,			/* region size in y */
	 cliplimit,		/* computed limit of bucket count */
	 *ptemp;		/* floor of p */
     float      *p;		/* base into which excess just fits */
{
    int        *h,		/* pointer to histogram */
    num_above,			/* number of buckets at or above limit */
    histsize,			/* total room in histogram */
    top,			/* high value in binary search */
    bottom,			/* low value in binary search */
    limit,			/* cliplimit - ptemp */
    excess,			/* number of pixels above cliplimit */
    N;				/* region area(in pixels) */
    
    N = xdim * ydim ;	/* region size */
    histsize = (cliplimit * maxlevels);	/* histogram size */
    
    if(histsize < N) {
	/* not enough room */
	fprintf(stderr, "invalid clip limit\n"); 
	exit(-1); 
    } else if(histsize == N) {
	/* set all bins to cliplimit */
	for(h = histo; h < histo + maxlevels; h++)
	    *h = cliplimit; 
	*ptemp = 0; 
	*p = 0.0; 
	return; 
    }
    
    
    if(calcexcess(cliplimit, histo, maxlevels) == 0) {
	/* no clipping needed */
	*ptemp = 0; 
	*p = 0.0; 
	return; 
    } else {
	/* binary search for ptemp */
	bottom = 0; 
	top = cliplimit; 
	excess = calcexcess(cliplimit, histo, maxlevels); 
	while((top - bottom) > 1) {
	    *ptemp = (top + bottom) / 2.0; 
	    limit = cliplimit - *ptemp; 
	    excess = calcexcess(limit, histo, maxlevels); 
	    if(*ptemp * maxlevels > excess)
		top = *ptemp; 
	    else
		bottom = *ptemp; 
	}
	*ptemp = bottom; 
	limit = cliplimit - *ptemp; 
	
	/* calculate number of buckets above(cliplimit-ptemp) */
	/* and also clip histogram at(cliplimit-ptemp) */
	num_above = 0; 
	for(h = histo; h < histo + maxlevels; h++)
	    if(*h >= limit) {
		num_above++; 
		*h = limit; 
	    }
	
				/* calculate p */
	*p = (*ptemp +(float)(excess - maxlevels *(*ptemp)) / 
	      (float)(maxlevels - num_above)); 
	return; 
    }
}

calcexcess(top, histo, maxlevels)
     int         top, *histo, maxlevels; 
{
    int         excess, 
    *h; 
    
    excess = 0; 
    for(h = histo; h < histo + maxlevels; h++)
	if(*h > top)
	    excess += *h - top; 
    return(excess); 
}
