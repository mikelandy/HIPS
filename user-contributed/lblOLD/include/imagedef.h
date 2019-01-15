/*	IMAGESDEF . H
#
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

This software is copyright (C) by the Lawrence Berkeley National Laboratory.
Permission is granted to reproduce this software for non-commercial
purposes provided that this notice is left intact.

It is acknowledged that the U.S. Government has rights to this software
under Contract DE-AC03-76SF00098 between the U.S.  Department of Energy
and the University of California.

This software is provided as a professional and academic contribution
for joint exchange. Thus, it is experimental, and is provided ``as is'',
with no warranties of any kind whatsoever, no support, no promise of
updates, or printed documentation. By using this software, you
acknowledge that the Lawrence Berkeley Laboratory and Regents of the
University of California shall have no liability with respect to the
infringement of other copyrights by any part of this software.

For further information about this notice, contact William Johnston,
Bld. 50B, Rm. 2239, Lawrence Berkeley National Laboratory, Berkeley, CA, 94720
(wejohnston@lbl.gov)

For further information about this software, contact:
	Jin Guojun
	Bld. 50B, Rm. 2239
	Lawrence Berkeley National Laboratory, Berkeley, CA, 94720
	g_jin@lbl.gov

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% AUTHOR:	Jin Guojun - LBL	4/1/91
*/


#ifndef	IMAGES_DEF
#define	IMAGES_DEF

#include "stdef.h"

#ifndef	io_test
extern	char	*io_test_msg[];
#define	io_test(io_fd, then_what)	{	\
	register int	iset = !isatty(io_fd);	\
	if (!iset) {	msg("device %s not ready\n", io_test_msg[io_fd]);\
		then_what; }	}
#endif	io_test

#ifdef	NonReliablePipe
#define	upread(buf, ibyte, fsize, fp)	\
	pread(fileno(fp), buf, ibyte*fsize) / ibyte
#else
#define	upread(buf, ibyte, fsize, fp)	\
	fread(buf, ibyte, fsize, fp)
#endif


#ifndef	PANEL_H

#ifndef MAX
# define MAX(A, B)	((A > B) ? A : B)
#endif
#ifndef MIN
# define MIN(A, B)	((A < B) ? A : B)
#endif
#ifndef ABS
# define ABS( A )	((A < 0) ? -(A) : A)
#endif
#ifndef	Sign
# define Sign(v)	((v < 0) ? -1 : v!=0)
#endif

typedef	struct	{
	int	min, max,
		maxcnt;
	} Mregister;

#endif	PANEL_H


#ifndef	cmap_t
#define	cmap_t	byte
#endif
#ifndef	sht_cmap_t
#define	sht_cmap_t	short
#endif
#ifndef	LKT
#define	LKT	float
#endif

typedef	struct	{
	cmap_t	r, g, b;
	} color_cell;

typedef	struct	{
	int	curve, lower, upper, bgrd, fgrd, intp;
	} CENR;

typedef	struct	{
	int	min, max, diff;	/* union with Mregister */
	LKT	*lkt;
	} InterpMap;

#ifndef	HIST_TITLE_STR
#define	HIST_TITLE_STR	"HIST01"
#endif
#ifndef	HELP_INFO
#define	HELP_INFO	"CTRL + button => panel"
#endif

#define	HEADER_READ	1
#define	HEADER_FREAD	2
#define	HEADER_WRITE	3
#define	HEADER_FWRITE	4
#define	HEADER_FROM	5
#define	HEADER_TO	6
#define	HEADER_TRANSF	7
#define	ADD_DESC	8

typedef	enum	{
	IT_HIPS =	1,
	IT_FITS,
	IT_RLE,
	IT_GIF,
	IT_PGM,
	IT_RAS,
	IT_TiFF,
	IT_PBM,
	IT_PPM,
	IT_PNM,
	IT_ICC,
	IT_COLOR_PS,	/* end regular type */
	IT_PICT,
	IT_JPEG,
	IT_XWD
} imagetype_t;

#define	HIPS	IT_HIPS	/* backward compatibility	*/
#define	FITS	IT_FITS
#define	RLE	IT_RLE
#define	GIF	IT_GIF
#define	PGM	IT_PGM
#define	RAS	IT_RAS
#define	TiFF	IT_TiFF
#define	PBM	IT_PBM
#define	PPM	IT_PPM
#define	PNM	IT_PNM
#define	ICC	IT_ICC
#define	COLOR_PS	IT_COLOR_PS
#define	PICT	IT_PICT
#define	JPEG	IT_JPEG
#define	XWD	IT_XWD
#define	ENDITYPE	XWD
#define	IName_List	"UNKNOWN", "HIPS", "FITS", "RLE", "GIF",	\
			"PGM",	"RAS",	"TIFF", "PBM", "PPM", "PNM",	\
		"ICC", "COLOR_PS", "PICT", "JPEG", "XWD",	"? END"

#ifndef	IMAGE_INIT_TYPE
#define	IMAGE_INIT_TYPE	HIPS
#endif

#define	FI_LOAD_FILE		1
#define	FI_LOAD_ROW		2
#define	FI_RLOAD_BUF		3
#define	FI_RESERVED		4
#define	FI_ACCESS_ABS_FRAME	5
#define	FI_SAVE_FILE		6
#define	FI_WHAT_FILE		7
#define	FI_DESC_ETA		8
#define	FI_GET_INFORMATION	9
#define	FI_HIPS_HEADER_FORMAT	10
#define	FI_INIT_NAME		11
#define	FI_PNM_MAXVAL		12

#include <ctype.h>
#ifndef	isfloat
#define	isfloat(c)	(isdigit(c) || c=='.' || c=='-' || c=='+' || toupper(c)=='E')
#endif

#ifndef	MType
# ifdef	IBMPC	/* for different Machine Memory allocating	*/
#   define	MType	long
# else
#   define	MType	int
# endif
#endif

#define	bound_check(v, b, t)	\
	if (b < 0)	b=0;	if (v+b > t)	v = t - b


#define	MASKFUNC_MAXABS		1
#define	MASKFUNC_MEANSQ		2
#define	MASKFUNC_SUMABS		3
#define	MASKFUNC_MAXX		4
#define	MASKFUNC_MAXFLR		5
#define	MASKFUNC_MAXSFLR	6
#define	MASKFUNC_MUL		7
#define	MASKFUNC_NORM		8
#define	MASKFUNC_DIFF		9
#define	MASKFUNC_ORIENT		10
#define	MASKFUNC_IDENT		11
#define	MAXMASKFUNCS	10
#define	MAXMASKSet	9

#ifndef	MaxMaskSZ
#define	MaxMaskSZ	9
#endif

typedef	struct	{
	char	*name, *history, *desc;
	int	m_func,
		m_sets,		/* # of mask sets (number of cubes) */
		n_mask,		/* # of masks in each set */
		m_size,		/* mask size	*/
		revs,		/* reverse all value in masks	*/
		bp_inc,
		norm_d,		/* normalization divisor */
		flag,		/* boundaries use	*/
		boundl, boundr,
		boundt, boundb,
		boundf, bound,
		minusd, plusd,
		*mval,		/* mask working place	*/
		**m_list,	/* can be any type. default is FP */
		val;
	} Mask_Win;

typedef	struct	{
	char	*name, *history, *desc;
	long	colormap;
	void	*dpy, *gc, *igc;
	long	frame, win, icon;
	void	*event;
	int	x0, y0, width, height,
		icon_width, icon_height,
		font_w, font_h, ascent;
	VType	*image, *icon_image;
	int	curve, linearlow, linearup, bgrd, fgrd,	/* elastic curve */
		scale, tmp_offset,	/* used for anything	*/
		mark_x, mark_y, resize_w, resize_h,
		sub_img_x, sub_img_y, sub_img_w, sub_img_h,
		mag_fact, mag_mode,	/* display magnified image */
		mag_x, mag_y, mag_w, mag_h,/* subimage currently being viewed */
		save_mag_x, save_mag_y, save_mag_w, save_mag_h,
		save_mag_fact, dpy_depth,
		frames, fn, channels, dpy_channels,
		*hist, *histp;	/* histogram buffer & pointer	*/
	VType	*src, *dest, *cnvt, *img_buf, *lkt;
	bool	sub_img, sub_img_enh,	/* sub window and enhanced	*/
		active, rw_cmap,	/* writable colormap	*/
		color_dpy,		/* False if black/white	*/
		dither_img, update,
		logscale, setscale,	/* use unique or own maxcnt	*/
		load_all, save_all;	/* if not true, do 1 frame at a time */
	int	in_type, mid_type, o_type, in_color, color_form,
		in_form, o_form, pxl_in, pxl_out;
	Mregister	mmm, *marray;
	eX_thread	*ethread;	/* extended event thread	*/
	io_content	i, o;		/* input/output structures	*/
	StdInterface	*eof, *errors, *header_handle, *std_swif;
	TableInterface	(*table_if)(), **table, tables;
	VType	**superimpose, *parts;
	short	draws, texts, stack_num;/* parts in part stack	*/
	void	(*map_scanline)(), (*MAG_scanline)();
	bool	binary_img, mono_img,	/* same info as color_form	*/
		mono_color, sep_colors,
		update_header;
	int	entries, ncmap, cmaplen, *pixel_table;
	cmap_t	**cmap;
#if defined EXTENDED_U_IMAGE | defined X_WINDOW_DEP
	int	lvls, lvls_squared,
		visual_class;
	VType	*dpy_visual;
#endif
	} U_IMAGE;	/* standard Union Image structure for filters */


extern	U_IMAGE	uimg;	/* a special-reserved-global variable	*/
#define	in_fp	uimg.i.fp
#define	out_fp	uimg.o.fp

#ifdef	SHOW_WARNINGS
	VType	syserr();
#else
	bool	syserr();
#endif

extern	char	*Progname, *Mversion, *ITypeName[];
extern	cmap_t	*reg_cmap[3];	/* also *r_cmap are global cmaps */
extern	sht_cmap_t	*r_cmap;
sht_cmap_t	*regmap_to_rlemap(/* cmap, number, channels, rle_hd */);

void	work_space_init(), DBwork_space_init(),	QuickSort();
VType	*DBvfft3d(), *DBvrft3d(), *vfft3d(), *vrft3d(),
	*rgbmap_to_othermap(/* cmap, number, reg_or_rle */);
float	*vfft2d(), *vrft2d();
double	*DBvfft2d(), *DBvrft2d();
int	load_w(), load_DBw(),
	Fourier(), DBFourier(),
	w_init(), w_load(),
	vfft(), vfft_2d(), vrft_2d(), vfftn(),
	dw_init(), dw_load(),
	dvfft(), dvfft_2d(), dvfrt_2d(), dvfftn();
bool	ccs_table_if();
extern	int	RED_to_GRAY, GREEN_to_GRAY, BLUE_to_GRAY;
extern	MType	gf_size;	/* this one may not exist */

#define	set_table_if(img, t_if)	(img)->table_if=(TableInterface *)tif

#ifndef	MaxColors
#define	MaxColors	256
#endif

#if	defined	COMMON_TOOL | defined	RLE_IMAGE

#include "rle.h"

#ifndef	SAVED_RLE_ROW
#define SAVED_RLE_ROW(img, y)	\
	((byte*)((U_IMAGE*)img)->dest +	\
	((y) * ((U_IMAGE*)img)->width * (img)->dpy_channels))
#define ORIG_RLE_ROW(img, y)	\
	((byte*)((U_IMAGE*)img)->src +	\
	((y) * ((U_IMAGE*)img)->width * (img)->dpy_channels))
#endif	RLE_ROW

#endif	RLE_IMAGE


#ifdef	COMMON_TOOL

typedef	struct	{
	unsigned int	Width, Height,
			CmapLen,
			ColorResolution,
			Background,
			AspectRatio;
	color_cell	ColorMap[MaxColors];
	bool	interlace, jumpover;	/* pass uninteresting images */
	} GS;

extern	GS	GifScreen;


/*	The format for the "ICC" header is:	*/

#ifndef	ICC_MAGIC
#define	ICC_MAGIC	5965600
#endif

typedef	struct	{
	MType	magic_num,	/*	Magic Number	4 bytes	*/
		hd_len,		/*	Header Length	4 bytes	*/
		/*	Extraneous data	(Header Length - 4) bytes	*/
		H_W, S_W,
		date, time,
		new_date, new_time;
	char	user_name[32];
    struct	{
	MType	len,		/*	Image header length	4 bytes	*/
		length;		/*	Header + data length	4 bytes	*/
	char	LFName[16];	/*	Logical File Name	16 bytes
			This field will cause KPR command not working	*/
	MType	File_Type,	/*	ICC is (7)		4 bytes	*/
		orig_width,	/*	Extraneous data		8 bytes	*/
		orig_height,
		x_size,		/*	Image X Size		4 bytes	*/
		y_size,		/*	Image Y Size		4 bytes	*/
		Ext_Data3[2],	/*	Extraneous data		8 bytes	*/
		gray_scale,	/*	color = 0, g/s = 1	4 bytes	*/
		planes;		/*	color = 3, g/s = 1	4 bytes	*/
	VType	*Ext_Data;	/*	Extraneous data		n bytes	*/
	} img_hd;
    }	ICC_HEADER;

extern	ICC_HEADER	icchd;

# ifndef	X_WINDOW_DEP
#	include	"pnm.h"
#	include	"ppmcmap.h"
# endif

# include	"tiffio.h"
# include	"tiffioPP.h"
extern	TIFF	*TIFFin;
extern	unsigned short	samplesperpixel, bitspersample;

# include	"rast.h"
extern	struct rasterfile	srhd;

#ifndef	RAST_8PAD	/* must be power of 2 */
#define	RAST_8PAD	2
#endif
#define	RAST_8COMP	(RAST_8PAD - 1)

#ifndef	RAST_ODD_PAD
#  if	RAST_8PAD == 2
#    define RAST_ODD_PAD(w)	((w) & RAST_8COMP)
#  else
#    define RAST_ODD_PAD(w)	(RAST_8PAD - ((w) & RAST_8COMP) & RAST_8COMP)
#  endif
#endif

#endif	COMMON_TOOL

#endif	IMAGES_DEF
