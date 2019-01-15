/*	COLOR_TO_Gray . C
%
% AUTHOR:	Jin Guojun - LBL	8/1/91
*/

#include "header.def"

#define	CNVT(x)		((x)*256 / 100)

#ifndef	cmap_t
#define	cmap_t	byte
#endif

/* weighting % -> fraction of full color */
int	RED_to_GRAY = CNVT(29),		/* 28% */
	GREEN_to_GRAY = CNVT(59),	/* 59% */
	BLUE_to_GRAY = CNVT(12);	/* 11% */
#define	work_v	register int	\
		red = RED_to_GRAY, green = GREEN_to_GRAY, blue = BLUE_to_GRAY


ilc_to_gray(out, rgb, n, alpha, map, Regular_T)
register byte	*out, *rgb;
register unsigned int	n;
bool	alpha;
cmap_t*	map[3];
{
work_v, v;

if(Regular_T)
    if (map)
	while (n--) {	/* may not work with 4 bit color images */
		if (alpha)	rgb++;	/* skip alpha channel */
		v = red*(map[0][*rgb++]);
		v += green*(map[1][*rgb++]);
		v += blue*(map[2][*rgb++]);
		*out++ = v>>8;
	}
    else while (n--) {
		if (alpha)	rgb++;
		v = red*(*rgb++);
		v += green*(*rgb++);
		v += blue*(*rgb++);
		*out++ = v>>8;
	}
else if (map)
    while (n--) {
	if (alpha)	rgb++;		/* skip alpha channel */
	v = blue*(map[0][*rgb++]);	/* may not work with 4 bit color images */
	v += green*(map[1][*rgb++]);
	v += red*(map[2][*rgb++]);
	*out++ = v>>8;
}
else while (n--) {
	if (alpha)	rgb++;
	v = blue*(*rgb++);
	v += green*(*rgb++);
	v += red*(*rgb++);
	*out++ = v>>8;
}
}


ill_to_gray(out, r, g, b, w)
register byte	*out, *r, *g, *b;
register unsigned int	w;
{
work_v;

while (w--)
	*out++ = (red*(*r++) + green*(*g++) + blue*(*b++)) >> 8;
}


map8_gray(out, rgb, n, map)
register byte	*out, *rgb;
register unsigned int	n;
cmap_t*	map[3];
{
work_v, v;

while (n--)	{
	v = red*(map[0][*rgb]);	/* may not work with 4 bit color images */
	v += green*(map[1][*rgb]);
	v += blue*(map[2][*rgb++]);
	*out++ = v >> 8;
}
}

sep_to_ilc(rgb, sep, len, fsize)
register char	*rgb, *sep;
{
if (len <= fsize)	{
register char	*g = sep + fsize, *b = g + fsize;
	while (len--)
		*rgb++ = *sep++,	*rgb++ = *g++,	*rgb++ = *b++;
}
return	len;
}
