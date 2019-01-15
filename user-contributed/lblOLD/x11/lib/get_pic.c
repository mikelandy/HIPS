/*	GET_PIC . C
#
%	Read an image from the input device and display it.
%
%	SAVED_ROW points to img->scan_data; ORIG_ROW -> img->data (src)
%	dpy_channels and color_form control color conversion in CCS
%
%	Copyright (c)	Jin Guojun -	All rights reserved
%
% AUTHOR:	Jin Guojun - LBL	4/1/92
*/

#include "function.h"
#undef	FORK_CONCURRENCY_OK	/* no fork for get_pic	*/
#include "uthread.h"

bool	shared_cmap, multi_hd;	/* for multi_frame RLE	*/
int	OsameI, ncolors=64;	/* enough for eyes	*/
static	int	concur_io;	/* for thread param I/O	*/
static	u_thread_t	tid;
Cursor	c_umbrella;
extern	int	row_dpy_mode;

#ifdef	_DEBUG_
int	u_thr_mode;
static	struct	timeval	ts_g, te_g;
extern	struct	timeval	global_start_time;
#endif
#define	ch_dch	img->img_channels,img->dpy_channels

#ifndef	NO_CONCURRENCY

read_test(img)
image_information	*img;
{
#ifdef	_DEBUG_
struct	timeval	ts, te;	/* ? not ts_g & te_g	*/
	consum_time(&ts, &te, 0);
	message("\nthr_img = %X, addr = %X\t", img, &img);
#endif
concur_io = (*img->std_swif)(FI_LOAD_FILE, img, 0, concur_io) < 0 ? RLE_EOF : 0;
#ifdef	_DEBUG_
	consum_time(&ts, &te, "\n..	read\t");
	message("ch %d dch %d\n", ch_dch);
#endif
}
#endif

#ifndef	MINIMUM_COLORs
#define	MINIMUM_COLORs	32
#endif
#define ROW_STORAGE	16
#define	icf	img->color_form
#define	STILL_WIN	(!stlw || stlw==img)
#define	R_T(s)	(img->title=rle_getcom(s, &rle_dflt_hdr))
#define	intype	img->in_type

get_pic(cur_img, win_geom, stlw, img_info, img_win_bg)
char	*win_geom;
image_information	*stlw, **img_info;
{
#ifdef	NO_CONCURRENCY
register
#endif
image_information	*img=img_info[cur_img];
byte	*save_scan[3], *read_scan[3];
int	max_h, icon_f, next_y, end_y, row_mode=row_dpy_mode,
	/* sort_colors is a very complicated variable. It controls color
		conversion, color map allocation (for Pseudo or Gray).
	    It combines the following things:
		shared colormap			:	shared_cmap
		convert input to RGB output	:	OsameI
		still window			:	STILL_WIN
		available colormap entries	:	VCTEntry
			L.121	GetVCTEntry() sets temp. VCTEntry for CFM_SGF.
			for CFM_SCF, this needs to be done somewhere before
			select_color_form() called; or spcially	handled
			after header_handle() returned in this program.
		display device type	:	img->dpy_depth
		...
	    This is the auto-color configuration flag.
	*/
	sort_colors = STILL_WIN && shared_cmap && (cur_img || !tuner_flag &&
		(VCTEntry < 1 || VCTEntry+MINIMUM_COLORs > specified_levels)),
	saved_VCTEntry=VCTEntry;	/* always save it	*/
register int	i;

#ifdef	_DEBUG_
	if (!global_start_time.tv_sec)	INIT_PERFORM_TIMING();
#endif
	/*	Read setup info. from files	*/

	if ( (i = -(*img->eof)(img->IN_FP)) ||	(
#ifdef	PRE_READ_HEADER
		!stlw
#else
		(STILL_WIN || intype==RLE)
#endif					/* should be (OsameI & sort_colors) ? */
		&& (i=(*img->header_handle)
		(HEADER_READ, img, 0, multi_hd, OsameI | sort_colors)) < 0))
		return	i;
	if (img->pxl_in > 1)
		return	prgmerr(0, "A non-BYTE image bpp=%d", img->pxl_in);

	if (intype==RLE)	{
		if (!img->title && !R_T("title")
			&& !R_T("TITLE") && !R_T("image_title"))
			R_T("IMAGE_TITLE");
		max_h = rle_dflt_hdr.ymax;
	} else	max_h = img->h-1;

	if (img->image &&
	    (img->w+1 != img->image->width || img->h != img->image->height)) {
		XResizeWindow(img->dpy, img->window, img->w, img->h);
		if (img->pixmap)
			XFreePixmap(img->dpy, img->pixmap),
			img->pixmap = NULL;
		if (stlw == img)
			img->map_scanline = NULL,	stlw = NULL;
	}

	if (!stlw)	{	/* better after img->????_color set up	*/
		find_appropriate_visual(img);	/* colormap = default cmap */
	} else	img->RGB++;

	sort_colors &= img->dpy_depth < 24 && img->img_num < 2;
	if (icf == CFM_SGF)	/* re-set VCTEntry for EXS	*/
		GetVctEntry(img->dpy, DefaultScreen(img->dpy), img->colormap, No);
	/*
	* If we are going to display a true color image in 1 channel (-w),
	* we have to copy the data any way, so it into the same spot and map
	* it into a saved_data array with only one color per scanline;
	* this saves memory.	read_scan[] is raw data.
	* If the number of image channels is equal to which is displayed,
	* we will not allocate new memory for it, but we will move it
	* throughout the saved_data along with save_scan.
	*
	* save_scan[] is the pointer to the current line in img->scan_data
	* that mapped data are in.
	* If we can't malloc the memory for the save_scan[], or we are doing
	* flip_book, then malloc the memory is not needed.
	*/

	if ((!stlw || tuner_flag) && !img->scan_data &&
		!(img->scan_data=nzalloc(SAVED_RLE_ROW(img, img->h), 1, No)))
		prgmerr(tuner_flag, "alloc for %s scanline buffer", img->name);

	/*	not True when scan_data is not allocated.	*/
	if (!img->scan_data || tuner_flag)	{
		img->data = NZALLOC(img->w * img->h, img->img_channels, No);
		if (!img->scan_data)	img->scan_data = img->data;
	}
	if (! img->data)	img->data = img->scan_data;
	if (sort_colors | tuner_flag)
		histinfo.histp = img->histp = img->hist =
			NZALLOC(sizeof(*(img->hist)) * HistoSize,
				img->dpy_channels, "hist");

#ifdef	_DEBUG_
	CONSUMED_TIME("\nread hdr");
#endif
#ifdef	FREE_GET_RAW_DISPLAY
    if (! (row_mode=ccs_get_row_ok(img)) )	{
#else
    if (!row_mode)	{
#endif
	byte*	pchange = img->data;	/* multiple-frame is applied to HIPS and FITS */
#ifndef	NO_CONCURRENCY
#ifdef	_DEBUG_
	message("\nmain_img = %X, addr = %X\t", img, img);
	CONSUMED_TIME("\nbefore create thr");
	consum_time(&ts_g, &te_g, 0);
	CONSUMED_TIME("GTErr");
#endif
	i = intype==FITS || img->img_num > 1 && icf != CFM_SCF ?
		img->RGB : (OsameI | sort_colors);
	if (!(sort_colors | tuner_flag))	{
		concur_io = i;
		u_thread_create(&tid, No, read_test, img);
	} else
#endif
	{
		u_fork(NULL, (*img->std_swif)(FI_LOAD_FILE, img, 0, i),
			concur_io);
		if (concur_io < 1)	{
			VCTEntry = saved_VCTEntry;
			return	RLE_EOF;
		}
	}
#ifdef	_DEBUG_
	CONSUMED_TIME("create thr");
	if (sort_colors)	mesg("\nTo8 w/o thrd\n");
#endif
	if (OsameI & sort_colors && img->img_channels > 1)	{
		To_8(img, reg_cmap, 0, MaxColors);
		img->dpy_channels = img->img_channels;
		if (tuner_flag)	img->lvls = specified_levels;
	}
	if (img->mono_img && !tuner_flag && intype != RLE &&
		img->scan_data==0 || pchange != img->data)
		img->scan_data = img->data;

	Find_min_max(img, img->hist, img->data, True, True);
	if (sort_colors && reg_cmap[0] && icf==CFM_SCF)
		calibrate_colors(img);
    }

	i = intype != RLE;
	if (reg_cmap[0] && icf != CFM_SGF	/* MTD	*/
			&& (sort_colors | (i || icf != CFM_SCF)))
		regmap_to_rlemap(reg_cmap, img->cmaplen, img->cmaplen &&
			img->img_channels < 2 && (i || sort_colors) ? 3 : 0,
			&rle_dflt_hdr);

	if (img->img_channels < img->dpy_channels)
		prgmerr(0, "_get_pic: img_channels %d < dpy_channels %d\n", ch_dch);

	u_window_api(img, NULL, 0);
	/* passing NULL icon factor pointer won't make icon & map windows */
	i = BuildColorImage(img, stlw, win_geom,
			img_win_bg == USE_ROOT_WIN ? NULL : &icon_f);
	if (img_win_bg != USE_ROOT_WIN)
		XSetWindowBackground(img->dpy, img->window, img_win_bg);

#ifdef	_DEBUG_
	CONSUMED_TIME("\nBuildColorImage\t");
#endif
#ifndef	NO_CONCURRENCY
	if (!row_mode && !(sort_colors | tuner_flag))	{
		u_thread_join(tid, NULL);
# ifdef	_DEBUG_
		CONSUMED_TIME("\njoin\t\t");
		consum_time(&ts_g, &te_g, "\ntotal COMB\t");
		message("CH %d DCH %d\n", ch_dch);
# endif
	}
#endif
	VCTEntry = saved_VCTEntry;
	if (i != SUCCESS)
		return	i;

/*	For each scan line, read, save, dither, and display it.	*/
#define	ShowRows()	{	\
    XPutImage(img->dpy, img->pixmap?img->pixmap:img->window, img->gc,	\
	img->image, 0, next_y, 0, next_y, img->w, rows_buffered);	\
    if (img->pixmap) XCopyArea(img->dpy, img->pixmap, img->window, img->gc,\
		0, next_y, img->w, rows_buffered, 0, next_y);	}
{
register int	image_y, inc_y=1, rows_buffered;
	if (row_mode < 0)
		end_y = inc_y = row_mode;
	else	end_y = img->h;
	next_y = image_y = /* end_y */	inc_y<0 ? img->h + inc_y : 0;

    for (rows_buffered=0; image_y!=end_y; image_y+=inc_y)	{

	if (img->scan_data) {	/* move scan up one line */
		save_scan[0] = SAVED_RLE_ROW(img, image_y);
		for (i=1; i < img->img_channels; i++)
			save_scan[i] = save_scan[i-1] + img->w;
	}
	if (tuner_flag) {
	    read_scan[0] = ORIG_RLE_ROW(img, image_y);
	    for (i=1; i<img->img_channels; i++)
		read_scan[i] = read_scan[i-1] + img->w;
	} else
		read_scan[0] = save_scan[0],
		read_scan[1] = save_scan[1],
		read_scan[2] = save_scan[2];
	if (row_mode && (*img->std_swif)(FI_LOAD_ROW, img, image_y, 0) != img->w)
		return	EOF;
	MapRGB(cur_img, stlw, img_info, read_scan, save_scan,
		image_y, img->w, max_h-image_y, icon_f);

	if (++rows_buffered >= ROW_STORAGE || row_mode) {
		ShowRows();
		if (inc_y < 0)	next_y = image_y + inc_y;
		else	next_y += rows_buffered;
		rows_buffered = 0;
	}
    }
    if (tuner_flag & row_mode)
	Find_min_max(img, img->hist, img->data, True, True);
    if (rows_buffered++ > 0)	ShowRows();
}
    if (!stlw && img->icn_pixmap) {
	XPutImage(img->dpy, img->icn_pixmap, img->icn_gc, img->icn_image,
		  0, 0, 0, 0, img->icn_w, img->icn_h);
	XClearWindow(img->dpy, img->icn_window);
    }
    if (!c_umbrella)
    	XCreateFontCursor(img->dpy, XC_umbrella);
    set_window_cursor(img->window, c_umbrella);
    Draw_ImageScrollBars(img);
    superimpose_images(img, -1, 0, 0, img->w, max_h);

#ifdef	_DEBUG_
CONSUMED_TIME("\nrest\t");
mesg("\n");
TOTAL_CONSUMED(progname);
#endif
return	SUCCESS;
}
