/* inrlvfft.cxx
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
	*/
dimen2size = dimen1len * row << 1;

if (uimg.in_form==IFMT_DVFFT2D || uimg.in_form==IFMT_DVFFT3D)	{
double	*otemp,
	*obuf = nzalloc(fsize, sizeof(*obuf), "fobuf"),
	*ibuf = nzalloc(dimen2size*(dimens?1:frm), sizeof(*ibuf), "fibuf");

	if (uimg.o_form==IFMT_BYTE)
		otemp = nzalloc(fsize, 1, "otemp");
	else	otemp = obuf;

	DBwork_space_init(MAX(MAX(cln, row), frm));
	if (!dimens && frm>1)	{
	int	r;
	register DBCOMPLEX	*cmptr, *cp;
	register int	c, n=frm, dimen1size = dimen1len;

		fsize = n * dimen2size;
		i = fread(ibuf, sizeof(*ibuf), fsize, in_fp);
		if (i != fsize)
			syserr("3d[%ld] read %d", fsize, i);

		load_DBw(frm);

		for (r=0; r<row; r++)
		    for (c=0; c<dimen1size; c++) {
			cp = (DBCOMPLEX*)ibuf + r*dimen1size + c;
			cmptr = DBcmpx_in;
			for (i = 0; i < n; i++, cp+=dimen2size>>1) {
				c_re (cmptr [i]) = c_re(*cp);
				c_im (cmptr [i]) = -c_im(*cp);
			}

			DBFourier(cmptr, n, DBcmpx_out);

			cp = (DBCOMPLEX*)ibuf + r*dimen1size + c;/* store back to src */
			cmptr = DBcmpx_out;
			for (i = 0; i < n; i++, cp+=dimen2size>>1)
				*cp = cmptr[i];
		    }
	}

	fsize = row * cln;
	for (f=0; f<frm; f++)	{
	    if (dimens)	{
		register DBCOMPLEX*	cmptr = (DBCOMPLEX*) ibuf;
		i = fread(ibuf, sizeof(*ibuf), dimen2size, in_fp);
		if (i != dimen2size)	syserr("2d[%d] read %d", dimen2size,i);
		i >>= 1;
		while (i--)
			c_im(cmptr[i]) = -c_im(cmptr[i]);
	    }

	    DBvrft2d(ibuf + (dimens ? 0 : f*dimen2size), cln, row, obuf);
	    if (uimg.o_form != IFMT_FLOAT)
		dtob(obuf, otemp, fsize, 1./(fsize*(uimg.in_form&1 ? frm:1)));
	    else
		dtof(obuf, otemp, fsize, 1./(fsize*(uimg.in_form&1 ? frm:1)));
	    i = fwrite(otemp, uimg.pxl_out, fsize, out_fp);
	    if (i != fsize)	syserr("3d[%d] write %d", fsize, i);
	}
}
else	{
float	*otemp,
	*obuf = nzalloc(fsize, sizeof(*obuf), "fobuf"),
	*ibuf = nzalloc(dimen2size*(dimens ? 1 : frm), sizeof(*ibuf), "fibuf");

	if (uimg.o_form == IFMT_BYTE)
		otemp = nzalloc(fsize, 1, "otemp");
	else	otemp = obuf;

	work_space_init(MAX(MAX(cln, row), frm));
	if (!dimens && frm>1)	{
	int	r;
	register COMPLEX	*cmptr, *cp;
	register int	c, n=frm, dimen1size = dimen1len;

		fsize = n * dimen2size;
		i = fread(ibuf, sizeof(*ibuf), fsize, in_fp);
		if (i != fsize)
			syserr("3d[%ld] read %d", fsize, i);

		load_w(frm);

		for (r=0; r<row; r++)
		    for (c=0; c<dimen1size; c++) {
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
		register COMPLEX*	cmptr = (COMPLEX*) ibuf;
		i = fread(ibuf, sizeof(*ibuf), dimen2size, in_fp);
		if (i != dimen2size)	syserr("2d[%d] read %d", dimen2size,i);
		i >>= 1;	/* size float -> complex	*/
		while (i--)
			c_im(cmptr[i]) = -c_im(cmptr[i]);
	    }

	    vrft2d(ibuf + (dimens ? 0 : f*dimen2size), cln, row, obuf);
	    if (uimg.o_form != IFMT_FLOAT)
		ftob(obuf, otemp, fsize, 1./(fsize*(uimg.in_form&1 ? frm:1)));
	    i = fwrite(otemp, uimg.pxl_out, fsize, out_fp);
	    if (i != fsize)	syserr("3d[%d] write %d", fsize, i);
	}
}
