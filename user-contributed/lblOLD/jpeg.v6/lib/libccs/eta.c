/*
%	ETA . C
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

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

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% AUTHOR:	Jin Guojun, LBL	- 1991
*/

#include "imagedef.h"

#define	HistoSize	256
#define	MaxColors	256
#ifndef	VSC_BASE
#define	VSC_BASE	1
#endif

/*==============================================*
*    computing the histogram, return maxvalue	*
*==============================================*/
histogram_calc(bp, bsize, hp)
register byte	*bp;
register int	bsize;
register int	*hp;
{
register int	i;
for (i=0; i<HistoSize; i++)
	hp[i] = 0;
while (bsize--)
	hp[*bp++]++;
for (i=HistoSize; --i & !hp[i];);
return	++i;
}


/*=======================================
*	maxout should be float point	*
*	maxdiff is max-min+1		*
*	the 1 is a cell for max.	*
*	lkt always start from 0.	*
*	foreground ELA start at 1	*
=======================================*/
eta_curve(lkt, vsc, maxdiff, type)
LKT	*lkt;
register float	vsc;
{
float	maxout=255.0;
register int	i;
register float	rel_val, scale;

register double	tmp;

	vsc = VSC_BASE*vsc / (maxdiff-1) + 1;	/* key place */
	if (vsc <= 0.)	vsc = VSC_BASE;
	if (vsc != 1.)	{
		for (i=tmp=maxdiff; i--;)	tmp = tmp*vsc + i;
		scale = maxout / tmp;
	}
	else	scale = maxout / ((maxdiff-1)*maxdiff >> 1);
#ifdef	_DEBUG_
	DEBUGMESSAGE("Scale=%f, C=%f, vsc=%f\n", scale, tmp, vsc);
#endif

	if (type)	/* foreground */
		for(i=0, rel_val=0; i<maxdiff; i++)
		{
		rel_val += i * (scale*=vsc);
		lkt[i] = rel_val;
		}
	else for (i=maxdiff, rel_val=maxout; i--;)
		{
		lkt[i] = rel_val;
		rel_val -= (maxdiff-i) * (scale*=vsc);
		}

	i = maxdiff - 1;
	if (*lkt > 255){
#	ifdef	_DEBUG_
		msg("lkt[0] = %d\n", *lkt);
#	endif
		*lkt = 255;
	}
	if (lkt[i] > 255)	lkt[i] = 255;
	else if (lkt[i] < 0)	lkt[i] = 0;
}
