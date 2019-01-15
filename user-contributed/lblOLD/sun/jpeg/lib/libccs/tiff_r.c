/*	TIFF_Read . C
%
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
% AUTHOR:	Jin Guojun - LBL	8/1/91
*/

#include "header.def"
#include "imagedef.h"

#define	obuf	img->src
#define	GetTiffValue(f, v)	TIFFGetField(TiffIn, f, v)

/*	load routine is not dependent on img->channels,
%	so, header_handle only need set up img->dpy_chaanels.
*/

read_tiff_image(img, colormap, TiffIn, TiffRGB, OsameI8)
U_IMAGE	*img;
cmap_t	*colormap[3];
TIFF	*TiffIn;
{
byte	*inbuf;
int	pm, maxval, width, height, row,
	samples = TiffIn->tif_dir.td_samplesperpixel;
unsigned short	config, bitspersample;
register byte	*obp;

GetTiffValue(TIFFTAG_IMAGEWIDTH, &width);
GetTiffValue(TIFFTAG_IMAGELENGTH, &height);

GetTiffValue(TIFFTAG_BITSPERSAMPLE, &bitspersample);
maxval = (1<<bitspersample) - 1;
GetTiffValue(TIFFTAG_PLANARCONFIG, &config);

switch (config) {
case PLANARCONFIG_CONTIG: {
register int	col, shift=bitspersample, pad=shift & 1, scale = 8 - shift + pad;
#define	Next_Sample(bit_shift, bits_ps, ibp)	\
	{	\
		if (!bit_shift)	{	bit_shift=8,	ibp++;	}	\
		bit_shift -= bits_ps;	\
	}

	inbuf = (byte*)NZALLOC(1, TIFFScanlineSize(TiffIn), "tiff-cell-buf");
	msg("pmetric %d\n", pm=TiffIn->tif_dir.td_photometric);
	for (row=0; row < height; row++) {
	register byte	*ibp=inbuf;
		obp = (byte*)obuf + width*row*img->dpy_channels;
		if (TIFFReadScanline(TiffIn, ibp, row, 0) < 0)
			break;
		shift = 8;
		switch (pm)	{
		case PHOTOMETRIC_MINISWHITE:
			if (scale) for (col=width; col--;)	{
			register int	v;
				Next_Sample(shift, bitspersample, ibp);
				v = maxval - ((*ibp >> shift) & maxval) << scale;
				*obp++ = v - (v > 0) * pad;
			}
			else unroll8_bwd(col=width, col, /* speed 8-bit proc. */
				*obp++ = maxval - *ibp++);
			break;
		case PHOTOMETRIC_MINISBLACK:
			if (scale) for (col=width; col--;)	{
			register int	v;
				Next_Sample(shift, bitspersample, ibp);
				v = ((*ibp >> shift) & maxval) << scale;
				*obp++ = v - (v > 0) * pad;
			}
			else	memcpy(obp, ibp, col=width),	/* speed 256 */
				*obp += col;
			break;
	/*	case PHOTOMETRIC_MASK:	*/

		case PHOTOMETRIC_RGB:
		case PHOTOMETRIC_SEPARATED:
		case PHOTOMETRIC_YCBCR:
		    if (img->mid_type == COLOR_PS)
			memcpy(obp, ibp, width * img->dpy_channels);
		    else if (img->color_dpy)	{	/* for safety */
			if (img->color_form != CFM_ILC)
				any_ilc_to_rle(obp, ibp, img, samples, No);
			else	ilc_transfer(obp, ibp, width, samples, No, 3);
		    } else	ilc_to_gray(obp, ibp, width, samples==4,
						NULL /* no mapping */, True);
			break;
		case PHOTOMETRIC_PALETTE:	/* indexed	*/
		if (!img->color_dpy)
			map8_gray(obp, ibp, width, colormap);
		else if (OsameI8)
			memcpy(obp, ibp, width * img->dpy_channels);
		else
			snf_to_rle(obp, ibp, width, bitspersample, colormap);
		break;
		default:	return	prgmerr(0, "unknown tiff format");
		}
	}
    }	break;
case PLANARCONFIG_SEPARATE:	{
register int	s;
int rowbytes = TIFFScanlineSize(TiffIn);

	inbuf = (byte*)NZALLOC(samples, rowbytes, "tiff-line-buf");
	for (row = 0; row < height; row++) {
	    obp = (byte*)obuf + width*row*img->dpy_channels;
	    for (s=0; s < samples; s++)
		if (TIFFReadScanline(TiffIn, inbuf+s*rowbytes, row, s)<0)
			return	prgmerr(DEBUGANY, "Tiff read scanline");
	    if (img->color_dpy || img->dpy_channels==samples)
		memcpy(obp, inbuf, width * samples);
	    else
		ill_to_gray(obp, inbuf, inbuf + rowbytes, inbuf + 2*rowbytes,
			width);
	}
    }	break;
default:	return	prgmerr(0, "not a applicable plan %d\n", config);
}
if (img->mid_type == RLE)
	if ((img->channels = img->dpy_channels) > 1)
		img->in_color = CFM_ILL;
return	width * row;
}
