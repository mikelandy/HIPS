/* 5/30/1988 Modified by Shie-Jue Lee:				*/
/*	1. data type for equal_const				*/
/*	2. Iteration controls for histogram calculation		*/

#include <stdio.h>
#include <signal.h>
#include <math.h>
#include "ahe.h"

#define Malloc(x) malloc((unsigned)(x))

#define iptr1(Y,X)    (im1+((Y) * numline)+(X))
#define iptr2(Y,X)    (im2+((Y) * numline)+(X))

#define mapval(J,K)	(*(maps + ((J) * regline) + (K)))

int         status;		/* progress of calc. indicators	 */

ahecalc_short(im1, im2, dimv, minmax, nreg, clipfrac, argv)
     /* CHANGED */
     unsigned short *im1;			/* input image	 */
     unsigned char  *im2;			/* output image */
     int *dimv,			/* array of image bounds	 */
	 minmax[2],		/* minimum and maximum inputs	 */
	 nreg[DIMNO];		/* number of regions in x, y, z  */
     float clipfrac;		/* fraction of average bucket cnt */
     char **argv;		/* command line for status report via 'ps' */
{


/* CHANGED */
    int tmpint ;
    int         min,		/* minimum grey value			 */
                max,		/* maximum grey value			 */
                maxlevels,	/* number of grey levels in image	 */
                numregions,	/* number of regions to be used		 */
               *del[DIMNO],	/* region sizes in each dimension	 */
               *curdel,		/* current region under consideration	 */
                regsize,	/* compute size of current region	 */
                excess,		/* excess pixels in each dimension	 */
                newdel[DIMNO],	/* size of current regions		 */
                start[DIMNO],	/* current starting position in image	 */
                regline,	/* number of regions in line of image	 */
                cliplimit,	/* computed limit of bucket count	 */
                limit,		/* cliplimit - ptemp			 */
                numline,	/* number of pixels per line of image	 */
               *histo,		/* histogram array			 */
                ptemp;		/* floor of p 				 */
    float      *cumhisto,	/* cummulative histogram array           */
                p;		/* base for cumulative histogram	 */

    /* CHANGED */
    register unsigned short *i1;	/* fast pointers to subsegments of 	 */
    register unsigned char *i2;		/* image arrays */

    int         ymin,		/* bounds for mapping of image		 */
                ymax,
                xmin,
                xmax;

    int         jval,		/* map indicies				 */
                kval;
    double      recy,		/* constants to step thru image in mapping */
                recx,
                dy,
                dx,
                cdy,
                lefty,
                righty;

    float      *m1,		/* four maps for bilinear interpolation	 */
               *m2,
               *m3,
               *m4;

    float     **maps;		/* pointers to intensity maps		 */
    float       equal_const;	/* constant to produce histogram equal. */

    int         i,
                j,
                k,
                loop;		/* utility infielders		 */
    int         x,
                y;		/* ditto			 */

    register int *h;		/* fast pointer to histogram	 */
    register float *c;		/* fast pointer to cum. histo.   */
    register float *m;		/* fast pointer to current	 */
    				/* intensity mapping		 */
#ifdef godot
    extern int  report ();	/* status reporting routine	 */
    signal (SIGUSR1, report);	/* attach report routine to sig. */
#endif

/* Preliminary setup	*/
    status = 0;
    min = minmax[0];
    max = minmax[1];

#ifdef DEBUG
    fprintf (stderr,"ahe: min = %d\n", min);	/***/
    fprintf (stderr,"ahe: max = %d\n", max);	/***/
#endif

    maxlevels = max - min + 1;	/* runs from 0 to max + 1 */

#ifdef DEBUG
     /*###*/ fprintf (stderr, "maxlevels = %d\n", maxlevels);
#endif

/* calculate constants for matrix indexing	*/
    numline = dimv[2];		/* pixels in a line	 */
    regline = nreg[2];		/* regions in a line	 */

/* Find the total number of regions to be considered:	*/
    numregions = 1;
    for (i = 0; i < DIMNO; i++)
	numregions *= nreg[i];

#ifdef DEBUG
     /*###*/ fprintf (stderr, "maxlevels = %d\n", maxlevels) ;
     /*###*/ fprintf (stderr, "numline = %d\n", numline);
     /*###*/ fprintf (stderr, "regline = %d\n", regline);
     /*###*/ fprintf (stderr, "numregions = %d\n", numregions);
#endif

/* Allocate the histogram map.					*/
    histo = (int *) Malloc(sizeof (int) * maxlevels);

/* Allocate the cummulative histogram map.			*/
    cumhisto = (float *) Malloc (sizeof (float) * maxlevels);

/* Allocate the intensity mapping tables:			*/
    maps = (float **) Malloc (sizeof (float *) * numregions);

    for (i = 0; i < numregions; i++)
	*(maps + i) = (float *) Malloc (sizeof (float) * maxlevels);

/* 
 * Compute the mappings for each region.  We step through
 * the regions, calulating the histogram and the histogram
 * equalizing mapping for each region.  The same histogram
 * array is used over for each region, but each region has
 * its own map. 
 */

/* 
 * Calculate region sizes.  If the image is not evenly divisible,
 * the excess pixels are distributed among the first dimv mod n
 * regions
 */

/* Allocate the arrays to hold the size of each region: 	*/
    for (i = 0; i < DIMNO; i++)
    {
	del[i] = (int *) Malloc (sizeof (int *) * nreg[i]);
	curdel = del[i];
	regsize = dimv[i] / nreg[i];
	excess = dimv[i] % nreg[i];
	for (j = 0; j < nreg[i]; j++)
	    *(curdel + j) = (j < excess) ? regsize + 1 : regsize;
    }

/*
 *	Determine cliplimit from range of intensities in image and
 *	the region size.  Image size/(number intensity levels * number
 *	of regions) gives approximation of number of pixels at each
 *	intensity in a region.  Clipfrac is used to scale the result.
 */
#ifdef DEBUG
	fprintf (stderr, "ahe: clipfrac %f\n", clipfrac);
#endif
    if (clipfrac > 0.)
    {
	cliplimit = clipfrac * (dimv[0] * dimv[1] * dimv[2]) /
	    (maxlevels * numregions) +.5;
#ifdef DEBUG
	fprintf (stderr, "ahe: cliplimit %d\n", cliplimit);
#endif
    }
    else
	/* 0 is flag for no clipping */
	cliplimit = 0;

#ifdef DEBUG
    fprintf (stderr, "ahecalc: clipfrac %f cliplimit %d\n", clipfrac, cliplimit);
#endif

/* Set up to calculate the histogram and mappings.  */

    status = 0;
    start[1] = start[2] = 0;
	for (j = 0; j < nreg[1]; j++)	/* loop thru y regions	 */
	{
	    for (k = 0; k < nreg[2]; k++)	/* loop thru x regions	 */
	    {
#ifdef REPORT
		printf ("status: %d/192", status);
		fflush (stdout);
#endif
		sprintf (argv[0], "status:%d/192", status++);
		newdel[1] = *(del[1] + j);	/* current region in y	 */
		newdel[2] = *(del[2] + k);	/* current region in x	 */
		equal_const = (double) (maxlevels - 1) / (newdel[1] * newdel[2]);

#ifdef DEBUG1
		 /*###*/ fprintf (stderr, " equal_const %f\n", equal_const);
		 /*###*/ fprintf (stderr, " newdel[1] %d\n", newdel[1]);
		 /*###*/ fprintf (stderr, " newdel[2] %d\n", newdel[2]);
#endif

		/* zero histogram array				 */
		for (h = histo; h < histo + maxlevels; h++)
		    *h = 0;
		h = histo;

		/* calculate histogram for this region		 */
		    for (y = start[1]; y < start[1] + newdel[1]; y++)
			for (x = start[2]; x < start[2] + newdel[2]; x++)
			    ++h[*(iptr1 (y, x))];

		/* clip histogram if user requested a limit         */
		if (cliplimit > 0)
		{
		    aheclip (histo, maxlevels, cliplimit, newdel[2], newdel[1],
			     &ptemp, &p);

		    /* calculate cumulative histogram			 */
		    limit = cliplimit - ptemp;
		    if (*histo == limit)
			*cumhisto = (float) cliplimit;
		    else
			*cumhisto = (float) *histo + p;

		    for (h = histo + 1, c = cumhisto + 1; h < histo + maxlevels;
			 h++, c++)
		    {
			if (*h == limit)
			{
			    *c = *(c - 1) + (float) cliplimit;
			}
			else
			{
			    *c = *(c - 1) + (float) *h + p;
			}
		    }
		}
		else
		{
		    /* no clipping requested */
		    *cumhisto = (float) *h;
		    for (h = histo + 1, c = cumhisto + 1; h < histo + maxlevels;
			 h++, c++)
		    {
			*c = *(c - 1) + (float) *h;

		    }
		}

#ifdef DEBUG1
		 /*###*/ fprintf (stderr, "j %d, k %d\n", j, k);
#endif

		/* calculate intensity mapping number		 */
		m = mapval (j, k);

		*m = 0;
		for (c = cumhisto; c < cumhisto + maxlevels; ++c, ++m)
		    *m = equal_const * (*c);

		/* increment starting point				 */
		start[2] += newdel[2];
	    }
	    start[2] = 0;
	    start[1] += newdel[1];
	}

/* Apply mappings to the image */
    start[0] = 0;
    loop = 0;

	    start[1] = 0;
	    ymin = 0;

	    for (j = -1; j < nreg[1]; j++)	/* y regions */
	    {
		if (j == -1)
		{
		    jval = 0;
		    dy = 0;
		    recy = 0;
		    ymax = (*(del[1])) / 2.0;
		}
		else
		if (j == nreg[1] - 1)
		{
		    jval = j - 1;
		    dy = 1;
		    recy = 0;
		    ymax = dimv[1];
		}
		else
		{
		    start[1] += *(del[1] + j);
		    jval = j;
		    dy = 0;
		    recy = 1.0 / (float) (*(del[1] + j));
		    ymax = start[1] + (*(del[1] + j + 1)) / 2.0;
		}

		for (y = ymin; y < ymax; y++)	/* y pixels in rgion */
		{
#ifdef REPORT
		    if ((loop % 4) == 0)
		    {
			printf ("status: %d/192", status);
			fflush (stdout);
		    }
		    if ((loop++ % 4) == 0)
			sprintf (argv[0], "status:%d/192", status++);
#endif
		    cdy = 1.0 - dy;
		    start[2] = 0;
		    xmin = 0; 

		    for (k = -1; k < nreg[2]; k++)	/* x regions */
		    {
			if (k == -1)
			{
			    kval = 0;
			    dx = 0;
			    recx = 0;
			    xmax = (*(del[2])) / 2.0;
			}
			else
			if (k == nreg[2] - 1)
			{
			    dx = 1;
			    kval = k - 1;
			    recx = 0;
			    xmax = dimv[2];
			}
			else
			{
			    start[2] += *(del[2] + k);
			    kval = k;
			    dx = 0;
			    recx = 1.0 / (float) (*(del[2] + k));
			    xmax = start[2] + (*(del[2] + k + 1)) / 2.0;
			}

			/* maps for the interpolation */
			m1 = mapval (jval, kval);
			m2 = mapval (jval, kval + 1);
			m3 = mapval (jval + 1, kval);
			m4 = mapval (jval + 1, kval + 1);

			i1 = iptr1 (y, xmin);
			i2 = iptr2 (y, xmin);
#ifdef FOO
			fprintf (stderr,"x, y= %d %d ",xmin,y) ;
#endif

			for (x = xmin; x < xmax; x++)	/* x pixels in region */
			{
			    lefty = dy * (m3[*i1]) + cdy * (m1[*i1]);

			    righty = dy * (m4[*i1]) + cdy * (m2[*i1]);

			    /* CHANGED */
			    tmpint = (int) (righty * dx
					 + lefty * (1.0 - dx));

                            if (tmpint > MAXSHORT)
			    {
#ifdef DEBUG
				fprintf (stderr,"%lf %lf %lf %lf %lf %d\n",lefty,righty,
					 dx,dy,cdy,tmpint) ;
#endif
                                tmpint = MAXSHORT ;
			    }
                            *i2 = (unsigned char) (tmpint >> 2) ;
			    i1++;
			    i2++;
			    dx += recx;
			}	/* end for x */
			xmin = xmax;
		    }		/* end for k */
		    dy += recy;
		}		/* end for y */
		ymin = ymax;
	    }			/* end for j */
}				/* end */

#ifdef godot
report ()
{
    signal (SIGUSR1, SIG_IGN);
    fprintf (stdout, "%3d 192\n", status);
    fflush (stdout);
    signal (SIGUSR1, report);
}
#endif
