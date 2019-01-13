/*	TORAST . C
#
%	convert others to SUN RASTer
%
%	Copyright (c)	Jin Guojun
%
% AUTHOR:	Jin Guojun - LBL	12/12/1991
*/

#include "header.def"
#include "imagedef.h"

arg_fmt_list_string	arg_fmt[] =	{
	{"-a", "%b", True, 1, 0, "add alpha channel"},
	{"-8", "%+ %d", 256, 2, 0,
		"quantizing to 8-bit with # colors [default # %.f]"},
	{"-d", "%-", No, 1, 0, "dither to 8"},
	{"-k", "%b", True, 1, 0, "keep working, and ignore input error"},
	{"-l", "%-", No, 1, 0, "rotate left 90"},
	{"-r", "%+", No, 1, 0, "rotate right 90"},
	{"-u", "%N", RT_STANDARD, 1, 0,
		"use uncompress mode. The default is RLE"},
	{"-o", "%s", No, 1, 1, "output file"},
	{"	[<] input [[> | -o] output]", NULL, 0, 0, 0, "end of usage"},
	NULL	};
U_IMAGE	uimg;
bool	rot, alpha, nostop, pr_t, to8, toN=256;

#define	rows	uimg.height
#define	cols	uimg.width

main(ac, av)
int	ac;
char*	av[];
{
int	i;
char	**fl, *of_name;

	if ((i=parse_argus(&fl, ac, av, arg_fmt,
		&alpha, &to8, &toN, &to8,
		&nostop, &rot, &rot, &pr_t, &of_name)) < 0)
		exit(i);

	if (of_name && !(out_fp=freopen(av[++i], "wb", stdout)))
		syserr("output file %s", of_name);
	if (i && !(in_fp=zreopen(uimg.name=fl[0], NULL, NULL)))
		syserr("input %s", fl[0]);

uimg.color_dpy = alpha ? 1 : -1;
format_init(&uimg, IMAGE_INIT_TYPE, RLE, RAS, *av, "A8-2");

io_test(fileno(in_fp), {parse_usage(arg_fmt); exit(0);});

if ((*uimg.header_handle)(HEADER_READ, &uimg, 0, 0, True))
	syserr("unknown image type");

(*uimg.std_swif)(FI_LOAD_FILE, &uimg, nostop ? NULL : uimg.name, True);

    if (rot++) {
	uimg.dest = uimg.src;
	uimg.src = nzalloc(uimg.channels*uimg.width, uimg.height, "rot");
	i = uimg.width;
	uimg.width = uimg.height;
	uimg.height = i;
	color_rotate_90(uimg.dest, uimg.src, i, uimg.width,
		uimg.color_form, rot);
	free(uimg.dest);	uimg.dest = NULL;
    }
    if (uimg.channels>1 && to8)
	To_8(&uimg, reg_cmap, to8>0, toN);

(*uimg.std_swif)(FI_SAVE_FILE, &uimg, pr_t, alpha);
exit(0);
}
