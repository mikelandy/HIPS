/*	ICC_R . C
#
%	Copyright (c)	Jin Guojun
%
%	The input file is an either padded to 8-pixel in width,
%		or any 3-separate-plane images.
%	The output can be:	SGF, ILC, ILL, or SEP.
%
% AUTHOR:	Jin Guojun - LBL	10/01/1991
*/

#include "header.def"
#include "imagedef.h"


read_sepplane_image(img, cnt, frames)
U_IMAGE	*img;
{
char	junk[16], *tmp;
register int	ch=img->channels, i=img->color_form;
int	fsize = img->width * img->height, ret, f=0,
	sep_to_sg = img->in_color==CFM_SEPLANE && (i==CFM_SGF || i==CFM_ILC);

if ((int)img->src < 0x10)	return	EOF;
  /*	verify_buffer_size(&img->src, fsize, img->dpy_channels, "sep-buf"); */
tmp = img->src;
if (cnt < 0)	cnt = 0;
if (sep_to_sg)	tmp = NZALLOC(ch, fsize, "sep-tmp");

    {	register char	*op;	/* The default is to convert to RLE */
	register int	row, inc = img->width * (i==CFM_ILL ? ch : 1);

	do {
	    op = tmp;
	    for (ch=0; ch<img->channels; ch++)	{
		if (img->color_form == CFM_ILL)
			op = tmp + img->width * ch;
		for (row=i=0; row++ < img->height; op+=inc) {
			i += (*img->i_read)(op, 1, img->width, img->IN_FP);
			if (cnt && !(*img->i_read)(junk, cnt, 1, img->IN_FP))
				break;
		}
		if (i != fsize)	{
		    prgmerr(DEBUGANY, "[%d] read SEP data[%d] %d", ch, fsize, i);
			break;
		}
	    }
	    if ((ret = i) < 1)	break;
	    if (sep_to_sg)	{	/* could be multi-threaded	*/
		op = img->src;
		op += f * (i=fsize) * img->dpy_channels;
		if (img->color_form == CFM_SGF)	/* convert 3-channel to 1 */
			ill_to_gray(op, tmp, tmp+i, tmp+(i<<1), i);
		else	{	/* is line_to_cell_color() better ?	*/
		register char	*r=tmp, *g=r+i, *b=g+i;
			while (i--)
				*op++ = *r++,	*op++ = *g++,	*op++ = *b++;
		}
	    }
	    if (!sep_to_sg)	tmp += fsize * img->dpy_channels;
	} while(++f < frames);
    }

if (sep_to_sg)	CFREE(tmp);
if (img->mid_type == RLE)	/* safety for RLE map_scan	*/
	img->channels = img->dpy_channels;
return	ret;
}
