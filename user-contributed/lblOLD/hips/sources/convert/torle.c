/*	TORLE . C
#
%	The format for the input file is any image.
%	The format for the output file is a RLE image.
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
% AUTHOR:	Jin Guojun - LBL	12/10/1991
*/

#include "imagedef.h"

arg_fmt_list_string	arg_fmt[] =	{
	{"-d", "%-", No, 1, 0,	"dither to 8-bit"},
	{"-k", "%b", True, 1, 0, "keep going when errors hanppen"},
	{"-l", "%-", No, 1, 0,	"rotate left 90"},
	{"-r", "%+", No, 1, 0,	"rotate right 90"},
	{"-s", "%f", 1.0, 1, 1, "scale"},
	{"-t", "%s", No, 1, 1,	"title"},
	{"-TIFF", "%N", TiFF, 1, 0, "-image_type	using for pipe"},
 {"	[<] input [[> |] output]", "0", 0, 0, 0, "end of usage"},
	NULL	};
U_IMAGE	uimg;
bool	nostop, rot, to8, init_type=IMAGE_INIT_TYPE;
float	scale;
#define	rows	uimg.height
#define	cols	uimg.width

main(ac, av)
int	ac;
char*	av[];
{
char	*title, **fl;
int	row, state;

	if ((state=parse_argus(&fl, ac, av, arg_fmt,
		&to8, &nostop, &rot, &rot, &scale, &title, &init_type)) < 0)
		exit(state);
	if (state && !(in_fp=zreopen(uimg.name=fl[0], NULL, NULL)))
		syserr("input file -- %s", fl[0]);

uimg.color_dpy = True;
format_init(&uimg, init_type, RLE, -1, *av, "D20-1");
io_test(fileno(in_fp), {parse_usage(arg_fmt); exit(0);});

	if ((*uimg.header_handle)(HEADER_READ, &uimg, 0, 0, True) < 0)
		syserr("unknown image type");

	(*uimg.std_swif)(FI_LOAD_FILE, &uimg, nostop ? NULL : uimg.name, True);
	rle_dflt_hdr.comments = zalloc(2, sizeof(char*), "comment");
	if (uimg.desc)
		rle_dflt_hdr.comments[0] = str_save(uimg.desc);
	if (title)
		rle_dflt_hdr.comments[1] = str_save(title);
	if (to8 && uimg.channels > 1)
		To_8(&uimg, reg_cmap, to8==1, 256),
		uimg.dpy_channels = uimg.channels;
	if (rot++)	{
		uimg.dest = uimg.src;
		uimg.src = nzalloc(uimg.channels*uimg.width, uimg.height, "rot");
		row = uimg.width;
		uimg.width = uimg.height;
		uimg.height = row;
		color_rotate_90(uimg.dest, uimg.src, row, uimg.width,
			uimg.color_form, rot);
		free(uimg.dest);
	}
	state = (*uimg.std_swif)(FI_SAVE_FILE, &uimg, No, NULL);
}
