/*	UPDATE . C
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
%	Run time handlers to update picture, and file handling
%	Including:
%		MapColor(), ResetORange(), SetColormap(),
%		FileAccess(), LoadImage(), HistoHandle().
%
% AUTHOR:	Jin, Guojun - LBL	4/1/1991
*/

#include "tuner.h"

#ifndef	TUNER_API_H
#include "ctrlpane.c"	/* use original control panel	*/
#endif

/*===============================================
%	to reset original image parameter	%
===============================================*/
void
ResetORange(img)  Image	*img;
{
register Mregister*	mmm;
	if (img->setscale)
		mmm = &(img->mmm);
	else if (img->color_dpy)
		mmm = img->marray + img->fn%img->channels;
	else	mmm = img->marray + img->fn;
	if (mmm->max < mmm->min  || mmm->max < VCTEntry)
		mmm->max = MaxColors-1;	/* never show small max value */
	SetSBarRPos(LSlider, img->linearlow=
#if	defined	DIRECT & !defined C_TUNER
		MAX(VCTEntry, mmm->min), 1);
#else
		mmm->min, 1);
#endif
	SetSBarRPos(LSlider, img->linearup=mmm->max, 2);
	if (slider==LSlider)
		DrawSlider(slider);
}


/* RETURNs:	(-1) newfile;	(n) reload n;	(0) save, or errors for load */

static	char	tbuf[128];

FileAccess(imgp, max_num, noi)	/* number of images in stack */
Image	*imgp[];
{
FILE*	fp;
char	info[128], errinfos[32];
register int	i=0;
bool	state=i;
struct	stat	statbuf;
#define	PCloseWin	"Not enough memory. Press 'c' to close some image"

if (ButtonState(FButton)==FileSave) {
    for (i=noi; i--;)
	if (imgp[i]->active)	break;
    if (i<0)	goto	FDone;		/* no active image */
    strcpy(tbuf, imgp[noi=i]->name);	/* noi point to active image */
    if (imgp[noi]->frames > 1 &&
	YesOrNo("Save ALL frames ? YES/NO\n(NO, save current frame)",
	imgp[noi]->font_h))	{
		imgp[noi]->save_all = imgp[noi]->frames;
    }	else	imgp[noi]->save_all = imgp[noi]->frames<=1;
}
FRedo:	DispInfo(Epanel, 0,
	"INPUT -- TAB to cat, BS to correct, ESC undo or TYPE new one",	Yellow);
if (state=TextLine(FButton, tbuf, sizeof(tbuf), 0, Exposure_handler, imgp, noi))
	switch (ButtonState(FButton))	{
	case FileLoad:	{
	char	sbuf[128];
		if (noi >= max_num)	{
			for (i=noi; i--;)
			    if (strcmp(tbuf, imgp[i]->name)==0)	break;
			if (i<0)	{
				XBell(Epanel->dpy, state=0);
				WaitOk(NULL, PCloseWin, 0);
				break;
			}
			gf_size = imgp[noi=i]->width*imgp[i]->height;
		}
		if (imgp[noi] && imgp[noi]->update)	{
			sprintf(info, "save %s ? y/n", imgp[noi]->name);
			i = (strlen(info)+2) * Epanel->font_w;
			DispInfo(Epanel, 0, info, white1);
			info[0] = 0;
			TextLine(FButton, info, sizeof(info), i,
				Exposure_handler, imgp, noi);
			if (info[0] != 'n')	{
				strcpy(sbuf, tbuf);
				ButtonState(FButton) = FileSave;
				FileAccess(imgp, max_num, noi+1);
				strcpy(tbuf, sbuf);
			}
		}
		sprintf(info, "Loading file %s", tbuf);
		DispInfo(Epanel, 0, info, Red);
#	ifdef	NO_POPEN
#	define	ZCAT_TMP	"/tmp/tuner.tmp" /* no getgid() for DOS	*/
#	else
#	define	ZCAT_TMP	NULL
#	endif
		state = (int)(fp=zreopen(tbuf, sbuf, ZCAT_TMP));
		if (state) {
		    if	(state=LoadImage(fp, imgp+noi, tbuf))
			if (state>0)	state = EOF;
			else /* < 0 */	state = noi;
		    else	sprintf(errinfos, "%s is wrong File", tbuf);
		}
		else	sprintf(errinfos, "File not found");
		if (!state)	{	/* bad or no file	*/
			DispInfo(Epanel, (strlen(info)+1)*Epanel->font_w,
				errinfos, Green);
			XBell(Epanel->dpy, state);
			WaitOk(NULL, errinfos, 0);
		}
		if (*((int*)sbuf))	/* ZCAT	*/
#	ifdef	NO_POPEN
			zclear(ZCAT_TMP);
#	else
			pclose(fp);	/* close it anyway!	*/
#	endif
	}	break;
	case FileSave:
		if (!stat(tbuf, &statbuf)) {
			TopWindow(Epanel, No, No);
			DrawPanel();	XFlush(Epanel->dpy);
			sprintf(info, "file %s exist! overwrite ? y/n", tbuf);
			DispInfo(Epanel, 0, info, White);
			i = strlen(info);
			info[0] = 0;
			TextLine(FButton, info, 2, ++i*Epanel->font_w,
				Exposure_handler, imgp, noi);
			if (info[0] == Esc)	break;
			if (info[0] != 'y')	goto	FRedo;
		}
		fp = freopen(tbuf, "wb", stdout);
		if (!fp) {
			DispInfo(Epanel, 256, "can't open", Green);
			XBell(Epanel->dpy, state=0);
			WaitOk(NULL, sys_errlist[errno], 0);
		}
		else {
		register Image* img=imgp[noi];
		    img->OUT_FP = fp;
		    superimpose_convert(img);
		    if (img->o_type != RLE) {
			char	*av[1];
			int	frames = img->frames,
				save_info = img->pixmap_failed;
			i = img->save_all;
			if (!i)	i++;
			img->frames = i;
			img->pxl_out = 1;
			img->o_form = IFMT_BYTE;
			((U_IMAGE*)img)->update_header = (*img->header_handle)
					(HEADER_TO, img, 0, No);
			sprintf(info, "writing %s", tbuf);
			DispInfo(Epanel, 0, info, White);
			i = ReadSlider(ESlider, 1);
			sprintf(info, "%s ETA -f%d, bf=%d\n",
				Progname, i, img->curve);
		/* only output a sub-image when img is a single frame image */
			av[0] = info;
			(*img->header_handle)(HEADER_WRITE, img,
						1, av, img->update);
			img->frames = frames;
			state=(*img->std_swif)(FI_SAVE_FILE, img, NULL,
				img->save_all ? 0 : img->fn);
			img->pixmap_failed = save_info;
		    }
		    else {
			bool	add_cmap=0, save_cur=0;
			if (img->dpy_channels==3 || !OsameI) {
				add_cmap = YesOrNo("apply screen color map ?", 0);
				if (img->channels == 3)/* don't save cmap */
					rle_dflt_hdr.ncmap = 0;
			}
			else if (img->cmaplen && img->in_cmap)
				regmap_to_rlemap(img->in_cmap, img->cmaplen,
						img->ncmap, &rle_dflt_hdr);
			save_cur = YesOrNo("save current setting ?", 0);
			/*	memcpy(&cmn_hd, img, sizeof(cmn_hd));	*/
			(*img->std_swif)(FI_DESC_ETA, img, info, cer);
			if (rle_dflt_hdr.comments)
				rle_dflt_hdr.comments[0] = str_save(img->desc);
			sprintf(info, "writing %s", tbuf);
			DispInfo(Epanel, 0, info, White);
			state = (*img->std_swif)(FI_SAVE_FILE, img, add_cmap,
					save_cur ? lkt : NULL);
		    }
		fclose(fp);
		}
		break;
	default:	DispInfo(Epanel, 0, "???", Green);
	}
FDone:
ButtonState(FButton)=FileLabel;
DrawButton(FButton);
return	state;
}


#ifdef	DIRECT

bool	frmchange;

#ifdef	C_TUNER

CreateColorTuner(dpy, dpy1, img, create_panel, map_panel, set_gray)
Display	*dpy, *dpy1;
Image	*img;
{
static	cct_set, cct;
if (!cct_set++)	{
	Set_Monitor(Monitor, dpy, dpy1, img->colormap);
	VCTEntry = GetVctEntry(Dpy, Screen, Monitor[0].cmap, set_gray);
	Light = GetGray(dpy, img->colormap, ncolors, 224);
}
if (create_panel && !cct++) {
extern XColor*	save_1st_cc;
	I_ED.copy = NZALLOC(sizeof(*I_ED.copy), 1, "I_copy");
#ifndef	TUNER_API_H
	map_panel = CreateTuner(img, 5, darkGray, map_panel);
#endif
	img->curve = ETALinear;
	save_1st_cc = graylevel;
}
return	create_panel;
}

eta_scan_map(img, eta_map)  Image	*img; bool eta_map;
{
int	i_factor=get_iconsize(img, 0);
byte	*mp[3], *srcp[3];
register int	i, l, my;

	for (l=img->height; l--;)	{
		mp[0] = SAVED_RLE_ROW(img, l);
		srcp[0] = eta_map ? mp[0] : ORIG_RLE_ROW(img, l);
		for (i=1; i<img->channels; i++)
			mp[i] = mp[i-1] + img->width,
			srcp[i] = srcp[i-1] + img->width;
		MapRGB(0, NULL, &img, srcp, mp, l, img->width, l, i_factor);
	}
}

MapColor(img, dp, fsize, colorp)  Image	*img; byte *dp; int fsize; register XColor *colorp /* start */;
{
XColor	TmpColor[MaxColors];
int	maxdiff=img->entries;
register int	i;
register LKT	*lktr=lkt, *lktg=lkt+MaxColors, *lktb=lkt+(MaxColors<<1);
register XColor	*tcp = TmpColor;

if (maxdiff > ncolors)
	maxdiff = ncolors;	/* limitation */
else if (maxdiff < 2)	return	prgmerr(0, "map 0 colors");

	memcpy(tcp, colorp, sizeof(*tcp)*maxdiff);
	for (i=0; i<maxdiff; i++)	{
		tcp[i].red = lktr[colorp[i].red>>8] << 8;
		tcp[i].green = lktg[colorp[i].green>>8] << 8;
		tcp[i].blue = lktb[colorp[i].blue>>8] << 8;
	}
XStoreColors(img->dpy, img->colormap, tcp, maxdiff);/* load colors to VCT	*/
XFlush(img->dpy);
#ifdef	_DEBUG_
dumpColor(img->dpy, img->colormap, cquire);
#endif
}

#else

/*=======================================================
%	important note:	Since lkt always invoked, so,	%
%	even reset has to put min to lkt[0], not 0.	%
=======================================================*/
MapColor(img, dp, fsize, colorp, start)  Image	*img; byte *dp; int fsize; register XColor *colorp; int start;
{
register int	i;
int	maxdiff = img->marray[img->fn].max - start + 1;

#ifdef	_DEBUG_
time_t	t0, t1;
time(&t0);
#endif

if (maxdiff < 1)
	maxdiff = 1;
/* assign data to image buffer */
img->image->data = (char *) dp;

if (img->sub_img) {
register int	j, min=img->marray[img->fn].min;
register LKT	*lktp=lkt;
register byte	*rp = dp + img->sub_img_y * img->width + img->sub_img_x;
    for (i=0; i<img->sub_img_h; i++)	{
	for (j=0; j<img->sub_img_w; j++)
		rp[j] = lktp[(rp[j] - min)&0xFF];
	XPutImage(img->dpy, img->win, img->gc, img->image,
		REALxy(img, img->sub_img_x, img->sub_img_y + i),
		img->sub_img_x, img->sub_img_y + i, img->sub_img_w, 1);
	rp += img->width;
    }
    img->update = True;
}
else if (frmchange)	{
	XPutImage(img->dpy, img->win, img->gc, img->image, 0, 0, 0, 0,
		img->width, img->height);
	frmchange=0;
#ifdef	_DEBUG_
	time(&t1);
	if (verbose)	message("time=%d, start=%d, end=%d\n", t1-t0, start, i);
#endif
}
else	{
int	max=maxdiff-1, shift = min_bits(max / ncolors);
register unsigned short	illuminance, scale = 65536 / lkt[max];
#ifndef	DIRECT_CUT_VCTEntry
	start = VCTEntry;	maxdiff = ncolors - 1;
#endif
    for (i=start; i<maxdiff+start; i++)	{
	int	n = i - start << shift;
	if (n >= max)	n = max;
#ifdef	DIRECT_REMAP_ENTRY
	colorp[i].pixel = (u_long) i;
#endif
	illuminance = scale * lkt[n];	/* set i to high byte	*/
	colorp[i].red = colorp[i].green = colorp[i].blue = illuminance;
	colorp[i].flags = DoAll;
    }
	/*	load colors to VCT	*/
    XStoreColors(img->dpy, img->colormap, colorp+start, maxdiff);
}
XFlush(img->dpy);
#ifdef	_DEBUG_
dumpColor(img->dpy, img->colormap, cquire);
#endif
}

#endif	C_TUNER and Back_to COMMON_DIRECT

Colormap
SetColormap(wa, colorp, ncolors, newmap, VcTolerance)  WinAttribute* wa; XColor* colorp;
	int ncolors; bool *newmap; int VcTolerance;
{
register int	i = XDisplayCells(wa->dpy, wa->screen);
Colormap	cmap = DefaultColormap(wa->dpy, wa->screen);

if (wa->dpy_depth < 24)	{
    if (ncolors > i)
	ncolors = i;

mknew:	message("NMap = %d\n", *newmap);
    if (*newmap)	VCTEntry = 24;
/* save first serveral entries for system */
    for (i=0; i < VCTEntry; i++)	{
	colorp[i].pixel = i;
	XQueryColor(wa->dpy, cmap, &colorp[i]);
    }

    if (*newmap)	{
	cmap = XCreateColormap(wa->dpy, wa->root, wa->visual, AllocAll);
	XStoreColors(wa->dpy, cmap, colorp, ncolors); /* load colors to VCT */
    }
    else	{
	u_long	pxls[MaxColors], plane_masks[8];
	i = VcTolerance;
	if (!XAllocColorCells(wa->dpy, cmap, 0, plane_masks, 0, pxls+i, ncolors)
	    && !XAllocColorCells(wa->dpy, cmap, 0, plane_masks, 0, pxls+i,
			ncolors - 4))	{	/* leave top 4 open	*/
		msg("can't alloc %d colors\n", ncolors);
		--*newmap;
		goto	mknew;
	}
	for (i += ncolors; i-- > VcTolerance;)
		colorp[i].pixel = pxls[i];
	XStoreColors(wa->dpy, cmap, colorp+VcTolerance, ncolors);
    }
    if (*newmap && ncolors > 254 && arrow)
	XRecolorCursor(wa->dpy, arrow, colorp+255, colorp+254);
    if (verbose)
	message("[%d] grey level %d\n", cmap, ncolors-VcTolerance);
    dumpColor(wa->dpy, cmap, cquire);
    XInstallColormap(wa->dpy, cmap);
}
return (wa->cmap=cmap);
}

#else	/* B/W regular routines */

Colormap
SetColormap(wa, colorp, ncolors, newmap, DoQuant)  WinAttribute* wa; XColor* colorp;
	int *ncolors; bool *newmap; bool DoQuant;
{
register	i=XDisplayCells(wa->dpy, wa->screen);
Colormap	newcmap, cmap = DefaultColormap(wa->dpy, wa->screen);

if (wa->dpy_depth < 24)	{
    if (*ncolors > i)
	*ncolors = i;

    if (DoQuant)	{
	newcmap = XCopyColormapAndFree(wa->dpy, cmap);
	XFreeColormap(wa->dpy, newcmap);
	*newmap = 0;
    }
#ifdef	OTHER_UPDATE_MODE
    if (*newmap){
	cmap = XCreateColormap(wa->dpy, wa->root, wa->visual,
		wa->dpy_depth<24 ? AllocAll : AllocNone);
#else
    if (*newmap && wa->dpy_depth < 24)	{
	cmap = XCreateColormap(wa->dpy, wa->root, wa->visual, AllocAll);
#endif
	XStoreColors(wa->dpy, cmap, colorp, *ncolors);/* load VCT colors */
	XInstallColormap(wa->dpy, cmap);
	if (arrow && *ncolors > 254)
	    XRecolorCursor(wa->dpy, arrow, colorp+255, colorp+254);
    } else {
	for (i=0; i < *ncolors; i++) {
	    if (!XAllocColor(wa->dpy, cmap, &colorp[i]))	{
		if((colorp[i].pixel=GetCloseColor(wa->dpy, cmap, *ncolors,
		    colorp[i].red>>8, colorp[i].green>>8, colorp[i].blue>>8))
			>0)	continue;
		if (*ncolors-i < *ncolors>>ToleranceFactor+1)	{
			for (i++; i<*ncolors; i++)
				colorp[i].pixel = colorp[i-1].pixel;
			break;
		}
	/* free newcmap and entries allocated in newcmap */
		newcmap = XCopyColormapAndFree(wa->dpy, cmap);
		XFreeColormap(wa->dpy, newcmap);
		*ncolors >>= 1+quant;
		*ncolors = CreateCLT(colorp, *ncolors, DoAll, quant, True, &histinfo);
		msg("try to make %d level color map\n", *ncolors);
		i = -1;	/* redo */
	    }
	}
    }
dumpColor(wa->dpy, cmap, cquire);
}
return (wa->cmap=cmap);
}

MapColor(img, rp, fsize)  Image	*img; register byte	*rp; register int	fsize;
{
register byte	*bp=img->img_buf;
XColor	*color = graylevel;
register int	i, min=img->marray[img->fn].min;
register LKT	*lktp=lkt;
int	w=img->width;
#ifdef	EXTENDED_COLOR
int i_fact = get_iconsize(img, 0);
byte	*scan =
# ifdef	USE_LAST_LINE
	bp + img->image->bytes_per_line*img->height - w;
# else
	NZALLOC(w, 1, "no8_scan");
# endif
#endif

if (img->sub_img)	{
register int	j;
    rp += img->sub_img_y * w + img->sub_img_x;
    bp += img->sub_img_y * w + img->sub_img_x;
    for (i=0; i<img->sub_img_h; i++)	{
	j = img->sub_img_w;
	if (img->dpy_depth==8)
	    while (j--)
		bp[j] = color[dgt[lktp[(rp[j] - min)&0xFF]]].pixel;
	else {
#ifdef	EXTENDED_COLOR
	LineScan(rp, scan, color, lktp, j, min, img->dpy_depth!=1);
	Map_Scanline(img, &scan, &scan, img->sub_img_x, img->sub_img_y + i,
		img->sub_img_w, i_fact);
#else
	goto	npseud;
#endif
	}
	XPutImage(img->dpy, img->win, img->gc, img->image,
		REALxy(img, img->sub_img_x, img->sub_img_y + i),
		img->sub_img_x, img->sub_img_y + i, img->sub_img_w, 1);
	bp += w;
	rp += w;
    }
} else	{
    if (img->dpy_depth == 8)
	for (i=0; i<fsize; i++)
		bp[i] = color[dgt[lktp[(rp[i] - min)&0xFF]]].pixel;
    else
#ifdef	EXTENDED_COLOR
	for (i=0; i<img->height; i++, rp+=w)	{
		LineScan(rp, scan, color, lktp, w, min, img->dpy_depth!=1);
		Map_Scanline(img, &scan, &scan, 0, i, w, i_fact);
	}
#else
npseud:	prgmerr(DEBUGANY, "system is not compiled with non-PSEUDO color");
#endif
	/*	Time Consuming Work	*/
	XPutImage(img->dpy, img->win, img->gc, img->image,
#ifdef	SCROLLBAR_on_CANVAS	/*	REALxy(img, 0,0)	*/
		0, 0,
#else
		img->x0, img->y0,
#endif
		0, 0, w, img->height);
}
#if	defined	EXTENDED_COLOR && !defined USE_LAST_LINE
	CFREE(scan);
#endif
}

#endif	DIRECT



#ifdef	C_TUNER

LoadImage(fp, imgp, name)  FILE	*fp; Image **imgp; char	*name;
{
register int	i;
register byte	*dp;
int	state = FileLoad;
image_information	*img = *imgp;

if (img)	{
	CFREE(img->scan_data);
	if (img->hist)	{
	register int*	hp=img->hist;
		for (i=HistoSize*img->img_channels; i--; hp[i] = 0);
	}
	DestroyColorImage(img);
	firstmap = 0;
	img->name = str_save(name);
	state = FileReLoad;
}
else	img = *imgp = ZALLOC(1, sizeof(*img), "load-img");

init_img_info(img, Dpy, RLE, cmn_hd.color_dpy);
img->IN_FP = fp;
if (((BUFFER*)fp)->flags == BUFFER_MAGIC)
	link_buffer(img, 0);
rw_set_id = NULL;	/* reset user color table */
init_img_flag(img);
img->name = name;

#ifdef	_DEBUG_
	message("marray=%u, img=%u\n", img->marray, img);
#endif
if ((i = get_pic(multi_hd=0, "", NULL, imgp, MGray)) < 0)
	return	0;

/*==============================*
*	it also sets gf_size	*
*==============================*/
if (!Find_min_max(img, histinfo.histp=img->hist, img->data, Yes, 0))
	return	state;	/* no tuner can be performed	*/
if (fButton)	{
	img->RGB = ButtonState(fButton) = ButtonSync;
	img->setscale = ButtonState(hButton)==HistScaleSet;
	img->update = ButtonState(DButton) =DataAnalys;
#ifdef	TUNER_API_H
	ResetORange(img);
#else
	ChangeSlider(img, &slider, ESlider, LSlider, heqButt, EButton);
	ResetORange(img);
	DrawPanel();
#endif
}
img->curve = ETALinear;
new_curve(lkt, &histinfo, img->marray, img->curve, 0, img, img->w*img->h);

message("%s is %d(r) x %d(c) x %d(n) -- %d\n",
	name, img->h, img->w, img->img_num, gf_size);

if (img->dpy_depth < 24)
    MapColor(img, img->scan_data, gf_size, graylevel
#ifndef	C_TUNER
		, MAX(VCTEntry, img->marray[img->RGB%3].min)
#endif
								);
HistoHandle(img, &histinfo, Yellow);
return	state;
}


#else	/* B-W handlers */

LoadImage(fp, imgf, name)  FILE*	fp; Image **imgf /* must be a double ptr. */; char	*name;
{
int	frames=(*imgf) ? (*imgf)->frames : 0, state=FileReLoad;
register int	i;
register Image	*img;

cursor = XCreateFontCursor(Dpy, XC_umbrella);

cmn_hd.IN_FP = fp;	cmn_hd.in_type = IMAGE_INIT_TYPE;
if ((*cmn_hd.header_handle)(HEADER_READ, &cmn_hd, 0, 0, False))
	return	False;

if (cmn_hd.in_form != IFMT_BYTE && !WaitOk(cmn_hd.frames & -8 ? AbortButt : 0,
	"images isn't in byte format", 0))	return	False;

cmn_hd.o_form = IFMT_BYTE;
cmn_hd.pxl_out = 1;

if (gf_size*frames != row*cln*frm)	{
	if (*imgf && (*imgf)->win) {
		DestroyImage(*imgf);
		state = FileReLoad;
	}
	else	state = FileLoad;
	BuildImage(imgf, name, cln, row, frames!=frm ? frm : 0,
		cmn_hd.in_type, HIPS, IBNeed);
	cmn_hd.data = NULL;
}
else	cmn_hd.data = (*imgf)->data;

img = *imgf;
histinfo.histp = img->hist;
img->colormap = Monitor[0].cmap;
img->channels = img->dpy_channels = 1;
img->history = cmn_hd.history;
cmn_hd.parts = img->parts;
cmn_hd.stack_num = img->stack_num;
memcpy(&img->superimpose, &cmn_hd.superimpose, 6 + 11*sizeof(int));
if (cmn_hd.desc)
	img->desc = str_save(cmn_hd.desc);
#ifdef	_DEBUG_
message("buf=%u, marray=%u, img=%u\n", img->data, img->marray, img);
#endif

gf_size = img->width * img->height;
cmn_hd.load_all = img->frames = frm;
(cmn_hd.std_swif)(FI_LOAD_FILE, &cmn_hd, 0, 0, False);
(*(*imgf)->r_close)(fp);
(*imgf)->data = cmn_hd.data;

/*===============================================================
%	compute the min, max, if allocating img->hist here,	%
%	scale Global hist to img->hist + HistoSize*i		%
%	after histogram return. (using for loop HistoSize times	%
%  for loop do checking first and finish 0 out with i = -1	%
===============================================================*/
for (i=img->frames; i--;)	{
register byte	*dp=img->data + i * gf_size;
	img->fn = i;
	Find_min_max(img, histinfo.histp, dp, Yes, True);
#ifdef	DIRECT
	{
# ifdef	DIRECT_CUT_VCTEntry	/* uniqure colormap	*/
	register int	j;
	for (j=0; j<gf_size; j++, dp++)
		if (*dp < VCTEntry)	*dp = VCTEntry;
# else	/* can be shared	*/
	register int	j=gf_size, shift = min_bits
		((img->marray[i].max - img->marray[i].min + 1)/ncolors),
		base=VCTEntry-(img->marray[i].min >> shift);
	register XColor	*colorp = graylevel;
	while (j--)	{
	register int	v = *dp;
		if ((v=colorp[(v >> shift) + base].pixel) < VCTEntry)
			v = VCTEntry;
		*dp++ = v;
	}
# endif
	}
#endif
}

img->curve = ETALinear;
img->setscale = ButtonState(hButton)==HistScaleSet;
img->update = ButtonState(DButton) = DataAnalys;
i = img->fn;	/* keep 0 all the way to the end */
img->mmm = img->marray[i];
#ifndef	TUNER_API_H
SetSBarRPos(LSlider, img->linearlow=img->mmm.min, 1);
SetSBarRPos(LSlider, img->linearup=img->mmm.max, 2);
DrawButton(DButton);
ChangeSlider(img, &slider, ESlider, LSlider, heqButt, EButton);
#endif
SetShowFramePos(img, fButton, i);
ResetORange(img);
new_curve(lkt, &histinfo, img->marray, img->curve, 0, img, gf_size);

message("%s is %d(r) x %d(c) x %d(f) -- %d [%d channels]\n",
	name, row, cln, frm, gf_size*frm, img->channels);

MapColor(img, img->data, gf_size
#ifdef	DIRECT
	, graylevel, MAX(VCTEntry, img->marray[i].min)
#endif
);

img->mmm = img->marray[i];
HistoHandle(img, &histinfo, lightGray);

/* select events to wait for. STRANGE ? this is not always useful  */
XSelectInput(img->dpy, img->win, I_Mask | KeyPressMask |
	StructureNotifyMask /*| ResizeRedirectMask*/);
XSelectInput(img->dpy, img->icon, ExposureMask | StructureNotifyMask);
return	state;
}

#endif	C_TUNER
