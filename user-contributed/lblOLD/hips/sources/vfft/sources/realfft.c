/*
%	REALFFT . C
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
% AUTHOR	Jin Guojun - LBL	12/30/90
*/

#include "complex.h"
#include "header.def"
#include "imagedef.h"

#define	row	uimg.height
#define	cln	uimg.width
#define	frm	uimg.frames

U_IMAGE	uimg;
bool	dimens;
char	usage[]="options\n\
-d	double format output\n\
-D2	2-dimension\n\
[<] image [> VFFT]\n";
VType	*itemp;
MType	fsize;


main (argc, argv)
int	argc;
char**	argv;
{
bool	dflag=0;
int	dimen1len, dimen2size, f, i;

format_init(&uimg, IMAGE_INIT_TYPE, HIPS, -1, *argv, "S20-1");

for (i=1; i<argc; i++)
    if (*argv[i] == '-'){
	int	k=1;
	switch (argv[i][k++]) {
	case 'D':
		dimens = (argv[i][k] == '2');
		break;
	case 'd':
		dflag++;	break;
#ifdef	IBMPC
	case 'i':if (avset(argc, argv, &i, &k, 1) &&
		    !(in_fp=freopen(argv[i]+k, "rb", stdin)))
			syserr("can't open %s as image input", argv[i]);
		break;
#endif
	case 'o':if (avset(argc, argv, &i, &k) &&
			freopen(argv[i]+k, "wb", stdout))	break;
		message("%s can't be opened", argv[i]);
	default:
info:		usage_n_options(usage, i, argv[i]);
	}
    }
    else if ((in_fp=freopen(argv[i], "r", stdin)) == NULL)
	syserr("can't open frame file - %s",argv[i]);

io_test(fileno(in_fp), goto	info);

(*uimg.header_handle)(HEADER_READ, &uimg, 0, 0);

fsize = row*cln;
dimen1len = (cln>>1) + 1;
dimen2size = dimen1len * row << 1;

work_space_init(MAX(MAX(cln, row), frm));

if (dflag){
double	*ibuf = nzalloc(fsize, sizeof(*ibuf), "fibuf"),
	*obuf = nzalloc(dimen2size*(dimens?1:frm), sizeof(*obuf), "fobuf");

	uimg.o_form = dimens ? IFMT_DVFFT2D : IFMT_DVFFT3D;
	uimg.pxl_out = 16;
}
else	{
float	*ibuf = nzalloc(fsize, sizeof(*ibuf), "fibuf"),
	*obuf = nzalloc(dimen2size*(dimens?1:frm), sizeof(*obuf), "fobuf");

	uimg.o_form = dimens ? IFMT_VFFT2D : IFMT_VFFT3D;
	uimg.pxl_out = 8;
	(*uimg.header_handle)(HEADER_WRITE, &uimg, argc, argv, True);
	if (uimg.pxl_in<4)
		itemp = nzalloc(fsize, uimg.pxl_in, "itemp");
	else	itemp = ibuf;

	for (f=0; f<frm; f++){
	    if (uimg.in_form != IFMT_FLOAT){
		i = upread(itemp, uimg.pxl_in, fsize, in_fp);
		if (i != fsize)
			syserr("frame %d [%ld] read %d", f, fsize, i);
		switch(uimg.pxl_in){
		case 1:	btof(itemp, ibuf, fsize);
			break;
		case 2:	stof(itemp, ibuf, fsize);
			break;
		case 3:	itof(itemp, ibuf, fsize);
			break;
		}
	    }

	    vfft2d(ibuf, cln, row, obuf + (dimens ? 0 : f*dimen2size));

	    if (dimens){
		i = fwrite(obuf, sizeof(*obuf), dimen2size, out_fp);
		if (i != dimen2size)
			syserr("2d[%d] write %d", dimen2size, i);
	    }
	}
	if (!dimens && frm>1){
	int	r;
	COMPLEX	*cp;
	register COMPLEX	*cmptr;
	register int	c, n=frm, dimen1size = dimen1len;

		load_w(frm);

		for (r=0; r<row; r++)
		    for (c=0; c<dimen1size; c++){
			cp = (COMPLEX*)obuf + r*dimen1size + c;
			cmptr = cmpx_in;
			for (i = 0; i < n; i++, cp+=dimen2size>>1)
				cmptr[i] = *cp;

			Fourier(cmptr, n, cmpx_out);

			cp = (COMPLEX*)obuf + r*dimen1size + c;/* store back to dst */
			cmptr = cmpx_out;
			for (i = 0; i < n; i++, cp+=dimen2size>>1)
				*cp = cmptr[i];
		    }
	/*	row = dimen1size;	real columns for IFMT_COMPLEX */
		fsize = frm*dimen2size;
		i = fwrite(obuf, sizeof(*obuf), fsize, out_fp);
		if (i != fsize)
			syserr("3d[%d] write %d", fsize, i);
	}
}
}

btof(ibp, obp, n)
register byte*	ibp;
register float	*obp;
register int	n;
{
register int	i;
for (i=0; i<n; i++)	*obp++ = *ibp++;
}

stof(ibp, obp, n)
register unsigned short	*ibp;
register float	*obp;
register int	n;
{
register int	i;
msg("%d stof\n", n);
for (i=0; i<n; i++)	*obp++ = *ibp++;
}

itof(ibp, obp, n)
register int*	ibp, n;
register float*	obp;
{
register int	i;
msg("%d itof\n", n);
for (i=0; i<n; i++)	*obp++ = *ibp++;
}

ftoc(ibp, obp, n)
register float*	ibp;
register double	*obp;
register int	n;
{
register int	i;
msg("%d FtoD\n", n);
for (i=0; i<n; i++)	*obp++ = *ibp++;
}
