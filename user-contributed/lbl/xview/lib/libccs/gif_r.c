/*	GIF_R . C
#
%	Copyright (c)	Jin Guojun
%
% AUTHOR:	Jin, Guojun - LBL	10/01/91
*/

#include "header.def"
#include "imagedef.h"

#define	MAX_LWZ_BITS	12
#define INTERLACE	0x40
#define COLORMAP_FLAG	0x80

#define	ReadFail(file, buffer, len)	((*img->i_read)(buffer, len, 1, file)!=1)
#define LM_to_uint(a,b)			( ((b)<<8) | (a) )

struct	{
	int	transparent, delayTime, inputFlag, disposal;
	} Gif89 = { -1, -1, -1, 0 };

static	int	ZeroDataBlock, ICount;
GS	GifScreen;


static
GetDataBlock(U_IMAGE	*img, byte	*buf)
{
byte	count;

	if (ReadFail(img->IN_FP, &count, 1))
		return	prgmerr(0, "getting junk size");

	ZeroDataBlock = !count;
	if ((count) && ReadFail(img->IN_FP, buf, count))
		return	prgmerr(0, "reading junk");
return count;
}

static
DoExtension(U_IMAGE	*img, int	label)
{
static char	buf[256];
char		*str;

	switch (label)	{
	case 0x01:		/* Plain Text Extension */
		str = "Plain Text Extension";
#ifdef notdef
		if (!GetDataBlock(img, buf));

		lpos   = LM_to_uint(buf[0], buf[1]);
		tpos   = LM_to_uint(buf[2], buf[3]);
		width  = LM_to_uint(buf[4], buf[5]);
		height = LM_to_uint(buf[6], buf[7]);
		cellw  = buf[8];
		cellh  = buf[9];
		foreground = buf[10];
		background = buf[11];

		while (GetDataBlock(img, buf)) {
			PPM_ASSIGN(image[ypos][xpos],
				cmap[v].r, cmap[v].g, cmap[v].b);
			++index;
		}
		return FALSE;
#else
		break;
#endif
	case 0xFF:		/* Application Extension */
		str = "Application Extension";
		break;
	case 0xFE:		/* Comment Extension */
		str = "Comment Extension";
		while (GetDataBlock(img, buf))
			DEBUGMESSAGE("gif comment: %s", buf);
		return	0;
	case 0xF9:		/* Graphic Control Extension */
		str = "Graphic Control Extension";
		(void) GetDataBlock(img, buf);
		Gif89.disposal    = (buf[0] >> 2) & 0x7;
		Gif89.inputFlag   = (buf[0] >> 1) & 0x1;
		Gif89.delayTime   = LM_to_uint(buf[1],buf[2]);
		if ((buf[0] & 0x1))
			Gif89.transparent = buf[3];

		while (GetDataBlock(img, buf));
		return	0;
	default:
		str = buf;
		sprintf(buf, "UNKNOWN (0x%02x)", label);
		break;
	}

	message("got a '%s' extension - please report it to koblas@mips.com",
		str);

	while (GetDataBlock(img, buf));
return	0;
}

static
GetCode(U_IMAGE	*img, int code_size, int flag)
{
static byte	buf[280];
static int	curbit, lastbit, done, last_byte;
int	i, j, ret;
byte	count;

	if (flag) {
		done = FALSE;
		return	lastbit = curbit =0;
	}

	if ((curbit+code_size) >= lastbit) {
		if (done)
		    if (curbit >= lastbit)
			return	prgmerr(0, "ran off the end of my bits");

		buf[0] = buf[last_byte-2];
		buf[1] = buf[last_byte-1];

		if ((count = GetDataBlock(img, &buf[2])) == 0)
			done = TRUE;

		last_byte = 2 + count;
		curbit = (curbit - lastbit) + 16;
		lastbit = (2+count) << 3;
	}
	for (i=curbit, j=ret=0; j < code_size; ++i, ++j)
		ret |= ((buf[i >> 3] & (1 << (i & 7))) != 0) << j;
	curbit += code_size;

return ret;
}

static
LWZReadByte(U_IMAGE	*img, int flag, int input_code_size)
{
static int	fresh = FALSE;
static int	code_size, set_code_size,
		max_code, max_code_size,
	firstcode, oldcode,
	clear_code, end_code,
	table[2][(1 << MAX_LWZ_BITS)],
	stack[(1 << (MAX_LWZ_BITS)) << 1], *sp;
int	incode;
register int	i, code;

	if (flag) {
		if ((set_code_size = input_code_size) > MAX_LWZ_BITS)
			return	-1;	/* for movies	*/
		code_size = set_code_size + 1;
		clear_code = 1 << set_code_size ;
		end_code = clear_code + 1;
		max_code_size = clear_code << 1;
		max_code = clear_code + 2;

		GetCode(img, 0, fresh=TRUE);

		for (i=code=0; i < clear_code; ++i) {
			table[0][i] = code;
			table[1][i] = i;
		}
		for (; i < (1<<MAX_LWZ_BITS); ++i)
			table[0][i] = table[1][i] = code;

		sp = stack;

		return	code;
	} else if (fresh) {
		fresh = FALSE;
		do {
			firstcode = oldcode = GetCode(img, code_size, FALSE);
		} while (firstcode == clear_code);
		return	firstcode;
	}

	if (sp > stack)
		return *--sp;

	while ((code = GetCode(img, code_size, FALSE)) >= 0) {
		if (code == clear_code) {
			for (i=code=0; i < clear_code; ++i) {
				table[0][i] = code;
				table[1][i] = i;
			}
			for (; i < (1<<MAX_LWZ_BITS); ++i)
				table[0][i] = table[1][i] = code;
			code_size = set_code_size+1;
			max_code_size = clear_code << 1;
			max_code = clear_code+2;
			sp = stack;
			firstcode = oldcode = GetCode(img, code_size, FALSE);
			return	firstcode;
		} else if (code == end_code) {
			int	count;
			byte	buf[260];

			if (ZeroDataBlock)
				return -2;

			while ((count = GetDataBlock(img, buf)) > 0);/* fetch junk */

			if (count)
				message("missing EOD in data stream (common occurance)");
			return -2;
		}

		incode = code;
		if (code >= max_code) {
			*sp++ = firstcode;
			code = oldcode;
		}

		while (code >= clear_code) {
			*sp++ = table[1][code];
			if (code == table[0][code])
			 return	prgmerr(0, "circular table entry BIG ERROR");
			code = table[0][code];
		}

		*sp++ = firstcode = table[1][code];

		if ((code = max_code) <(1<<MAX_LWZ_BITS)) {
			table[0][code] = oldcode;
			table[1][code] = firstcode;
			++max_code;
			if ((max_code >= max_code_size) &&
				(max_code_size < (1<<MAX_LWZ_BITS))) {
				max_code_size <<= 1;
				++code_size;
			}
		}

		oldcode = incode;
		if (sp > stack)
			return *--sp;
	}
return code;
}

static
ReadImage(U_IMAGE *img, color_cell *cmap, int interlace, bool ignore)
{
bool	OsameI = img->color_form==img->in_color ||
		img->color_form == CFM_SEPLANE || img->mid_type == COLOR_PS;
byte	c, *obp, *scan[3];
int	ypos=0, pass=0, len=img->width;
register unsigned int	r=RED_to_GRAY, g=GREEN_to_GRAY, b=BLUE_to_GRAY;
register int	v, xpos=0;

	obp = scan[0] = (byte*)img->src;
	for (v=1; v<img->dpy_channels; v++)
		scan[v] = scan[v-1] + len;
	/*	Initialize the Compression routines	*/
	if (ReadFail(img->IN_FP, &c, 1))
		return	prgmerr(DEBUGANY, "EOF / read on image data");

	if (LWZReadByte(img, TRUE, c) < 0)
		return	prgmerr(DEBUGANY, "reading image");

	/*	If this is an "uninteresting picture" ignore it	*/
	if (ignore) {
		message("skipping image...");
		while (LWZReadByte(img, FALSE, c) >= 0);
		return	0;
	}

	DEBUGMESSAGE("reading %d x %d %s GIF image",
			len, img->height, interlace ? "interlaced" : "");

	while ((v=LWZReadByte(img, FALSE, c)) >= 0) {

		if (OsameI)
			obp[xpos] = v;
		else if (img->color_form != CFM_SGF)	/* RAST	*/
			scan[0][xpos] = cmap[v].r,
			scan[1][xpos] = cmap[v].g,
			scan[2][xpos] = cmap[v].b;
		else	/* grayscale */
		obp[xpos] = (r*cmap[v].r + g*cmap[v].g + b*cmap[v].b) >> 8;

		if (++xpos==len) {
		    xpos = 0;
		    if (interlace) {
			switch (pass) {
			case 0:
			case 1:	ypos += 8; break;
			case 2:
				ypos += 4; break;
			case 3:
				ypos += 2; break;
			}
			if (ypos >= img->height) {
				switch (++pass) {
				case 1:	ypos = 4; break;
				case 2:	ypos = 2; break;
				case 3:	ypos = 1; break;
				}
			}
		    } else	ypos++;
		    obp = (byte*)img->src + ypos*len;
		    scan[0] = (byte *)img->src + ypos*len*img->dpy_channels;
		    for (v=1; v<img->dpy_channels; v++)
			scan[v] = scan[v-1] + len;
		}
	}
	if (img->mid_type == RLE)
		img->channels = img->dpy_channels;
return	(v == -2) ? ypos * len : v;	/* sometimes ypos = img->h + 1 */
}


gif_header_handle(int job,
	U_IMAGE	*img,
	int	ac,	/* can be used as image numbers */
	char	**av,
	VType	*vva_p	/* O_same_I */	)
{
bool	hasGlobalColormap, status=0;
int	img_num = (ac && ac < 64) ? ac : 1;	/* max 64 animators	*/
byte	buf[16], c, version[4];

    switch (job) {
    case HEADER_READ:
    case HEADER_FREAD:
	if (!av || img->in_type != GIF) {
		if (!av)
			(*img->r_seek)(img->IN_FP, 0, 0);
		if (ReadFail(img->IN_FP, buf, 6))
			prgmerr(DEBUGANY, "reading magic number");
		if (strncmp(buf, "GIF", 3))
			return	EOF;

		strncpy(version, buf+3, 3);
		version[3] = ICount = 0;
		if ((strcmp(version, "87a")) && (strcmp(version, "89a"))) {
		    DEBUGMESSAGE("not '87a' or '89a' version");
		    status = EOF-1;
		}
		if (ReadFail(img->IN_FP, buf, 7))
			return	prgmerr(DEBUGANY, "read gif screen descriptor");

		GifScreen.Width	= LM_to_uint(buf[0], buf[1]);
		GifScreen.Height = LM_to_uint(buf[2], buf[3]);
		GifScreen.CmapLen = img->cmaplen = 2 << (buf[4] & 7);
		GifScreen.ColorResolution = (((buf[4] & 0x70) >> 3) + 1);
		GifScreen.Background	= buf[5];
		GifScreen.AspectRatio	= buf[6];

		img->in_type = GIF;
		img->in_form = IFMT_SCF;
		img->in_color = img->color_form = CFM_SCF;
		img->channels = img->dpy_channels = img->pxl_in = 1;

		if (hasGlobalColormap = buf[4] & COLORMAP_FLAG)
		    if (!ReadRGBMap(img,
			GifScreen.ColorMap, GifScreen.CmapLen, vva_p))
			return	prgmerr(DEBUGANY, "reading global colormap");

		if (GifScreen.AspectRatio) {
		register float	r = ( GifScreen.AspectRatio + 15.0 ) / 64.0;
		message("warning - non-square pixels; to fix do a '%cscale %g'",
			r < 1.0 ? 'x' : 'y', r < 1.0 ? 1.0 / r : r);
		}
	}
	Loop {
	    if (ReadFail(img->IN_FP, &c, 1))
		return	prgmerr(DEBUGANY, "EOF / read error on image data");

	    if (c == ';')	/* GIF terminator */
		if (ICount < img_num)
			return	prgmerr(0, "only %d image%s found in file",
				 ICount, ICount>1 ? "s" : "");

	    if (c == '!') { 	/* Extension */
		if (ReadFail(img->IN_FP, &c, 1))
			return	prgmerr(DEBUGANY, "EOF on reading extention function code");
		DoExtension(img, c);
		continue;
	    }

	    if (c != ',') {	/* Not a valid start character */
		DEBUGMESSAGE("bogus character 0x%02x, ignoring", (int) c);
		continue;
	    }
	    ++ICount;	break;
	}	/* end Loop */

	if (ReadFail(img->IN_FP, buf, 9))
		return	prgmerr(DEBUGANY, "reading left/top/width/height");

	/*    img->binary_img = !(buf[8] & 0x7);	*/
	if (buf[8] & COLORMAP_FLAG)	/* useGlobalColormap	*/	{
		if (!ReadRGBMap(img, GifScreen.ColorMap,
			img->cmaplen = 1 << ((buf[8] & 0x7) + 1), vva_p))
			return	prgmerr(DEBUGANY, "reading local colormap");
	} else if (!hasGlobalColormap)	/* bug!	*/
		img->in_form = IFMT_SGF,
		img->in_color = img->color_form = CFM_SGF;
	img->width = LM_to_uint(buf[4], buf[5]);
	img->height = LM_to_uint(buf[6], buf[7]);
	GifScreen.interlace = buf[8] & INTERLACE;
	GifScreen.jumpover = ICount != img_num;
	break;
    case HEADER_WRITE:
    default:	status = prgmerr(0, "no sunch gif job %d", job);
    }
return	status;
}

read_gif_image(U_IMAGE	*img, GS *gs, int num_img)
{
/* Since out (dpy) image channels >= input image's, so use dpy_channels
verify_buffer_size(&img->src, img->width*img->height, img->dpy_channels,
		"gif-src");	*/
return	ReadImage(img, gs->ColorMap, gs->interlace, gs->jumpover);
}

