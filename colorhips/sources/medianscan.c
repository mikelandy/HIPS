#ifndef lint
static char SccSID[] = "@(#)medianscan.c	1.2 7/27/88";
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

#include	"cscan.h"
#define	RED	1
#define	GREEN	2
#define	BLUE	3

extern int	PIXELS, OUTBINS;
extern u_char	*lr, *lg, *lb;

struct box {
	u_char		rmin, rmax, gmin, gmax, bmin, bmax;
	int		total;
	struct box	*next;
} *boxlist;

static int	histo[LEVELS][LEVELS][LEVELS];
static int	bins_made;
int splitbox();
void binsert();

void mscan(redbuf, greenbuf, bluebuf)	/* choose a set of lut points by
					median-cut histogram analysis of
					the input image */
u_char *redbuf, *greenbuf, *bluebuf;
{
int		i;

	boxlist = (struct box *)(calloc(1, sizeof(struct box)));
	boxlist->rmin = boxlist->gmin = boxlist->bmin = 0;
	boxlist->rmax = boxlist->gmax = boxlist->bmax = LEVELS - 1;
	boxlist->total = PIXELS;

	for (i = 0; i < PIXELS; i++)
		(*(*(*(histo + trunc(*redbuf++))
				+ trunc(*greenbuf++))
					+ trunc(*bluebuf++)))++;

	bins_made = 1;
	i= 0;
	while (boxlist && bins_made < OUTBINS) {
		if (splitbox(boxlist) == FAIL) {
			*(lr + i) = untrunc(boxlist->rmax);
			*(lg + i) = untrunc(boxlist->gmax);
			*(lb + i) = untrunc(boxlist->bmax);
			i++;
		}
		boxlist = boxlist->next;
	}
	for ( ; boxlist; boxlist = boxlist->next) {
		{
		int r, g, b, red, green, blue, pixels;
		red = green = blue = 0;
		for (r = boxlist->rmin; r <= boxlist->rmax; r++)
			for (g = boxlist->gmin; g <= boxlist->gmax; g++)
				for (b = boxlist->bmin;b <= boxlist->bmax;b++) {
					pixels	= histo[r][g][b];
					red	+= r *	pixels;
					green	+= g *	pixels;
					blue	+= b *	pixels;
				}
		*(lr + i) = untrunc(red/boxlist->total);
		*(lg + i) = untrunc(green/boxlist->total);
		*(lb + i) = untrunc(blue/boxlist->total);
		i++;
		}
	}
}

int splitbox(bp)
struct box *bp;
{
int		bin[LEVELS], total = 0;
register short	k, j, i;
struct box	*left, *right;
short		dimension;
u_char		rextent, gextent, bextent;

	for (i = bp->rmin; i <= bp->rmax; i++) {
		for (j = bp->gmin; j <= bp->gmax; j++)
			for (k = bp->bmin; k <= bp->bmax; k++)
				if (histo[i][j][k])
					goto foundrmin;
	}
foundrmin:	bp->rmin = i;
	for (i = bp->rmax; i >= bp->rmin; i--) {
		for (j = bp->gmin; j <= bp->gmax; j++)
			for (k = bp->bmin; k <= bp->bmax; k++)
				if (histo[i][j][k])
					goto foundrmax;
	}
foundrmax:	bp->rmax = i;
	for (i = bp->gmin; i <= bp->gmax; i++) {
		for (j = bp->bmin; j <= bp->bmax; j++)
			for (k = bp->rmin; k <= bp->rmax; k++)
				if (histo[k][i][j])
					goto foundgmin;
	}
foundgmin:	bp->gmin = i;
	for (i = bp->gmax; i >= bp->gmin; i--) {
		for (j = bp->bmin; j <= bp->bmax; j++)
			for (k = bp->rmin; k <= bp->rmax; k++)
				if (histo[k][i][j])
					goto foundgmax;
	}
foundgmax:	bp->gmax = i;
	for (i = bp->bmin; i <= bp->bmax; i++) {
		for (j = bp->rmin; j <= bp->rmax; j++)
			for (k = bp->gmin; k <= bp->gmax; k++)
				if (histo[j][k][i])
					goto foundbmin;
	}
foundbmin:	bp->bmin = i;
	for (i = bp->bmax; i >= bp->bmin; i--) {
		for (j = bp->rmin; j <= bp->rmax; j++)
			for (k = bp->gmin; k <= bp->gmax; k++)
				if (histo[j][k][i])
					goto foundbmax;
	}
foundbmax:	bp->bmax = i;

	rextent = bp->rmax - bp->rmin,
	gextent = bp->gmax - bp->gmin,
	bextent = bp->bmax - bp->bmin;

	if (rextent == 0 && gextent == 0 && bextent == 0) /* only one color */
		return(FAIL);

	left	= (struct box *)(calloc(1, sizeof(struct box)));
	right	= (struct box *)(calloc(1, sizeof(struct box)));
	right->rmin = left->rmin = bp->rmin;
	right->rmax = left->rmax = bp->rmax;
	right->gmin = left->gmin = bp->gmin;
	right->gmax = left->gmax = bp->gmax;
	right->bmin = left->bmin = bp->bmin;
	right->bmax = left->bmax = bp->bmax;

	if (rextent > gextent) {
		if (rextent > bextent)
			dimension = RED;
		else
			dimension = BLUE;
	}
	else {
		if (gextent > bextent)
			dimension = GREEN;
		else
			dimension = BLUE;
	}

	switch (dimension) {
	case RED:
		for(i = bp->rmin; i <= bp->rmax; i++)
			bin[i] = 0;
		for(i = bp->rmin; i <= bp->rmax; i++) {
			for (j = bp->gmin; j <= bp->gmax; j++)
				for (k = bp->bmin; k <= bp->bmax; k++)
					bin[i] += histo[i][j][k];
			total += bin[i];
			bin[i] = total;
			if (bin[i] > bp->total/2)
				break;
		}
		if (i == bp->rmin)
			left->rmax = i;
		else
			left->rmax = i - 1;
		right->rmin = left->rmax + 1;
		left->total = bin[left->rmax];
		right->total = bp->total - left->total;
		break;

	case GREEN:
		for(i = bp->gmin; i <= bp->gmax; i++)
			bin[i] = 0;
		for(i = bp->gmin; i <= bp->gmax; i++) {
			for (j = bp->bmin; j <= bp->bmax; j++)
				for (k = bp->rmin; k <= bp->rmax; k++)
					bin[i] += histo[k][i][j];
			total += bin[i];
			bin[i] = total;
			if (bin[i] > bp->total/2)
				break;
		}
		if (i == bp->gmin)
			left->gmax = i;
		else
			left->gmax = i - 1;
		right->gmin = left->gmax + 1;
		left->total = bin[left->gmax];
		right->total = bp->total - left->total;
		break;

	case BLUE:
		for(i = bp->bmin; i <= bp->bmax; i++)
			bin[i] = 0;
		for(i = bp->bmin; i <= bp->bmax; i++) {
			for (j = bp->rmin; j <= bp->rmax; j++)
				for (k = bp->gmin; k <= bp->gmax; k++)
					bin[i] += histo[j][k][i];
			total += bin[i];
			bin[i] = total;
			if (bin[i] > bp->total/2)
				break;
		}
		if (i == bp->bmin)
			left->bmax = i;
		else
			left->bmax = i - 1;
		right->bmin = left->bmax + 1;
		left->total = bin[left->bmax];
		right->total = bp->total - left->total;
		break;
	}

	binsert(left);
	binsert(right);
	bins_made++;

	return(SUCCESS);
}

void binsert(bp)		/* the boxlist is maintained in descending
						order of population */
struct box *bp;
{
struct box *bp1, *bp2;

	bp1 = bp2 = boxlist;
	while (bp1 && bp1->total > bp->total) {
		bp2 = bp1;
		bp1 = bp1->next;
	}
	bp->next = bp1;
	if (bp1 == boxlist)
		boxlist = bp;
	else
		bp2->next = bp;
}
