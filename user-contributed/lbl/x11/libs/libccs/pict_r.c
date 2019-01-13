/*	PICT_Read . C
#
%	input PICT (Mac); output 24-bit RLE (ILL), SGF, or SEPLANE.
%
% AUTHOR:	Jin Guojun - LBL
*/

#include "header.def"
#include "imagedef.h"

typedef	unsigned short	u_sht;

typedef	struct	{
	u_sht	top, left, bottom, right;
	} Rect;

typedef	struct	{
	Rect	Bounds;
	u_sht	version,
		packType;
	longword
		packSize,
		hRes,
		vRes;
	u_sht	pixelType,
		pixelSize,
		cmpCount,
		cmpSize;
	longword
		planeBytes,
		pmTable,
		pmReserved;
	} pixMap;

typedef	struct	{
	u_sht	red, green, blue;
	} ct_entry;

typedef	struct	{
	char*	name;
	int	len;
	void	(*impl)();
	char*	description;
	} opdef;

#define WORD_LEN (-1)
/* seems like RGB colours are 6 bytes, but Apple says they're variable */
#define	RGB_LEN	(6)
#define NA	(0)

#define	RVD	"reserved"
#define	RVAU	"reserved for Apple use"
#define	RECT	"rect"
#define	RECT_sAaA	"rect, startAngle, arcAngle"
#define	SPOR	skip_poly_or_region

/* for reserved opcodes of known length */
#define res(length) \
	{ RVD, (length), NULL, RVAU }

/* for reserved opcodes of length determined by a function */
#define resf(skipfunction) \
	{ RVD, NA, (skipfunction), RVAU }

static	void	Clip(), BkPixPat(), PnPixPat(), FillPixPat(), LongText(),
		DHText(), DVText(), DHDVText(), skip_poly_or_region(),
		BitsRect(), BitsRegion(), Opcode_9A(), LongComment(),
		read_rect(), dump_rect(), do_bitmap(), blit(),
		read_pattern(), read_pixmap(), rectinter(), compact_image();
static	byte	*expand_buf();
static	ct_entry	*read_colour_table();

static opdef	optable[] = {
/* 0x00 */	{ "NOP", 0, NULL, "nop" },
/* 0x01 */	{ "Clip", NA, Clip, "clip" },
/* 0x02 */	{ "BkPat", 8, NULL, "background pattern" },
/* 0x03 */	{ "TxFont", 2, NULL, "text font (u_sht)" },
/* 0x04 */	{ "TxFace", 1, NULL, "text face (byte)" },
/* 0x05 */	{ "TxMode", 2, NULL, "text mode (u_sht)" },
/* 0x06 */	{ "SpExtra", 4, NULL, "space extra (fixed point)" },
/* 0x07 */	{ "PnSize", 4, NULL, "pen size (point)" },
/* 0x08 */	{ "PnMode", 2, NULL, "pen mode (u_sht)" },
/* 0x09 */	{ "PnPat", 8, NULL, "pen pattern" },
/* 0x0a */	{ "FillPat", 8, NULL, "fill pattern" },
/* 0x0b */	{ "OvSize", 4, NULL, "oval size (point)" },
/* 0x0c */	{ "Origin", 4, NULL, "dh, dv (u_sht)" },
/* 0x0d */	{ "TxSize", 2, NULL, "text size (u_sht)" },
/* 0x0e */	{ "FgColor", 4, NULL, "foreground color (longword)" },
/* 0x0f */	{ "BkColor", 4, NULL, "background color (longword)" },
/* 0x10 */	{ "TxRatio", 8, NULL, "numer (point), denom (point)" },
/* 0x11 */	{ "Version", 1, NULL, "version (byte)" },
/* 0x12 */	{ "BkPixPat", NA, BkPixPat, "color background pattern" },
/* 0x13 */	{ "PnPixPat", NA, PnPixPat, "color pen pattern" },
/* 0x14 */	{ "FillPixPat", NA, FillPixPat, "color fill pattern" },
/* 0x15 */	{ "PnLocHFrac", 2, NULL, "fractional pen position" },
/* 0x16 */	{ "ChExtra", 2, NULL, "extra for each character" },
/* 0x17 - 0x19	*/	res(0),	res(0),	res(0),
/* 0x1a */	{ "RGBFgCol", RGB_LEN, NULL, "RGB foreColor" },
/* 0x1b */	{ "RGBBkCol", RGB_LEN, NULL, "RGB backColor" },
/* 0x1c */	{ "HiliteMode", 0, NULL, "hilite mode flag" },
/* 0x1d */	{ "HiliteColor", RGB_LEN, NULL, "RGB hilite color" },
/* 0x1e */	{ "DefHilite", 0, NULL, "Use default hilite color" },
/* 0x1f */	{ "OpColor", RGB_LEN, NULL, "RGB OpColor for arithmetic modes" },
/* 0x20 */	{ "Line", 8, NULL, "pnLoc (point), newPt (point)" },
/* 0x21 */	{ "LineFrom", 4, NULL, "newPt (point)" },
/* 0x22 */	{ "ShortLine", 6, NULL, "pnLoc (point, dh, dv (-128 .. 127))" },
/* 0x23 */	{ "ShortLineFrom", 2, NULL, "dh, dv (-128 .. 127)" },
/* 0x24 - 0x27	*/	res(WORD_LEN),	res(WORD_LEN),
			res(WORD_LEN),	res(WORD_LEN),
/* 0x28 */	{ "LongText", NA, LongText, "txLoc (point), count (0..255), text" },
/* 0x29 */	{ "DHText", NA, DHText, "dh (0..255), count (0..255), text" },
/* 0x2a */	{ "DVText", NA, DVText, "dv (0..255), count (0..255), text" },
/* 0x2b */	{ "DHDVText", NA, DHDVText, "dh, dv (0..255), count (0..255), text" },
/* 0x2c - 0x2F	*/	res(WORD_LEN),	res(WORD_LEN),
			res(WORD_LEN),	res(WORD_LEN),
/* 0x30 */	{ "frameRect", 8, NULL, RECT },
/* 0x31 */	{ "paintRect", 8, NULL, RECT },
/* 0x32 */	{ "eraseRect", 8, NULL, RECT },
/* 0x33 */	{ "invertRect", 8, NULL, RECT },
/* 0x34 */	{ "fillRect", 8, NULL, RECT },
/* 0x35 - 0x37	*/	res(8),	res(8),	res(8),
/* 0x38 */	{ "frameSameRect", 0, NULL, RECT },
/* 0x39 */	{ "paintSameRect", 0, NULL, RECT },
/* 0x3a */	{ "eraseSameRect", 0, NULL, RECT },
/* 0x3b */	{ "invertSameRect", 0, NULL, RECT },
/* 0x3c */	{ "fillSameRect", 0, NULL, RECT },
/* 0x3d - 0x3F	*/	res(0),	res(0),	res(0),
/* 0x40 */	{ "frameRRect", 8, NULL, RECT },
/* 0x41 */	{ "paintRRect", 8, NULL, RECT },
/* 0x42 */	{ "eraseRRect", 8, NULL, "rect" },
/* 0x43 */	{ "invertRRect", 8, NULL, "rect" },
/* 0x44 */	{ "fillRRrect", 8, NULL, "rect" },
/* 0x45 - 0x47	*/	res(8),	res(8),	res(8),
/* 0x48 */	{ "frameSameRRect", 0, NULL, "rect" },
/* 0x49 */	{ "paintSameRRect", 0, NULL, "rect" },
/* 0x4a */	{ "eraseSameRRect", 0, NULL, "rect" },
/* 0x4b */	{ "invertSameRRect", 0, NULL, "rect" },
/* 0x4c */	{ "fillSameRRect", 0, NULL, "rect" },
/* 0x4d - 0x4F	*/	res(0),	res(0),	res(0),
/* 0x50 */	{ "frameOval", 8, NULL, "rect" },
/* 0x51 */	{ "paintOval", 8, NULL, "rect" },
/* 0x52 */	{ "eraseOval", 8, NULL, "rect" },
/* 0x53 */	{ "invertOval", 8, NULL, "rect" },
/* 0x54 */	{ "fillOval", 8, NULL, "rect" },
/* 0x55 - 0x57	*/	res(8),	res(8),	res(8),
/* 0x58 */	{ "frameSameOval", 0, NULL, "rect" },
/* 0x59 */	{ "paintSameOval", 0, NULL, "rect" },
/* 0x5a */	{ "eraseSameOval", 0, NULL, "rect" },
/* 0x5b */	{ "invertSameOval", 0, NULL, "rect" },
/* 0x5c */	{ "fillSameOval", 0, NULL, "rect" },
/* 0x5d */	res(0),
		res(0),
/* 0x5f */	res(0),
/* 0x60 */	{ "frameArc", 12, NULL, RECT_sAaA },
/* 0x61 */	{ "paintArc", 12, NULL, RECT_sAaA },
/* 0x62 */	{ "eraseArc", 12, NULL, RECT_sAaA },
/* 0x63 */	{ "invertArc", 12, NULL, RECT_sAaA },
/* 0x64 */	{ "fillArc", 12, NULL, RECT_sAaA },
/* 0x65 */	res(12),
		res(12),
/* 0x67 */	res(12),
/* 0x68 */	{ "frameSameArc", 4, NULL, RECT_sAaA },
/* 0x69 */	{ "paintSameArc", 4, NULL, RECT_sAaA },
/* 0x6a */	{ "eraseSameArc", 4, NULL, RECT_sAaA },
/* 0x6b */	{ "invertSameArc", 4, NULL, RECT_sAaA },
/* 0x6c */	{ "fillSameArc", 4, NULL, RECT_sAaA },
/* 0x6d */	res(4),
		res(4),
/* 0x6f */	res(4),
/* 0x70 */	{ "framePoly", NA, SPOR, "poly" },
/* 0x71 */	{ "paintPoly", NA, SPOR, "poly" },
/* 0x72 */	{ "erasePoly", NA, SPOR, "poly" },
/* 0x73 */	{ "invertPoly", NA, SPOR, "poly" },
/* 0x74 */	{ "fillPoly", NA, SPOR, "poly" },
/* 0x75 */	resf(SPOR),
		resf(SPOR),
		resf(SPOR),
/* 0x78 */	{ "frameSamePoly", 0, NULL, "poly (NYI)" },
/* 0x79 */	{ "paintSamePoly", 0, NULL, "poly (NYI)" },
/* 0x7a */	{ "eraseSamePoly", 0, NULL, "poly (NYI)" },
/* 0x7b */	{ "invertSamePoly", 0, NULL, "poly (NYI)" },
/* 0x7c */	{ "fillSamePoly", 0, NULL, "poly (NYI)" },
/* 0x7d */	res(0),
/* 0x7e */	res(0),
/* 0x7f */	res(0),
/* 0x80 */	{ "frameRgn", NA, SPOR, "region" },
/* 0x81 */	{ "paintRgn", NA, SPOR, "region" },
/* 0x82 */	{ "eraseRgn", NA, SPOR, "region" },
/* 0x83 */	{ "invertRgn", NA, SPOR, "region" },
/* 0x84 */	{ "fillRgn", NA, SPOR, "region" },
/* 0x85 */	resf(SPOR),
		resf(SPOR),
/* 0x87 */	resf(SPOR),
/* 0x88 */	{ "frameSameRgn", 0, NULL, "region (NYI)" },
/* 0x89 */	{ "paintSameRgn", 0, NULL, "region (NYI)" },
/* 0x8a */	{ "eraseSameRgn", 0, NULL, "region (NYI)" },
/* 0x8b */	{ "invertSameRgn", 0, NULL, "region (NYI)" },
/* 0x8c */	{ "fillSameRgn", 0, NULL, "region (NYI)" },
/* 0x8d */	res(0),
		res(0),
/* 0x8f */	res(0),
/* 0x90 */	{ "BitsRect", NA, BitsRect, "copybits, rect clipped" },
/* 0x91 */	{ "BitsRgn", NA, BitsRegion, "copybits, rgn clipped" },
/* 0x92 */	res(WORD_LEN),
		res(WORD_LEN),
		res(WORD_LEN),
		res(WORD_LEN),
		res(WORD_LEN),
/* 0x97 */	res(WORD_LEN),
/* 0x98 */	{ "PackBitsRect", NA, BitsRect, "packed copybits, rect clipped" },
/* 0x99 */	{ "PackBitsRgn", NA, BitsRegion, "packed copybits, rgn clipped" },
/* 0x9a */	{ "Opcode_9A", NA, Opcode_9A, "the mysterious opcode 9A" },
/* 0x9b */	res(WORD_LEN),
		res(WORD_LEN),
		res(WORD_LEN),
		res(WORD_LEN),
/* 0x9f */	res(WORD_LEN),
/* 0xa0 */	{ "ShortComment", 2, NULL, "kind (u_sht)" },
/* 0xa1 */	{ "LongComment", NA, LongComment, "kind (u_sht), size (u_sht), data" }
};

static	int	align, version;
static	Rect	picFrame;
static	u_sht	*red, *green, *blue,
		rowlen;
static longword	planelen;

static
read_byte(img)
U_IMAGE*	img;
{
	++align;
/*
	return	fgetc(img);
*/	return	(*img->r_seek)(img->IN_FP, 0, SEEK_GETB);
}

static
read_u_sht(img)
U_IMAGE*	img;
{
int	bh, bl;

	bh = read_byte(img);	bl = read_byte(img);
	if (bh<0 || bl<0)
		return	-1;
	return((bh << 8) | bl);
}

static longword
read_long(img)
U_IMAGE*	img;
{
int	lh = read_u_sht(img);
	return((lh << 16) | read_u_sht(img));
}

static
skip(n, img)
U_IMAGE*	img;
{
char	buf[1024];

	for (align += n; n > 0; n -= 1024)
	    if ((*img->i_read)(buf, n > 1024 ? 1024 : n, 1, img->IN_FP) != 1)
		return	prgmerr(DEBUGANY, "EOF / while skip %d", n);
return	0;
}

static
read_n(n, buf, img)
char*	buf;
U_IMAGE	*img;
{
	align += n;
	if ((*img->i_read)(buf, n, 1, img->IN_FP) != 1)
		return	prgmerr(0, "EOF / read %d", n);
return	n;
}

/*
*	data in version 2 is u_sht aligned. Odd size data is padded with 0.
*/
static
get_op(version, img)
U_IMAGE*	img;
{
	if ((align & 1) && version == 2)
		read_byte(img);	/* align for opcode */

	/*	reading opcode	*/
	if (version == 1)
		return	read_byte(img);
	else
		return	read_u_sht(img);
}


pict_header_handle(job, img, ac, av, assist)
U_IMAGE	*img;
int	ac;
char	**av;
VType	*assist;
{
byte	ch;
u_sht	picSize;

    switch (job) {
    case HEADER_READ:
    case HEADER_FREAD:
	if (!av)
		(*img->r_seek)(img->IN_FP, 0, 0);

	/*	pass 512 byte header	*/
	skip(512, img);
	picSize = read_u_sht(img);

	DEBUGMESSAGE("picture size = %d (0x%x)", picSize, picSize);

	read_rect(&picFrame, img);

	if (DEBUGANY) {
		dump_rect("Picture frame:", &picFrame);
		message("Picture size is %d x %d",
			picFrame.right - picFrame.left,
			picFrame.bottom - picFrame.top);
	}

	/* allocation is same for version 1 or version 2.  We are super-duper
	 * wasteful of memory for version 1 picts.  Someday, we'll separate
	 * things and only allocate a byte per pixel for version 1 (or heck,
	 * even only a bit, but that would require even more extra work).
	 */

	img->width = rowlen = picFrame.right - picFrame.left;
	img->height = picFrame.bottom - picFrame.top;
	planelen = rowlen * img->height;

	img->pxl_in = 1;
	img->channels = 3;
	img->in_type = PICT;

	while (!(ch = read_byte(img)));

	if (ch != 0x11)
pict_hderr:	return	EOF;

	switch (read_byte(img)) {
	case 1:
		version = 1;
		img->in_form = IFMT_SCF;
		break;
	case 2:
		if (read_byte(img) != 0xff)	{
			mesg("version 2, only subcode 0xff");
			goto	pict_hderr;
		}
		img->in_form = IFMT_SEPLANE;
		version = 2;
		break;
	default:	goto	pict_hderr;
	}
	img->in_color = CFM_SEPLANE;
	break;
    case HEADER_WRITE:
    default:	return	prgmerr(0, "no such a pict job %d\n", job);
    }
DEBUGMESSAGE("PICT version %d", version);
return	0;
}

read_pict_image(img, OsameI8)
U_IMAGE	*img;
{
int	opcode,	len, rows = img->height, cols = img->width;

red = NZALLOC(rows * cols, 3 * sizeof(*red), "pict-rect");
green = red + rows * cols;
blue = green + rows * cols;

	while((opcode = get_op(version, img)) != 0xFF) {
	    if (opcode < 0xA2) {
		if (optable[opcode].impl != NULL)
			(*optable[opcode].impl)(version, img);
		else if (optable[opcode].len >= 0)
			skip(optable[opcode].len, img);
		else switch (optable[opcode].len) {
		case WORD_LEN:
			len = read_u_sht(img);
			skip(len, img);
			break;
		default:
			return	prgmerr(DEBUGANY, "can't do length of %d",
					optable[opcode].len);
		}
	    }
	    else if (opcode == 0xC00) {
		DEBUGMESSAGE("HeaderOp");
		skip(24, img);
	    }
	    else if (opcode > 0xA1 && opcode < 0xB0) {
		DEBUGMESSAGE("skip reserved 0x%X", opcode);
		skip(read_u_sht(img), img);
	    }
	    else if (opcode >= 0xB0 && opcode < 0xD0) {
		/* just a reserved opcode, no data */
		DEBUGMESSAGE("reserved 0x%X", opcode);
	    }
	    else if (opcode >= 0xD0 && opcode < 0x100) {
		DEBUGMESSAGE("skip 0x%X", opcode);
		skip(read_long(img), img);
	    }
	    else if (opcode >= 0x100 && opcode < 0x8000) {
		DEBUGMESSAGE("skip 0x%X", opcode);
		skip((opcode >> 7) & 255, img);
	    }
	    else if (opcode >= 0x8000 && opcode < 0x8100) {
		/* just a reserved opcode */
		DEBUGMESSAGE("reserved 0x%X", opcode);
	    }
	    else if (opcode >= 8100 && opcode < 0x10000) {
		DEBUGMESSAGE("skipping 0x%X", opcode);
		skip(read_long(img), img);
	    }
	    else	return	prgmerr(0, "can't handle opcode of %X", opcode);
	}
	if (img->mid_type==ICC || img->color_form==CFM_SEPLANE)	{
		compact_image(red, img->src, planelen);
		compact_image(green, (char*)img->src + planelen, planelen);
		compact_image(blue, (char*)img->src + (planelen<<1), planelen);
	}
	else if (img->color_form == CFM_SGF)	{
		compact_image(red, red, planelen);
		compact_image(blue, blue, planelen);
		compact_image(green, green, planelen);
		ill_to_gray(img->src, red, green, blue, planelen);
	}
	else	{	/*	default to RLE	*/
	char	*p = img->src;
	register int	lin_inc;
	    for (len=lin_inc=0; len < rows; len++)	{
		compact_image(red + lin_inc, p, cols);	p += cols;
		compact_image(green+ lin_inc, p, cols);	p += cols;
		compact_image(blue + lin_inc, p, cols);	p += cols;
		lin_inc += cols;
	    }
	}
	CFREE(red);
if (img->mid_type == RLE)
	img->channels = img->dpy_channels;
return	cols * rows;
}


static
rectequal(r1, r2)
Rect	*r1, *r2;
{
return	(r1->top == r2->top && r1->bottom == r2->bottom &&
	r1->left == r2->left && r1->right == r2->right);
}

static
rectsamesize(r1, r2)
Rect	*r1, *r2;
{
return	(r1->right - r1->left == r2->right - r2->left &&
	r1->bottom - r1->top == r2->bottom - r2->top);
}

static void
rectinter(r1, r2, r3)
Rect	*r1, *r2, *r3;
{
	r3->left = MAX(r1->left, r2->left);
	r3->top = MAX(r1->top, r2->top);
	r3->right = MIN(r1->right, r2->right);
	r3->bottom = MIN(r1->bottom, r2->bottom);
}


static void
read_rect(r, img)
Rect*	r;
U_IMAGE*	img;
{
	r->top = read_u_sht(img);
	r->left = read_u_sht(img);
	r->bottom = read_u_sht(img);
	r->right = read_u_sht(img);
}

static void
dump_rect(s, r)
char*	s;
Rect*	r;
{
	message("%s (%d,%d) (%d,%d)", s, r->left, r->top, r->right, r->bottom);
}

static void
compact_image(in, out, len)
register u_sht*	in;
register byte*	out;
register int	len;
{
	unroll8_bwd(, len, *out++ = (*in++ >> 8) & 0xFF);
}

/*	READ_PIXMAP . C
%
% George Phillips <phillips@cs.ubc.ca>
% Department of Computer Science
% University of British Columbia
%
% This could use read_pixmap, but I'm too lazy to hack read_pixmap.
%
% Permission is granted to freely distribute this program in whole or in
% part provided you don't make money off it, you don't pretend that you
% wrote it and that this notice accompanies the code.
%
% Modified:	Jin, Guojun
% Date:	Sat, Feb 15, 1992
% ITG - Lawrence Berkeley Laboratory
*/

#if __STDC__
static byte*
unpackbits(Rect* bounds, u_sht rowBytes, int pixelSize, U_IMAGE* img)
#else /*__STDC__*/
static byte*
unpackbits(bounds, rowBytes, pixelSize, img)
Rect	*bounds;
u_sht	rowBytes;
#endif /*__STDC__*/
{
byte	*linebuf,
	*pm, *pm_ptr;
register int	i,j;
u_sht	pixwidth;
int	linelen, len;
byte*	bytepixels;
int	buflen, pkpixsize;

	if (pixelSize <= 8)
		rowBytes &= 0x7fff;

	/*	unpacking packbits	*/

	pixwidth = bounds->right - bounds->left;

	pkpixsize = 1;
	if (pixelSize == 16)
		pixwidth *= (pkpixsize = 2);
	else if (pixelSize == 32)
		pixwidth *= 3;

	if (!rowBytes)
		rowBytes = pixwidth;

	/* we're sloppy and allocate some extra space because we can overshoot
	* by as many as 8 bytes when we unpack the raster lines.  Really, I
	* should be checking to see if we go over the scan line (it is
	* possbile) and complain of a corrupt file.  That fix is more complex
	* (and probably costly in CPU cycles) and will have to come later.
	*/
	pm = (byte*)NZALLOC(pixwidth * (bounds->bottom - bounds->top) + 8,
		sizeof(*pm), "packbits rectangle");

	/* Sometimes we get rows with length > rowBytes.  I'll allocate some
	* extra for slop and only die if the size is _way_ out of wack.
	*/
	linebuf = (byte*)NZALLOC(rowBytes + 100, 1, "line buf");

	if (rowBytes < 8) {
	/* ah-ha!  The bits aren't actually packed.  This will be easy */
		for (i=0; i < bounds->bottom - bounds->top; i++) {
			pm_ptr = pm + i * pixwidth;
			read_n(buflen = rowBytes, linebuf, img);
			bytepixels = expand_buf(linebuf, &buflen, pixelSize);
			for (j = 0; j < buflen; j++)
				*pm_ptr++ = *bytepixels++;
		}
	} else	for (i=0; i < bounds->bottom - bounds->top; i++) {
			pm_ptr = pm + i * pixwidth;
			if (rowBytes > 250 || pixelSize > 8)
				linelen = read_u_sht(img);
			else
				linelen = read_byte(img);

			DEBUGMESSAGE("linelen: %d", linelen);

			if (linelen > rowBytes)
			    message("linelen > rowbytes! (%d > %d) at line %d",
				linelen, rowBytes, i);

			read_n(linelen, linebuf, img);

			for (j=0; j < linelen; j += buflen + 1) {
			register int	k, l;
				if (linebuf[j] & 0x80) {
					len = ((linebuf[j] ^ 255) & 255) + 2;
					buflen = pkpixsize;
					bytepixels = expand_buf(linebuf + j+1, &buflen, pixelSize);
					for (l=len; l--; bytepixels -= buflen)
					    for (k=buflen; k--;)
						*pm_ptr++ = *bytepixels++;
				}
				else {
					len = (linebuf[j] & 255) + 1;
					buflen = len * pkpixsize;
					bytepixels = expand_buf(linebuf + j+1, &buflen, pixelSize);
					for (k=buflen; k--;)
						*pm_ptr++ = *bytepixels++;
				}
			}
	}
	CFREE(linebuf);
return	pm;
}

static void
Opcode_9A(version, img)
U_IMAGE*	img;
{
#ifdef DUMP
	int	ch;
	FILE	*fp = fopen("data", "w");
	if (fp == NULL)
		exit(1);
	while ((ch = fgetc(fp)) != EOF)	fputc(ch, fp);
	exit(0);
#else
pixMap	p;
Rect	srcRect,
	dstRect;
byte*	pm;
int	pixwidth;
u_sht	mode;

	/* skip fake len, and fake EOF */
	skip(4, img);
	read_u_sht(img);	/* version */
	read_rect(&p.Bounds, img);
	pixwidth = p.Bounds.right - p.Bounds.left;
	p.packType = read_u_sht(img);
	p.packSize = read_long(img);
	p.hRes = read_long(img);
	p.vRes = read_long(img);
	p.pixelType = read_u_sht(img);
	p.pixelSize = read_u_sht(img);
	p.pixelSize = read_u_sht(img);
	p.cmpCount = read_u_sht(img);
	p.cmpSize = read_u_sht(img);
	p.planeBytes = read_long(img);
	p.pmTable = read_long(img);
	p.pmReserved = read_long(img);

	if (p.pixelSize == 16)
		pixwidth <<= 1;
	else if (p.pixelSize == 32)
		pixwidth *= 3;

	read_rect(&srcRect, img);
	if (DEBUGANY)	dump_rect("source rectangle:", &srcRect);

	read_rect(&dstRect, img);
	if (DEBUGANY)	dump_rect("destination rectangle:", &dstRect);

	mode = read_u_sht(img);
	DEBUGMESSAGE("mode = %x", mode);

	pm = unpackbits(&p.Bounds, 0, p.pixelSize, img);

	blit(&srcRect, &(p.Bounds), pixwidth, pm, p.pixelSize,
		&dstRect, &picFrame, rowlen, NULL, mode);

	CFREE(pm);
#endif
}

#if __STDC__
static void
do_pixmap(int version, u_sht rowBytes, int is_region, U_IMAGE* img)
#else /*__STDC__*/
static void
do_pixmap(version, rowBytes, is_region, img)
u_sht rowBytes;
U_IMAGE*	img;
#endif /*__STDC__*/
{
pixMap	p;
u_sht	mode, pixwidth;
byte*	pm;
ct_entry*	colour_table;
Rect	srcRect,
	dstRect;

	read_pixmap(&p, NULL, img);

	pixwidth = p.Bounds.right - p.Bounds.left;

	DEBUGMESSAGE("%d x %d rectangle", pixwidth,
			p.Bounds.bottom - p.Bounds.top);

	colour_table = read_colour_table(img);

	read_rect(&srcRect, img);
	if (DEBUGANY)
		dump_rect("source rectangle:", &srcRect);

	read_rect(&dstRect, img);
	if (DEBUGANY)
		dump_rect("destination rectangle:", &dstRect);

	mode = read_u_sht(img);
	DEBUGMESSAGE("mode = %x", mode);

	if (is_region)
		skip_poly_or_region(version, img);

	/*	unpacking rectangle	*/

	pm = unpackbits(&p.Bounds, rowBytes, p.pixelSize, img);

	blit(&srcRect, &(p.Bounds), pixwidth, pm, 8,
		&dstRect, &picFrame, rowlen, colour_table, mode);

	CFREE(colour_table);
	CFREE(pm);
}

static void
BitsRect(version, img)
U_IMAGE*	img;
{
u_sht	rowBytes;

	/*	for bitsrect	*/
	rowBytes = read_u_sht(img);

	DEBUGMESSAGE("rowbytes = 0x%x (%d)", rowBytes, rowBytes & 0x7FFF);

	if (rowBytes & 0x8000)
		do_pixmap(version, rowBytes, 0, img);
	else
		do_bitmap(version, rowBytes, 0, img);
}

static void
BitsRegion(version, img)
U_IMAGE*	img;
{
u_sht	rowBytes;

	/*	for bitsregion	*/
	rowBytes = read_u_sht(img);

	if (rowBytes & 0x8000)
		do_pixmap(version, rowBytes, 1, img);
	else	do_bitmap(version, rowBytes, 1, img);
}

static void
do_bitmap(version, rowBytes, is_region, img)
u_sht	rowBytes;
U_IMAGE*	img;
{
Rect	Bounds,
	srcRect,
	dstRect;
u_sht	mode;
byte*	pm;
static ct_entry colour_table[] = { {65535L, 65535L, 65535L}, {0, 0, 0} };

	read_rect(&Bounds, img);
	read_rect(&srcRect, img);
	read_rect(&dstRect, img);
	mode = read_u_sht(img);

	if (is_region)
		skip_poly_or_region(version, img);

	/*	unpacking rectangle	*/
	pm = unpackbits(&Bounds, rowBytes, 1, img);

	blit(&srcRect, &Bounds, Bounds.right - Bounds.left, pm, 8,
		&dstRect, &picFrame, rowlen, colour_table, mode);

	CFREE(pm);
}

static void
blit(srcRect, srcBounds, srcwid, srcplane, pixSize, dstRect, dstBounds, dstwid, colour_map, mode)
Rect	*srcRect,
	*srcBounds;
int	srcwid;
byte*	srcplane;
int	pixSize;
Rect	*dstRect,
	*dstBounds;
int	dstwid;
ct_entry	*colour_map;
int	mode;
{
Rect	clipsrc,
	clipdst;
register byte*	src;
register u_sht	*reddst,
		*greendst,
		*bluedst;
register int	i, j;
int	dstoff, xsize, ysize,
	srcadd, dstadd, pkpixsize;
ct_entry* ct;

	/* almost got it.  clip source rect with source bounds.
	* clip dest rect with dest bounds.
	* If they're not the same size - die!
	* (it would require zeroing some area!)
	* co-ordinate translations are actually done!
	*/
	rectinter(srcBounds, srcRect, &clipsrc);
	rectinter(dstBounds, dstRect, &clipdst);

	if (DEBUGANY) {
		dump_rect("copying from:", &clipsrc);
		dump_rect("to:          ", &clipdst);
	}

	if (!rectsamesize(&clipsrc, &clipdst))
		message("warning, rectangles of different sizes after clipping!");


	/* lots of assumptions about 8 bits per component, chunky bits, etc! */

	pkpixsize = 1;
	if (pixSize == 16)
		pkpixsize = 2;

	src = srcplane + (clipsrc.top - srcBounds->top) * srcwid +
		(clipsrc.left - srcBounds->left) * pkpixsize;
	dstoff = (clipdst.top - dstBounds->top) * dstwid +
		(clipdst.left - dstBounds->left);

	reddst = red + dstoff;
	greendst = green + dstoff;
	bluedst = blue + dstoff;

	xsize = clipsrc.right - clipsrc.left;
	ysize = clipsrc.bottom - clipsrc.top;
	srcadd = srcwid - xsize * pkpixsize;
	dstadd = dstwid - xsize;

	switch (pixSize) {
	case 8:
		for (i=0; i < ysize; i++) {
			for (j=xsize; j--;) {
				ct = colour_map + *src++;
				*reddst++ = ct->red;
				*greendst++ = ct->green;
				*bluedst++ = ct->blue;
			}
			src += srcadd;
			reddst += dstadd;
			greendst += dstadd;
			bluedst += dstadd;
		}
		break;
	case 16:
		for (i = 0; i < ysize; ++i) {
			for (j = 0; j < xsize; ++j) {
				*reddst++ = (*src & 0x7c) << 9;
				*greendst = (*src++ & 3) << 14;
				*greendst++ |= (*src & 0xe0) << 6;
				*bluedst++ = (*src++ & 0x1f) << 11;
			}
			src += srcadd;
			reddst += dstadd;
			greendst += dstadd;
			bluedst += dstadd;
		}
		break;
	case 32:
		srcadd = (srcwid / 3) - xsize;
		for (i=0; i < ysize; ++i) {
			for (j=xsize; j--;)
				*reddst++ = *src++ << 8;

			reddst += dstadd;
			src += srcadd;

			for (j=xsize; j--;)
				*greendst++ = *src++ << 8;

			greendst += dstadd;
			src += srcadd;

			for (j=xsize; j--;)
				*bluedst++ = *src++ << 8;

			bluedst += dstadd;
			src += srcadd;
		}
	}
}

static byte*
expand_buf(buf, len, bits_per_pixel)
byte*	buf;
int*	len;
int	bits_per_pixel;
{
static byte pixbuf[256 * 8];
register byte	*src, *dst;
register int	i;

	src = buf;
	dst = pixbuf;

	switch (bits_per_pixel) {
	case 8:
	case 16:
	case 32:
		return	buf;
	case 4:
		for (i=0; i < *len; i++) {
			*dst++ = (*src >> 4) & 15;
			*dst++ = *src++ & 15;
		}
		*len <<= 1;
		break;
	case 2:
		for (i=0; i < *len; i++) {
			*dst++ = (*src >> 6) & 3;
			*dst++ = (*src >> 4) & 3;
			*dst++ = (*src >> 2) & 3;
			*dst++ = *src++ & 3;
		}
		*len <<= 2;
		break;
	case 1:
		for (i = 0; i < *len; i++) {
			*dst++ = (*src >> 7) & 1;
			*dst++ = (*src >> 6) & 1;
			*dst++ = (*src >> 5) & 1;
			*dst++ = (*src >> 4) & 1;
			*dst++ = (*src >> 3) & 1;
			*dst++ = (*src >> 2) & 1;
			*dst++ = (*src >> 1) & 1;
			*dst++ = *src++ & 1;
		}
		*len <<= 3;
		break;
	default:
		prgmerr(0, "bad bits per pixel in expand_buf");
		return	NULL;
	}
return	pixbuf;
}

static void
Clip(version, img)
U_IMAGE*	img;
{
	skip(read_u_sht(img) - 2, img);
}

static void
read_pixmap(p, rowBytes, img)
pixMap	*p;
u_sht	*rowBytes;
U_IMAGE*	img;
{
	/*	getting pixMap header	*/

	if (rowBytes != NULL)
		*rowBytes = read_u_sht(img);

	read_rect(&p->Bounds, img);
	p->version = read_u_sht(img);
	p->packType = read_u_sht(img);
	p->packSize = read_long(img);
	p->hRes = read_long(img);
	p->vRes = read_long(img);
	p->pixelType = read_u_sht(img);
	p->pixelSize = read_u_sht(img);
	p->cmpCount = read_u_sht(img);
	p->cmpSize = read_u_sht(img);
	p->planeBytes = read_long(img);
	p->pmTable = read_long(img);
	p->pmReserved = read_long(img);

	DEBUGMESSAGE("pixelType:\t%d\npixelSize:\t%d\ncmpCount:\t%d\ncmpSize:\t%d",
		p->pixelType, p->pixelSize, p->cmpCount, p->cmpSize);

	if (p->pixelType)
		prgmerr(0, "sorry, I only do chunky format");
	if (p->cmpCount != 1)
		prgmerr(0, "sorry, cmpCount != 1");
	if (p->pixelSize != p->cmpSize)
		prgmerr(0, "oops, pixelSize != cmpSize");
}

static	ct_entry*
read_colour_table(img)
U_IMAGE	*img;
{
longword	ctSeed;
u_sht	ctFlags, ctSize,
	val;
int	i;
ct_entry	*colour_table;

	/*	getting color table info	*/

	ctSeed = read_long(img);
	ctFlags = read_u_sht(img);
	ctSize = read_u_sht(img);

	DEBUGMESSAGE("ctSeed:  %d\nctFlags: %d\nctSize:  %d",
			ctSeed, ctFlags, ctSize);

	/*	reading colour table	*/

	colour_table = (ct_entry*) NZALLOC(sizeof(ct_entry), ctSize + 1,
			"colour table");

	for (i=0; i <= ctSize; i++) {
		if ((val = read_u_sht(img)) > ctSize)
			prgmerr(0, "pixel value greater than colour table size");
		/* seems that if we have a device colour table, the val is
		 * always zero, so I assume we allocate up the list of colours.
		 */
		if (ctFlags & 0x8000)
			val = i;
		colour_table[val].red = read_u_sht(img);
		colour_table[val].green = read_u_sht(img);
		colour_table[val].blue = read_u_sht(img);

		DEBUGMESSAGE("%d: [%d,%d,%d]", val,
				colour_table[val].red,
				colour_table[val].green,
				colour_table[val].blue, 0);
	}

	return(colour_table);
}

/* these 3 do nothing but skip over their data! */
static void
BkPixPat(version, img)
U_IMAGE*	img;
{
	read_pattern(img);
}

static void
PnPixPat(version, img)
U_IMAGE*	img;
{
	read_pattern(img);
}

static void
FillPixPat(version, img)
U_IMAGE*	img;
{
	read_pattern(img);
}

/*	this just skips over a version 2 pattern.  Probabaly will return
	a pattern in the fabled complete version.
*/
static void
read_pattern(img)
U_IMAGE*	img;
{
u_sht	PatType,
	rowBytes;
pixMap	p;
byte	*pm;
ct_entry	*ct;

	/*	Reading a pattern	*/

	PatType = read_u_sht(img);

	switch (PatType) {
	case 2:
		skip(8, img); /* old pattern data */
		skip(5, img); /* RGB for pattern */
		break;
	case 1:
		skip(8, img); /* old pattern data */
		read_pixmap(&p, &rowBytes, img);
		ct = read_colour_table(img);
		pm = unpackbits(&p.Bounds, rowBytes, p.pixelSize, img);
		CFREE(pm);
		CFREE(ct);
		break;
	default:
		prgmerr(0, "unknown pattern type in read_pattern");
	}
}

/* more stubs for text output */

#define	skip_text(fp)	skip(read_byte(fp), fp);

static void
LongText(version, img)
U_IMAGE*	img;
{
	skip(4, img);
	skip_text(img);
}

static void
DHText(version, img)
U_IMAGE*	img;
{
	skip(1, img);
	skip_text(img);
}

static void
DVText(version, img)
{
	skip(1, img);
	skip_text(img);
}

static void
DHDVText(version, img)
{
	skip(2, img);
	skip_text(img);
}

static void
skip_poly_or_region(version, img)
{
	/*	skipping polygon or region	*/
	skip(read_u_sht(img) - 2, img);
}

static void
LongComment(img)
{
	/*	skipping longword comment	*/
	skip(2, img);
	skip(read_u_sht(img), img);
}
