#ifndef lint
static char SccSID[] = "@(#)colorfuncs.c	1.4 6/14/89";
#endif
/*
	Copyright 1989 Alan Shaw and Eric Schwartz.
	No part of this software may be distributed or sold without the prior
	agreement of Prof. Eric Schwartz, Dept. of Psychiatry, NYU School of
	Medicine, 550 1st Ave., New York, New York, 10016.
colorfuncs.c
*/

#include	<math.h>
#include	<sys/types.h>
#include	<suntool/sunview.h>

#include	<hipl_format.h>

extern Pixwin		*imgpw, *bitpw, *lutpw;
extern int		BackRed, BackGreen, BackBlue;
extern struct header	hd;
extern int		lthresh, uthresh;
extern double		exponent;

static u_char	red[256], green[256], blue[256];
static u_char	Ored[256], Ogreen[256], Oblue[256]; /* original map */
static double	Lred[256], Lgreen[256], Lblue[256];	/* linear grayscale map;
				this is the same as red, green, blue
				if no lut sweep is in effect */
static int	low = 0, high = 255;	/* when stretchfunc is called, the
					range of Lred, Lgreen, Lblue is
					narrowed.  These variables keep
					track of that. */
static u_char	*Red, *Green, *Blue; /* pointers to current colormap */

setupcolormap(colors)
u_char	*colors;
{
register u_char	*inred, *outred, *Outred, *ingreen, *outgreen, *Outgreen,
		*inblue, *outblue, *Outblue;
register	i;
int		pid;
char		name[50];

	inred	= colors;	outred	= red;		Outred = Ored;
	ingreen	= colors + 256;	outgreen = green;	Outgreen = Ogreen;
	inblue	= colors + 512;	outblue	= blue;		Outblue = Oblue;

	for (i = 0; i < 256; i++)
		*Outred++ = *outred++ = *inred++,
		*Outgreen++ = *outgreen++ = *ingreen++,
		*Outblue++ = *outblue++ = *inblue++;

	pid = getpid();
	sprintf(name, "colormap%d", pid);

	Red = red, Green = green, Blue = blue;

	pw_setcmsname(imgpw, name);
	pw_putcolormap(imgpw, 0, 256, red, green, blue);
	pw_setcmsname(bitpw, name);
	pw_putcolormap(bitpw, 0, 256, red, green, blue);
	pw_setcmsname(lutpw, name);
	pw_putcolormap(lutpw, 0, 256, red, green, blue);
}

setupfullgraycolormap()
{
#define CMS_FULLGRAY                "fullgray_zero_red"
register	i;
int		pid;
char		name[50];
    
	for (i = 0; i < 256; i++)
		Lred[i] = Lgreen[i] = Lblue[i] = Ored[i] = Ogreen[i] = Oblue[i]
				= red[i] = green[i] = blue[i] = i;

	Ored[0]		= red[0]	= BackRed;
	Ogreen[0]	= green[0]	= BackGreen;
	Oblue[0]	= blue[0]	= BackBlue;

	pid = getpid();
	sprintf(name, "%s%d", CMS_FULLGRAY, pid);

	Red = red, Green = green, Blue = blue;

	pw_setcmsname(imgpw, name);
	pw_putcolormap(imgpw, 0, 256, red, green, blue);
	pw_setcmsname(bitpw, name);
	pw_putcolormap(bitpw, 0, 256, red, green, blue);
	pw_setcmsname(lutpw, name);
	pw_putcolormap(lutpw, 0, 256, red, green, blue);
}

#define luminance(x)	green[x]
/* this will work as long as colors are restricted to grays */

restore_lut()
{
register	i;

	erase_lutgraph();

	for (i = 0; i < 256; i++) {
		Lred[i] = red[i] = Ored[i], Lgreen[i] = green[i] = Ogreen[i],
					Lblue[i] = blue[i] = Oblue[i];
	}

	low = 0, high = 255;

	draw_lutgraph();
	putluts();
}

powerfunc()
{
double		scale;
register	i;
double		ftemp[256];

	erase_lutgraph();

	for (i = low; i <= high; i++)
		ftemp[i] = pow((double)(i - low)/(double)(high - low),
								exponent);
	scale = (ftemp[high] > ftemp[low]) ? 255 : (double)(255)/ftemp[1];
	for (i = lthresh; i <= uthresh; i++)
		red[i] = green[i] = blue[i] = Lred[i] = Lgreen[i] = Lblue[i]
						= ftemp[i] * scale + 0.5;
	red[0]		= BackRed;
	green[0]	= BackGreen;
	blue[0]		= BackBlue;

	draw_lutgraph();

	putluts();
}

powerpoint(x, y)
int	x, y;
{
double	ftemp;

	exponent = 1.;

	if (x > high
	||  x < low)
		return;

	if (y < 255 * (x - lthresh) / (uthresh - lthresh))
		while (ftemp = 255 * pow((double)(x - low)/(double)(high - low),
								exponent) > y)
		exponent += 0.02;
	else
		while (ftemp = 255 * pow((double)(x - low)/(double)(high - low),
								exponent) < y)
		exponent -= 0.02;

	powerfunc();
}

stretchfunc()
{
register	i;
register double	value, increment;

	erase_lutgraph();

	value = 0;
	increment = ((double) 255)/(uthresh - lthresh);
	for (i = 0; i < lthresh; i++)
		Lred[i] = BackRed, Lgreen[i] = BackGreen, Lblue[i] = BackBlue;
	for (i = lthresh; i <= uthresh; i++) {
		Lred[i] = Lgreen[i] = Lblue[i]
				= red[i] = green[i] = blue[i] = value + 0.5;
		value += increment;
	}
	for (i = uthresh + 1; i < 256; i++)
		Lred[i] = Lgreen[i] = Lblue[i] = 255;

	low = lthresh, high = uthresh;

	draw_lutgraph();

	putluts();
}

/*
stretchfunc(bias, scale)
int	bias;
double	scale;
{
register		i;
register double		CurrentValueF;
int			CurrentValueI;

	erase_lutgraph();

	CurrentValueF = -bias;
	red[0]		= BackRed;
	green[0]	= BackGreen;
	blue[0]		= BackBlue;

	High = 255;
	for (i = 1; i < 256; i++) {
		CurrentValueI = CurrentValueF + 0.5;
		if (CurrentValueI > 255) {
			nominal[i] = red[i] = green[i] = blue[i] = 255;
			if (i - 1 < High)
				High = i - 1;
		}
		else if (CurrentValueI <= 0) {
			nominal[i] = red[i] = green[i] = blue[i] = 0;
			Low = i + 1;
		}
		else {
			red[i] = green[i] = blue[i] = CurrentValueI;
			nominal[i] = CurrentValueF;
		}
		CurrentValueF += scale;
	}

	draw_lutgraph();

	putluts();
}
*/

sweepfunc(thresh)
int thresh;
{
	if (thresh > 254)
		return;
	pw_put(lutpw, thresh, 255 - luminance(thresh), 0);
	red[thresh]	= BackRed;
	green[thresh]	= BackGreen;
	blue[thresh]	= BackBlue;
	pw_put(lutpw, thresh, 255 - luminance(thresh), 255);

	putluts();
}

unsweepfunc(thresh)
{
	if (thresh < 1)
		return;
	pw_put(lutpw, thresh, 255 - luminance(thresh), 0);
	red[thresh]	= Lred[thresh];
	green[thresh]	= Lgreen[thresh];
	blue[thresh]	= Lblue[thresh];
	pw_put(lutpw, thresh, 255 - luminance(thresh), 255);

	putluts();
}

sweepfunc_down(thresh)
int thresh;
{
	if (thresh < 1)
		return;
	pw_put(lutpw, thresh, 255 - luminance(thresh), 0);
	red[thresh]	= BackRed;
	green[thresh]	= BackGreen;
	blue[thresh]	= BackBlue;
	pw_put(lutpw, thresh, 255 - luminance(thresh), 255);

	putluts();
}

unsweepfunc_down(thresh)
{
	if (thresh > 254)
		return;
	pw_put(lutpw, thresh, 255 - luminance(thresh), 0);
	red[thresh]	= Lred[thresh];
	green[thresh]	= Lgreen[thresh];
	blue[thresh]	= Lblue[thresh];
	pw_put(lutpw, thresh, 255 - luminance(thresh), 255);

	putluts();
}

ranluts_off()
{
	Red = red, Green = green, Blue = blue;
	putluts();
}

set_background(r, g, b)
int	r, g, b;
{
register	i;

	red[0] = BackRed = r, green[0] = BackGreen = g, blue[0] = BackBlue = b;
	for (i = 1; i < lthresh; i++)
		red[i] = BackRed, green[i] = BackGreen, blue[i] = BackBlue;
	for (i = uthresh; i < 255; i++)
		red[i] = BackRed, green[i] = BackGreen, blue[i] = BackBlue;
	putluts();
}

putluts()
{
	pw_putcolormap(imgpw, 0, 256, Red, Green, Blue);
	pw_putcolormap(bitpw, 0, 256, Red, Green, Blue);
	pw_putcolormap(lutpw, 0, 256, Red, Green, Blue);
}

draw_lutgraph()
{
register	i;

	pw_batch_on(lutpw);
	for (i = 1; i < 256; i ++)
		pw_put(lutpw, i, 255 - luminance(i), 255);
	pw_batch_off(lutpw);
}

erase_lutgraph()
{
register	i;

	pw_batch_on(lutpw);
	for (i = 1; i < 256; i ++)
		pw_put(lutpw, i, 255 - luminance(i), 0);
	pw_batch_off(lutpw);
}

output_lutfunc(buffer)
u_char *buffer;
{
register u_char	*ptr = buffer;
register int size = hd.ocols * hd.orows * hd.sizepix / (8 * sizeof(u_char));


	while (size--) {
		*ptr = luminance(*ptr);
		ptr++;
	}
}

static u_char		ranred[256], rangreen[256], ranblue[256];

ranluts()
{
H__RANDTYPE H__RANDOM();
register i;

	H__SRANDOM((int)time(0));
	for (i = 1; i < 255; i++) {
		ranred[i]	= (u_char)(H__RANDOM() % 256);
		rangreen[i]	= (u_char)(H__RANDOM() % 256);
		ranblue[i]	= (u_char)(H__RANDOM() % 256);
	}
	ranred[255] = rangreen[255] = ranblue[255] = (u_char)255;
							/* make cursor white */


	Red = ranred, Green = rangreen, Blue = ranblue;
	putluts();
}

h_boolean ranluts_are_on()
{
	if (Red == ranred)
		return(TRUE);
	return(FALSE);
}
