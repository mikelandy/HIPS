/*	Find_Min_Max . C
%
%	Copyright (c)	see included header file
*/

#include "function.h"

CENR	cer[3];	/*	color editor register	*/

Find_3_min_max(img, histp, dp, nouse)
U_IMAGE	*img;
int	*histp;
byte	*dp;
{
register int	i, j, m, *hp;

if (img->color_form != CFM_SCF) {
    for (i=0; i<img->channels; i++)	{
	hp = histp + i*HistoSize;
	j = -1;
	while (!hp[++j]);
/*	img->marray[i].min = MAX(VCTEntry, j);	*/
	cer[i].lower = img->marray[i].min = j;
	m = 0;
	while (++j<HistoSize)
		if (m < hp[j])	m = hp[j];
	img->marray[i].maxcnt = m;
	while (!hp[--j]);
	cer[i].upper = img->marray[i].max = j;
	cer[i].curve = ETALinear;
    }
} else	{
register int	l=HistoSize, v, max_cnt=0;
	hp = histp;
	while (l--)
		if (hp[l] > max_cnt)
			max_cnt = hp[l];
	for (i=0; i<3; i++)	{
	    for (j=l=256, m=0; j--;) {
		v = reg_cmap[i][j];
		if (v < l)	l = v;
		if (v > m)	m = v;
	}
	cer[i].lower = img->marray[i].min = l;
	cer[i].upper = img->marray[i].max = m;
	cer[i].curve = ETALinear;
	img->marray[i].maxcnt = max_cnt;
    }
}
img->mmm = img->marray[0];
message("maxc1=%d, maxc2=%d, maxc3=%d\n",
	img->marray[0].maxcnt,img->marray[1].maxcnt,img->marray[2].maxcnt);
}


Find_min_max(img, histp, dp, histocalc, permanent)
U_IMAGE	*img;
int	*histp;
byte	*dp;
{
if (histp)	{
register int	*hp=histp, fsize=img->width * img->height, i = -1;
    if (!dp)	dp = (byte *)img->src + img->fn * fsize;
    if (histocalc)
	img->marray[img->fn].maxcnt = histogram(dp, fsize, histp, img);
    if (img->color_dpy)
	Find_3_min_max(img, histp, dp, No);
    else	{
	while (hp[++i]==0);
	img->mmm.min = i;
	fsize = HistoSize;
	while (hp[--fsize]==0);
	img->mmm.max = fsize;
	img->mmm.maxcnt = img->marray[img->fn].maxcnt;
	if (permanent)	{
		img->marray[img->fn].min = i;
		img->marray[img->fn].max = fsize;
	}
    }
    if (img->color_dpy)
	for (i=img->channels; i<3; i++)
		img->marray[i] = img->marray[0];
}
return	(int) histp;
}
