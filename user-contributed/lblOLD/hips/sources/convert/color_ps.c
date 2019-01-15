/* color_ps.c - read any color image and produce a PostScript file
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
Bld. 50B, Rm. 2239, Lawrence Berkeley National Laboratory, Berkeley, CA, 94720.
(wejohnston@lbl.gov)

For further information about this software, contact:
	Jin Guojun
	Bld. 50B, Rm. 2275
	Lawrence Berkeley National Laboratory, Berkeley, CA, 94720.
	g_jin@lbl.gov

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% compile:
%	cc -O -o color_ps color_ps.c -lscs5 -lccs -lhips -lrle -ltiff -ljpeg
%
% AUTHOR:	Jin Guojun - LBL	11/11/1991
*/

#include "header.def"
#include "imagedef.h"

#ifndef	PAGE_COLS
#define	PAGE_COLS	618.75	/* 8.25 inch width	*/
#endif
#ifndef	PAGE_ROWS
#define	PAGE_ROWS	810.0	/* 10.8 inch height	*/
#endif
#ifndef	MAX_LINE_LEN
#define	MAX_LINE_LEN	30
#endif

#define	INCH	.013834635
#define	CONVERT	75

arg_fmt_list_string	arg_fmt[] = {
	{"-k", "%b", True, 1, 0, "keep working and ignore errors"},
	{"-p", "%d", 9.6, 1, 0, "position of the image top edge"},
	{"-w", "%f", PAGE_COLS, 1, 1, "page width {8.5}"},
	{"-s[cale]", "%f", 1.0, 1, 1, "spatial factor"},
	{"-t", "%s", No, 1, 1, "print title string under the image"},
	{"I/O:	[<] color_image", "0", 0, 0, 0,
		"For gamma enhancement and rotate, use toicc or toany"},
	NULL	};
U_IMAGE	uimg;
bool	nostop, topp;
char*	title;

void	psinit(), putrest(), printfoot(), sendbyte(), sendcell();
extern	cmap_t*	rlemap_to_regmap();
extern	TIFF*	TIFFin;

#define	rows	uimg.height
#define	cols	uimg.width

main(ac, av)
int	ac;
char	*av[];
{
char	**fl;
int	bps, nf, padright, row, maxval=255, nmaxval;
float	scale, fpg_w;
pixel**	xel24;

	if ((nf=parse_argus(&fl, ac, av, arg_fmt,
		&nostop, &topp, &fpg_w, &scale, &scale, &title)) < 0)
		exit(nf);
	if (topp)
		topp = PAGE_ROWS - topp * CONVERT;
	if (fpg_w != PAGE_COLS)
		fpg_w *= CONVERT;
	if (nf && (in_fp=zreopen(uimg.name=fl[0], NULL, NULL)) != stdin)
		syserr("input file -- %s", fl[0]);

uimg.color_dpy = True;	/* ensure output color format.	*/
format_init(&uimg, IMAGE_INIT_TYPE, COLOR_PS, -1, *av, "N1-1");

io_test(fileno(uimg.IN_FP), {parse_usage(arg_fmt); exit(-1);});

if ((*uimg.header_handle)(HEADER_READ, &uimg, 0, 0, True) < 0)
	syserr("unknown image type");

if (uimg.in_type==PPM || uimg.in_type==PNM)
	xel24 = (pixel**)(*uimg.std_swif)(FI_PNM_MAXVAL, &uimg, &maxval);
(*uimg.std_swif)(FI_LOAD_FILE, &uimg, nostop ? NULL : uimg.name,
		True /* don't change format, and save PNM loading buffer */);

/* Figure out bps */
bps = min_bits(maxval);
if (bps > 2 && bps < 4)
	bps = 4;
else if (bps > 4 && bps < 8)
	bps = 8;
else if (bps > 8)
	prgmerr(1, "maxval of %d is too large for PostScript", maxval);
nmaxval = (1 << bps) -1;

/* Compute padding to round cols * bps up to the nearest multiple of 8. */
padright = ( (((cols * bps + 7) >> 3) << 3) - cols * bps ) / bps;

psinit(uimg.name ? uimg.name : "stdin", cols, rows, bps, scale, topp, fpg_w);
    switch (uimg.in_type) {
	register int	col;
	case PPM:
	case PNM:
		for (row=0; row < rows; row++)	{
		register pixel	*pP = xel24[row];
		    for (col=0; col < cols; col++, pP++)
			sendcell(PPM_GETR(*pP), PPM_GETG(*pP), PPM_GETB(*pP));
		    for (col = 0; col < padright; col++, pP++)
			sendcell(PPM_GETR(*pP), PPM_GETG(*pP), PPM_GETB(*pP));
		}
		break;
	default:
		if (uimg.channels == 3)
		    if (uimg.color_form==CFM_ILL ||
			uimg.in_type==TiFF && uimg.color_form != CFM_ILC) {
			byte*	scan[3];
				scan[2] = (byte *)uimg.src - cols;
				for (row=0; row < rows; row++)	{
					scan[0] = scan[2] + cols;
					scan[1] = scan[0] + cols;
					scan[2] = scan[1] + cols;
				    for (col=0; col < cols; col++)
					sendcell(scan[0][col],
						scan[1][col],
						scan[2][col]);
				}
		    }
		    else {
			register byte*	ibp = (byte*)uimg.src;
			register int	cinc=1, pinc=uimg.channels;
			if (uimg.color_form != CFM_ILC)
				pinc = cinc,	cinc = cols*rows;
				for (col=cols*rows; col--; ibp+=pinc)
					sendcell(*ibp, ibp[cinc], ibp[cinc<<1]);
			}
		else	{
		register byte	*bp=(byte *) uimg.src;
		byte	*cmap[3];
			cmap[0] = reg_cmap[0];
			cmap[1] = reg_cmap[1];
			cmap[2] = reg_cmap[2];
			if (cmap[0] == NULL)	/* RLE only	*/
				rlemap_to_regmap(cmap, &rle_dflt_hdr);
			for (row=0; row < rows; row++, bp+=col)
			    for (col=0; col < cols; col++)
				sendcell(cmap[0][bp[col]],
					cmap[1][bp[col]],
					cmap[2][bp[col]]);
		}
    }
    putrest(0);
    printfoot(title);
exit(0);
}

#undef	cols
#undef	rows

int	item, bitshift, itemsperline, total;
#define HSBUFSIZ 384

static void
psinit(name, cols, rows, bps, scale, top_edge, page_width)
char	*name;
int	cols, rows, bps;
float	scale, page_width;
{
float	scols, srows, margin_x, margin_y;

	scols = scale * cols * 0.96;	/*   0.96 is the multiple of   */
	srows = scale * rows * 0.96;	/* 72/300 that is closest to 1 */
	margin_x = (page_width - scols) / 2;
	margin_y = (PAGE_ROWS - srows) / 2;
	if (top_edge)
		top_edge = margin_y - top_edge,
		margin_y += top_edge;
	message("image printed as %.2f(h) x %.2f(w)\n", srows*INCH, scols*INCH);
	if (margin_x < 0.0 || margin_y < 0.0)
		message("warning : image too large for page\n");

	printf("%%!PS-Colorps-2.0 EPSF-2.0)\n");
	printf("%%%%Creator: color_ps\n");
	printf("%%%%Title: %s.ps\n", name ? name : "STDIN");
	printf("%%%%Pages: 1\n");
	printf("%%%%BoundingBox: %d %d %d %d\n", (int)margin_x, (int)margin_y,
		(int)(margin_x + scols), (int) (margin_y + srows));
	printf("%%%%EndComments\n");
	printf("%%%%EndProlog\n" );
	printf("%%%%Page: 1 1\n" );
	printf("/picstr %d string def\n", HSBUFSIZ );
	printf("gsave\n" );
	printf("%g %g translate\n", margin_x, margin_y);
	printf("%g %g scale\n", scols, srows);
	printf("%d %d %d\n", cols, rows, bps);
	printf("[ %d 0 0 -%d 0 %d ]\n", cols, rows, rows);
	printf("{ currentfile picstr readhexstring pop }\n");
	printf("false 3\n");
	printf("colorimage\n");

	bitspersample = bps;
	bitshift = 8 - bitspersample;
}

putpschar(item)
{
char *hexits = "0123456789abcdef";

	putchar(hexits[item >> 4]);
	putchar(hexits[item & 0x0F]);
	if (++itemsperline == MAX_LINE_LEN) {
		putchar('\n');
		itemsperline = 0;
	}
}

void
sendbyte(b)
byte	b;
{
static int	item=0;
	item += b << bitshift;
	if (!bitshift)
		putpschar(item),
		item = 0,	total++,
		bitshift = 8 - bitspersample;
	else	bitshift -= bitspersample;
}

void
sendcell(r, g, b)	/* purpose is that we can transfer RGB to HSB here */
byte	r, g, b;
{
	sendbyte(r);
	sendbyte(g);
	sendbyte(b);
}

void
putrest(r)
{
	if (bitshift > 0)
		putpschar(r);
	while (total++ % HSBUFSIZ)
		putpschar(r);
	printf("\ngrestore\n");
}

void
printfoot(title)
char	*title;
{
	if (title) {
		printf("gsave\n%d %d\n", 10, 10);
		printf("(%s) show\n", title);
		printf("grestore");
	}
	printf("showpage\n");
}

