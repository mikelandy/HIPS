/*	VFFTPASS . C
#
%	Symmetric Filter for VFFT image transform.
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
% compile:	cc -O -o DEST/vfftpass vfftpass.c -lscs3 -lccs -lhips -lm
%
% AUTHOR:	Guojun Jin - Lawrence Berkeley Laboratory	5/1/91
*/

#include "vfft_fil.h"

U_IMAGE	uimg;

arg_fmt_list_string	arg_fmt[] =	{
	{"-b", "%f", 0, 1, 1, "base value. [Default=%.f], good max => 2.0"},
	{"-c", "%N", CURVE_CONVEX, 1, 0, "convex curve. Default is concave curve"},
	{"-L", "%N", CURVE_LINEAR, 1, 0, "Linear filter"},
	{"-f", "%f", 0, 1, 1, "elastic scale factor"},
	{"-F", "%d", 16, 1, 1,
		"number of Frames in sample [%.f]"},
	{"-G", "%b", True, 1, 0, "generate gnu_plot data of curves only"},
	{"-hi", "%b", True, 1, 0, "high-pass filter. Default is low-pass"},
	{"-H", "%f", .005, 1, 0, "high pass adjust"},
	{"-min", "%f", .01, 1, 1, "minimum value at bottom [%.2f]"},
	{"-n", "%b", True, 1, 0, "negate filter (1D -hi)"},
	{"-r", "%f", 1, 1, 1,
		"radius factor. default is %.f (100%). Range 0.1 - 1.0"},
	{"-v", "%b", True, 1, 1, "verbose"},
	{"-o", "%s", NULL, 1, 1, "output name"},
	{"	[<] VFFT [> [-o] F.VFFT]", "0", 0, 0, 0, "End of Notes"},
	NULL	};
bool	dimens, curve_dir, hi_pass, Table, Msg;
VType	*itemp;
Filter	*lkt[3], flr, sc;
int	dimen1len, h_rows, h_frm, sframes,
	r_radius, c_radius, f_radius;
MType	fsize, vsize;


main (argc, argv)
int	argc;
char**	argv;
{
bool	neg=0, vflag, sample=0;
char	**fl, *ofname;
int	f, j;
Filter	base, Hbase;
Filter	radius_rate, *filter3d, *filter_work;
register int	i;

format_init(&uimg, IMAGE_INIT_TYPE, HIPS, -1, *argv, "Oc1-2");

if ((i=parse_argus(&fl, argc, argv, arg_fmt,
		&base,
		&curve_dir, &curve_dir,
		&sc, &sframes,
		&Table,	&hi_pass, &Hbase,
		&flr,	&neg, &radius_rate,
		&Msg, &ofname)) < 0)	exit(i);
if (ofname && !freopen(ofname, "wb", stdout))
	syserr("%s can't be opened", ofname);

if (i && (in_fp=freopen(fl[0], "r", stdin)) == NULL)
	syserr("can't open frame file - %s", fl[0]);
if (radius_rate < 0 || radius_rate > 1.0)
	radius_rate = 1.0;

io_test(fileno(in_fp), sample=!iset);

if (sample) {	/* no input file */
	uimg.width = uimg.height = 128;
	uimg.frames = sframes;
	uimg.pxl_in = 8;
	uimg.in_form = IFMT_VFFT3D;
	(*uimg.std_swif)(FI_INIT_NAME, &uimg, "VPASS", 0);
}
else	{
	(*uimg.header_handle)(HEADER_READ, &uimg, 0, 0);
	if (uimg.in_form < IFMT_VFFT3D || uimg.in_form  > IFMT_VVFFT3D)
		syserr("image is not a VFFT format");
	vflag = uimg.in_form==IFMT_VVFFT3D || uimg.in_form==IFMT_DVVFFT3D;
}

if (!vflag)	/* the VVFFT must be in 3D */
	dimens = !(uimg.in_form & 1); /* if it's 2D */

dimen1len = (cln>>1) + 1;
h_rows = row >> 1;
h_frm = frm >> 1;
fsize = row*cln;
vsize = row*dimen1len;
f_radius = radius_rate * h_frm;
r_radius = radius_rate * h_rows;
c_radius = radius_rate * dimen1len;
if (f_radius < 1)	f_radius = 1;
if (r_radius < 1)	r_radius = 1;
if (c_radius < 1)	c_radius = 1;

lkt[lktCol] = zalloc(cln, sizeof(Filter), "lktcol");
lkt[lktRow] = zalloc(row, sizeof(Filter), "lktrow");
lkt[lktFrm] = zalloc(frm, sizeof(Filter), "lktfrm");

if (hi_pass)	sc = -sc;
i = uimg.o_form = uimg.in_form;
uimg.pxl_out = uimg.pxl_in;

eta_f_curve(lkt[lktCol], sc, c_radius, neg, curve_dir, flr, "samples", i);
if (Table)	GTable(lkt, c_radius, hi_pass);
eta_f_curve(lkt[lktRow], sc, r_radius, neg, curve_dir, flr, "lines", i);
if (Table)	GTable(lkt, r_radius, hi_pass);
eta_f_curve(lkt[lktFrm], sc, f_radius, neg, curve_dir, flr, "layers", i);
if (Table)	GTable(lkt, f_radius, hi_pass);
if (hi_pass){
register Filter	*lktp=lkt[lktFrm];
	for (i=f_radius; i--;)
		lktp[i] = 1. - lktp[i] + .01;
	lktp[0] = Hbase;
/*	for (i=r_radius, lktp=lkt[lktRow]; i--;)
		lktp[i] = 1. - lktp[i];
	for (i=c_radius, lktp=lkt[lktCol]; i--;)
		lktp[i] = 1. - lktp[i];
*/
	if (Msg)
		dump_lkt(lktp, f_radius),
		msg("\n%d hi_pass => floor=%f\n", f_radius,flr);
}
if (Table)	exit(0);	/* only send plot data to stdout */

filter3d = nzalloc(sizeof(*filter3d)<<1, r_radius*c_radius, "filter3d");
filter_work = filter3d + r_radius * c_radius;
ibuf = nzalloc(uimg.pxl_in, vsize, "ibuf");
obuf = zalloc(uimg.pxl_in, vsize, "obuf");	/* be zeroed */

{
register Filter	*filterp=filter3d, *llktp=lkt[lktCol];
	memcpy(filterp, llktp, c_radius*sizeof(*filterp));
	filterp += c_radius;
	for (i=1; i<r_radius; i++){
	register Filter	scale = lkt[lktRow][i];
	    for (j=0; j<c_radius; j++)
		*filterp++ = scale * llktp[j];
	}
	if (hi_pass){	/* reverse low to high in 2D instead of in 1D	*/
		filterp = filter3d;
		for (i=r_radius*c_radius; i--;)
			filterp[i] = 1. - filterp[i];
		filterp[0] += Hbase;
	}
}
(*uimg.header_handle)(HEADER_WRITE, &uimg, argc, argv, True);

if (sample){
register Filter	*filterp=filter_work;

	{
	register float	*ibp=ibuf;
	    for (i=vsize << 1; i--;)
		ibp[i] = 256.;
	}
	for (f=h_frm; f--;){
	register Filter	scale = lkt[lktFrm][f];
		for (i=r_radius*c_radius; i--;)
			filterp[i] = filter3d[i] * scale;
		filtering(ibuf, obuf, base, filterp, f+1);
	}
	if (frm & 1)
		filtering(ibuf, obuf, base, filter3d, f+1);
	for (f=0; f<h_frm; f++){
	register Filter	scale = lkt[lktFrm][f];
		for (i=r_radius*c_radius; i--;)
			filterp[i] = filter3d[i] * scale;
		filtering(ibuf, obuf, base, filterp, f+h_frm);
	}
	exit(0);
}

if (dimens){
	message("%s: 2 dimension filter - %d frames\n", Progname, frm);
	for (f=0; f<frm; f++){
		i = upread(ibuf, uimg.pxl_in, vsize, in_fp);
		if (i != vsize)
			syserr("f%d [%d] read %d", f+1, vsize, i);
		filtering(ibuf, obuf, base, filter3d, f+1);
	}
}
else	{
register Filter	*filterp=filter_work;

	if (hi_pass){
	register Filter	scale = lkt[lktFrm][0];
		for (i=r_radius*c_radius; i--;)
			filterp[i] = filter3d[i] * scale;
	}
	else	filterp = filter3d;
	i = upread(ibuf, uimg.pxl_in, vsize, in_fp);
	if (i != vsize)
		syserr("f%d [%d] read %d", 1, vsize, i);
	filtering(ibuf, obuf, base, filterp, 1);

	for (f=1; f<f_radius; f++){
	register Filter	scale = lkt[lktFrm][f];
		filterp = filter_work;
		for (i=r_radius*c_radius; i--;)
			filterp[i] = filter3d[i] * scale;
		i = upread(ibuf, uimg.pxl_in, vsize, in_fp);
		if (i != vsize)
			syserr("f%d [%d] read %d", f+1, vsize, i);
		filtering(ibuf, obuf, base, filterp, f+1);
	}

	for (; frm>1 && f<frm-(f_radius<<1); f++){	/* maybe send all 0 frames */
		i = upread(ibuf, uimg.pxl_in, vsize, in_fp);
		if (i != vsize)
			syserr("f%d [%d] read %d", f+1, vsize, i);
		filtering(ibuf, obuf, base, filterp, f+1);
	}

	for (; f<frm-1; f++){
	register Filter	scale = lkt[lktFrm][frm-1-f];
		filterp = filter_work;
		for (i=r_radius*c_radius; i--;)
			filterp[i] = filter3d[i] * scale;
		i = upread(ibuf, uimg.pxl_in, vsize, in_fp);
		if (i != vsize)
			syserr("f%d [%d] read %d", f+1, vsize, i);
		filtering(ibuf, obuf, base, filterp, f+1);
	}

	i = upread(ibuf, uimg.pxl_in, vsize, in_fp);
	if (i != vsize)
		syserr("f%d [%d] read %d", f+1, vsize, i);
	if (hi_pass)	{
	register Filter	scale = lkt[lktFrm][0];
		for (i=r_radius*c_radius; i--;)
			filterp[i] = filter3d[i] * scale;
	}
	else	filterp = filter3d;
	filtering(ibuf, obuf, base, filterp, f+1);
}
}


filtering(i_buf, o_buf, base_v, filterp, f)
VType	*i_buf, *o_buf;
Filter	base_v;
register Filter	*filterp;
{
register float	*ibp=i_buf, *obp=o_buf;
register int	i, j;

	if (base_v)
	    for (i=r_radius*c_radius; i--;)
		filterp[i] += base_v;
	ibp += hi_pass * ((h_rows-r_radius+1)*dimen1len - c_radius);

	for (i=0; i<r_radius; i++){
	register int	dis = (row - 1 - (i<<1)) * dimen1len << 1;
	    for (j=0; j<c_radius; j++){
		obp[dis] = ibp[dis] * *filterp;
		*obp++ = *ibp++ * *filterp;
		obp[dis] = ibp[dis] * *filterp;
		*obp++ = *ibp++ * *filterp++;
	    }
	    obp += dimen1len - c_radius << 1;
	    ibp += dimen1len - c_radius << 1;
	}

	i = fwrite(o_buf, uimg.pxl_in, vsize, out_fp);
	if (i != vsize)
		syserr("f%d [%d] write %d", f, vsize, i);
}
