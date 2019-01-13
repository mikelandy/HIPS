/*	VFFTFILTER . C
#
%	Circular Filter for VFFT image transform.
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
% compile:
%	cc -O -o DEST/vfftfilter vfftfilter.c -lscs3 -lccs -lhips -lrle -lm
%
% AUTHOR:	Jin Guojun - Lawrence Berkeley Laboratory	5/1/91
*/

#include "vfft_fil.h"

U_IMAGE	uimg;

arg_fmt_list_string	arg_fmt[] =	{
	{"-b", "%f", 0, 1, 1, "base value. [Default=%.f], good max => 2.0"},
	{"-c", "%N", CURVE_CONVEX, 1, 0, "convex curve. Default is concave curve"},
	{"-L", "%N", CURVE_LINEAR, 1, 0, "Linear filter"},
	{"-fe", "%f", 0, 1, 1, "elastic scale factor"},
	{"-fu", "%d %s", FUNC_ELASTIC, 2, 1, "filter function [table name]\n\
	filter functions:	\n\
	default:	elastic	\n\
	1		ideal	\n\
	2		exponential\n\
	3		butterworth\n\
	4		right tri-angle\n\
	5		user defined standard table\n\
	6 table_name	user defined table (ascii)\n\
	7 table_name	user defined table (binary)\n"},
	{"-F", "%d", 1, 1, 1,
		"number of Frames in sample [%.f in 2D]. more F # is 3D"},
	{"-G", "%b", True, 1, 0, "generate gnu_plot data of curves only"},
	{"-hi", "%b", True, 1, 0, "high-pass filter. Default is low-pass"},
	{"-min", "%f", .01, 1, 1, "minimum value at bottom [%.2f]"},
	{"-p", "%f", 2, 1, 1, "powers [%.f]"},
	{"-r", "%f", .3333, 1, 1, "radius [%.4f]"},
	{"-R", "%f", 1, 1, 1,
		"radius factor. default is %.f (100%). Range 0.1 - 1.0"},
	{"-v", "%b", True, 1, 1, "verbose"},
	{"-o", "%s", NULL, 1, 1, "output name"},
	{"	[<] VFFT [> F.VFFT]", "0", 0, 0, 0, "End of Notes"},
	NULL	};

char	*funame, **fl, *ofname;
bool	dimens, curve_dir, hi_pass, Table, Msg;
VType	*itemp;
Filter	*lkt, flr, power, radius, sc;
int	dimen1len, h_rows, h_frm, sframes,
	r_radius, c_radius, f_radius, max_radius;
MType	fsize, vsize;


main (argc, argv)
int	argc;
char**	argv;
{
bool	vflag, sample=0;
int	f, fn, max_dis;
MType	*ldis_p2, *d2_dis_p2;
Filter	base, radius_rate, *filter3d, **mask_plane;
register int	i;

format_init(&uimg, IMAGE_INIT_TYPE, HIPS, *argv, "Oc1-2");

if ((i=parse_argus(&fl, argc, argv, arg_fmt,
		&base,
		&curve_dir, &curve_dir,
		&sc,
		&fn, &funame,
		&sframes,
		&Table,	&hi_pass,
		&flr,
		&power,
		&radius, &radius_rate,
		&Msg, &ofname)) < 0)	exit(i);
if (ofname && !freopen(ofname, "wb", stdout))
	syserr("%s can't be opened", ofname);

if (i && (in_fp=freopen(fl[0], "r", stdin)) == NULL)
	syserr("can't open frame file - %s", fl[0]);
if (radius_rate < 0 || radius_rate > 1.0)
	radius_rate = 1.0;

io_test(stdout_fd, prgmerr('o', "must open output"));

io_test(fileno(in_fp), sample=!iset);

if (sample){	/* no input file */
	cln = row = 128;
	uimg.frames = sframes + !sframes;
	uimg.pxl_in = 8;
	uimg.in_form = IFMT_VFFT3D;
	(*uimg.std_swif)(FI_INIT_NAME, &uimg, *argv, 0);
}
else	{
	(*uimg.header_handle)(HEADER_READ, &uimg, 0, 0);
	if (uimg.in_form < IFMT_VFFT3D || uimg.in_form  > IFMT_VVFFT3D)
		syserr("image is not a VFFT format");
	vflag = uimg.in_form==IFMT_VVFFT3D || uimg.in_form==IFMT_DVVFFT3D;
}
uimg.o_form = uimg.in_form;
uimg.pxl_out = uimg.pxl_in;

if (!vflag)
	dimens = !(uimg.in_form & 1); /* if it is 2D */
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

max_radius = MAX(MAX(h_frm, h_rows), dimen1len);
max_dis = sqrt((double)(h_frm*h_frm + h_rows*h_rows + dimen1len*dimen1len));
lkt = zalloc(max_dis, sizeof(*lkt), "lkt");
ldis_p2 = zalloc(max_radius, sizeof(*ldis_p2), "ldis_p2");
d2_dis_p2 = nzalloc((max_radius+1)*max_radius>>1, sizeof(*d2_dis_p2), "d2d");

filter3d = nzalloc(sizeof(*filter3d), max_radius*max_radius, "filter3d");
ibuf = nzalloc(uimg.pxl_in, vsize, "ibuf");
obuf = zalloc(uimg.pxl_in, vsize, "obuf");	/* be zeroed */

mask_plane = nzalloc(max_radius, sizeof(*mask_plane), "mask_p");
mask_plane[0] = filter3d;
for (i=1; i<max_radius; i++)
	mask_plane[i] = mask_plane[i-1] + max_radius;

#ifdef	_DEBUG_
message("Max_R=%d, f_r=%d, r_r=%d, c_r=%d, max_dis=%d, func=%d\n",
	max_radius, f_radius, r_radius, c_radius, max_dis, fn);
#endif

/*	filling distance line	*/
for (i=1; i<max_radius; i++)
	ldis_p2[i] = i*i;
/*	filling distance right tri_angle plane	*/
{
register int	r, c, *dp=d2_dis_p2;
for (r=0; r<max_radius; r++)
    for (c=r; c<max_radius; c++)
	*dp++ = ldis_p2[r] + ldis_p2[c];
}

if (fn>1 && sc)	radius = sc * 0.002;
if (hi_pass)	sc = -sc;
lkt_function(lkt, fn, max_dis);

if (Table)	exit(0);	/* only send plot data to stdout. */

(*uimg.header_handle)(HEADER_WRITE, &uimg, argc, argv, True);

if (sample){
	{
	register float	*ibp=ibuf;
	    for (i=vsize << 1; i--;)
		ibp[i] = 256.;
	}
	for (f=h_frm; f--;){
		fill_mask_plane(d2_dis_p2, max_radius, ldis_p2[f], mask_plane, lkt);
		filtering(ibuf, obuf, base, filter3d, f+1);
	}
	if (frm & 1){
		fill_mask_plane(d2_dis_p2, max_radius, 1, mask_plane, lkt);
		filtering(ibuf, obuf, base, filter3d, f+1);
	}
	for (f=0; f<h_frm; f++){
		fill_mask_plane(d2_dis_p2, max_radius, ldis_p2[f], mask_plane, lkt);
		filtering(ibuf, obuf, base, filter3d, f+h_frm);
	}
	exit(0);
}

if (dimens){
	message("%s: 2 dimension filter - %d frames\n", Progname, frm);
	fill_mask_plane(d2_dis_p2, max_radius, ldis_p2[0], mask_plane, lkt);
	for (f=0; f<frm; f++){
		i = upread(ibuf, uimg.pxl_in, vsize, in_fp);
		if (i != vsize)
			syserr("f%d [%d] read %d", f+1, vsize, i);
		filtering(ibuf, obuf, base, filter3d, f+1);
	}
}
else	{
	for (f=0; f<f_radius; f++){
		fill_mask_plane(d2_dis_p2, max_radius, ldis_p2[f], mask_plane, lkt);
		i = upread(ibuf, uimg.pxl_in, vsize, in_fp);
		if (i != vsize)
			syserr("f%d [%d] read %d", f+1, vsize, i);
		filtering(ibuf, obuf, base, filter3d, f+1);
	}

	for (; frm>1 && f<frm-(f_radius<<1); f++){	/* maybe send all 0 frames */
		fill_mask_plane(d2_dis_p2, max_radius, ldis_p2[f_radius], mask_plane, lkt);
		i = upread(ibuf, uimg.pxl_in, vsize, in_fp);
		if (i != vsize)
			syserr("f%d [%d] read %d", f+1, vsize, i);
		filtering(ibuf, obuf, base, filter3d, f+1);
	}

	for (; f<frm; f++){
		fill_mask_plane(d2_dis_p2, max_radius, ldis_p2[frm-1-f], mask_plane, lkt);
		i = upread(ibuf, uimg.pxl_in, vsize, in_fp);
		if (i != vsize)
			syserr("f%d [%d] read %d", f+1, vsize, i);
		filtering(ibuf, obuf, base, filter3d, f+1);
	}

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
	    for (i=max_radius*max_radius; i--;)
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
	    filterp += max_radius - c_radius;
	}

	i = fwrite(o_buf, uimg.pxl_in, vsize, out_fp);
	if (i != vsize)
		syserr("f%d [%d] write %d", f, vsize, i);
}

fill_mask_plane(dis_p2, ldis_len, framedis, filter_buf, lkt)
register MType	*dis_p2;
register Filter	**filter_buf, *lkt;
{
register int	r, c;
register Filter	fdis = framedis;

for (r=0; r<ldis_len; r++){
    filter_buf[r][r] = lkt[(int)(sqrt(fdis+*dis_p2++) + 0.5)];
    for (c=r+1; c<ldis_len; c++){
    register int distance = sqrt(fdis+*dis_p2++) + 0.5;
	filter_buf[r][c] = filter_buf[c][r] = lkt[distance];
    }
/*	filter_buf[r][c] = filter_buf[c][r] =
		lkt[(int)(sqrt(fdis+*dis_p2++) + 0.5)];
*/
}
}

Filter
power_func(fn, i, r)
register int	fn, i;
register Filter	r;
{
    if (fn == FUNC_EXP)
	if (curve_dir)	r = exp(-pow(r / i, power));
	else		r = exp(-pow(i / r, power));
    else if (curve_dir)	r = 1. / (1 + pow(r / i, power));
	else		r = 1. / (1 + pow(i / r, power));
return	r;
}

lkt_function(lkt, fn, ns)
Filter	*lkt;
{
char	str[128], stmp[64];
register Filter	R = radius * ns;
register int	i, rev;

sprintf(str, "radius = %.3f, filter is ", (fn<6) ? radius : 1.);

switch (fn) {
case FUNC_IDEAL:
	funame = sprintf(stmp, "ideal filter %.2f\n", R);
	strcat(str, stmp);
	if (hi_pass)
	    for (i=R; i<ns; i++)
		lkt[i] = 1.;
	else
	    for (i=R; i--;)
		lkt[i] = 1.;
	break;
case FUNC_EXP:
case FUNC_BUTTERWORTH:
	strcat(str, fn==FUNC_EXP ? "exponetial\n" : "butterworth\n");
	if (hi_pass){
		lkt[0] = flr;
		if (curve_dir) for (i=ns; --i;)
			lkt[i] = power_func(fn, i, R);
		else for (i=ns, rev=ns-1; --i;)
			lkt[rev-i] = power_func(fn, i, R);
	}
	else if (curve_dir)	for (i=ns, rev=ns-1; i--;)
			lkt[rev-i] = power_func(fn, i, R);
	else	for (i=ns; i--;)
			lkt[i] = power_func(fn, i, R);
	break;
case FUNC_RIGHT_TRI_ANGLE:
	strcat(str, "right tri_angle\n");
	if (hi_pass)
	    for (i=0; i<ns; i++)
		lkt[i] = (i+flr) / ns;
	else
	    for (i=0; i<ns; i++)
		lkt[i] = (ns-1-i+flr) / ns;
	break;
case FUNC_STD_TABLE:
case FUNC_ASCII_TABLE:	case FUNC_BINARY_TABLE:
	if (strlen(funame)==0)
		funame = "filter_table";
	{
	char	buf[64];
	int	len;
	FILE	*fp = fopen(funame, fn==FUNC_ASCII_TABLE ? "r" : "rb");
		if (!fp)	syserr("open %s", funame);
		if (fn==FUNC_ASCII_TABLE)	{
			fscanf(fp, "%d", &len);
			if (len > ns)	len = ns;
			for (i=0; i<len; i++)
				fscanf(fp, "%f", lkt+i);
		} else	{
			len = getw(fp);	/* get table length */
			if (len > ns)	len = ns;
			fread(lkt, sizeof(*lkt), len, fp);
		}
		sprintf(buf, "%d user defined %s\n", len, funame);
		strcat(str, buf);
		for (;i<ns; i++)
			lkt[i] = lkt[i-1];
	}
	break;
default:
	strcat(str, "elastic\n");
	eta_f_curve(lkt, sc, ns, hi_pass, curve_dir, flr, "ela", uimg.in_form);
}
mesg(str);
if (Table)	GTable(lkt, ns, hi_pass);
(*uimg.header_handle)(ADD_DESC, &uimg, str);
}
