/*
%	TOTIFF . C
%
% AUTHOR:	Jin Guojun - LBL	1991
*/

#include "header.def"
#include "imagedef.h"

arg_fmt_list_string	arg_fmt[] =	{
	{"-none", "%Nh", COMPRESSION_NONE, 1, 0,
		"no compression at all (default = LZW)"},
	{"-pac", "%Nh", COMPRESSION_PACKBITS, 1, 0, "packbits"},
	{"-g3", "%Nh", COMPRESSION_CCITTFAX3, 1, 0, "compress"},
	{"-g4", "%Nh", COMPRESSION_CCITTFAX4, 1, 0, "compress"},
	{"-pred", "%h", 0, 1, 1, "predictor"},
	{"-row", "%d", 0, 1, 1, "rowsperstrip"},
	{"-msb", "%Nh", FILLORDER_MSB2LSB, 1, 0, "MSB to LSB"},
	{"-lsb", "%Nh", FILLORDER_LSB2MSB, 1, 0, "LSB to MSB"},
	{"-2d", "%|h", GROUP3OPT_2DENCODING, 1, 0, "2D encoding"},
	{"-fill", "%|h", GROUP3OPT_FILLBITS, 1, 0, "fill bits"},
	{"-k", "%b", True, 1, 0, "keep working, ignore input error"},
	{"-revs", "%+", No, 1, 0, "RGB to BGR"},
	{"-sep", "%b", PLANARCONFIG_SEPARATE, 1, 0,
		"SEPARATE PLANE for 24-bit image?"},
{"I/O:	[<] input [[> |] output]", "0", 0, 0, 0, "end of usage"},
	NULL	};

bool	revs;
U_IMAGE	uimg;
TIFF*	TIFFout;

#define	rows	uimg.height
#define	cols	uimg.width
#define	ocfm	uimg.color_form
#define	SetTiFF(ctrl, val)	TIFFSetField(TIFFout, ctrl, val)
#define	WriteTiFF(buf, w, rgb)	TIFFWriteScanline(TIFFout, buf, w, rgb)

main(ac, av)
int	ac;
char*	av[];
{
char*	*fl;
unsigned short	compression=COMPRESSION_LZW, config=PLANARCONFIG_CONTIG,
	fillorder=0, predictor=0, photometric=PHOTOMETRIC_MINISBLACK,
	samplesperpixel=1, bitspersample=8;
long	g3options=0, rowsperstrip;
int	bytesperrow, i, nostop, row;
register byte	*obp;

	if ((i=parse_argus(&fl, ac, av, arg_fmt,
		&compression, &compression, &compression, &compression,
		&predictor, &rowsperstrip,
		&fillorder, &fillorder,
		&g3options, &g3options,
		&nostop, &revs,	&config)) < 0)
		exit(i);
	if (predictor < 0 && predictor > 2 || rowsperstrip < 0)	{
info:		parse_usage(arg_fmt);	exit(0);	}

	if (i && (in_fp=freopen(fl[0], "rb", stdin)) == NULL)
		syserr("open input %s", fl[0]);

uimg.color_dpy = -1;
format_init(&uimg, IMAGE_INIT_TYPE, RLE, TiFF, *av, "S1-2");

io_test(fileno(in_fp), goto	info);

if ((*uimg.header_handle)(HEADER_READ, &uimg, 0, 0, Yes))
	syserr("unknown image type");

TIFFout = TIFFFdOpen(stdout_fd, "Standard Output", "w");
if (!TIFFout)
	syserr("open TIFF output file");
if (fillorder)
	revs = !revs,
	SetTiFF(TIFFTAG_FILLORDER, fillorder);	/* for tiff.3.0	*/
if (revs && ocfm==CFM_ILC)	ocfm = CFM_ILL;
(*uimg.std_swif)(FI_LOAD_FILE, &uimg, nostop ? NULL : uimg.name,
	True /* don't change format, and save PNM loading buffer */);

	switch (ocfm) {	/* Figure out TIFF parameters. */
	case CFM_BITMAP:
		bitspersample = 1;
		bytesperrow = (cols + 7) / 8;
		if (uimg.in_color != ocfm)
			uimg.dest = nzalloc(bytesperrow, 1, "bm-obuf");
		if (uimg.in_type==RAS)	photometric = PHOTOMETRIC_MINISWHITE;
		break;
	case CFM_SGF:
		bitspersample = min_bits(255);
		i = 8 / bitspersample;
		bytesperrow = (cols + i - 1) / i;
		break;
	case CFM_SCF:
		photometric = PHOTOMETRIC_PALETTE;
		bytesperrow = cols;
		if (!r_cmap)
			r_cmap = (sht_cmap_t*) rle_dflt_hdr.cmap;
		break;
	case CFM_ILL:
		uimg.dest = nzalloc(cols, 3, "24-obuf");
	case CFM_ILC:
	default:
		samplesperpixel = 3;
		photometric = PHOTOMETRIC_RGB;
		bytesperrow = cols * 3;
	}

	if (!rowsperstrip)	rowsperstrip = 8192 / bytesperrow;

/* Set TIFF parameters. */
	SetTiFF(TIFFTAG_IMAGEWIDTH, cols);
	SetTiFF(TIFFTAG_IMAGELENGTH, rows);
	SetTiFF(TIFFTAG_BITSPERSAMPLE, bitspersample);
	SetTiFF(TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
	SetTiFF(TIFFTAG_COMPRESSION, compression);
	if (compression == COMPRESSION_CCITTFAX3 && g3options)
		SetTiFF(TIFFTAG_GROUP3OPTIONS, g3options);
	if (compression == COMPRESSION_LZW && predictor)
		SetTiFF(TIFFTAG_PREDICTOR, predictor);
	SetTiFF(TIFFTAG_PHOTOMETRIC, photometric);
	if (uimg.desc)
		SetTiFF(TIFFTAG_DOCUMENTNAME, uimg.desc);
	SetTiFF(TIFFTAG_IMAGEDESCRIPTION, "totiff");
	SetTiFF(TIFFTAG_SAMPLESPERPIXEL, samplesperpixel);
	SetTiFF(TIFFTAG_ROWSPERSTRIP, rowsperstrip);
	/*	SetTiFF(TIFFTAG_STRIPBYTECOUNTS, rows / rowsperstrip);	*/
	SetTiFF(TIFFTAG_PLANARCONFIG, config);
	if (reg_cmap[0])
		regmap_to_rlemap(reg_cmap, uimg.cmaplen, 3, &rle_dflt_hdr),
		i = 1 << rle_dflt_hdr.cmaplen,
		TIFFSetField(TIFFout, TIFFTAG_COLORMAP,
			r_cmap, r_cmap+i, r_cmap+(i<<1));

	obp = (byte*)uimg.src;

	/*	Write the TIFF buf	*/
    for (row=0; row < rows; row++, obp+=cols) {
	switch (ocfm) {
	case CFM_ILL:
	    if	(config == PLANARCONFIG_SEPARATE)	{
		if (WriteTiFF(obp, row, 0) < 0)	goto	wrterr;
		obp += cols;
		if (WriteTiFF(obp, row, 1) < 0)	goto	wrterr;
		obp += cols;
		if (WriteTiFF(obp, row, 2) < 0)	goto	wrterr;
	    }
	    else	{
	    register byte*	bp = uimg.dest, *inr=obp, *ing=(obp+=cols);
		if (revs) for (i=0, obp+=cols; i<cols; i++)	{
			*bp++ = obp[i];
			*bp++ = ing[i];
			*bp++ = inr[i];
		}
		else	for (i=0, obp+=cols; i<cols; i++)	{
			*bp++ = inr[i];	*bp++ = ing[i];	*bp++ = obp[i];
		}
		goto	wrt;
	    }	break;
	case CFM_BITMAP:
	if (uimg.in_color == ocfm)
	case CFM_SGF:
	case CFM_SCF:
	case CFM_ILC:
		uimg.dest = (char*)uimg.src + bytesperrow * row;
	else	{
	register int	col, bitshift;
	register char	*bp = uimg.dest;

	    bitshift = 8 - bitspersample;
	    *bp = 0;
	    for (col=0; col < cols; col++) {
		if (obp[col])
			*bp |= 1 << bitshift;
		bitshift -= bitspersample;
		if (bitshift < 0) {
			bitshift = 8 - bitspersample;
			*++bp = 0;
		}
	    }
	}
wrt:	if (WriteTiFF(uimg.dest, row, 0) < 0)
wrterr:		syserr("write scanline on row %d", row);
	break;
	default:	prgmerr(-1, "unknown format");
	}
    }
TIFFFlushData(TIFFout);
TIFFClose(TIFFout);

exit(0);
}
