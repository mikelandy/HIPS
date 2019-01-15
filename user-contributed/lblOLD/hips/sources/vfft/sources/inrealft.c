/*
%	INREALFT . C
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
% AUTHOR	Guojun Jin - LBL	12/30/90
*/

#include "complex.h"
#include "header.def"
#include "imagedef.h"

U_IMAGE	uimg;

bool	dimens;
char	usage[]="options\n\
	-B	BYTE formated output\n\
	[<] VFFT [> image]\n";
VType	*otemp;
MType	fsize;
int	Omin, Omax=255;

#define	cln	uimg.width
#define	row	uimg.height
#define	frm	uimg.frames


main (argc,argv)
int	argc;
char**	argv;
{
bool	bflag=0;
int	dimen1len, dimen2size, f, i;

format_init(&uimg, IMAGE_INIT_TYPE, HIPS, -1, *argv, "S20-1");
uimg.o_form=IFMT_FLOAT;

for (i=1; i<argc; i++)
    if (*argv[i] == '-')	{
	int	k=1;
	switch (argv[i][k++])	{
	case 'D':
		dimens = argv[i][k] == '2';
		break;
	case 'B':
		bflag++;	break;
	case 'O':
		if (i+1 >= argc)	break;
		if (strcmp(argv[i]+k, "max")==0)
			Omax = atoi(argv[++i]);
		else if (strcmp(argv[i]+k, "min")==0)
			Omin = atoi(argv[++i]);
		break;
#ifdef	IBMPC
	case 'i':if (avset(argc, argv, &i, &k, 1) &&
			freopen(argv[i]+k, "rb", stdin))	break;
		message("can't open %s as image input", argv[i]);
		goto	errout;
#endif
	case 'o':if (avset(argc, argv, &i, &k, 1) &&
			freopen(argv[i]+k, "wb", stdout))	break;
		message("%s can't be opened", argv[i]);
	default:
errout:		usage_n_options(usage, i, argv[i]);
	}
    }
    else if ((in_fp=freopen(argv[i], "r", stdin) == NULL))
	syserr("can't open frame file - %s", argv[i]);

io_test(fileno(in_fp), goto	errout);

(*uimg.header_handle)(HEADER_READ, &uimg, 0, 0);
fsize = row * cln;
dimen1len = (cln>>1) + 1;
dimen2size = dimen1len * row << 1;
dimens = (uimg.in_form==IFMT_DVFFT2D || uimg.in_form==IFMT_VFFT2D);

work_space_init(MAX(MAX(cln, row), frm));

if (uimg.in_form == IFMT_DVFFT2D || uimg.in_form == IFMT_DVFFT3D)	{
double	*ibuf = nzalloc(fsize, sizeof(*ibuf), "fibuf"),
	*obuf = nzalloc(dimen2size*(dimens?1:frm), sizeof(*obuf), "fobuf");

	uimg.o_form = IFMT_FLOAT;
	uimg.pxl_out = sizeof(float);
}
else	{
float	*obuf = nzalloc(fsize, sizeof(*obuf), "fobuf"),
	*ibuf = nzalloc(dimen2size*(dimens?1:frm), sizeof(*ibuf), "fibuf");

	if (bflag)	{
		uimg.o_form = IFMT_BYTE;
		uimg.pxl_out = 1;
		otemp = nzalloc(fsize, 1, "otemp");
	}
	else{	uimg.o_form = IFMT_FLOAT;
		uimg.pxl_out = sizeof(float);
		otemp = obuf;
	}
	(*uimg.header_handle)(HEADER_WRITE, &uimg, argc, argv, True);

	if (!dimens && frm>1)	{
	int	r;
	register COMPLEX	*cmptr, *cp;
	register int	c, n=frm, dimen1size = dimen1len;

		fsize = frm*dimen2size;
		i = fread(ibuf, sizeof(*ibuf), fsize, in_fp);
		if (i != fsize)
			syserr("3d[%ld] read %d", fsize, i);

		load_w(frm);

		for (r=0; r<row; r++)
		    for (c=0; c<dimen1size; c++){
			cp = (COMPLEX*)ibuf + r*dimen1size + c;
			cmptr = cmpx_in;
			for (i = 0; i < n; i++, cp+=dimen2size>>1) {
				c_re (cmptr [i]) = c_re(*cp);
				c_im (cmptr [i]) = -c_im(*cp);
			}

			Fourier(cmptr, n, cmpx_out);

			cp = (COMPLEX*)ibuf + r*dimen1size + c;/* store back to src */
			cmptr = cmpx_out;
			for (i = 0; i < n; i++, cp+=dimen2size>>1)
				*cp = cmptr[i];
		    }
	}

	fsize = row * cln;
	for (f=0; f<frm; f++)	{
	    if (dimens)	{
		i = fread(ibuf, sizeof(*ibuf), dimen2size, in_fp);
		if (i != dimen2size)	syserr("2d[%d] read %d", dimen2size,i);
	    }

	    vrft2d(ibuf + (dimens?0:f*dimen2size), cln, row, obuf);
	    if (uimg.o_form != IFMT_FLOAT)	{
		ftob(obuf, otemp, fsize, 1./(fsize*(uimg.in_form&1 ? frm:1)));
	    }
	    i = fwrite(otemp, uimg.pxl_out, fsize, out_fp);
	    if (i != fsize)	syserr("3d write[%d] %d", fsize, i);
	}
}
}


ftob(ibp, obp, n, scale)
register float*	ibp;
register byte	*obp;
register MType	n;
register float	scale;
{
register int	i;

#ifdef	_DEBUG_
message("%d FtoB\n", n);
#endif
for (i=0; i<n; i++)
	*obp++ = *ibp++ * scale;
}
