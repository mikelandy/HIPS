/*	SYSENV . C
#
%	Image system initial ROUTINE
%
%	Copyright (c)	Jin Guojun -	All right reserved
%
% AUTHOR:	Jin Guojun - LBL	10/1/90
*/

#include "header.def"
#include "imagedef.h"

#ifndef	H_H
#define	H_H	header_handle
#endif
#define	arg_list	job, img, ac, av, assist

#ifdef	INCLUDE_MORE_SYS
#include <sys/types.h>
#endif
#include <sys/stat.h>

static	struct	stat	statbuf;

extern	int	std_interface(), pict_header_handle(), jpeg_header_handle();

bridge_header_handle(swif_h_alist)
swif_h_alist_def;
{
register int	status;	/*	Hierarchy :	*/
    switch (job) {
    case HEADER_READ:
	av = (bool)av | (((BUFFER *)img->IN_FP)->flags != BUFFER_MAGIC &&
		ftell(img->IN_FP) < 0);
	status = pull_Itype(img);
	goto	hsw;
    case HEADER_TO:
	status = img->o_type;
hsw:	switch (status)	{
	case HIPS:
#if	defined	HIPS_IMAGE | defined HIPS_PUB
		if (!(status=hips_header_handle(arg_list)))	break;
#endif
	case FITS:
#ifdef	FITS_IMAGE
		if (!(status=fits_header_handle(arg_list)))	break;
#endif
	case RLE:
#ifdef	RLE_IMAGE
		if (!(status=rle_header_handle(arg_list)) ||
			status != RLE_NOT_RLE)
			break;
#endif
#ifdef	COMMON_TOOL
	case GIF:
		if (!(status=gif_header_handle(arg_list)))	break;
	case RAS:
		if (!(status=rast_header_handle(arg_list)))	break;
	case PNM:	case PBM:	case PGM:	case PPM:
		if (!(status=pnm_header_handle(arg_list)))	break;
	case ICC:
		if (!(status=icc_header_handle(arg_list)))	break;
/* other cases of first MAGIC character = 0 should be below this line */
#	ifdef	JPEG_IMAGE
		if (!(status=xwd_header_handle(arg_list)))	break;
	case JPEG:
		if (!(status=jpeg_header_handle(arg_list)))	break;
#	endif
	case PICT:
#	ifdef	PICT_IMAGE
		if (!(status=pict_header_handle(arg_list)))	break;
#	endif
	case 't':	if (img->table_if &&
		!(status = (*img->table_if)(arg_list)) ||
		((BUFFER*)img->IN_FP)->flags == BUFFER_MAGIC)	break;

	case TiFF:	/*	av===0 for lseek	*/
		if (!(status=tiff_header_handle(arg_list)))	break;
#endif
	default:	status = prgmerr(0, "%d Unknown image type", job);
	}
	if (job==HEADER_READ && !status)	{
		select_color_form(img, assist);
		if (img->in_form == IFMT_SCF && img->o_type == HIPS)
			img->in_form = IFMT_BYTE;	/* vague format	*/
	}
	if (img->frames < 1)	img->frames++;	/* insurance.	*/
	break;
    case HEADER_TRANSF:
	img->pxl_out = img->pxl_in;	img->o_form = img->in_form;
	img->color_form = img->in_color;
	break;
    case HEADER_WRITE:
    default:
hhd_bridge:	status =
#if	defined	HIPS_IMAGE | defined HIPS_PUB
			hips_header_handle(arg_list);
#else
			rle_header_handle(arg_list);
#endif
    }
return	status;
}



format_init(img, in_type, mid_type, o_type, pgname, vers)
U_IMAGE	*img;
char	*pgname, *vers;
{
img->in_type = in_type;
img->mid_type = mid_type;
img->o_type = (o_type < 0) ? mid_type : o_type;
img->mag_fact = img->update_header = 1;	/* when output 3D sub-image, set it to False */

Progname = pgname;
Mversion = vers;

#ifdef	HIPS2_HF
hipserrprt = hipserrlev = HEL_SEVERE;
#endif

img->errors = prgmerr;
img->std_swif = std_interface;
img->dpy_channels = img->color_dpy ? 3 : 1;
if (!img->OUT_FP || fstat(fileno(img->OUT_FP), &statbuf))
	img->OUT_FP = stdout;
if (!img->IN_FP || fstat(fileno(img->IN_FP), &statbuf))
	img->IN_FP = stdin;
reset_pipe_read(img);

if (in_type > mid_type) in_type = mid_type;

	switch (in_type) {
#ifdef	COMMON_TOOL
# ifdef	JPEG_IMAGE
	case JPEG:	img->H_H = jpeg_header_handle;	break;
# endif
# ifdef	PICT_IMAGE
	case PICT:	img->H_H = pict_header_handle;	break;
# endif
#endif
	case HIPS:
	default:/* otherwise use bridge handler, a basic common tool */
		img->H_H = bridge_header_handle;
	}
	if (in_type > ENDITYPE)	{
		message("unknown format %d\n", in_type);
		return	in_type;
	}
return	0;
}

available_type(name_str)	/* non-case sensitive	*/
register char	*name_str;
{
register int	i;
	for (i=0; name_str[i]; i++)
		name_str[i] = toupper(name_str[i]);
	for (i=ENDITYPE; i-- && strcmp(name_str, ITypeName[i]););
return	i;
}

pull_Itype(img)	/*	try for piping	*/
U_IMAGE	*img;
{
register int	c, t = img->in_type;
    if (t != TiFF)	{	/*	default:	*/
	switch (c=(*img->r_seek)(img->IN_FP, 0, SEEK_PEEK))	{
	case 'H':	t = HIPS;	break;
	case 'S':	t = FITS;	break;
	case 'R':	t = RLE;	break;
	case 0x95:	/* Little Endian	*/
	case 'Y':	t = RAS;	break;
	case 'G':	t = GIF;	break;
	case 'I':	/* L.E.	*/
	case 'M':	t = TiFF;	break;
	case 'P':	t = PNM;	break;
	case 0xFF:	t = JPEG;	break;
	/* try ICC first, other 0s no piping, and must cascade after ICC */
	case 0	:	t = ICC;	break;
	default:
		if (isalnum(c) && img->table_if)
			t = 't';	/* use table int_f	*/
	}
	(*img->r_seek)(img->IN_FP, c, SEEK_UGETB);
    }
return	t;
}

bytes_in_colortype(type)
{
if (type < CFM_SCF || type==CFM_BITMAP)
	return	1;
return	3;
}


usage_n_options(usages, nth, nth_para)	/* should go somewhere else	*/
char	*usages, *nth_para;
{
syserr("wrong option[%d] {%s}\n%s", nth, nth_para, usages);
}
