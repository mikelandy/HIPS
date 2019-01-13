/*	CONVERTU . C
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
	Lawrence Berkeley National Laboratory, Berkeley, CA, 94720.
	g_jin@lbl.gov

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%	Converting interface for new HIPS_2 or other types.
%	Make image filters have flexibility to handle different types.
%	Usually, the HIPS is checked first, and then other types.
%
%	For color image input, the pxl_in always = 1.
%
%	The libscs1 requests FITS_IMAGE directives for handling HIPS and FITS
%	images. The libscs2 requests another directive, RLE_IMAGE, to handle
%	RLE image. The COMMON_TOOL directive requires to link with libtiff.a.
%	Once COMMON_TOOL directive is defined, in general, this system will
%	be able to handle following images:
%
%	HIPS, FITS, GIF, ICC, JPEG, PICT, PNM, RLE, SUN-RASTER, TIFF and XWD
%
%	Color Map:
%		input colormap (8-bit color) is in reg_cmap. get_pic() or
%	other re-map reg_cmap to rle_dflt_hdr.cmap or other colormaps.
%
%	Channels:
%		Input channel (img->channels) in returning of header_handler
%	is the original channels. It can be changed in read_???_image()
%	routine depended on img->mid_type and img->color_form. The output
%	channel (img->dpy_channels) depends on output device and img->o_type.
%
% AUTHOR:	Jin Guojun - LBL	7/25/91
*/

#include "function.h"

char*	ITypeName[] = {IName_List}, WHAT_Str[] = "?";

#define	Real_STR(str)	(str ? str : WHAT_Str)


#if	defined HIPS_IMAGE || defined HIPS_PUB

comfirm_pixel_size(img)  U_IMAGE *img;
{
	if (img->pxl_in == 3)
		img->pxl_in = 1;	/* for PFRGB	*/
	if (!img->pxl_in) {
	register int	vsize;
		switch (img->in_form)	{
		case IFMT_VFFT3D:
		case IFMT_VFFT2D:
		case IFMT_VVFFT3D:
			vsize = sizeof(float)<<1;
			break;
		case IFMT_DVFFT3D:
		case IFMT_DVFFT2D:
		case IFMT_DVVFFT3D:
			vsize = sizeof(double)<<1;
			break;
		case IFMT_HIST:
			vsize  = sizeof(int);
			break;
#ifdef	HIPS_PUB
		case IFMT_SHORT:	vsize = 2;	break;
		case IFMT_LONG:
		case IFMT_FLOAT:	vsize = 4;	break;
#endif
		default:vsize = 1;
#ifndef	HIPS_PUB
			msg("%d: undetermined size", img->in_form);
#endif
		}
		img->pxl_in = vsize;
	}
}

#endif

#include	"hdhandle.c"

#ifdef	FITS_IMAGE

FITS_BASE	fhd;

fits_header_handle(swif_h_alist)
swif_h_alist_def;
{
switch(job) {
case HEADER_READ:
case HEADER_FREAD:
	if (!av)	(*img->r_seek)(img->IN_FP, 0, SEEK_CUR);
	if ((*img->eof)(img->IN_FP))
		return	EOF;	/* save no changes for other headers	*/
	if (!hostype)	{
		hostype = check_host();
		FTy = comfirm_host(img, hostype, FTy);
	}
	if (get_fits_head(&fhd, img, hostype, FTy))
		return	EOF;
	img->in_type = FITS;
	img->in_color = img->color_form = CFM_SGF;
	goto	fit_set;
case HEADER_FROM:
	img->frames = fhd.naxis3;
	img->width = fhd.naxis2;
	img->height = fhd.naxis1;
	img->pxl_in = fhd.bits_pxl>>3;
	img->in_form = img->pxl_in>>1;
	img->history = fhd.history;
fit_set:
	img->dpy_channels = img->channels = img->mono_img = 1;
#if	defined	COMMON_TOOL | defined RLE_IMAGE
	if (img->mid_type==RLE)
		rle_dflt_hdr.ncmap = 0;
#endif
	break;
case HEADER_TO:
case HEADER_WRITE:	/* to standard output */
case HEADER_FWRITE:
	if(img->o_type==FITS)	{
		fhd.naxis3 = img->frames;
		fhd.naxis1 = img->height;
		fhd.naxis2 = img->width;
		fhd.history = img->history;
		fhd.bits_pxl = img->pxl_out<<3;
		if (job==HEADER_TO)	break;
		put_fits_head(&fhd, img, ac);
	}
#if defined	(COMMON_TOOL) | defined RLE_IMAGE
	else	return	rle_header_handle(job, img, ac, av, assist);
#endif
	break;
default:	message("unknown job %d\n", job);
	return	job;
}
return	0;
}
#endif	FITS_IMAGE


#if	defined	COMMON_TOOL | defined	RLE_IMAGE

rle_header_handle(swif_h_alist)
swif_h_alist_def;
{
register int	i;

switch (job) {
case HEADER_READ:
case HEADER_FREAD:	/* av indicates a multiple-frame image */
	if (!av)
		(*img->r_seek)(img->IN_FP, 0, SEEK_CUR);
	rle_dflt_hdr.rle_file = img->IN_FP;
	if ((i=rle_get_setup(&rle_dflt_hdr)) != RLE_SUCCESS)
		return	i;
	img->in_type = RLE;
case HEADER_FROM:
	img->mag_h = img->height =
		rle_dflt_hdr.ymax - (img->y0 = rle_dflt_hdr.ymin) + 1;
	img->mag_w = img->width =
		rle_dflt_hdr.xmax - (img->x0 = rle_dflt_hdr.xmin) + 1;
	/*	for rle_getrow	*/
	rle_dflt_hdr.xmin = 0;	rle_dflt_hdr.xmax = img->width - 1;
	img->pxl_in = 1;

	img->in_color = rle_dflt_hdr.ncolors;
	/* kludge do it, dangerous when change CFM & IFMT definions */
	if (img->in_color == 3)
		img->in_color = CFM_ILL,
		img->in_form = IFMT_ILL;
	else if (img->in_color &= rle_dflt_hdr.ncmap)
		img->cmaplen = 1 << rle_dflt_hdr.cmaplen,
		img->in_form = IFMT_SCF;
	else	img->in_form = IFMT_SGF;
	img->color_form = img->in_color;

	if (rle_dflt_hdr.comments && rle_dflt_hdr.comments[0])
		img->desc = (char*)str_save(rle_dflt_hdr.comments[0]);
	/*	We are only interested in R.G.B.	*/
	for (i=3; i < rle_dflt_hdr.ncolors; i++)
		RLE_CLR_BIT(rle_dflt_hdr, i);
	RLE_CLR_BIT(rle_dflt_hdr, RLE_ALPHA);	/* rle.h L112 */
	img->channels = rle_dflt_hdr.ncolors & 3;
	img->dpy_channels &= img->channels;
	break;
case HEADER_TO:
case HEADER_WRITE:	/* to standard output */
case HEADER_FWRITE:
	if (img->o_type != RLE)	return
#	ifdef	COMMON_TOOL
		icc_header_handle(job, img, ac, av, assist);
#	else
		EOF;
#	endif
	rle_dflt_hdr.xmin = rle_dflt_hdr.ymin = 0;
	rle_dflt_hdr.ymax = img->height - 1;
	rle_dflt_hdr.xmax = img->width - 1;
	rle_dflt_hdr.ncolors = img->channels;
	if (rle_dflt_hdr.comments)
		rle_dflt_hdr.comments[0] = img->desc;
	if (job==HEADER_TO)	break;
	rle_put_setup(&rle_dflt_hdr);
	break;
default:message("unknown job %d\n", job);
	return	job;
}
return	0;
}

#endif	RLE_IMAGE



int	row_dpy_mode;

ccs_get_row_ok(img)
register U_IMAGE*	img;
{
register int	type=img->in_type, r_mode=row_dpy_mode && img->IN_FP==stdin;

if (type==RLE && r_mode)	return	-1;	/* decreasing mode	*/

r_mode |= type==HIPS && img->color_form==CFM_SGF;

#ifdef	STREAM_IMAGE
	if ((r_mode &= !get_sinfo(True, 0)) &&	/* not in compressed type */
		(type=get_sinfo(False, 0)) > 0)	/* is in stream mode.	*/
		for (type*=img->width; type--;)
			(*img->r_seek)(img->IN_FP, 0, SEEK_GETB);
#endif
return	r_mode;
}

ccs_get_row(img, row, no_buf)
U_IMAGE	*img;
register int	row;
{
register int	w = img->width;
register char*	buf = (char*)img->src + (no_buf ? 0 : w * row * img->channels);
switch (img->in_type)	{
case HIPS:
	return	(*img->i_read)(buf, 1, w, img->IN_FP);
#ifdef	RLE_IMAGE
case RLE:	{
	char*	scan[3];
	scan[0] = buf;	scan[1] = buf + w;	scan[2] = buf + (w << 1);
	rle_getrow(&rle_dflt_hdr, scan);
	return	w;
	}
#endif
default:
	return	prgmerr(0, "cannot get_row for %s", ITypeName[img->in_type]);
}
}


/*	STD_swif for data handling.
	The frame size	= frames * in_channels ; if using pipe read();
	otherwise,	= frames * dpy_channels
*/
std_interface(needs, img, fmt, vva_list)
U_IMAGE	*img;
char	*fmt;
VType	*vva_list;	/* actually, it should be < ... > in Turbo C. */
{
int	i=img->in_color, icf=img->color_form,
	status=0, fsize=img->width*img->height,
	frames=(needs==FI_SAVE_FILE) ? (img->save_all ? img->save_all : 1) *
			img->pxl_out : (img->load_all ? img->load_all : 1) *
			img->pxl_in * img->dpy_channels;
char	*pany = img->src;

switch(needs) {
case FI_LOAD_FILE:	/* vva_list is multi_hd (RLE) or OsameI8 */
	verify_buffer_size(&img->src, frames, fsize, "std_swif-src");
	pany = img->src;
aload:	switch(img->in_type)	{
#ifdef	FITS_IMAGE
	case FITS:
		status = fsize - read_fits_image(img, FTy, DCMP, No, No);
		break;
#endif
#if defined HIPS_IMAGE | defined HIPS_PUB
	case HIPS:
	rdHIPS:
#ifdef	STREAM_IMAGE
		if (is_stream_image(0))	{
			hhd.image = pany;
			if (fread_image(img->IN_FP, &hhd, img->fn, "stream") > -1)
				status = fsize;
			break;
		}
#endif
		img->cnvt = img->src;
		if (i == CFM_SEPLANE && icf != CFM_SEPLANE)	{
			img->src = pany;
			status = read_sepplane_image(img, 0,
				icf==CFM_SGF ? frames : frames/img->channels);
			img->src = img->cnvt;
		}
		else	{
			if (img->channels != img->dpy_channels)
			    if (i == CFM_ILC)
				frames *= img->channels,
				verify_buffer_size(&pany, frames, fsize,
					"ILC-src"),
				img->src = pany;
			    else if (i == CFM_SGF /* only here for get_pic */
					|| icf == CFM_SEPLANE)
				frames /= img->dpy_channels;
			status = (*img->i_read)(pany, frames, fsize, img->IN_FP);
		}
		frames /= img->channels;
		if (i == CFM_SCF)	{
		    if (!img->color_dpy) {
			map8_gray(img->src, pany, fsize, reg_cmap);
			img->color_form = CFM_SGF;
		    }
#if	defined	COMMON_TOOL | defined RLE_IMAGE
		    else if (!vva_list && img->mid_type == RLE &&
				icf != CFM_SEPLANE && img->dpy_channels == 3) {
			register char	*obp, *ibp;
			register int	l;
			    for (l=img->height; l--;)	{
				obp = (char*)img->src + img->width * 3 * l;
				ibp = (char*)img->src + img->width * l;
				ras8_to_rle(obp, ibp, img->width, img, reg_cmap, 1);
			    }
			    img->color_form = CFM_ILL; /* change color form */
			}
		}
		else if (i==CFM_ILC || i==CFM_ALPHA) {
		    if (icf == CFM_ILL)	{
			img->src = pany;
			pany = NZALLOC(img->channels, img->width, "db");
			for (frames=img->width*img->channels, i=img->height; i--;) {
			register byte*	p = ORIG_RLE_ROW(img, i);
				memcpy(pany, p, frames);
				any_ilc_to_rle(p, pany, img, img->channels, 0);
			}
			CFREE(pany);	img->src = img->cnvt;
		    }
#endif
		    else if (icf==CFM_SGF)
			ilc_to_gray(img->src, pany + 1, fsize * frames,
				img->in_color==CFM_ALPHA, NULL, 0);
		}
		break;
#endif
#if	defined	COMMON_TOOL | defined	RLE_IMAGE
	case RLE:
		if ((frames /= img->channels) > 1 &&
		verify_buffer_size(&img->src, frames, fsize, "multi-rle"))
			pany = img->src;
		frames += !frames;
		while (frames--)	{
			status = read_rle_image(img, reg_cmap, 0);
#ifndef	PRE_READ_HEADER
			if (!frames)	break;
#endif
			if (rle_get_setup(&rle_dflt_hdr) != RLE_SUCCESS ||
				rle_dflt_hdr.xmax - rle_dflt_hdr.xmin + 1
					!= img->width || img->height !=
				rle_dflt_hdr.ymax - rle_dflt_hdr.ymin + 1)
			break;
			img->src = (char*)img->src + fsize;
		}
		if (pany)	img->src = pany;
		break;
#endif
#ifdef	COMMON_TOOL
	case GIF:	status = read_gif_image(img, &GifScreen, 1);
		break;
	case ICC:	status = read_sepplane_image(img,
				icchd.img_hd.x_size - img->width, 1);
		break;
	case PBM:
	case PPM:
	case PNM:
		status = read_pnm_image(img, maxval, format, xel24, cht, reg_cmap);
		if (xel24 && !vva_list)
			free_2d_discrete(xel24, img->height);
		break;
	case PGM:	status = read_pgm_image(img, maxval, format);
		break;
	case RAS:	status = read_rast_image(img, &srhd, vva_list);
		break;
	case TiFF:
		status = read_tiff_image(img, reg_cmap, TIFFin, TiffRGB, vva_list);
		break;
#	ifdef	PICT_IMAGE
	case PICT:	status = read_pict_image(img, vva_list);
		break;
#	endif
#	ifdef	JPEG_IMAGE
	case JPEG:	status = read_jpeg_image(img, vva_list, 0);
		break;

	case XWD:	status = read_xwd_image(img, NULL, No);
		break;
#	endif
#endif
	default:
	status = prgmerr(0, "unable to process this type %d\n", img->in_type);
	}
	if (status < 0 || status != fsize)
		prgmerr(fmt, "%s: load %s[%d] %d\n",
			Real_STR(fmt), ITypeName[img->in_type], fsize, status);
	if (icf==CFM_SEPLANE && icf != i)
		any_to_seplane(img, img->dpy_channels, No, reg_cmap,
				needs == FI_RLOAD_BUF);
	break;

#if defined HIPS_IMAGE | defined HIPS_PUB
case FI_ACCESS_ABS_FRAME:
	switch (img->in_type) {
	case HIPS:
		i = (int) vva_list - img->fn;
		img->fn = (int) vva_list;
		(*img->r_seek)(img->IN_FP, i * fsize, SEEK_CUR);
		goto	rlbHIPS;
	default:	status = prgmerr(0, "only access HIPS");
	}	break;
#endif
case FI_LOAD_ROW:	status = ccs_get_row(img, fmt, vva_list);
	break;
case FI_RLOAD_BUF:
	if (img->in_type != HIPS)	img->src = fmt;
	switch (img->in_type) {
#ifdef	FITS_IMAGE
	case FITS:
		status = read_fits_image(img, FTy, DCMP, No, No);
		break;
#endif
#if defined HIPS_IMAGE | defined HIPS_PUB
	case HIPS:
rlbHIPS:	pany = fmt;	fmt = 0;
		goto	rdHIPS;	/*	buffer must be allocated	*/
#endif
#ifdef	JPEG_IMAGE
	case JPEG:
		status = read_jpeg_image(img, 0, 1);
	break;
#endif
	default:	goto	aload;
	}
	if (status != fsize)	prgmerr(0,"%s: load[%d] %d\n",
			ITypeName[img->in_type], fsize, status);
	break;

case FI_SAVE_FILE:
	switch (img->o_type)	{
	case FITS:
		status = prgmerr(0, "not linked to fits write routine\n");
		break;
#if defined HIPS_IMAGE | defined HIPS_PUB
	case HIPS:	{	/* only color RLE to SGF	*/
	int	ch=img->dpy_channels, typediff=img->mid_type==RLE && ch > 1;
	register int	w=img->width;
	register char*	bp = pany = (fmt ? fmt : (char*)img->src) +
				(int)vva_list * fsize;

		if (img->sub_img) {
			pany = bp += ch * img->sub_img_y * w + img->sub_img_x;
			w = img->sub_img_w;
			if (typediff)	{
				for (; ch--; bp = pany += img->width)
				    for (i=img->sub_img_h; i--;)	{
					status += fwrite(bp, img->pxl_out, w,
						img->OUT_FP) - w;
					bp += img->width * ch;
				    }
			} else while (frames--)	{
				for (i=img->sub_img_h; i--; bp+=img->width)
				    status += fwrite(bp, img->pxl_out, w,
					img->OUT_FP) - w;
				bp = pany += fsize;
			}
		}
		else if (typediff)	{
		    bp = NZALLOC(ch, fsize, "r-h");
		    for (i=0; i < frames; i++)	{
			ilc_to_sep(bp, pany + fsize * i, w, img->height, ch);
			status = fwrite(bp, ch, fsize, img->OUT_FP);
		    }
		    status -= fsize;	CFREE(bp);
		}
		else	status=(i=fwrite(bp, ch*frames, fsize, img->OUT_FP)) - fsize;

		if (img->update=status)
			status = prgmerr(0, "%s[%d] update %d",
					ITypeName[img->o_type], fsize, i);
	}	break;
#endif
#if	defined	COMMON_TOOL | defined RLE_IMAGE
	case RLE:	write_rle(img, vva_list, fmt);	break;
#endif

#if	defined	COMMON_TOOL & defined	WRITE_ANY_IMAGE
	case RAS:	write_rast(img, vva_list, fmt);	break;
#endif
	default:
		status = prgmerr(0, "unavailable type %d\n", img->o_type);
	}
	break;

case FI_WHAT_FILE:
	status = img->mid_type;
	break;

#ifdef	COMMON_TOOL
case FI_PNM_MAXVAL:
	status = (int) xel24;
	*((int*)fmt) = maxval;
	break;
#endif

case FI_DESC_ETA:
	{
	char	*tmp_buf;
	CENR	*cer=(CENR *) vva_list;
	int	c_c[3];
	    for (i=0; i<img->channels; i++)
		switch (cer[i].curve)	{
		case ETALinear:
			c_c[i] = cer[i].upper;	break;
		case ETAForeGD:
			c_c[i] = cer[i].fgrd;	break;
		case ETABackGD:
			c_c[i] = cer[i].bgrd;
		default:mesg("\a");	/* \7	*/
		}
	    sprintf(fmt, "%s [%d : %d]=R{%d}: -f%d; G{%d}: -f%d; B{%d}: -f%d\n",
		Progname, img->fn, img->linearup, cer[0].curve, c_c[0],
		cer[1].curve, c_c[1], cer[2].curve, c_c[2]);
	/* not use realloc(), for saving original description */
	    i = img->desc ? strlen(img->desc) : 0;
	    tmp_buf = nzalloc(strlen(fmt)+i+1, 1, "t_b");
	    if (img->desc)
		strcpy(tmp_buf, img->desc),
		CFREE(img->desc);
	    else
		tmp_buf[0] = 0;
	    strcat(tmp_buf, fmt);
	    img->desc = tmp_buf;
#ifdef  HIPS_IMAGE
	    if (img->o_type == HIPS)
		hhd.seq_desc = img->desc,
		hhd.sizedesc = pointer_buffer_size(tmp_buf);
#endif
	}
	break;

case FI_INIT_NAME:
	img->name = str_save(Real_STR(fmt));
#ifdef  HIPS_IMAGE
	if (img->o_type==HIPS)	{
		init_header(&hhd, img->name, "", img->frames, "",
			img->height, img->width, img->o_form, 1, vva_list ?
			(char*)vva_list : img->desc ? img->desc : "orig_init");
		img->pxl_out = hhd.sizepix;
	}
	break;

case FI_HIPS_HEADER_FORMAT:
	if (!hhd.orig_name || !hhd.seq_desc && !hhd.seq_history ||
		!hhd.orows || !hhd.ocols || !hhd.num_frame)
		status = (int)WHAT_Str;
	else	status = (int)formatheader(&hhd);
#endif
	break;

default:message("need help ? %d\n", status=needs);
}
if (needs == FI_RLOAD_BUF && img->in_type != HIPS)	img->src = pany;
return	status;
}
