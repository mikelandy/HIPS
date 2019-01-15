/*	JPEG_W . C
%
%	Copyright (c)	Lawrence Berkeley Labroatory
%
% AUTHOR:	Jin Guojun - LBL, Image Technology Group  1992
*/

#include "jinclude.h"
#ifdef INCLUDES_ARE_ANSI
#include <stdlib.h>	/* to declare exit() */
#endif
#include <ctype.h>	/* to declare tolower() */


LOCAL int	input_row_pos;

#ifdef	STREAM_IMAGE_LIB

LOCAL int	stream_is_linked;
void
config_stream_link(int yn)
{	stream_is_linked = yn;	}

#endif

METHODDEF void
input_init(compress_info_ptr cinfo)
{
register U_IMAGE*	img = cinfo->img;
#ifdef	STREAM_IMAGE_LIB
	if (!stream_is_linked)
#endif
		(*img->header_handle)(HEADER_READ, img, 0, 0, 0);
	cinfo->image_width = img->width;
	cinfo->image_height = img->height;
	cinfo->data_precision = 8;
	switch(img->color_form) {
	case CFM_SGF:
		 cinfo->in_color_space = CS_GRAYSCALE;
		cinfo->input_components = img->dpy_channels;
		break;
	case CFM_ILL:
	default:
		cinfo->in_color_space = CS_RGB;
		cinfo->input_components = 3;
	}
}

METHODDEF void
get_input_row(compress_info_ptr cinfo, JSAMPARRAY pixel_row)
{
register U_IMAGE*	img = cinfo->img;
register int	col = img->width;
register JSAMPROW	pr = pixel_row[0], pg = pixel_row[1], pb = pixel_row[2];
register byte	*p = (byte*) img->src + input_row_pos * img->dpy_channels * col;
	switch(img->color_form)	{
	case CFM_SCF:
		while (col--)	{
		register int	c = *p++;
			*pr++ = reg_cmap[0][c];
			*pg++ = reg_cmap[1][c];
			*pb++ = reg_cmap[2][c];
		}
		break;
	case CFM_SEPLANE:
		mesg("warning -- not ready\n");
	case CFM_ILC:
		snf_to_rle(pr, p, col, 3, NULL); /* ? */
		break;
	case CFM_ILL:
	    if (img->in_color == CFM_SEPLANE)	{
		p = (byte*) img->src + input_row_pos * col;
		memcpy(pb, p + (col * img->height << 1), col);
		memcpy(pg, p + col * img->height, col);
	} else	{
		memcpy(pb, (char*)p+(col<<1), col);
		memcpy(pg, (char*)p+col, col);
	}
	case CFM_SGF:
		memcpy(pr, (char*)p, col);
		break;
	default:	prgmerr('f', "strange color form %d", img->color_form);
	}
	input_row_pos++;
}

METHODDEF void
input_term(compress_info_ptr cinfo, int term)
{
	if (term)	free(cinfo->img->src);
	else	input_row_pos = 0;
}


void
select_file_type (compress_info_ptr cinfo, int is_targa)
{
	if (is_targa) {
#ifdef TARGA_SUPPORTED
		jselrtarga(cinfo);
#else
		ERREXIT(cinfo->emethods, "Targa support was not compiled");
#endif
	} else	{
		cinfo->methods->input_init = input_init;
		cinfo->methods->get_input_row = get_input_row;
		cinfo->methods->input_term = input_term;
	}
}
