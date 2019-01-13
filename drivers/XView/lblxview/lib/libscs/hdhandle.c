/*	HeaDer_HANDLE. C
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
	Bld. 50B, Rm. 2275
	Lawrence Berkeley National Laboratory, Berkeley, CA, 94720
	g_jin@lbl.gov

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% AUTHOR:	Jin Guojun - LBL	7/25/91
*/

#ifdef	COMMON_TOOL

#ifndef	MaxColors
#define MaxColors 256
#endif


ICC_HEADER	icchd;

icc_header_handle(swif_h_alist)
swif_h_alist_def;
{
int	num;

switch (job) {
case HEADER_READ:
case HEADER_FREAD:
	if (!av)
		(*img->r_seek)(img->IN_FP, 0, SEEK_SET);
	(*img->i_read)(&num, sizeof(num), 1, img->IN_FP);
	if (num != ICC_MAGIC)
other_hd:	return	EOF;
	(*img->i_read)(&icchd.hd_len, sizeof(icchd.hd_len), 1, img->IN_FP);
	(*img->i_read)(&icchd.H_W, icchd.hd_len - sizeof(icchd.hd_len), 1, img->IN_FP);
	(*img->i_read)(&icchd.img_hd.len, sizeof(icchd.img_hd.len), 1, img->IN_FP);
	(*img->i_read)(&icchd.img_hd.length,
		icchd.img_hd.len-sizeof(icchd.img_hd.len), 1, img->IN_FP);
	if (icchd.img_hd.length - icchd.img_hd.len -
		icchd.img_hd.x_size * icchd.img_hd.y_size)
		goto	other_hd;
	img->width = icchd.img_hd.orig_width;
	img->height = icchd.img_hd.orig_height;
	if (!(img->width | img->height) || img->height != icchd.img_hd.y_size
		|| ((img->width + 7) & 0xfffffff8L != icchd.img_hd.x_size))
		img->width = icchd.img_hd.x_size,
		img->height = icchd.img_hd.y_size;
	img->pxl_in = img->frames = 1;
	img->channels = img->dpy_channels = icchd.img_hd.planes;
	img->in_type = ICC;
	img->in_color = img->color_form = CFM_SGF;
	if (img->channels==3)
		img->in_form = IFMT_SEPLANE,
		img->in_color = CFM_SEPLANE;
	else	img->in_form = IFMT_SGF,
		img->mono_img = True;
	break;
default:	message("unknown icc job %d\n", job);
	return	job;
}
return	0;
}


static	int	format, maxval;
static	xel	**xel24;
static colorhash_table	cht;

static	cmap_t*
pnmmap_to_regmap(rg_cmap, chv, colors)  cmap_t	*rg_cmap[3]; colorhist_vector chv; int	colors;
{
register int	i=3, j=1<<min_bits(colors-1);
cmap_t	*cmap[3];

verify_buffer_size(rg_cmap, j, i*sizeof(*cmap), "pnm-rcmap");
cmap[0] = rg_cmap[0];
cmap[1] = rg_cmap[1] = rg_cmap[0] + j;
cmap[2] = rg_cmap[2] = rg_cmap[1] + j;

for (i=0; i < colors; ++i) {
	cmap[0][i] = PPM_GETR(chv[i].color);
	cmap[1][i] = PPM_GETG(chv[i].color);
	cmap[2][i] = PPM_GETB(chv[i].color);
	}
return	cmap[0];
}

pnm_header_handle(swif_h_alist)
swif_h_alist_def;
{
gray	gmaxval;	/* byte */
colorhist_vector	chv;
xel	*xelrow, p;
int	depth, colors;
register int	r;

    switch (job) {
    case HEADER_READ:
    case HEADER_FREAD:
	if (img->in_type > RLE && img->in_type != PNM)	goto	pnm_df;
	if (!av)
		(*img->r_seek)(img->IN_FP, 0, 0);
	format = pbm_readmagicnumber(img->IN_FP);	/* check magic # */
	img->pxl_in = 1;
	switch (PNM_FORMAT_TYPE(format)) {
	case PPM_TYPE:
	maxval = ppm_readppminitrest(img->IN_FP, &img->width, &img->height);
	xel24 = (xel**)alloc_2d_discrete(img->width, img->height, sizeof(xel));
		for (r=0; r < img->height; r++)
		    pnm_readpnmrow(img->IN_FP, xel24[r], img->width, maxval, format);
		/* Figure out the proper depth and colormap */
		chv = ppm_computecolorhist(xel24, img->width, img->height,
				MaxColors, &colors);
		if (chv) {
			DEBUGMESSAGE("%d colors found", colors);
			if (maxval != 255)
			    for (r=0; r < colors; r++)
				PPM_DEPTH(chv[r].color, chv[r].color, maxval, 255);

		/* Force white to slot 0 and black to slot 1, if possible. */
			PPM_ASSIGN(p, 255, 255, 255);
			ppm_addtocolorhist(chv, &colors, MaxColors, &p, 0, 0);
			PPM_ASSIGN(p, 0, 0, 0);
			ppm_addtocolorhist(chv, &colors, MaxColors, &p, 0, 1);

			if (colors != 2) {
			/* convert the ppm colormap into the RLE colormap. */
				depth = 8;
				img->cmaplen = colors;
				img->in_color = CFM_SCF;
				img->in_form = IFMT_SCF;
				pnmmap_to_regmap(reg_cmap, chv, colors);
			}
			if (cht)	ppm_freecolorhash(cht);
			cht = ppm_colorhisttocolorhash(chv, colors);
			ppm_freecolorhist(chv);
			img->channels = img->mono_img = 1;
			img->in_type = PNM;
			if (colors==2)	{	/* Monochrome. */
				depth = 1;
				goto	pbm_tp;
			}
		}
		else	img->channels = 3,
			img->in_type = PPM,
			img->in_form = IFMT_ILC,
			img->in_color = CFM_ILC;
	break;
	case PGM_TYPE:
		pgm_readpgminitrest(img->IN_FP, &img->width, &img->height, &gmaxval);
		maxval = (xelval) gmaxval;
		if (maxval > 255)
#	ifdef	BIGGRAYS
			img->in_form = IFMT_SHORT,
			img->pxl_in = sizeof(short);
#	else
		return	prgmerr(0, "maxval (%d) is greater than 255", maxval);
#	endif
		img->channels = img->mono_img = 1;
		img->in_type = PGM;
		img->in_color = CFM_SGF;
	break;
	case PBM_TYPE:
		pbm_readpbminitrest(img->IN_FP, &img->width, &img->height);
		maxval = img->channels = img->mono_img = 1;
		img->in_type = PBM;
pbm_tp:		img->in_form = IFMT_BITMAP;
		img->in_color = CFM_BITMAP;
	break;
	default:
pnm_df:		return	EOF;
	}
	if (maxval > 255 && depth != 1)
		message("maxval is not 255 - automatically rescaling colors");

	break;
    case HEADER_WRITE:
    default:	return	prgmerr(0, "not a pnm job %d\n", job);
    }
return	0;
}


static	bool	TiffRGB;
TIFF	*TIFFin;
unsigned short	samplesperpixel, bitspersample;

tiff_header_handle(swif_h_alist)
swif_h_alist_def;
{
unsigned short	maxval;

switch (job) {
case HEADER_READ:
case HEADER_FREAD:
	if (!av)
		lseek(fileno(img->IN_FP), 0, 0);
	TIFFin = TIFFFdOpen(fileno(img->IN_FP), img->name ? img->name :"", "r");
	if (!TIFFin)
		return	EOF;
	TIFFGetField(TIFFin, TIFFTAG_BITSPERSAMPLE, &bitspersample);
	TIFFGetField(TIFFin, TIFFTAG_SAMPLESPERPIXEL, &samplesperpixel);
	if (samplesperpixel > 1)
		TiffRGB++;
	switch (samplesperpixel) {
	default:message("handle %d samples.\n", samplesperpixel);
	case 1:
		img->channels = 1;
		if (img->color_dpy && !assist)
			img->dpy_channels = 3;
		else	img->dpy_channels = img->mono_img = 1;
		if (TIFFin->tif_dir.td_photometric==PHOTOMETRIC_PALETTE) {
		register int	i = MIN(MaxColors, (maxval=1<<bitspersample));

		    img->in_color = CFM_SCF;
		    img->in_form = IFMT_SCF;
		    img->cmaplen = i;
		    if (verify_buffer_size(reg_cmap, i, 3*sizeof(cmap_t),
				"tif_cmap"))	{
			reg_cmap[1] = reg_cmap[0] + i;
			reg_cmap[2] = reg_cmap[1] + i;
		    }
		    while (i--) {
			reg_cmap[0][i] = TIFFin->tif_dir.td_colormap[0][i] >> 8;
			reg_cmap[1][i] = TIFFin->tif_dir.td_colormap[1][i] >> 8;
			reg_cmap[2][i] = TIFFin->tif_dir.td_colormap[2][i] >> 8;
			}
		}
		else if (bitspersample==1)	/* always convert to SGF */
			img->in_color = CFM_SGF,
			img->in_form = IFMT_BITMAP;
		else	prgmerr(0, "TIFF format-8");
		break;
	case 3:
		if (TIFFin->tif_dir.td_planarconfig==PLANARCONFIG_SEPARATE)
			img->in_color = CFM_ILL,
			img->in_form = IFMT_ILL;
		else	img->in_color = CFM_ILC,
			img->in_form = IFMT_ILC;
		goto	next34;
	case 4:	img->in_color = CFM_ALPHA;
		img->in_form = IFMT_ALPHA;
next34:		img->dpy_channels = img->channels = 3;
		if (!img->color_dpy)
			img->dpy_channels = 1;
		break;
	}

	TIFFGetField(TIFFin, TIFFTAG_IMAGEWIDTH, &img->width);
	TIFFGetField(TIFFin, TIFFTAG_IMAGELENGTH, &img->height);

	img->in_type = TiFF;
	img->pxl_in = sizeof(byte);
	break;
case HEADER_WRITE:
default:	return	prgmerr(0, "no such a tiff job %d\n", job);
}
return	0;
}

#endif	COMMON_TOOL



#ifdef	HIPS_IMAGE

#ifdef	TC_Need
#define	ntohs	ShortSwap
#define	ntohl	LongSwap
#elif	SOLARIS
#include <netinet/in.h>
#endif

void
si_ltob(si_ep, draw)  register superimpose_elems *si_ep; bool draw;
{
	si_ep->x0 = ntohs(si_ep->x0);
	si_ep->y0 = ntohs(si_ep->y0);
	si_ep->w = ntohs(si_ep->w);
	si_ep->h = ntohs(si_ep->h);
	si_ep->color = ntohl(si_ep->color);
	if (draw)
		si_ep->elem.draw.angle1 = ntohs(si_ep->elem.draw.angle1),
		si_ep->elem.draw.angle2 = ntohs(si_ep->elem.draw.angle2);
}

#define	si_elems	"si_elems"
#define si_draws	"si_draws"
#define	si_texts	"si_texts"
#define	si_str	"sit%03d"

static	char	sbuf[8];

put_superimpose_param(img, hhd)  U_IMAGE*	img; struct header*	hhd;
{
if (img->superimpose || img->draws | img->texts)	{
register superimpose_elems*	si_ep;
register int	i=img->draws;
	if (findparam(hhd, si_elems) != NULLPAR)
		hhd->paramdealloc = False;
	setparam(hhd, si_elems, PFSHORT, 2, &img->draws);
	if (i)	{
		si_ep = img->superimpose[0];
#ifdef	LITTLE_ENDIAN
		while (i--)	si_ltob(si_ep+i, Yes);
#endif
		setparam(hhd, si_draws, PFBYTE, img->draws * sizeof(*si_ep),
			si_ep);
	}
	if (i = img->texts)	{
		si_ep = img->superimpose[1];
#ifdef	LITTLE_ENDIAN
		while (i--)	si_ltob(si_ep+i, No);
		i = img->texts;
#endif
		setparam(hhd, si_texts, PFBYTE, img->texts * sizeof(*si_ep),
			si_ep);
		while (i--)	{
		sprintf(sbuf, si_str, i);
		setparam(hhd, sbuf, PFBYTE,
			strlen(si_ep[i].elem.text.content) + 1,
			si_ep[i].elem.text.content);	/* + 1 is for NULL term. */
		}
	}
}
}

get_superimpose_param(img, hhd)  U_IMAGE*	img; struct header*	hhd;
{
int	n=2;
short	*boo;
register superimpose_elems*	si_ep;

if (!findparam(hhd, si_elems))	return	False;
if (!img->superimpose)	img->superimpose = ZALLOC(n, sizeof(void*), "si_dp");
if (getparam(hhd, si_elems, PFSHORT, &n, &boo) != HIPS_OK)
	return	EOF;

img->draws = ntohs(boo[0]);	n = 2;
if ((findparam(hhd, si_draws) != NULLPAR | n) &&
    getparam(hhd, si_draws, PFBYTE, &n, img->superimpose) != HIPS_OK)
	img->draws = 0;
#ifdef	LITTLE_ENDIAN
else for (si_ep=img->superimpose[0], n=img->draws; n--;)
	si_ltob(si_ep+n, Yes);
#endif
img->texts = ntohs(boo[1]);	n = 2;
if (findparam(hhd, si_texts) != NULLPAR  &&
    getparam(hhd, si_texts, PFBYTE, &n, img->superimpose+1) == HIPS_OK)	{
	register int	i=img->texts;
	si_ep = img->superimpose[1];
	while (i--)	{
	sprintf(sbuf, si_str, i);	n = 2;	/* non-single char	*/
	getparam(hhd, sbuf, PFBYTE, &n, &si_ep[i].elem.text.content);
#ifdef	LITTLE_ENDIAN
	si_ltob(si_ep+i, No);
#endif
	}
} else	img->texts = 0;
return	img->texts | img->draws;
}

/*=======================================================================
%	For writing 3D sub-image, use HEADER_TO to set sub-image size,	%
%	then use HEADER_WRITE with img->update_header==0 to output	%
%	the pre-set header. Otherwise, the HEADER_WRITE only output a	%
%	sub-image of an image having a single frame.			%
%	Unsuccessful reading returns (-1) EOF, otherwise returns 0 and	%
%	img->in_type is set to proper input image type such as HIPS.	%
=======================================================================*/

struct	header	hhd;	/*	basic structure	*/

hips_header_handle(swif_h_alist)
swif_h_alist_def;
{
FILE	*o_fp=img->OUT_FP, *i_fp=img->IN_FP;
int	hform;

switch (job) {
case HEADER_FREAD:
	i_fp = (FILE*)ac;
case HEADER_READ:
#ifdef	STREAM_IMAGE
	set_jpeg_uimg(img);
#endif
	if (img->in_type != HIPS || H_fread_header(i_fp, &hhd))
		return	EOF;
#ifdef	STREAM_IMAGE
	get_jpeg_uimg(img);
#endif
	goto	set_img;
case HEADER_FROM:
	if (ac)	memcpy(&hhd, ac, sizeof(hhd));
set_img:
	hform = hhd.pixel_format;
	ac = ac=='p' && hform !=IFMT_ILC;/* treate RGB frame as G.S. frames */
	img->in_type = HIPS;
	img->cmaplen = 0;
	img->color_form = CFM_SGF;	/* default to gray scale	*/
	img->channels = hhd.numcolor;
	if (findparam(&hhd, "cmap") != NULLPAR)	{
	VType*	cmapp;
	int	nc;
		getparam(&hhd, "cmap", PFBYTE, &nc, &cmapp);
		if (nc % 3 || nc > MaxColors*3)
			prgmerr(0, "strange colormap = %d", nc);
		else	{
			if (reg_cmap[0])
				CFREE(reg_cmap[0]);
			reg_cmap[0] = cmapp;
			img->cmaplen = (nc /= 3);
			reg_cmap[1] = reg_cmap[0] + nc;
			reg_cmap[2] = reg_cmap[1] + nc;
			img->in_color = img->color_form = CFM_SCF;
		}
	}
	else	if (img->channels==3)	{	/* kludge handle SEPLANE */
			if (hform==IFMT_ILC)	/* PFRGB */
				img->in_color = CFM_ILC;
#	ifdef	STREAM_IMAGE
			else if (img->in_form==IFMT_STREAM)
				img->color_form = CFM_ILL;
#	endif
			else	img->in_color = CFM_SEPLANE;
		}
		else if (hform==IFMT_ALPHA)
			img->in_color = CFM_ALPHA,
			img->channels = 4;
		else if (hform == IFMT_ILC)
			img->in_color = CFM_ILC,
			img->channels = 3;
		else	img->in_color = CFM_SGF;

	/*	should select_color_form here	*/
	if ((img->in_color == CFM_SEPLANE) & (ac | !img->color_dpy)) {
		img->dpy_channels = 1;
		if (!ac)	img->color_form = CFM_SGF;
	}

#if	defined COMMON_TOOL | defined RLE_IMAGE
	/* not force to stick at 1 channel	*/
	if (img->mid_type==RLE && img->cmaplen && !assist)	{
		rle_dflt_hdr.ncmap = img->mono_img = 0;
	/* should be handled by select_color_form()
		if (img->color_form != CFM_SGF)
			img->dpy_channels = img->channels = 3;	*/
	}
#endif
	img->frames = hhd.num_frame / hhd.numcolor;
	img->in_form = hhd.pixel_format;
	img->width = hhd.ocols;
	img->height = hhd.orows;
	img->pxl_in = pxl_size(hhd);
	comfirm_pixel_size(img);
	img->history = hhd.seq_history;
	img->desc = hhd.seq_desc;
	img->sub_img_w = hhd.cols;
	img->sub_img_h = hhd.rows;
	img->sub_img_x = hhd.fcol;
	img->sub_img_y = hhd.frow;
	get_superimpose_param(img, &hhd);
#ifdef	STREAM_IMAGE
	set_jpeg_uimg(img);
#endif
	break;
case HEADER_TO:
	if (img->o_type != HIPS)
hhd_other:	return
#ifdef	FITS_IMAGE
		fits_header_handle(job, img, ac, av, assist);
#else
		EOF;
#endif
	if (img->o_type != img->in_type || ac	/* as initial_header_flag */)
		(*img->std_swif)(FI_INIT_NAME, img, img->name, 0);
	if ((hhd.numcolor=img->dpy_channels) == 3 && img->color_form == CFM_ILC)
		hhd.numcolor = 1,	/* wired HIPS2 format	*/
		hhd.pixel_format = IFMT_ILC;
	else {	hhd.pixel_format = img->o_form;
		if (img->color_form == CFM_SEPLANE)
			hhd.numcolor = 3,	/* kludge handle SEPLANE. */
			hhd.pixel_format = IFMT_BYTE;
	}
	hhd.num_frame = img->frames * hhd.numcolor;
	hhd.fcol = hhd.frow = 0;
/*
	hhd.fcol = img->sub_img_x;	hhd.frow = img->sub_img_y;
*/
	if (img->sub_img)	{
	    if (img->sub_img_w && img->sub_img_h) {
		hhd.cols = img->sub_img_w;	/* if ROI then set ROI */
		hhd.rows = img->sub_img_h;
	    }
		hhd.ocols = img->sub_img_w;
		hhd.orows = img->sub_img_h;
	} else	{
		hhd.ocols = hhd.cols = img->width;
		hhd.orows = hhd.rows = img->height;
	}

	if (img->desc)
		hhd.seq_desc = img->desc;
	else if (!hhd.seq_desc)
		hhd.seq_desc = WHAT_Str;
	if (img->dest && !hhd.image)
		hhd.image = (byte *) img->dest;
	set_pxl_size(hhd, img->pxl_out);
	break;
case HEADER_FWRITE:
	o_fp = (FILE*)assist;	/* use given file pointer */
case HEADER_WRITE:
	if (img->o_type == HIPS) {
	    if (img->update_header) {
		if (img->o_type != img->in_type || img->update_header < 0)
			(*img->std_swif)(FI_INIT_NAME, img, img->name, 0);
		if ((hhd.numcolor=img->dpy_channels) == 3
			&& img->color_form == CFM_ILC)
			hhd.numcolor = 1,	/* only SEP = 3	*/
			hhd.pixel_format = IFMT_ILC;
		else	hhd.pixel_format = img->o_form;
		hhd.num_frame = img->frames * hhd.numcolor;
		hhd.fcol = hhd.frow = 0;	/* not ROI for output	*/
		if (img->sub_img && img->frames==1) {
			hhd.ocols = img->sub_img_w;
			hhd.orows = img->sub_img_h;
		} else	{
			hhd.cols = hhd.ocols = img->width;
			hhd.rows = hhd.orows = img->height;
		}
		set_pxl_size(hhd, img->pxl_out);
	    }
	    put_superimpose_param(img, &hhd);
	    if (img->cmaplen && img->color_dpy && img->color_form==CFM_SCF) {
#ifdef	RLE_IMAGE
		if (!reg_cmap[0])	/* must be in rlemap	*/
			rlemap_to_regmap(reg_cmap, &rle_dflt_hdr);
#endif
		setparam(&hhd, "cmap", IFMT_BYTE, 3*img->cmaplen, reg_cmap[0]);
	    }
	    if (assist)
		H_update_header(&hhd, ac, av);
	    H_fwrite_header(o_fp, &hhd, img->name ? img->name:"std_in");
	}
	else	goto	hhd_other;
	break;
case ADD_DESC:
	if (img->o_type != img->in_type)
		(*img->std_swif)(FI_INIT_NAME, img, img->name, 0);
	H_add_seq_desc(&hhd, ac);
	img->desc = hhd.seq_desc;
	break;
default:message("unknown job %d\n", job);	return	job;
}
return	0;
}

#elif	defined	HIPS_PUB

hips_header_handle(swif_h_alist)
swif_h_alist_def;
{
FILE	*o_fp=img->OUT_FP, *i_fp=img->IN_FP;
int	hform;

switch (job) {
case HEADER_FREAD:
	i_fp = (FILE*) ac;
case HEADER_READ:
	if (img->in_type != HIPS || hpub_frdhdr(i_fp, &hform, &img->height,
			&img->width, &img->frames, &img->channels))
		return	EOF;
	ac = ac=='p' && hform !=IFMT_ILC;/* treate RGB frame as G.S. frames */
	img->in_type = HIPS;
	img->cmaplen = 0;
	img->color_form = CFM_SGF;	/* default to gray scale	*/
		if (img->channels==3)	{	/* kludge handle SEPLANE */
			if (hform==IFMT_ILC)	/* PFRGB */
				img->in_color = CFM_ILC;
#	ifdef	STREAM_IMAGE
			else if (img->in_form==IFMT_STREAM)
				img->color_form = CFM_ILL;
#	endif
			else	img->in_color = CFM_SEPLANE;
		}
		else if (hform==IFMT_ALPHA)
			img->in_color = CFM_ALPHA,
			img->channels = 4;
		else if (hform == IFMT_ILC)
			img->in_color = CFM_ILC,
			img->channels = 3;
		else	img->in_color = CFM_SGF;

	/*	should select_color_form here	*/
	if ((img->in_color == CFM_SEPLANE) & (ac | !img->color_dpy)) {
		img->dpy_channels = 1;
		if (!ac)	img->color_form = CFM_SGF;
	}

#if	defined COMMON_TOOL | defined RLE_IMAGE
	/* not force to stick at 1 channel	*/
	if (img->mid_type==RLE && img->cmaplen && !assist)	{
		rle_dflt_hdr.ncmap = img->mono_img = 0;
		if (img->color_form != CFM_SGF)
			img->dpy_channels = img->channels = 3;
	}
#endif
	img->frames /= img->channels;
	img->in_form = hform;
	comfirm_pixel_size(img);
	img->history =
	img->desc = NULL;
	img->sub_img_w = img->width;
	img->sub_img_h = img->height;
	img->sub_img_x =
	img->sub_img_y = 0;
	break;
case HEADER_TO:
	if (img->o_type != HIPS)
hhd_other:	return
#ifdef	FITS_IMAGE
		fits_header_handle(job, img, ac, av, assist);
#else
		EOF;
#endif
	break;
case HEADER_FWRITE:
	o_fp = (FILE *) assist;	/* use given file pointer */
case HEADER_WRITE:
	if (img->o_type == HIPS) {
		job = img->frames * (ac = img->dpy_channels);
		if (ac == 3 && img->color_form == CFM_ILC)
			img->o_form = IFMT_ILC,	ac = 1;
		hpub_fwrthdr(o_fp, img->o_form, img->height, img->width,
			job, ac);
	}
	else	goto	hhd_other;
	break;
case ADD_DESC:
default:	message("unknown job %d\n", job);	return	job;
}
return	0;
}

#endif
