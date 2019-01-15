/*
%	TO3DPLOT . C
%
%	input any image, and output 3D GNUPLOT data.
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

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
	Bld. 50B, Rm. 2275,
	Lawrence Berkeley Laboratory, Berkeley, CA, 94720
	g_jin@lbl.gov
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% AUTHOR:	Jin Guojun - LBL	3/10/1992
*/

#include "header.def"
#include "imagedef.h"

arg_fmt_list_string	arg_fmt[] = {
	{"-k", "%b", True, 1, 0, "keep going when errors hanppen"},
	{"-r", "%b", True, 1, 0, "rotate right 90"},
	{"-s[x][y]", "%f", 1., 1, 1,
		"scale for X or Y or both (-s #) coordinate in float"},
	{"-t", "%s", No, 1, 1, "title"},
	{"I/O:	[<] input [>] output\n", "0", 0, 0, 0, "END"}, NULL	};

U_IMAGE	uimg;
bool	nostop, rot;
float	scale, xscale=1., yscale=1.;

main(ac, av)
int	ac;
char*	av[];
{
char	*title = *av, **fl;
int	frm, nf;

format_init(&uimg, IMAGE_INIT_TYPE, HIPS, -1, *av, "Mar15-2");

	if ((nf=parse_argus(&fl, ac, av, arg_fmt,
		&nostop, &rot, &scale, &xscale, &yscale, &title)) < 0)
		exit(nf);
	if (nf && freopen(uimg.name = *fl, "rb", stdin) != stdin)
		syserr("input file -- %s", av[frm]);
	if (scale != 1.)
		xscale = yscale = scale;

	io_test(fileno(in_fp), exit(0));

	if ((*uimg.header_handle)(HEADER_READ, &uimg, 0, 0, True) < 0)
		syserr("unknown image type");

	fprintf(out_fp, "# %s\n# set parametric (for splot)\n", title);

    {	register int	col, row;
	for (frm=uimg.frames; frm--;)	{

	(*uimg.std_swif)(FI_LOAD_FILE, &uimg, nostop ? NULL : uimg.name, True);

		fprintf(out_fp, "# 3D gnuplot %s frame %d\n", uimg.name, frm);
		switch (uimg.in_form)	{
		case IFMT_BYTE:	{
		register byte	*bp = uimg.src;
			for (row=0; row<uimg.height; row++)
			    for (col=0; col<uimg.width; col++)
				fprintf(out_fp, "%f %f %d\n",
					col*xscale, row*yscale, *bp++);
		}	break;
		case IFMT_SHORT:	{
		register byte	*sp = uimg.src;
			for (row=0; row<uimg.height; row++)
			    for (col=0; col<uimg.width; col++)
				fprintf(out_fp, "%f %f %d\n",
					col*xscale, row*yscale, *sp++);
		}	break;
		case IFMT_LONG:	{
		register long	*ip = uimg.src;
			for (row=0; row<uimg.height; row++)
			    for (col=0; col<uimg.width; col++)
				fprintf(out_fp, "%f %f %d\n",
					col*xscale, row*yscale, *ip++);
		}	break;
		case IFMT_FLOAT:	{
		register byte	*fp = uimg.src;
			for (row=0; row<uimg.height; row++)
			    for (col=0; col<uimg.width; col++)
				fprintf(out_fp, "%f %f %f\n",
					col*xscale, row*yscale, *fp++);
		}	break;
		default:
			prgmerr('f', "only handle byte to float format");
		}
	}
    }
}
