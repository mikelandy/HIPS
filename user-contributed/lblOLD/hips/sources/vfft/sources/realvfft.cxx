dimen2size = dimen1len * row << 1; /* use non complex represent complex size */

if (dflag)	{
double	*itemp,
	*ibuf = nzalloc(fsize, sizeof(*ibuf)<<1, "fibuf"),
	*obuf = nzalloc(dimen2size*(dimens?1:frm), sizeof(*obuf), "fobuf");

	uimg.o_form = dimens ? IFMT_DVFFT2D : IFMT_DVFFT3D;
	uimg.pxl_out = 16;
	(*uimg.header_handle)(HEADER_WRITE, &uimg, argc, argv, True);

	itemp = ibuf;

	DBwork_space_init(MAX(MAX(cln, row), frm));

	for (f=0; f<frm; f++){
	    i = upread(itemp, uimg.pxl_in, fsize, stdin);
	    if (i != fsize)
		syserr("frame %d [%ld] read %d", f, fsize, i);
	    if (uimg.in_form != IFMT_DOUBLE){
		switch(uimg.in_form){
		case IFMT_BYTE:	btodc(itemp, fsize);	break;
		case IFMT_SHORT:stodc(itemp, fsize);	break;
		case IFMT_LONG:	itodc(itemp, fsize);	break;
		case IFMT_FLOAT:ftodc(itemp, fsize);
		}
	    }

	    DBvfft2d(ibuf, cln, row, obuf + (dimens ? 0 : f*dimen2size));

	    if (dimens){
		i = fwrite(obuf, sizeof(*obuf), dimen2size, out_fp);
		if (i != dimen2size)
			syserr("2d[%d] write %d", dimen2size, i);
	    }
	}
	if (!dimens && frm>1){
	int	r;
	DBCOMPLEX	*cp;
	register DBCOMPLEX	*cmptr;
	register int	c, n=frm, dimen1size = dimen1len;

		load_DBw(frm);

		for (r=0; r<row; r++)
		    for (c=0; c<dimen1size; c++){
			cp = (DBCOMPLEX*)obuf + r*dimen1size + c;
			cmptr = DBcmpx_in;
			for (i = 0; i < n; i++, cp+=dimen2size>>1)
				cmptr[i] = *cp;

			DBFourier(cmptr, n, DBcmpx_out);

			cp = (DBCOMPLEX*)obuf + r*dimen1size + c;/* store back to dst */
			cmptr = DBcmpx_out;
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
else	{
float	*itemp,
	*ibuf = nzalloc(fsize, sizeof(*ibuf)<<1, "fibuf"),
	*obuf = nzalloc(dimen2size*(dimens?1:frm), sizeof(*obuf), "fobuf");

	uimg.o_form = dimens ? IFMT_VFFT2D : IFMT_VFFT3D;
	uimg.pxl_out = 8;
	(*uimg.header_handle)(HEADER_WRITE, &uimg, argc, argv, True);

	itemp = ibuf;

	work_space_init(MAX(MAX(cln, row), frm));

	for (f=0; f<frm; f++)	{
	    i = upread(itemp, uimg.pxl_in, fsize, stdin);
	    if (i != fsize)
		syserr("frame %d [%ld] read %d", f, fsize, i);
	    if (uimg.in_form != IFMT_FLOAT){
		switch (uimg.in_form)	{
		case IFMT_BYTE:	btoc(itemp, fsize);	break;
		case IFMT_SHORT:stoc(itemp, fsize);	break;
		case IFMT_LONG:	itoc(itemp, fsize);	break;
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
