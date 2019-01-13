/*	HISTOGRAM . C
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

This software is copyright (C) by the Lawrence Berkeley Laboratory.
Permission is granted to reproduce this software for non-commercial
purposes provided that this notice is left intact.

It is acknowledged that the U.S. Government has rights to this software
under Contract DE-AC03-765F00098 between the U.S.  Department of Energy
and the University of California.

This software is provided as a professional and academic contribution
for joint exchange. Thus, it is experimental, and is provided ``as is'',
with no warranties of any kind whatsoever, no support, no promise of
updates, or printed documentation. By using this software, you
acknowledge that the Lawrence Berkeley Laboratory and Regents of the
University of California shall have no liability with respect to the
infringement of other copyrights by any part of this software.

For further information about this notice, contact William Johnston,
Bld. 50B, Rm. 2239, Lawrence Berkeley Laboratory, Berkeley, CA, 94720.
(wejohnston@lbl.gov)

For further information about this software, contact:
	Jin Guojun
	Bld. 50B, Rm. 2275, Lawrence Berkeley Laboratory, Berkeley, CA, 94720.
	g_jin@lbl.gov

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% AUTHOR:	Jin Guojun	4/1/91
*/

#include "function.h"

#define	HistoSize	MaxColors

/*==============================================*
*    computing the histogram, return maxcnt	*
*==============================================*/
histogram(bp, bsize, histp, img)
register byte	*bp;
int	bsize, *histp;
U_IMAGE	*img;
{
register int	chan, i=HistoSize, j=0, *hp=histp;
int	channels=img?img->dpy_channels:1;

for (chan=i*channels; chan--;)
	histp[chan] = j;
	if (channels==1 || img && img->color_form==CFM_SEPLANE)
	    for (chan=channels; chan--; bp+=i)
		for (j=bsize; j--; hp[*bp++]++);
	else if (img->color_form == CFM_ILL)	{
	    for (i=j; i<img->height; i++)
		for (chan=0; chan<channels; chan++)	{
			hp = histp + chan*HistoSize;
			for (j=img->width; j--; ++hp[*bp++]);
		}
	} else	for (j=bsize; j--;)	/* must be RAS	*/
		hp[*bp++]++,	(hp+i)[*bp++]++,	(hp+(i<<1))[*bp++]++;
    for (i=HistoSize*channels, hp=histp; i--;)	/* here j <= 0	*/
	if (j < hp[i])	j = hp[i];
#ifdef	_DEBUG_
if (DEBUG3OH)	dump_tbl(hp, HistoSize, 4, "hist");
#endif
return	j;
}

dump_tbl(lkt, n, cln, name)
register LKT*	lkt;
char	*name;
{
register int	i;
char	parameter[16];

if (cln>4)	i=2;
else	i=4;
sprintf(parameter, "%%%dd[%%0%dX]", 80/cln - i - 2, i);
for (i=0; i<n;){
	message(parameter, lkt[i], lkt[i]);
	if (!(++i % cln))	mesg("\n");
}
message("\n%d members in %s\n", i, name);
}

/*===============================================
%	to reset (linearize) look_up table	%
===============================================*/
void
ResetLKT(lkt, img)
register LKT	*lkt;
register U_IMAGE* img;
{
register int	i, entrances=MaxColors;
register Mregister *mmm = img->color_dpy ? img->marray+img->fn%img->channels :
		img->marray+img->fn;
	if (img->mid_type != HIPS || img->color_dpy)	/* any color display */
		for (i=entrances; i--;)	lkt[i] = i;
	else {
	register int	bottom = mmm->min;
		for (i=bottom; i<entrances; i++)
			lkt[i-bottom] = i; /* linearize lkt */
	}
}
