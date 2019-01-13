#ifndef lint
static char SccSID[] = "@(#)peanocurve.c	1.3 7/27/88";
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

u_char		*curve;
short		xyz[LEVELS][LEVELS][LEVELS];
					/* these are the forward and reverse
					Peano scan maps */

static short	*X, *Y, *Z,			/* input image  */
		*XX, *YY, *ZZ;			/* working vectors */

static short	*Xout, *Yout, *Zout;		/* result buffers */
static short	*Xoutptr, *Youtptr, *Zoutptr; /* pointers into result buffers */

static int		points;

extern char		*Progname;
void xyzcurve(),draw(),plot(),rule1(),rule2(),rule3(),rule4(),rule5();

void peanocurve()
{
register int	i, size;
register u_char	*byteptr;
int		reps;

static short Xseed[] = {0, 1, 1, 0, 0, 1, 1, 0};
static short Yseed[] = {0, 0, 1, 1, 1, 1, 0, 0};
static short Zseed[] = {0, 0, 0, 0, 1, 1, 1, 1};

	if (MYBITS < 1) {
		fprintf(stderr, "%s: BITS is less than one\n", Progname);
		exit(1);
	}

	points = 8;
	if (((X = (short *)calloc(points, sizeof(short))) == NULL)
	||  ((Y = (short *)calloc(points, sizeof(short))) == NULL)
	||  ((Z = (short *)calloc(points, sizeof(short))) == NULL)) {
		fprintf(stderr, "%s: can't allocate core\n", Progname);
		exit(1);
	}

	if ((curve = (u_char *)calloc(3 * COLORS, sizeof(u_char))) == NULL) {
		fprintf(stderr, "%s: can't allocate core\n", Progname);
		exit(1);
	}

	if (MYBITS == 1) {
		Xout = Xseed, Yout = Yseed, Zout = Zseed;

		size = COLORS;
		Xoutptr = Xout, Youtptr = Yout, Zoutptr = Zout, byteptr = curve;
		while (size--) {
			*byteptr++ = untrunc(*Xoutptr++);
			*byteptr++ = untrunc(*Youtptr++);
			*byteptr++ = untrunc(*Zoutptr++);
		}
		xyzcurve();
		return;
	}

	for (i = 0; i < points; i++)
		X[i] = Xseed[i], Y[i] = Yseed[i], Z[i] = Zseed[i];
	draw();

	reps = MYBITS - 2;
	while (reps--) {
		points *= 8;
		free(X), free(Y), free(Z);
		free(XX), free(YY), free(ZZ);
		X = Xout, Y = Yout, Z = Zout;
		draw();
	}


	size = COLORS;
	Xoutptr = Xout, Youtptr = Yout, Zoutptr = Zout, byteptr = curve;
	while (size--) {
		*byteptr++ = untrunc(*Xoutptr++);
		*byteptr++ = untrunc(*Youtptr++);
		*byteptr++ = untrunc(*Zoutptr++);
	}
	xyzcurve();
	return;
}

void xyzcurve()
{
register int	i;

	for (i = 0; i < COLORS; i++)
		xyz[trunc(curve[3*i])][trunc(curve[3*i+1])][trunc(curve[3*i+2])]
		= i;
}

static int x, y, z;
static int outpoints;

void draw()
{
	outpoints = 8 * points;
	if (((XX = (short *)calloc(points, sizeof(short))) == NULL)
	||  ((YY = (short *)calloc(points, sizeof(short))) == NULL)
	||  ((ZZ = (short *)calloc(points, sizeof(short))) == NULL)
	||  ((Xoutptr = Xout = (short *)calloc(outpoints, sizeof(short)))
								== NULL)
	||  ((Youtptr = Yout = (short *)calloc(outpoints, sizeof(short)))
								== NULL)
	||  ((Zoutptr = Zout = (short *)calloc(outpoints, sizeof(short)))
								== NULL)) {
		fprintf(stderr, "%s: can't allocate core\n", Progname);
		exit(1);
	}

	x = y = z = 0;	plot();
	rule1();
		x++;	plot();
	rule2();
		y++;	plot();
	rule2();
		x--;	plot();
	rule3();
		z++;	plot();
	rule3();
		x++;	plot();
	rule4();
		y--;	plot();
	rule4();
		x--;	plot();
	rule5();
}

void plot()
{
	*Xoutptr++	= x;
	*Youtptr++	= y;
	*Zoutptr++	= z;
}


static void plotstuff()
{
register int i;

	for (i = 1; i < points; i++) {
		x += XX[i] - XX[i - 1];
		y += YY[i] - YY[i - 1];
		z += ZZ[i] - ZZ[i - 1];
		plot();
	}
}

void rule1()
{
register int i;

	for (i = 0; i < points; i++) {
		XX[i]	=	- Z[points - 1 - i];
		YY[i]	=	Y[points - 1 - i];
		ZZ[i]	=	X[points - 1 - i];
	}

	plotstuff();
}

void rule2()
{
register int i;

	for (i = 0; i < points; i++) {
		XX[i]	=	X[points - 1 - i];
		YY[i]	=	- Z[points - 1 - i];
		ZZ[i]	=	Y[points - 1 - i];
	}

	plotstuff();
}

void rule3()
{
register int i;
	for (i = 0; i < points; i++) {
		XX[i]	=	- Y[points - 1 - i];
		YY[i]	=	- X[points - 1 - i];
		ZZ[i]	=	- Z[points - 1 - i];
	}

	plotstuff();
}

void rule4()
{
register int i;
	for (i = 0; i < points; i++) {
		XX[i]	=	Y[i];
		YY[i]	=	- Z[i];
		ZZ[i]	=	- X[i];
	}

	plotstuff();
}

void rule5()
{
register int i;

	for (i = 0; i < points; i++) {
		XX[i]	=	Z[points - 1 - i];
		YY[i]	=	Y[points - 1 - i];
		ZZ[i]	=	- X[points - 1 - i];
	}

	plotstuff();
}
