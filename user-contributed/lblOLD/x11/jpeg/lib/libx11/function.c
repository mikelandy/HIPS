/*	FUNCTION . C
#
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
%	This file includes Elastic Tuning Algorithm (ETA) (c). That maxout can
%	not exceed 255 is important since only BYTE format accepted by TUNER.C.
%	The real Elastic can handle byte, short, int and float format.
%	So for non-byte format, scale it to byte for analysizing and
%	apply Elastic to original format for final result.
%	For others using ETA, use eta_curve().
%
%	Quantization is useful while graylevel is less than 32.
%	The algorithm of quantization in this program is that sort the
%	histogram in frequency order and apply the elastic function to
%	corresponding index (grey level) to get those significant grey
%	levels and fill them to Pseudo-color table -- graylevel, and then,
%	fill as many regular stepped grey levels from top value to bottom
%	as possible (If not possible, then the closest value will be used).
%	The final step is to create digitizer table according to graylevel.
%
% Changes:	4-20-91
%	All functions are mapped by quantization table(dgt), The dgt is scaling
%	table (pseudo direct map table). The lkt is an enhancement table which
%	always use the first entry to store minimum pixel mapping. The
%	graylevel table is the real index table.
%		The histogram equalization function shift minimum entry to 0
%	so that multiple table index can correctly apply for every function.
%	This change makes whole system to be consistent and more reliable.
%	i.e. finally, the DIRECT version and the QUANTIZING version
%	all use 256 level entries. The DIRECT directly maps pixel values to
%	graylevel entries. The regular version uses dgt to map pixel values
%	to graylevel entries. Same thing has to be done on reset routine, but
%	not applied on color version. The color version must keep regular
%	linear table.
%	The HistoHandle() also added a segment program to use lkt to map hist,
%	which is a trick work like program in CreateCLT() and FillDGT().
%
%		7-12-91
%	Interpolation for all enhancement on main peak area.
%
%		7-1-1992
%	Add canvas in windows.
%
% AUTHOR:	Jin Guojun - LBL	4/1/91
*/

#include "function.h"

#ifndef	VSC_BASE
#define	VSC_BASE	.01
#endif

bool	cntz, top;	/* count zero, top value clipping */
int	topv, *dgt;
PColor	MGray, Light;
Panel	*Epanel;
Slider	*ESlider, *LSlider, *slider, *CSlider, *QSlider;
Button	*DButton, *EButton, *FButton, *fButton, *hButton, *QButton,
	*ZButton, *maxButt, *Interpolate;
PressButton	*RstButt, *heqButt, *rfsButt;
HistoInfo	histinfo;


/*===============================================
%	Initial and Display image information	%
%	on either top or bottom of the window	%
%	of a panel to which xbutton belongs.	%
===============================================*/
SetParameterWin(img, xbutton, Font_Height, top)
Image	*img;
XButtonEvent	*xbutton;
{
register int	py=img->frame==img->win ? 0 : img->y0;
	if (xbutton->y
#ifdef	SCROLLBAR_on_CANVAS
			- img->y0
#endif
				> (img->resize_h>>1) || top)
		img->tmp_offset = Font_Height+IIMargin;
	else	img->tmp_offset = -Font_Height-IIMargin,
		py =
#ifdef	SCROLLBAR_on_CANVAS
		py +
#endif
			img->resize_h + img->tmp_offset;
return	py;
}

/*===============================================================
%	draw a sub-window either on the top or buttom of a      %
%	window to display corresponding image information	%
===============================================================*/
void
ParameterWin(ii, hinf, x, y, y0, is_hist)
Image	*ii;
HistoInfo	*hinf;
{
int	fn=(ii->color_dpy ? ii->fn % ii->channels : ii->frames>1 ? ii->fn : 0),
	yv, pos, vor, vr, w=ii->width, h=ii->height, mi=ii->mmm.min;
char	buf[128];
byte	*rp = ii->data + (ii->color_dpy ? 0 : w*h*fn);
LKT*	lkt = hinf->lkt;

	if (x<0)	x=0;
	if (y<0)	y=0;
	if (x>w)	x=w;
	if (y>h)	y=h;

#ifdef	SCROLLBAR_on_CANVAS
#define	SoC	ii->x0 +
#define	SoX
#define	SoY
	if (!is_hist)	mi=ii->marray[fn].min;
#else
#define	SoC
#define	SoX	img->x0 +
#define	SoY	img->y0 +
	x += ii->x0;
	if (!is_hist)	y += ii->y0,	mi=ii->marray[fn].min;
#endif
	pos = (is_hist | !ii->color_dpy ? y :
		y * ii->channels + ii->fn % ii->channels) * w + x;
	if (is_hist)	{
		if (ii->color_dpy)
			yv = y % (HistoSize+(BFRAME<<1));
		else	yv = y;
		yv = (int)((1 - (float)(yv-BFRAME)/HistoSize) * ii->mmm.maxcnt)
			>> hinf->scale;
		while (x && --x >= HistoSize);
		vor = ii->image->data[pos] != HBGround;
		vr = ii->mark_x - 1;
	} else	{
		yv = y;
		vor = rp[pos];
		vr = lkt[vor - mi];
	}
	if (is_hist || ii->dpy_channels==1)
		sprintf(buf, "%d %d = %d [%d] {sub_image %d %d}", x, yv,
			vor, vr, ii->sub_img_w, ii->sub_img_h);
	else	{
	register int	vog=rp[pos + w], vob=rp[pos + (w<<1)];
		lkt += MaxColors;
		sprintf(buf, "%d %d = %d %d %d [%d %d %d] {sub_image %d %d}",
		x, yv, vor, vog, vob, vr, lkt[vog - mi],
		(lkt + MaxColors)[vob - mi], ii->sub_img_w, ii->sub_img_h);
	}
	XClearArea(ii->dpy, ii->win, SoC 0, y0, 0, abs(ii->tmp_offset), 0);
	XSetForeground(ii->dpy, ii->gc, White);
	XDrawString(ii->dpy, ii->win, ii->gc, SoC 20, y0+ii->ascent,
		buf, strlen(buf));
	XFlush(ii->dpy);
}

/*===============================================================
%	Crop a sub-window. If either height or width < 4	%
%	discard it. The recover routine must be associated	%
%	with exposure handle routine.				%
===============================================================*/
TrackSubWin(img, hinf, ox, oy, shp, cropButton, y0)
Image	*img;
HistoInfo	*hinf;
{
Window	parent, child;
int	pw, ph, cx, cy, key_button;
float	f = -img->mag_fact;

if (f < 0)	f = -1 / f;
ox = f * (SoX ox) + img->mag_x,
oy = f * (SoY oy) + img->mag_y;

while(1)	{
	ImageEvent(img, PointerMotionMask);
    if (XQueryPointer(img->dpy, img->win, &parent, &child, &pw, &ph, &cx, &cy,
		&key_button))	{
	if (!(key_button & cropButton))	break;

	if (img->sub_img)
		Draws(img, 0, 1, img->sub_img);
	cx = f * (SoX cx) + img->mag_x;
	cy = f * (SoY cy) + img->mag_y;
	if (cx < 0)	cx = 0;
	else if (cx > img->width)
		cx = img->width;
	if (cy < 0)	cy = 0;
	else if (cy > img->height)
		cy = img->height;
	pw = cx - ox;
	ph = cy - oy;
	if (pw < 0 && shp != DrawsLine)	{
		pw = -pw;
		img->sub_img_x = cx;
	}
	else	img->sub_img_x = ox;
	if (ph < 0 && shp != DrawsLine)	{
		ph = -ph;
		img->sub_img_y = cy;
	}
	else	img->sub_img_y = oy;
	img->sub_img_w = pw;
	img->sub_img_h = ph;
	img->sub_img = Draws(img, 0, 1, shp);
	ParameterWin(img, hinf, img->sub_img_x, img->sub_img_y, y0, 0);
    }
}
ClearParameterWin(img, y0);
return	img->sub_img;
}

VType*
map_1_to_3(src, dest, cmap, w, h)
register char	*src, *dest;
cmap_t	**cmap;
{
register int	r = h;
	if (!dest)	dest = NZALLOC(w*3, r, "1_to_3");
	while (r--) {
		snf_to_rle(dest, src, w, 8, cmap);
		dest += w*3;
		src += w;
	}
return	(VType*)(dest - h*w*3);
}

void
LinearQuantization(cp, totalevel, cflag, rev)
register XColor	*cp;
register int	totalevel, cflag;
{
register int	i, gray, scale = 65536 / totalevel;

    for (i=0; i<totalevel; i++)	{
	if (rev)
		gray = totalevel - i - 1;
	else	gray = i;
	cp[i].pixel = gray;	/* for newmap only */
	cp[i].red = cp[i].green = cp[i].blue = scale * gray;
	cp[i].flags = cflag;
    }
}

/*==========================================
	Create the Color Lookup Table.
==========================================*/
CreateCLT(cp, totalevel, cflag, DoQuant, rev, hinf)
XColor	*cp;
HistoInfo	*hinf;
{
register short	j;
register int	i;
int	new_colors;

if (DoQuant & 1)	{
Mregister	mmm;
int	qlkt[MaxColors], *histp=hinf->histp,
	*newhist=(int*)ZALLOC(MaxColors, sizeof(*newhist), "CltHist");
	for (i=0; i<MaxColors; i++)
		newhist[((LKT *)(hinf->lkt))[i]] += histp[i];
	for (i=0; i<MaxColors; i++)	{
		QSArray[i].value = newhist[i];
		QSArray[i].QsIndex = i;	/* grey level to index */
	}
	QuickSort(0, MaxColors-1, MaxColors, QSArray);
	for (i=0; i<MaxColors; i++)
		if (!QSArray[i].value)	break;
	if (DEBUGANY)	ShowA(QSArray, i);
	mmm.min = 0;	mmm.max = totalevel;
	new_curve(qlkt, hinf, &mmm, ETAQuant, i, 0, DoQuant>>1);
	new_colors = totalevel << 1; /* new level may have more close values */
	for (i=0; i<totalevel; i++) {	/* put important color at first. */
		dgt[i] = QSArray[qlkt[i]].QsIndex;
		j = dgt[i] << 8;	/* set to high byte */
		cp[i].red = cp[i].green = cp[i].blue = j;
		cp[i].flags = cflag;
#ifdef	TRICK_Quant
		newhist[qlkt[i]] = -1;	/* use newhist as flag registers */
	}
	for (j=0; j<totalevel<<1; j++){	/* put other colors */
		if (newhist[j] < 0)	continue;
		dgt[i] = QSArray[j].QsIndex;	/* save grey levels */
		cp[i].red=cp[i].green=cp[i].blue = dgt[i]<<8;
		cp[i++].flags = cflag;
	}
	for (i=0; i<new_colors; i++)	/* store grey levels */
		QSArray[i].value = dgt[i];
#else
	}
	LinearQuantization(cp+totalevel, totalevel, cflag, rev);
	for (i=0; i<totalevel; i++)	/* store grey levels */
		QSArray[i].value = dgt[i];
	for (; i<new_colors; i++)
		QSArray[i].value = cp[i].red >> 8;
#endif
	CFREE(newhist);
}
else{	LinearQuantization(cp, totalevel, cflag, rev);
	for (i=0; i<totalevel; i++)
		QSArray[i].value = cp[i].red >> 8;
	new_colors = totalevel;
	}
FillDGT(new_colors, dgt);
return	new_colors;
}

/*=======================================================================
%	After create ColorLookTable, graylevel.rgb>>8 = QSArray.value	%
%	1.	quicksort histogram in high frequency order		%
%	2.	using elastic function pick significant grey levels	%
%	3.	filling Pseudo-color table -- graylevel -- with result 2
%	4.	filling graylevel with less frequency grey levels	%
%	5.	3 & 4 save grey levels into QSArray[i].value		%
%	6.	filling Digitizer Table -- FillDGT			%
=======================================================================*/
FillDGT(n, dgtp)
register int	n, *dgtp;
{
register int	i, j;
	for (j=0; j<MaxColors; j++)
		dgtp[j] = 0;
	for (i=0; i<n; i++)	/* build index's index	*/
		dgtp[QSArray[i].value] = i;
	i = j-1;
	while (!dgtp[--j]);
	dgtp[i] = dgtp[j];
	do {
	    for (j=i; --j;)
		if (dgtp[j])	break;
	    if (i-j > 1) {
		register int	k;
		for (k=j+i >> 1; k>j; k--)
			dgtp[k] = dgtp[j];
		k = j+i >> 1;
		if (i-j & 1)	k++;
		for (;k<i; k++)
			dgtp[k] = dgtp[i];
	    }
	} while(i=j);	/* assignment */
#ifdef	_DEBUG_
	if (verbose>2)	dump_tbl(dgtp, MaxColors, 8, "dgt");
#endif
}


/*=======================================
*	maxout should be float point	*
*	maxdiff is max-min+1,		*
*	the 1 is a cell for boundary.	*
*	lkt always start from 0.	*
*	foreground ELA start at 1.	*
*	if type=Quant, hsize=QuantV.	*
=======================================*/
new_curve(lkt, hinf, mmm, type, Maxout, img, hsize)
LKT	*lkt;
HistoInfo	*hinf;
Mregister	*mmm;
int	Maxout;
Image	*img;
{
int	flr, *histp=hinf->histp, maxdiff=mmm->max - mmm->min + 1;
float	maxout;
register int	i;
register float	rel_val, scale, vsc;

if (img && img->color_dpy)	{
register int	boff=img->fn%3 * MaxColors;
	lkt += boff;
	histp += boff;
	if ((img->dpy_depth==24) | (img->entries<8) ||
		img->color_dpy && Maxout < 0)
		maxdiff = 256;
	else if (maxdiff>2)	/* unpacked bitmap */
		maxdiff = img->entries;
	else if (type != ETALinear)
			maxdiff <<= 1;
}
if (type != ETAQuant)	{
	flr = img->linearlow;
	Maxout = img->linearup;
}
else	flr = 1;

maxout = Maxout;

if (verbose || maxdiff<8)
    message("min=%d, max=%d, flr=%d, top=%.2f, maxdiff=%ld\n",
	mmm->min, mmm->max, flr, maxout, maxdiff);

if (type==ETALinear && maxdiff > 1)	{
	scale = (maxout - flr) / --maxdiff;
	for (i=maxdiff; i; i--)
		lkt[i] = i * scale + flr;
	i = maxdiff;	/* cut top	*/
}
else if (maxdiff < 3)
	return;
else if (type & ETAHistoEq) {
	/*===============================
	%	Histogram Equalization	%
	%	this is direct mapping	%
	%	regular vsc += hist[i]	%
	===============================*/
	float	h_avg = hsize;
	int	incr;
		if (!cntz)  h_avg -= histp[0];	/* take off 0's conuts */
		h_avg /= (maxout-flr);
		for (i=vsc=0; i<mmm->min; i++)
			vsc += histp[i];	/* very important and tricky */
		for (rel_val=flr, vsc=0, i=!cntz; i < maxdiff; i++) {
			vsc += histp[i+mmm->min];
			incr = vsc/h_avg;
			lkt[i] = rel_val + (incr>>1);
			rel_val += incr;
			vsc -= incr*h_avg;
		}
    }
else{	{
	register double	tmp;
		if (type & ETAQuant)
			vsc = hsize;	/* = ReadSlider(QSlider, 1);	*/
		else	vsc = img->curve==ETABackGD ? img->bgrd : img->fgrd;
		vsc = VSC_BASE*vsc/mmm->max + 1;
		if (vsc <= 0.)	vsc = VSC_BASE;
		if (vsc != 1.)	{
			for (i=tmp=maxdiff; i--;)	tmp = tmp*vsc + i;
			scale = (maxout-flr) / tmp;
		}
		else	scale = (maxout-flr) / ((maxdiff-1)*maxdiff >> 1);
#	ifdef	_DEBUG_
		if (verbose)
		    message("Scale=%f, C=%f, vsc=%f\n", scale, tmp, vsc);
#	endif
	}

	if (!(type & ~ETAQuant))	/* foreground */
		for(i=0, rel_val=flr; i<maxdiff; i++)
		{
		rel_val += i * (scale*=vsc);
		lkt[i] = rel_val;
		}
	else for (i=maxdiff, rel_val=Maxout; i--;)
		{
		lkt[i] = rel_val;
		rel_val -= (maxdiff-i) * (scale*=vsc);
		}
	i = maxdiff - 1;
	if (*lkt < flr)	{
#	ifdef	_DEBUG_
		msg("lkt[0] = %d\n", *lkt);
#	endif
		*lkt = flr;
	}
	else if (*lkt > 255)	{
#	ifdef	_DEBUG_
		msg("lkt[0] = %d\n", *lkt);
#	endif
		*lkt = 255;
	}
	if (lkt[i] > 255)	lkt[i] = 255;
	else if (lkt[i] < 0)	lkt[i] = 0;
    }
if (top)
    for (; lkt[i]>Maxout-top; i--)
	lkt[i] = topv;
if(verbose>1)	dump_tbl(lkt, maxdiff, 8, "lkt");
}

/*==============================================*
*	computing the min & max for integer	*
*	& trans float to integer for scaling	*
*==============================================*/
find_min_max(bufp, hist, mmm, r_width, r_height, width)
register byte	*bufp;
register int	*hist;
Mregister	*mmm;
{
register int	i, j=0;

for (i=HistoSize; i--;)
	hist[i] = j;
for (i=r_height; i--; bufp+=width)
    for (j=r_width; j--;)
	hist[bufp[j]]++;
for (i=0, j=HistoSize; hist[i]==0 && i<j; i++);
while (hist[--j]==0);
mmm->min = i;
mmm->max = j;
return	mmm->maxcnt = j - i + 1;
}

interpolation(in, out, x_regions, y_regions, imp, img, hinf)
byte	*in, *out;
InterpMap	*imp;
Image	*img;
HistoInfo	*hinf;
{
int	i, j, maxdiff, x, y, mx, my,
	rgn_h, rgn_w, xfrac, yfrac, regions, region;
InterpMap	*imu, *imd, *mlu, *mld, *mru, *mrd;
float	xinc, yinc, xfracinc, yfracinc, xrate, yrate,
	xcomp, ycomp;

rgn_h = img->height / y_regions;
rgn_w = img->width / x_regions;
xfrac = img->width - rgn_w * x_regions;
yfrac = img->height - rgn_h * y_regions;
regions = x_regions * y_regions;
hinf->histp = hinf->hist;

{
register byte	*tmpp=in;
for (i=region=maxdiff=0; i<y_regions; i++, tmpp+=(rgn_h-1)*img->width)
    for (j=0; j<x_regions; j++, region++, tmpp+=rgn_w)	{
    register int diff=find_min_max(tmpp, hinf->histp, imp+region, rgn_w, rgn_h,
			img->width);
	if (maxdiff < diff)
		maxdiff = diff;
	/* building new histogram table	*/
	new_curve(imp[region].lkt, hinf, imp+region, img->curve, 0, img,
		rgn_w * rgn_h);
    }
}
for (i=regions; i--;)
    if (imp[i].diff < maxdiff)	{
#ifdef	_DEBUG_
	message("lkt%d = %d\n", i, imp[i].diff);
#endif
	for (j=imp[i].diff; j<maxdiff; j++)
		imp[i].lkt[j] = imp[i].lkt[j-1];
    }

/* generate new histogram	*/
xinc = 1. / rgn_w;
yinc = 1. / rgn_h;
xfracinc = 1. / xfrac;
yfracinc = 1. / yfrac;
mx = x_regions;
for (i=0; i<y_regions; i++)	{
    yrate = 1.;
    my = i * mx;
    if (i+1==y_regions)
	mx = -mx;
    imu = imp + my;
    imd = imu + mx;
    for (y=0; y<rgn_h; y++, yrate-=yinc)	{
	ycomp = 1. - yrate;
	for (j=0; j<x_regions; j++){
	    xrate = 1.;
	    mlu = imu + j;
	    mld = imd + j;
	    if (j+1==x_regions)	{
		mru = mlu - 1;
		mrd = mld - 1;
	    }
	    else{
		mru = mlu + 1;
		mrd = mld + 1;
	    }
	    for (x=0; x<rgn_w; x++, xrate-=xinc){
	    register int	A, B, C, D, where = *in++;
		A = where - mlu->min;
		B = where - mld->min;
		C = where - mru->min;
		D = where - mrd->min;
		if (A < 0)	A = 0;
		if (B < 0)	B = 0;
		if (C < 0)	C = 0;
		if (D < 0)	D = 0;
		xcomp = 1. - xrate;
		*out++ = xrate * (mlu->lkt[A] * yrate + mld->lkt[B] * ycomp) +
			xcomp * (mru->lkt[C] * yrate + mrd->lkt[D] * ycomp);
	    }
	}
	if (xfrac){
	    xrate = 1.;
	    mlu = mru = imu + j-1;
	    mld = mrd = imd + j-1;
	    for (x=0; x<xfrac; x++, xrate-=xfracinc){
	    register int	where = *in++;
		xcomp = 1. - xrate;
		*out++ = xrate * (mlu->lkt[where - mlu->min] * yrate +
				mld->lkt[where - mld->min] * ycomp) +
			xcomp * (mru->lkt[where - mru->min] * yrate +
				mrd->lkt[where - mrd->min] * ycomp);
	    }
	}
    }
}
if (yfrac)
	for (j=img->width*yfrac; i--;)
		*out++ = *in++;
hinf->histp = img->hist;
}
