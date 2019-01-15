/*	FILTER_Func . C
#
%	ETA Filter Generator for VFFT filters
%
%	Copyright (c)	Jin Guojun -	All Rights Reserved
%
% AUTHOR:	Jin Guojun - Lawrence Berkeley Laboratory	5/1/91
*/

#include "vfft_fil.h"


eta_f_curve(lkt, sc, maxdiff, neg, curve, flr, name, iform)
float*	lkt;
Filter	sc, flr;
char	*name;
{
register int	i;
Filter	rel_val, maxout=1.;
register Filter	scale, vsc = .01 * sc / maxdiff + 1;

if (maxdiff < 2)	{
	lkt[0] = 1.;
	return;
}
if (curve==CURVE_LINEAR)	{
	scale = (maxout-flr)/maxdiff;
	if (neg) for (i=maxdiff; i--;)
		lkt[i] = i * scale + flr;
	else for (i=maxdiff--; i--;)
		lkt[maxdiff-i] = i * scale + flr;
}
else{
	{
	register double	tmp;
	    if (vsc != 1.)	{
		for (i=tmp=maxdiff; i--;)	tmp = tmp*vsc + i;
		scale = (maxout-flr) / tmp;
	    }
	else	scale = (maxout-flr) / ((maxdiff-1)*maxdiff >> 1);
	}
	if (!neg)
	   if (!curve)
		for(i=maxdiff, rel_val=flr; i--;)
		{
		rel_val += (maxdiff-i) * (scale*=vsc);
		if (rel_val > maxout)	lkt[i]=maxout;
		else	lkt[i] = rel_val;
		}
	   else for (i=0, rel_val=maxout; i<maxdiff; i++)
		{
		lkt[i] = rel_val;
		rel_val -= i * (scale*=vsc);
		if (rel_val<flr && !iform)	rel_val=flr;
		}
	else if (!curve)
		for(i=0, rel_val=flr; i<maxdiff; i++)
		{
		rel_val += i * (scale*=vsc);
		lkt[i] = rel_val;
		}
	   else for (i=maxdiff, rel_val=maxout; i--;)
		{
		lkt[i] = rel_val;
		rel_val -= (maxdiff-i) * (scale*=vsc);
		}
	}
if (Msg)	{
	dump_lkt(lkt, maxdiff);
	msg("\n%d %s => sc=%f, scale=%f\n", maxdiff, name, sc, scale);
}
}

GTable(lp, max, hi_pass)
register float *lp;
{
register int i;
for (i=0; i<max; i++)
	printf("%d %f\n", hi_pass ? max-i : i, *lp++);
printf("\n\n");
}

dump_lkt(lktp, n)
register float	*lktp;
{
register int	i;
for (i=0; i<n;)	{
	message("%10.3f", lktp[i]);	if (!(++i & 7))	mesg("\n");	}
}
