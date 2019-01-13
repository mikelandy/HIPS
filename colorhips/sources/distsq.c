#ifndef lint
static char SccSID[] = "@(#)distsq.c	1.2 7/27/88";
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

#include <hipl_format.h>
#include "cscan.h"

extern int	OUTBINS;
extern u_char	*lr, *lg, *lb;

static short square[] = {0, 1, 4, 9, 16, 25, 36, 49, 64, 81, 100, 121, 144,
		169, 196, 225, 256, 289, 324, 361, 400, 441, 484, 529, 576,
		625, 676, 729, 784, 841, 900, 961};

short	**redsquares, **greensquares, **bluesquares;
void error();

void distsq_setup()
{
register short	i, L;
register short	rlevel, glevel, blevel;

	if (((redsquares = (short **)
			malloc(LEVELS * sizeof(short *))) == NULL)
	||  ((greensquares = (short **)
			malloc(LEVELS * sizeof(short *))) == NULL)
	||  ((bluesquares = (short **)
			malloc(LEVELS * sizeof(short *))) == NULL))
		error("can't allocate core");

	for (i = 0; i < LEVELS; i++)
		if (((*(redsquares + i) = (short *)
				malloc(OUTBINS * sizeof(short))) == NULL)
		||  ((*(greensquares + i) = (short *)
				malloc(OUTBINS * sizeof(short))) == NULL)
		||  ((*(bluesquares + i) = (short *)
				malloc(OUTBINS * sizeof(short))) == NULL))
			error("can't allocate core");

	for (i = 0; i < OUTBINS; i++) {
		rlevel = (short)trunc(*(lr + i));
		glevel = (short)trunc(*(lg + i));
		blevel = (short)trunc(*(lb + i));
		for (L = 0; L < LEVELS; L++) {
			*(*(redsquares + L) + i) = *(square + abs(L - rlevel));
			*(*(greensquares + L)+ i) = *(square + abs(L - glevel));
			*(*(bluesquares + L) + i) = *(square + abs(L - blevel));
		}
	}
}

#ifdef LOCALSEARCH
#define	CELLSIDE	4
#define	CELLBITS 	2

static short closeness;

static short distsqtocell(i, rcell, gcell, bcell)
short	i, rcell, gcell, bcell;
{
short	R = rcell << CELLBITS, G = gcell << CELLBITS, B = bcell << CELLBITS;
short	rsqmin, rsqmax, gsqmin, gsqmax, bsqmin, bsqmax, j, temp;
h_boolean	inside = FALSE;

	if (((trunc(*(lr + i)) >> CELLBITS) == rcell)
	&&  ((trunc(*(lg + i)) >> CELLBITS) == gcell)
	&&  ((trunc(*(lb + i)) >> CELLBITS) == bcell)) {
						/* lut pt is within the cell */
		inside = TRUE;
	}

	rsqmin = rsqmax = redsquares[R][i];
	for (j = 1; j < CELLSIDE; j++) {
		if (rsqmin > (temp = redsquares[++R][i]))
			rsqmin = temp;
		else if (rsqmax < temp)
			rsqmax = temp;
	}

	gsqmin = gsqmax = greensquares[G][i];
	for (j = 1; j < CELLSIDE; j++) {
		if (gsqmin > (temp = greensquares[++G][i]))
			gsqmin = temp;
		else if (gsqmax < temp)
			gsqmax = temp;
	}

	bsqmin = bsqmax = bluesquares[B][i];
	for (j = 1; j < CELLSIDE; j++) {
		if (bsqmin > (temp = bluesquares[++B][i]))
			bsqmin = temp;
		else if (bsqmax < temp)
			bsqmax = temp;
	}

	if (closeness > rsqmax + gsqmax + bsqmax)
		closeness = rsqmax + gsqmax + bsqmax;

	if (inside)
		return(0);
	return(rsqmin + gsqmin + bsqmin);
}

static struct Cellist	****list;
static h_boolean		***listed;
static short		*dists;

void cellist_setup()
{
register short	j, i;

	if ((listed = (h_boolean ***)
			malloc((LEVELS >> CELLBITS) * sizeof(h_boolean **))) == NULL)
		error("can't allocate core");
	for (i = 0; i < LEVELS >> CELLBITS; i++) {
		if  ((listed[i] = (h_boolean **)
				malloc((LEVELS >> CELLBITS) * sizeof(h_boolean *)))
									== NULL)
			error("can't allocate core");

		for (j = 0; j < LEVELS >> CELLBITS; j++)
			if  ((listed[i][j] = (h_boolean *)calloc(LEVELS >> CELLBITS,
							sizeof(h_boolean))) == NULL)
				error("can't allocate core");
	}

	if (((list = (struct Cellist ****)
			malloc((LEVELS >> CELLBITS)
					* sizeof(struct Cellist ***))) == NULL)
	||  ((dists = (short *)
			calloc(OUTBINS, sizeof(short))) == NULL))
		error("can't allocate core");

	for (i = 0; i < LEVELS >> CELLBITS; i++) {
		if ((list[i] = (struct Cellist ***)malloc((LEVELS >> CELLBITS)
					* sizeof(struct Cellist **))) == NULL)
			error("can't allocate core");
	
		for (j = 0; j < LEVELS >> CELLBITS; j++)
			 if ((list[i][j] = (struct Cellist **)
					calloc(LEVELS >> CELLBITS,
					sizeof(struct Cellist *))) == NULL)
				error("can't allocate core");
	}
}

static void insert(i, rcell, gcell, bcell)
short i, rcell, gcell, bcell;
{
struct Cellist *ptr, *ptr2, *ptr3;

	ptr = ptr2 = list[rcell][gcell][bcell];

	while ((ptr) && (ptr->celldistsq < dists[i])) {
		ptr2 = ptr;
		ptr = ptr->next;
	}
	ptr3 = (struct Cellist *)malloc(sizeof(struct Cellist));
	ptr3->bindex		= i;
	ptr3->celldistsq	= dists[i];
	ptr3->next		= ptr;
	if (ptr == list[rcell][gcell][bcell])
		list[rcell][gcell][bcell] = ptr3;
	else
		ptr2->next = ptr3;
}

struct Cellist *cellist(R, G, B)
short R, G, B;
{
short rcell, gcell, bcell;
	if (!listed[rcell=R>>CELLBITS][gcell=G>>CELLBITS][bcell=B>>CELLBITS]) {
		short			distsqtocell();
		register short		i;

			closeness = INFINITY_HIPS;
			for (i = 0; i < OUTBINS; i++)
				dists[i] = distsqtocell(i, rcell, gcell, bcell);

			for (i = 0; i < OUTBINS; i++)
				if (dists[i] <= closeness)
					insert(i, rcell, gcell, bcell);

			listed[rcell][gcell][bcell] = TRUE;
	}

	return(list[rcell][gcell][bcell]);
}

#endif
