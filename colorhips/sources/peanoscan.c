#ifndef lint
static char SccSID[] = "@(#)peanoscan.c	1.2 7/27/88";
#endif
/*
	Copyright 1988 Alan Shaw and Eric Schwartz.
	No part of this software may be distributed or sold without the prior
	agreement of Prof. Eric Schwartz, Dept. of Psychiatry, NYU School of
	Medicine, 550 1st Ave., New York, New York, 10016.
*/
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>

#include	<hipl_format.h>
#include	"cscan.h"

#ifdef	FILES

#define	thrindex(x,y,z)	(LEVELSSQ * (x) + LEVELS * (y) + (z))
#define	peano(x,y,z)	(*(xyz + thrindex(trunc(x), trunc(y), trunc(z))))

#else

#define	peano(x,y,z)	(*(*(*(xyz + trunc(x)) + trunc(y)) + trunc(z)))

#endif
	/* x, y, z range from 0 to 255 */

extern int	PIXELS, OUTBINS;
extern u_char	*lr, *lg, *lb;

static u_char	*mycurve;

#ifdef	FILES

static short	*xyz;

#else

extern u_char	*curve;
extern short	xyz[LEVELS][LEVELS][LEVELS];

#endif
				 /* curve is the buffer into which the
					peano scan ordering is to be read. */
				/* xyz is the reordered peano scan.
					xyz[x][y][z] = peano(x,y,z). */
static int	histo[COLORS];
void error(),binning(),add_to_bin();
int bigbins(),close_bin();

void pscan(redbuf, greenbuf, bluebuf)	/* choose a set of lut points by
					Peano-scan histogram analysis of
					the input image */
u_char *redbuf, *greenbuf, *bluebuf;
{
register int	i;

/* first we initialise the xyz and curve arrays: */

#ifdef	FILES

	/* read in the arrays: */

	{
	FILE *fp;

	mycurve = (u_char *) hmalloc(3 * COLORS * sizeof(u_char));
	fp = ffopen(SCANFILE, "r");
	if (fread(mycurve, 3 * COLORS * sizeof(u_char),1,fp) != 1)
		error("error during read");
	fclose(fp);

	xyz = (short *) hmalloc(COLORS * sizeof(short));
	fp = ffopen(XYZFILE, "r");
	if (fread((short *) xyz, COLORS * sizeof(short),1,fp) != 1)
		error("error during read");
	fclose(fp);
	}

#else

	peanocurve();	/* make the curve and xyz arrays from scratch. */
	mycurve = curve;

#endif

/* now we make the histogram of the input colors along the Peano curve: */

	for (i = 0; i < PIXELS; i++)
		(*(histo + peano(*redbuf++, *greenbuf++, *bluebuf++)))++;

/* then we select lut points based on the histogram: */

	binning();
}

static int	pix_left;
static int	red,green, blue,bin;
static int	bin_size, bin_bound;
static h_boolean	ASSIGNED[COLORS];

void binning()
{
int	i;
int	index = 0;

	pix_left = PIXELS;
	bin_size = PIXELS/OUTBINS;

	while (((index = bigbins(index, bin_size)) < OUTBINS)
	&&     (bin_size != pix_left/(OUTBINS - index)))
		bin_size = pix_left/(OUTBINS-index);

	i = bin = bin_bound = red = green = blue = 0;

	while (i < COLORS) {
		if ((*(ASSIGNED + i)) || (*(histo + i) == 0)) {
			i++;
			continue;
		}

		if (bin_bound + *(histo + i) <= bin_size) {
			add_to_bin(i++);
			continue;
		}

		if (bin_size - bin_bound > bin_bound + *(histo + i) - bin_size)
			add_to_bin(i++);

		if (index == OUTBINS - 1) {
			while ( i < COLORS) {
				if (!(*(ASSIGNED + i)) && (*(histo + i)))
					add_to_bin(i);  
				i++;
			}
			close_bin(index);
			break;
		}
		else {
			index = close_bin(index);
		}
	}
}

int bigbins(index, bin_size)
int index, bin_size;
{
int i, threei;
	for (i = 0; i < COLORS; i++)
		if ((*(histo + i) > bin_size) && !(*(ASSIGNED + i))) {
			threei = 3 * i;
			*(lr + index) = *(mycurve + threei++);
			*(lg + index) = *(mycurve + threei++);
			*(lb + index) = *(mycurve + threei);
			*(ASSIGNED + i) = TRUE;
			pix_left -= *(histo + i);
			index++;
		}
	return(index);
}

void add_to_bin(i)
int i;
{
register int threei = 3 * i;
	bin		+=	*(histo + i);
	bin_bound	+=	*(histo + i);
	red		+=	*(histo + i) * (int)(*(mycurve + threei++));
	green		+=	*(histo + i) * (int)(*(mycurve + threei++));
	blue		+=	*(histo + i) * (int)(*(mycurve + threei));
}

int close_bin(index)
int index;
{
u_char	redchar, greenchar, bluechar;
short	i, pvalue;
	if (bin == 0)
		return(index);

/*
	Here we are checking for duplicate peano(*(lr+i),*(lg+i),*(lb+i)).
	These represent duplicate bin values (i.e. differ only in
	last 3 bits of r,g,b).
*/
	redchar = red/bin;
	greenchar = green/bin;
	bluechar = blue/bin;
	bin = red = green = blue = 0;
/*
	if (index < OUTBINS - 1) {
		pvalue = peano(redchar, greenchar, bluechar);
		for (i = 0; i < index; i++)
			if (peano(*(lr + i), *(lg + i), *(lb + i)) == pvalue) {
I guess we should split this bin.
			}
	}
*/
	*(lr + index) = redchar;
	*(lg + index) = greenchar;
	*(lb + index) = bluechar;
	bin_bound -= bin_size;
	return(index + 1);
}

static u_char	*LR, *LG, *LB;

void mapping_peano()	/* map by distance along the peano curve */
{
int		i, threei, a, compare();
int		*order, *boundary;
extern u_char	LUTMAP[LEVELS][LEVELS][LEVELS];
	if (((order = (int *) calloc(OUTBINS, sizeof(int))) == 0)
	||  ((boundary = (int *) calloc(257, sizeof(int))) == 0))
		error("can't allocate core");

	for (i = 0; i < OUTBINS; i++)
		*(order + i) = i;

	qsort((char *)order, OUTBINS, sizeof(int), compare);

	if (((LR = (u_char *) calloc(256, sizeof(u_char))) == 0)
	||  ((LG = (u_char *) calloc(256, sizeof(u_char))) == 0)
	||  ((LB = (u_char *) calloc(256, sizeof(u_char))) == 0))
		error("can't allocate core");

	for (i = 0; i < OUTBINS; i++) {
		*(LR + i) = *(lr + *(order + i));
		*(LG + i) = *(lg + *(order + i));
		*(LB + i) = *(lb + *(order + i));
	}

	lr = LR;
	lg = LG;
	lb = LB;

	*boundary = 0;
	for (i = 1; i < OUTBINS; i++)
		*(boundary + i) =
			(peano(*(lr + i - 1), *(lg + i - 1), *(lb + i - 1))
		    +	peano(*(lr + i), *(lg + i), *(lb + i))
		    +	1)/2;
	*(boundary + OUTBINS) = COLORS;

	for (a = 0; a < OUTBINS; a++)
		for (i = *(boundary + a); i < *(boundary + a + 1); i++) {
			threei = 3 * i;
			*(*(*(LUTMAP + trunc(*(mycurve + threei)))
			+ trunc(*(mycurve + threei + 1)))
			+ trunc(*(mycurve + threei + 2))) = a;
		}
}

int compare(elt1, elt2)
int *elt1, *elt2;
{
	return(peano(*(lr + *elt1), *(lg + *elt1), *(lb + *elt1))  -
	       peano(*(lr + *elt2), *(lg + *elt2), *(lb + *elt2)));
}
