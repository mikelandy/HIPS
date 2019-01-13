/*	Quick_Sort . C
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
%
% AUTHOR:	Jin, Guojun - LBL	4/1/91
*/

#include "function.h"

QSCell	QSArray[MaxColors];	/* for quick sort */

void
QuickSort(MinQS, MaxQS, MaxSort, A)
register QSCell	*A;
{
register int	i, j;
register QSType	comp;
register QSCell	temp;
   i = MinQS;	j = MaxQS;
   comp = A[(MinQS + MaxQS) >> 1].value;
   do
   {	while (A[i].value > comp)	i++;
	while (comp > A[j].value)	j--;
	if (i==j)	goto INC;
	if (i < j)
	{	temp = A[i];	A[i] = A[j];	A[j] = temp;	/* swap */
INC :		if (i < MaxSort)	i++;
		if (j > 0)	j--;
	}
   }	while (i <= j);
   if (MinQS < j)	QuickSort(MinQS, j, MaxSort, A);
   if (i < MaxQS)	QuickSort(i, MaxQS, MaxSort, A);
}

void
ShowA(A, Size)
register QSCell	A[];	/* Print out the Array */
{
register int	i;
	for (i=0; i < Size;){
		message("%7d[%2X]", A[i].value, A[i].QsIndex);
		if (!(++i % 7)) mesg("\n");
	}
msg("\n%d sets sorted\n", i);
}

calibrate_colors(img)
U_IMAGE	*img;
{
MType	trans[MaxColors];
register	int	i;
register QSCell	*qp=QSArray;

{
register int	*hp=img->histp;

	for (i=img->cmaplen; i--;)	{
		qp[i].QsIndex = i;
		qp[i].value = hp[i];
	}
}
	i = img->cmaplen;
	QuickSort(0, i-1, i, qp);

	while (--i && !qp[i].value);
	img->cmaplen = ++i;

	img->cmap = NZALLOC(sizeof(img->cmap), 3, "pCalib");
	img->cmap[0] = NZALLOC(sizeof(*img->cmap), i, "cCalib");
{
register cmap_t	*r=img->cmap[0], *g=(img->cmap[1]=r+i), *b=(img->cmap[2]=g+i);
	while (i--)	{
		trans[qp[i].QsIndex] = i;
		r[i] = reg_cmap[0][qp[i].QsIndex];
		g[i] = reg_cmap[1][qp[i].QsIndex];
		b[i] = reg_cmap[2][qp[i].QsIndex];
	}
	CFREE(reg_cmap[0]);
	reg_cmap[0] = r;
	reg_cmap[1] = g;
	reg_cmap[2] = b;
}
{
register byte	*p = img->src;
	for (i=img->width * img->height; i--; p++)
		*p = trans[*p];
}
}
