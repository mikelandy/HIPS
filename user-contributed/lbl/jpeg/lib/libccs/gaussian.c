/* Gaussian . C
%
%	It creates an one-dimensional gaussian mask with standard deviation
%	"sigma" with mask size = "window.
%	The values of the mask are scaled to ensure the sum(maskarr[i]) = 1.0;
%	and the scaling factor is returned by function.
%	The "precision" argument (positive integer) determines
%	on how many intervals each entry of the mask is computed.
%	If precision=1, each mask-entry is computed by considering
%	only its mid-point; else "precision" equally spaced points
%	are computed for each array-entry.
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
%
% AUTHOR:	Jin Guojun - LBL	10/1/90
*/

#include <math.h>
#include "stdef.h"
#ifndef	Float
#define	Float	float
#endif

double
gauss_mask(sigma, window, maskarr, precision, prt_chan)
double	sigma;
Float	*maskarr;
FILE	*prt_chan;
{
register int	i=1;
int	j, il,
	nh = il = window >> i;
bool	even = !(window & 1);
double	Const, divisor, sum,
	estimate, deltax;
register double	x=1.;
#define	scale	sum

if (precision < i)
	message("gauss_mask:\n\
	The precision must be a positive integer, now is %d\n", precision=i);
if (!sigma)	prgmerr(0, "gauss_mask: zero sigma? is %f now\n", sigma=x);

Const	= x / (sigma * sqrt(2.0 * M_PI));
divisor	= -.5/ (sigma * sigma);
deltax	= x / precision;
x	= (deltax - x + even) / 2.;	/* aways <= 0	*/

for(i=nh, il-=even; i < window; i++,il--)	{
	for(estimate=0, j=precision; j--; x+=deltax)
		estimate += (Const * exp(x * x * divisor));
	maskarr[i] = maskarr[il] = estimate / precision;
}

if (prt_chan)	{
	fprintf(prt_chan, "   sigma=%lf, size=%d, precision=%d \n",
		sigma, window, precision);
	for(i=window; i--;)	fprintf(prt_chan, "    %f\n", maskarr[i]);
}
for (sum=0, i=window; i--;)	sum += maskarr[i];
scale = 1. / sum;
for(i=window; i--;)	maskarr[i] *= scale;
return	scale;
}
