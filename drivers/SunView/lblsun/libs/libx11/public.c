/*	PUBLIC . C
#
%	Copyright (c)	Jin Guojun - All rights reserved
%
%		initial & run time public handlers (updating)
%	Including:
%		DisplayMessage(), DrawInImage(), Exposure_handler(),
%		PlaceArea(), ToImageStr(), XDeleteObject()
%		dumpColor(), SetPanelColor(), Set_Monitor()
%		HistoHandle(), Map_HistoWin(img), histochange()
%	superimpose_convert(), embed_texts(), embed_draws()
%	AdoptColor(), Annotation(), BuildImage(), LineScan()
%	RestoreArea(), WaitButtonPress_n_InfoImage()
%
% AUTHOR:	Jin Guojun - LBL	4/1/1991
*/

#ifndef	Cursor_Flushing
#define	Cursor_Flushing	15
#endif
#ifndef	OBJECT_FLUSHING_TIMES
#define	OBJECT_FLUSHING_TIMES	4
#define	OBJECT_FLUSHING_FREQ	5
#endif

#include "tuner.h"

bool	start_fast = 1;	/* how to get system color */
int	fnt_r, fnt_g, fnt_b;
InterpMap	*IM;	/* interpolation matrix */

#ifndef	HISTO_BACKGROUND
#define	HISTO_BACKGROUND	lightGray
#endif

/*=======================================================
%	positive cquire to quire the colormap		%
%	negtive cquire to quire user color table	%
=======================================================*/
void
dumpColor(Display *dpy, Colormap cmap, int cquire)
{
XColor	qcolor;
register int	i;

if (!cquire)	return;
msg("colormap = %d\n", cmap);
if (cquire>0)
    for (i=0; i < cquire; i++)	{
	qcolor.pixel = (u_long) i;
	XQueryColor(dpy, cmap, &qcolor);
	message("vct[%3d]:(%3u) red =%8X, green =%8X, blue =%8X\n", i,
		qcolor.pixel, qcolor.red, qcolor.green, qcolor.blue);
    }
else	for (i=0; i<-cquire; i++)
	message("gtable[%3d]:(%3u) red =%8x, green =%8x, blue =%8x\n", i,
		graylevel[i].pixel, graylevel[i].red,
		graylevel[i].green, graylevel[i].blue);
}

SetPanelColor(Display *dpy, Colormap cmap, XColor cp[], int depth, bool fast)
{
if (fast>1)	{
	Red = GetUserColor(cp, depth, 255, 0, 0);
	Green = GetUserColor(cp, depth, 0, 255, 0);
	Blue = GetUserColor(cp, depth, 0, 0, 255);
	Yellow = GetUserColor(cp, depth, 255, 255, 0);
} else {
	Red = GetCloseColor(dpy, cmap, depth, NULL, 255, 15, 15);
	Green = GetCloseColor(dpy, cmap, depth, NULL, 15, 255, 20);
	Blue = GetCloseColor(dpy, cmap, depth, NULL, 15, 15, 255);
	Yellow = GetCloseColor(dpy, cmap, depth, NULL, 255, 255, 20);
}
if (Visible==darkGray) {
	Visible = GetGray(dpy, cmap, depth, 144);
	darkGray = GetGray(dpy, cmap, depth, 80);
	lightGray = GetGray(dpy, cmap, depth, 160);
	Gray = GetGray(dpy, cmap, depth, 192);
}

Light = GetUserGray(cp, depth, 224);
MGray = GetGray(dpy, cmap, depth, 96);

if (verbose)
	fprintf(stderr, "[%d] greylevels %d\n\
	W=%d, Blk=%d, MG=%u LG=%u Gy=%u, Lt=%u; Y=%X, R=%X, G=%X, B=%X\n",
	cmap, ncolors, White, Black, MGray, lightGray, Light, Gray,
	Yellow, Red, Green, Blue);
}

/*=======================================================================
%  End of each line is always cut at a spcae except the word length	%
%	exceeding line width, then a hyphen is added at broken place	%
=======================================================================*/
void
DisplayMessage(Panel *mw, char *msgsp, int sp_limit, int refresh)
{
int	nl;
register int	i, ll, sp, sl=mw->width/mw->font_w-2, l, mp;
char	*sbuf = zalloc(sizeof(*sbuf), sl+2, "sbuf");
static	char*	msgp;
static	int	resp;

if (!refresh)
	TopWindow(mw, No, 1),
	resp = sp_limit,
	msgp = msgsp;	/* save current string */
else if (!msgsp) 	/* use saved string */
	msgsp = msgp,
	sp_limit = resp;

	if (!(l = strlen(msgsp)))	return;	/* my not need	*/
	for (i=mp=ll=0; mp < l; mp += ++nl) {
		nl = ((int)strchr(msgsp+mp, '\n') - (int)msgsp) - mp;
		if (nl<0 || nl>l)
			nl = l - mp;
		if (nl > sl)	ll += nl / sl;
		ll++;
	}
	i = mw->height / mw->font_h;
	if (ll > i)	ll = i;
	sp = (i - ll - 1) * mw->font_h / ll;
	if (sp_limit && sp > sp_limit)
		sp = sp_limit;
	if (sp > (mw->height>>2))
		sp = mw->font_h;
	XSetForeground(mw->dpy, mw->gc, white1);
	for (i=mp=0; i<ll; i++)	{
		nl = ((int)strchr(msgsp+mp, '\n') - (int)msgsp) - mp + 1;
		if (i+1==ll) {
			l = strlen(msgsp+mp);
			if (l > sl)	l = sl,	ll++;
		}
		else	if (nl<0 || nl>sl)	l = sl;
			else	l = nl;
		strncpy(sbuf, msgsp+mp, l);	sbuf[l] = 0;
		if (i+1 != ll && (strchr(sbuf, ' ') || strchr(sbuf, '\n')))
		    while (sbuf[l] != ' ' && sbuf[l] != '\n')	l--;
		if (l)
		    XDrawString(mw->dpy, mw->win, mw->gc,
			mw->font_w, (i+1)*(mw->font_h+sp), msgsp+mp, l);
		if (msgsp[mp+l] && msgsp[mp+l] != ' ' && msgsp[mp+l] != '\n')
		    XDrawString(mw->dpy, mw->win, mw->gc,
			mw->font_w*(l+1), (i+1)*(mw->font_h+sp), "-", 1);
		else	++l;
		mp += l;
	}
	CFREE(sbuf);
XFlush(mw->dpy);
}

void
MakeTrueColorPanel()
{
	black1 = 0;
	white1 = 0xFFFFFF;
	gray1 = 0x808080;
	Red = 0x00FF;
	Green = 0x00FF00;
	Blue = 0xFF0000;
	Yellow = 0x00FFFF;
	Visible = 0x909090;
	darkGray = 0x505050;
	lightGray = 0xA0A0A0;
	Gray = 0xC0C0C0;
	Light = 0xE0E0E0;
	MGray = 0x606060;
}

void
Set_Monitor(WinAttribute *mnt, Display	*dpy, Display *dpy1, Colormap	cmap)
{
if (mnt == NULL)
	mnt = &Monitor[0];
if (!dpy1)
	dpy1 = dpy;
mnt->dpy = dpy;
mnt->screen = XDefaultScreen(dpy);
if (!cmap || dpy == dpy1)
	mnt->cmap = DefaultColormap(dpy, mnt->screen);
else	mnt->cmap = cmap;
mnt->root = XDefaultRootWindow(dpy);
mnt->visual = XDefaultVisual(dpy, mnt->screen);
mnt->dpy_depth = XDefaultDepth(dpy, mnt->screen);

mnt++;
mnt->dpy = dpy1;
mnt->screen = XDefaultScreen(dpy1);
if (!cmap || dpy != dpy1)
	cmap = DefaultColormap(dpy1, mnt->screen);
mnt->cmap = cmap;
mnt->root = XDefaultRootWindow(dpy1);
mnt->visual = XDefaultVisual(dpy1, mnt->screen);
mnt->dpy_depth = XDefaultDepth(dpy1, mnt->screen);

mnt--;	/* always let Monitor[0] be color & Monitor[1] be mono	*/
if (mnt->dpy_depth < mnt[1].dpy_depth)	{	/* if not, swap	*/
register int	tmp;
	mnt->dpy = dpy1;
	mnt[1].dpy = dpy;
	tmp = mnt->screen;
	mnt->screen = mnt[1].screen;
	mnt[1].screen = tmp;
	mnt[1].cmap = mnt->cmap;
	mnt->cmap = cmap;
	tmp = (int) mnt->root;
	mnt->root = mnt[1].root;
	mnt[1].root = (Window)tmp;
	tmp = (int) mnt->visual;
	mnt->visual = mnt[1].visual;
	mnt[1].visual = (Visual*) tmp;
	tmp = mnt->dpy_depth;
	mnt->dpy_depth = mnt[1].dpy_depth;
	mnt[1].dpy_depth = tmp;
}
if (ncolors > 1<<mnt->dpy_depth)
	ncolors = 1<<mnt->dpy_depth;
}

BuildImage(imgf, name, w, h, mfrm, IType, OType, IBflag)
Image	**imgf;
char	*name;
{
	CreateImage(imgf, name, Monitor, w, h, StdIconWidth, I_Mask, IBflag);
	/*	I_Mask | LeaveWindowMask	*/
	format_init(*imgf, IType, OType, -1, Progname, "A5-2");
	XDefineCursor((*imgf)->dpy, (*imgf)->win, cursor);
	XSetWindowBackground((*imgf)->dpy, (*imgf)->win, MGray);
	if (mfrm)	{
		if ((*imgf)->marray)	CFREE((*imgf)->marray);
		(*imgf)->marray = ZALLOC(sizeof(Mregister), mfrm, "3m_array");
	}
	if ((*imgf)->data)	CFREE((*imgf)->data);
	(*imgf)->hist = (int*)NZALLOC(HistoSize, sizeof(*(*imgf)->hist), "hist");
	XMapWindow((*imgf)->dpy, (*imgf)->frame);
	XMapWindow((*imgf)->dpy, (*imgf)->win);

#ifdef	EXTENDED_COLOR
	if ((*imgf)->dpy_depth != 8)	{
	extern	int	log2_levels;
	register Image*	i = *imgf;
		if (OType==HIPS)
			i->mono_img = True;
		if (!i->visual_class)
			i->visual_class = -1;
		i->lvls = 1 << (log2_levels=8);
	/*	init_img_info(i, 0, OType, cmn_hd.color_dpy);	*/
		find_appropriate_visual(i);
		get_dither_colors(i);
		init_color(i);		/* build dither array here */
		choose_scanline_converter(i);
	}
#endif
}

HistoHandle(Image *img, HistoInfo *hinfo, int grid_color)
{
static	int	last_channels;

if (hinfo->map)	{
int	channel, grid, grids, bground, border, bar, frac_grid, maxcnt,
	*newhist=(int*)ZALLOC(HistoSize*sizeof(*newhist), img->channels, "newhist");
register int	i, j, k, image_size, image_frame, *hp;
	border = grid = grid_color;
	bground = HBGround;
	if (hinfo->neg) {
		border = grid = hinfo->neg - grid;
		bground = hinfo->neg - bground;
	}
	image_size=HistoSize, image_frame=HistoSize+(BFRAME<<1);
	if (hinfo->grids)
		grids = image_size / hinfo->grids;
	else	grids = image_size;
	frac_grid = image_size % grids;

    for (channel=0; channel<img->channels; channel++)	{
	{
	register LKT*	lktp = (LKT*)hinfo->lkt + channel * HistoSize;
	register int	*histp = hinfo->histp + channel * image_size;
	k = img->mmm.min;	j = img->mmm.max - k + 1;
	for (i=0, hp=newhist + channel*HistoSize; i<j; i++)
		hp[lktp[i]] += histp[i+k];
	}
	if (img->setscale | img->color_dpy)
		maxcnt = img->mmm.maxcnt;
	else	maxcnt = img->marray[img->fn].maxcnt;
	if (!maxcnt)	maxcnt = 8192;
#ifdef	_DEBUG_
	if (verbose>1)	dump_tbl(hp, image_size, 4, "hp");
#endif

	if (img->channels == 1)
		bar = Green;
	else switch (channel) {
		case 0:	bar = Red;	break;
		case 1:	bar = Green;	break;
		case 2:	bar = Blue;
		}

	if (img->dpy_depth == 8) {
	register byte*	bhp = (byte*)hinfo->his->img_buf +
				channel*image_frame*image_frame;
	    for (i=0; i<image_frame; i++)
		for (j=0; j<image_frame; j++)
		    if (!j || j>=image_frame-BFRAME ||
			!i || i>=image_frame-BFRAME)
			*bhp++ = border;
		    else if (!((i-frac_grid) % grids) || !(j % grids))
			*bhp++ = grid;
		    else
			*bhp++ = bground;
	    for (i=BFRAME; i<image_size+BFRAME; i++) {	/* horizontal process */
		bhp = (byte*)hinfo->his->img_buf + i + image_frame *
			(channel * image_frame + image_size);
		j = (image_size * hp[i-BFRAME] << hinfo->scale) / maxcnt;
		if (j > image_size)	j = image_size;

		for (k=0; k<j; k++) {	/* vertical process	*/
			*bhp = bar;
			bhp -= image_frame;
		}
	    }
	} else	{
	register int*	ihp = (int*)hinfo->his->img_buf +
				channel*image_frame*image_frame;
	    for (i=0; i<image_frame; i++)
		for (j=0; j<image_frame; j++)
		    if (!j || j>=image_frame-BFRAME ||
			!i || i>=image_frame-BFRAME)
			*ihp++ = border;
		    else if (!((i-frac_grid) % grids) || !(j % grids))
			*ihp++ = grid;
		    else
			*ihp++ = bground;
	    for (i=BFRAME; i<image_size+BFRAME; i++)	{
		ihp = (int*)hinfo->his->img_buf + i + image_frame *
			(channel * image_frame + image_size);
		j = (image_size * hp[i-BFRAME] << hinfo->scale) / maxcnt;
		if (j > image_size)	j = image_size;

		for (k=0; k<j; k++) {	/* vertical process	*/
			*ihp = bar;
			ihp -= image_frame;
		}
	    }
	}
    }
    CFREE(newhist);
    if (last_channels != img->channels)
	last_channels = img->channels,
	hinfo->his->resize_h = last_channels * 258,
	XResizeWindow(hinfo->his->dpy, hinfo->his->frame,
		image_frame, image_frame*img->channels);
    XPutImage(hinfo->his->dpy, hinfo->his->win, hinfo->his->gc, hinfo->his->image,
	0, 0, 0, 0, image_frame, image_frame*img->dpy_channels);
    if (hinfo->his->sub_img)
	DrawVMark(hinfo->his, 0, 0);
}
XFlush(hinfo->his->dpy);
hinfo->change=hinfo->map;
}

LineScan(register byte	*p, register byte *scan,
	register XColor	*color,
	register LKT*	lktp,
	register int	w, int min, int tc)
{
if (tc)	while(w--)
	scan[w] = color[dgt[lktp[(p[w] - min)&0xFF]]].pixel;
else	while(w--)
	scan[w] = dgt[lktp[(p[w] - min) & 0xFF]];
}

PickUpColor(Image* img, int *r, int *g, int *b)
{
byte	*p;
int	v=SetParameterWin(img, img->event, img->font_h, 0);

DisplayMessage(NoteWin,
	"move mouse to choose a color\nrelease button to pick it up", 0, 0);
    while (!ImageEvent(img, ButtonReleaseMask))
	if (ImageEvent(img, PointerMotionMask))
		ParameterWin(img, &histinfo, img->event->xbutton.x,
			img->event->xbutton.y, v, False);
	v = img->width;
	p = img->data + img->channels * v * (img->event->xbutton.y
#ifndef	SCROLLBAR_on_CANVAS
		+ img->y0) + img->x0
#else
		)
#endif
			+ img->event->xbutton.x;
    if (img->channels==1)	{
	v = *p;
	if (img->in_cmap)
		*r = img->in_cmap[0][v],
		*g = img->in_cmap[1][v],
		*b = img->in_cmap[2][v];
    }
    else	{
	*r = *p;
	*g = p[v];
	*b = p[v << 1];
	v = (*r * RED_to_GRAY + *g * GREEN_to_GRAY + *b * BLUE_to_GRAY) >> 8;
    }
HidingPanel(NoteWin);
return	v;
}

/*	ANNOTATE an image by given colors.	*/
void
RestoreArea(img, x, y, c, r, w, h)
Image	*img;
{
register int	x0 = (x += c * w),	y0 = (y += r * h);
	toREALxy(img, x0, y0);
	XPutImage(img->dpy, img->win, img->gc, img->image, x0, y0, x, y, w, h);
}

static	int	Acolor, Afont;

Annotation(Image *img, int *y0, KeySym	*keysym, XComposeStatus	*stat)
{
int	max_char, max_cinr, num_cols, num_rows, sp, x, y, len, fnt_w, fnt_h;
char	*buf = NZALLOC(32, 32, "annotate");
color_union	fnt_cu;
XEvent	*event = img->event;

	if (img->dpy_depth > 8)
		return	WaitOk(0, "no annotation on true color device", 0);
	Acolor = AdoptColor(img, fnt_r, fnt_g, fnt_b, precision);

    while (1)	{
	fnt_w = img->font_w,	fnt_h = img->font_h;
	fnt_cu.v.r = fnt_r;
	fnt_cu.v.g = fnt_g;
	fnt_cu.v.b = fnt_b;
	switch (WaitButtonPress_n_InfoImage(img, &histinfo, y0, arrow))	{
	case Button1:
	max_cinr = num_cols = num_rows = sp = 0;
	x = event->xbutton.x;	y = event->xbutton.y;
	max_char = (img->width - x) / fnt_w;

	while (1)	{
		FlushingCursor(img, Exposure_handler, &img, 1, x + num_cols*fnt_w,
			y+num_rows*fnt_h, fnt_w, 2, Cursor_Flushing, 1);
		if (!(len=XLookupString(event, buf+sp, 1024-sp, keysym, stat)))
			continue;

		buf[sp + len] = 0;
		if ((len=buf[sp]) == BS)	{
		    if (num_cols)	{
			sp--;	num_cols--;
		    } else if (num_rows)	{
			sp--;	num_rows--;
			while((len=sp - num_cols) && buf[len-1] != CR)
				num_cols++;
		    }	else	continue;
		    RestoreArea(img, x, y-fnt_h, num_cols, num_rows, fnt_w, fnt_h);
		}
		else	{
		    if (len == Esc || len == CTRL_Y || sp+1 >= 1024)
			break;
		    if (++num_cols == max_char || len == CR)	{
			if (buf[sp] != CR)	{
				buf[sp+1] = buf[sp];	/* split line	*/
				buf[sp] = CR;	/* and insert a CR	*/
			}
			num_rows++;
			num_cols = 0;
		    }
		    if (max_cinr < num_cols)
			max_cinr = num_cols;
		    if (len != CR)
			XSetForeground(img->dpy, img->gc, Acolor),
			XDrawString(img->dpy, img->win, img->gc,
			x+(num_cols-1)*fnt_w, y+num_rows*fnt_h, buf+sp, 1);
		    sp++;
		}
	}
	if (buf[sp] != CR)	num_rows++;
	if (len != Esc)
		superimpose_add_elem(img, fnt_cu.v, Acolor, buf, -1,
			y, x, num_rows, max_cinr);
	else	RestoreArea(img, x, y-fnt_h, 0, 0, fnt_w * max_cinr,
			fnt_h * num_rows + XStringBaseHigh);
	break;

	case Button2:
	if ((len=GetNew_Img_Font(img)) < 0)
		Afont = len;
	else	Acolor = len;
	break;

	case Button3:
	default:	CFREE(buf);	return;
	}
    }
}

WaitButtonPress_n_InfoImage(Image *img, HistoInfo *hinfo, int*	y0, Cursor cursor)
{
	/* clear previous button events	*/
	RemoveImageEvent(img, ButtonAction);

	*y0 = SetParameterWin(img, img->event, img->font_h, 0);
	XDefineCursor(img->dpy, img->win, cursor);
	while (!ImageEvent(img, ButtonPressMask))	{
		RemoveImageEvent(img, ButtonReleaseMask);
		if (ImageEvent(img, PointerMotionMask))
			ParameterWin(img, hinfo, img->event->xbutton.x,
				img->event->xbutton.y, *y0, False);
		while (ImageEvent(img, ExposureMask))
			Exposure_handler(img->event, img, No);
	}
	ClearParameterWin(img, *y0);
return	img->event->xbutton.button;
}


XDeleteObject(Image *img, int who)
{
TopWindow(img, No, No);
FlushObject(img, who, OBJECT_FLUSHING_TIMES, OBJECT_FLUSHING_FREQ);
if (YesOrNo("Delate It ?", img->sub_img=0))
	superimpose_delete_elem(img, who),
	XClearArea(img->dpy, img->win, img->sub_img_x, img->sub_img_y,
		img->sub_img_w, img->sub_img_h, Yes);
TopWindow(img, Exposure_handler, No);
}

reposition_image(img, x0, y0, w, h)
Image	*img;
{
int	rs=0;
	if (x0 < img->x0)
		img->x0 = x0,	rs++;
	else if (x0 + w > img->x0 + img->resize_w)
		img->x0 += x0 + w - img->resize_w,	rs++;
	if (y0 < img->y0)
		img->y0 = MAX(y0-1, 0),	/* no negative	*/	rs++;
	else if (y0 + h > img->y0 + img->resize_h)
		img->y0 += y0 + h - img->resize_h,	rs++;
	if (rs)	{
#ifdef	SCROLLBAR_on_CANVAS
		if (img->frame != img->win)
			XMoveWindow(img->dpy, img->win, -img->x0, -img->y0);
#endif
		TopWindow(img, Exposure_handler, No);
	/*	exposure_r(img, Draws, img->x0,img->y0,
				img->resize_w,img->resize_h, True);	*/
	}
}


PlaceArea(img, x, y, dx, dy)
register Image	*img;
register int	x, y, dx, dy;
{
register int	x0=x, y0=y;
	toRELATIVExy(img, x, y);
	XPutImage(img->dpy, img->win, img->gc, img->image,
			x0, y0, x, y, dx, dy);
/* #	ifdef	C_TUNER	*/
	if (img->color_dpy && img->refresh_pixmap)
		XCopyArea(img->dpy, img->win, img->refresh_pixmap,
			img->gc, x, y, dx, dy, x0, y0);
/* #	endif	*/
}

ToImageStr(img, cv, fcolor, sub_w, fh, x0, y0)
Image	*img;
color_channel	cv;
register int	fcolor;
{
	reposition_image(img, x0, y0, sub_w, fh);	/* for XGetImage */
	toRELATIVExy(img, x0, y0);
{
register int	h, y, xw, w=img->width;
XImage	*ximg = XGetImage(img->dpy, img->win, x0, y=y0, sub_w, h=fh,
		AllPlanes, img->image->format);
register byte	*px = (byte *) ximg->data;
byte	*p[3], *scan[3];
int	ff=get_iconsize(img, 0),	pxw=ximg->bytes_per_line;
long	mask24 = 0xFFFFFF;
if (img->dpy_depth > 8)	{
	pxw <<= 2;
	fcolor = (cv.b << 16) & (cv.g << 8) & cv.r;
	if (ImageByteOrder(img->dpy) != LSBFirst)
		mask24 <<= 8;
}
toREALxy(img, x0, y);

    if (img->dpy_channels==1)	{
	register byte	*pi = p[0] = img->data + y*w + x0;
	scan[0] = (img->color_dpy ? img->scan_data : (byte*)img->image->data)
		+ y*w + x0;
	for (; h--; p[0]=(pi+=w), px+=pxw, scan[0]+=w) {
	    for (xw=sub_w; xw--;)	{
		if (img->dpy_depth > 8)	{
			if (((int*)px)[xw] & mask24 != fcolor)
				continue;
		} else	if (px[xw] != fcolor)	continue;
		pi[xw] = fcolor;
	    }
	    if (img->color_dpy)
		Map_Scanline(img, p, scan, x0, y++, sub_w, ff);
	    else
		memcpy(scan[0], px, sub_w);
	}
    } else {
	p[0] = img->data + w*img->dpy_channels*y + x0;
	scan[2] = img->scan_data + w*(img->dpy_channels*y - 1) + x0;
	for (; h--; px+=pxw, p[0]+=w*img->dpy_channels)	{
		p[1] = p[0] + w;
		p[2] = p[1] + w;
		for (xw=sub_w; xw--;)	{
		    if (img->dpy_depth > 8)	{
			if (((int*)px)[xw] & mask24 != fcolor)
				continue;
		    } else if (px[xw] != fcolor)	continue;
			p[0][xw] = cv.r;
			p[1][xw] = cv.g;
			p[2][xw] = cv.b;
		}
/* #	ifdef	C_TUNER	*/
		if (img->color_dpy)	{
			scan[0] = scan[2] + w;
			scan[1] = scan[0] + w;
			scan[2] = scan[1] + w;
			Map_Scanline(img, p, scan, x0, y++, sub_w, ff);
		}
/* #	endif	*/
	}
    }
XDestroyImage(ximg);
}
PlaceArea(img, x0, y0, sub_w, fh);
img->update = True;
}


char	*DrawShape[]={" ", "Arc", "Line", "Rectangle"};

DrawInImage(Image* img, int *y0, char	*emsg, char *buf)
{
Cursor	dii_cursor=XCreateFontCursor(img->dpy, XC_crosshair);
int	b, lw=1, shape=DrawsLine, x, y,
	color = AdoptColor(img, fnt_r, fnt_g, fnt_b, precision);
color_channel	cc;
/*
	if (img->dpy_depth > 8) {
		WaitOk(0, "no draw for true color display", 0);
		return;
	}
*/
while(1)	{
	cc.r = fnt_r;
	cc.g = fnt_g;
	cc.b = fnt_b;
	XSetLineAttributes(img->dpy, img->gc, lw, 0, CapButt, 0);
	b = WaitButtonPress_n_InfoImage(img, &histinfo, y0, dii_cursor);
	RemoveImageEvent(img, ButtonAction)	XBell(img->dpy, 0);
	if (b == Button3)	break;
	if (b == Button2)	{
		shape = ++shape & 3;
		if (!shape)	{
			shape++;
			sprintf(buf + 4, "line width = %d", lw);
			InputFrom_Panel(MsgButton, buf, 16, 8, buf + 4,
					"width ", Green, No);
			sscanf(buf, "%d", &lw);
		}
		DisplayMessage(NoteWin, DrawShape[shape], 4, 0);
		continue;
	}

	*y0 = SetParameterWin(img, img->event, img->font_h, 0);
	TrackSubWin(img, &histinfo, x=img->event->xbutton.x,
		y=img->event->xbutton.y, shape, Button1Mask, *y0);
	DisplayMessage(NoteWin, "press MIDDLE button to comfrim", 4, 0);
	b = WaitButtonPress_n_InfoImage(img, &histinfo, y0, dii_cursor)
		== Button2;
	XSetForeground(img->dpy, img->gc, color);
	Draws(img, 0, !b, img->sub_img);
	if (b)	{	/* comfirm!	*/
		superimpose_add_elem(img, cc, color, lw, shape, 0, 0, CapButt, 0);
		img->update = True;
	}
	DisplayMessage(NoteWin, emsg, 4, img->sub_img=0);
}
XSetLineAttributes(img->dpy, img->gc, 0, 0, CapButt, 0);
}

AdoptColor(img, r, g, b, prec)
Image	*img;
{
register int	c = img->color_form==CFM_SCF ? CloseColor_in_Map
		(img->in_cmap, img->cmaplen, r, g, b, prec) :
		(RED_to_GRAY * r + GREEN_to_GRAY * g + BLUE_to_GRAY * b) >> 8;
	if (!img->color_dpy)
		c >= 2;
#ifndef	ORDERED_SYSTEM_COLOR_ENTRY
	if (!img->color_form)	/* == CFM_SGF. Lazy to pick from cmap	*/
		c = !c;	/* first 2 system entries are reversed */
#endif
	TopWindow(img, 	Exposure_handler, No);
TopWindow(img, Exposure_handler, No);
return	c;
}

embed_draws(register Image*	img, int id)
{
superimpose_elems*	si_ep = img->superimpose[0] + id;
register int	lw=si_ep->elem.draw.line_w,
	w=si_ep->w,	h=si_ep->h,
	x=si_ep->x0,	y=si_ep->y0;

	switch (si_ep->e_type)	{
	case DrawsLine:
		if (w < 0)	{	x += w;	w = -w;	}
		if (h < 0)	{	y += h;	h = -h;	}
		break;
	case DrawsArc:
		x -= w;		y -= h;
		h <<= 1;	w <<= 1;
/*		break;
	case DrawsRect:
		if (x > img->sub_img_x)
			x = img->sub_img_x;
		if (y > img->sub_img_y)
			y = img->sub_img_y;
*/	}
	x -= lw;	y -= lw;
	w += lw << 1;	h += lw << 1;
	bound_check(w, x, img->width);
	bound_check(h, y, img->height);
	ToImageStr(img, si_ep->v, si_ep->color, w, h, x, y);
}

embed_texts(img, x, y, fnt_w, fnt_h, color, cv)
Image	*img;
register int	x, y, fnt_w, fnt_h;
{
	if (fnt_w + x >= img->width)
		fnt_w = img->width - x - 1;
	if (fnt_h + y >= img->height)
		fnt_h = img->height - y - 1;
	ToImageStr(img, cv, color, fnt_w, fnt_h, x, y);
}

superimpose_convert(Image *img)
{
if (img->o_type != HIPS && img->superimpose)	{
int	t = DefaultScreen(img->dpy);
int	rw = MIN(img->width, DisplayWidth(img->dpy, t)) - 20,
	rh = MIN(img->height, DisplayHeight(img->dpy, t)) -25 ;

	if (img->resize_h < rh || img->resize_w < rw)	{
		XMoveResizeWindow(img->dpy, img->frame, 1, 1, rw, rh);
		while (!ImageEvent(img, StructureNotifyMask) ||
			img->event->xconfigure.window != img->frame);
		do	ResizeWindow(img, img->event);
		while (ImageEvent(img, StructureNotifyMask));
	}
	TopWindow(img, Exposure_handler, No);
	if (t=img->texts)	{
	    while (t--)	{
		register superimpose_elems* si_ep = img->superimpose[1] + t;
		embed_texts(img, si_ep->x0, si_ep->y0 - si_ep->elem.text.ascent,
			si_ep->elem.text.fw * si_ep->cols,
			si_ep->elem.text.fh * si_ep->rows,
			si_ep->color, si_ep->v);
		superimpose_delete_elem(img, ~t);
	    }
	    CFREE(img->superimpose[1]);
	}
	if (t=img->draws)	{
		while (t--)
			embed_draws(img, t);
		CFREE(img->superimpose[0]);
	}
	CFREE(img->superimpose);
	img->superimpose = NULL;
	img->texts = img->draws = 0;
}
}

/*=======================================================
%	public histo_handle for both color and b/w	%
=======================================================*/
void
histchange(Button *b, Image*	img, HistoInfo *histinfo)
{
register int	i;
switch (ButtonState(b)) {
case HistBgNeg:	{
	register byte	*hbp = histinfo->his->img_buf;
	i = histinfo->his->width * histinfo->his->height;
		for (; --i; hbp++)
			*hbp = -1 - *hbp;
	}
	XPutImage(histinfo->his->dpy, histinfo->his->win, histinfo->his->gc,
		histinfo->his->image, 0, 0,
		0, 0, histinfo->his->width, histinfo->his->height);
	histinfo->neg = ~histinfo->neg;
	break;
case HistBgGrid:
	if (histinfo->grids)
		histinfo->grids=0;
	else	histinfo->grids=5;	/* number of grids */
	HistoHandle(img, histinfo, HISTO_BACKGROUND);
}
}

Map_HistoWin(U_IMAGE	*img)
{
	if (histinfo.map)	{
		XUnmapWindow(histinfo.his->dpy, histinfo.his->frame);
		histinfo.map = histinfo.his->sub_img = 0;
	}
	else	{
		XMapWindow(histinfo.his->dpy, histinfo.his->frame);
#ifdef	SCROLLBAR_on_CANVAS
		XMapWindow(histinfo.his->dpy, histinfo.his->win);
#endif
		histinfo.map = !histinfo.change;
		HistoHandle(img, &histinfo, HISTO_BACKGROUND);
		histinfo.change=histinfo.map=1;
	}
}
