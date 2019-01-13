/*
%	TOICC . C
%
% Read any images (man ccs) and output a ICC image padded to 8-pixel width.
%
% AUTHOR:	Jin Guojun, LBL - 1991
*/

#include "header.def"
#include "imagedef.h"
#include <time.h>

#ifndef	ICC_HEADER_LEN
#define	ICC_HEADER_LEN	60
#endif

arg_fmt_list_string	arg_list[] =	{
	{"-k", "%b", True, 1, 0, "	keep going & ignore error"},
	{"-f", "%b", True, 1, 1, "	enhance on forground"},
	{"-g", "%f", 0., 1, 1, "	gamma for background"},
	{"-l", "%-", 0, 1, 0, "	rotate left"},
	{"-r", "%+", 0, 1, 0, "	rotate right"},
	{"-t", "%s", No, 1, 1, "	title"},
	{"-w", "%b", True, 1, 0, "	without header output"},
{"	[<] input [[ > | ] output]", "0", 0, 0, 0, "End"},	NULL};

ICC_HEADER	icc_hd = {ICC_MAGIC, ICC_HEADER_LEN};
U_IMAGE	uimg;
bool	Etype, nostop, rot, wh;
float	gamma;
int	*hist;
char	*obuf[4];

#define	rows	uimg.height
#define	cols	uimg.width

#ifndef	GAMMA_BASE
#define	GAMMA_BASE	2.49998
#endif

main(ac, av)
int	ac;
char*	av[];
{
char	**fl, *title, date_time[8];
int	nf, maxval, new_rows, new_cols, r_size;
long	clock;
MType	x_size, row, cnt;
struct	tm*	date;
pixel**	xel24;

	uimg.color_dpy = True;
	format_init(&uimg, IMAGE_INIT_TYPE, COLOR_PS, ICC, *av, "D20-1");

	if ((nf=parse_argus(&fl, ac, av, arg_list,
		&nostop, &Etype, &gamma, &rot, &rot, &title, &wh)) < 0)
		exit(nf);
	if (gamma)	{
		if (Etype)
			gamma = -gamma;
		gamma -= GAMMA_BASE;
	}
	if (nf && !freopen(uimg.name=fl[0], "rb", stdin))
		syserr("input file -- %s", fl[0]);

io_test(fileno(in_fp), {parse_usage(arg_list); exit(-1);});

if ((*uimg.header_handle)(HEADER_READ, &uimg, 0, 0, True) < 0)
	syserr("unknown image type");

if (uimg.in_type==PPM || uimg.in_type==PNM)
	xel24 = (pixel**)(*uimg.std_swif)(FI_PNM_MAXVAL, &uimg, &maxval);
(*uimg.std_swif)(FI_LOAD_FILE, &uimg, nostop ? NULL : uimg.name,
	True /* don't change format, and save PNM loading buffer */);

clock = time((long *)0);
date  = localtime(&clock);

if (rot)
	new_rows = cols,	new_cols = rows;
else	new_rows = rows,	new_cols = cols;

r_size = x_size = (new_cols + 7) & 0xFFFFFFF8L;
	/* creation date */
	icc_hd.date = (((long)date->tm_year << 24) & 0xFF000000) +
		(((long)(date->tm_mon + 1) << 16) & 0x00FF0000) +
		(((long)date->tm_mday << 8)	& 0x0000FF00) +
		((long)date->tm_hour		& 0x000000FF);
	icc_hd.time = (((long)date->tm_min << 24) & 0xFF000000) +
		(((long)date->tm_sec << 16) & 0x00FF0000);
	if (uimg.name)
		strncpy(icc_hd.user_name, uimg.name, 32);
	/* subfile header length */
	icc_hd.img_hd.len = sizeof(icc_hd.img_hd);
        /* subfile length */
	icc_hd.img_hd.length = icc_hd.img_hd.len + (x_size * new_rows);
	icc_hd.img_hd.File_Type = 7;	/* ICC type */
	icc_hd.img_hd.orig_width = new_cols;
	icc_hd.img_hd.x_size = x_size;	/* image new width */
	icc_hd.img_hd.y_size = icc_hd.img_hd.orig_height = new_rows;
	icc_hd.img_hd.gray_scale =
		uimg.color_form==CFM_SGF;	/* color=0, greyscale=1	*/
	icc_hd.img_hd.planes =
		icc_hd.img_hd.gray_scale ? 1 : 3;/*	plane count	*/

	if (!wh)
		fwrite(&icc_hd, sizeof(icc_hd), 1, out_fp);
	msg("image size:   %ld(w) x %ld(h)\n", x_size, new_rows);
	if (rot)
		r_size = cols;
	for (cnt=0; cnt < icc_hd.img_hd.planes; cnt++)
		obuf[cnt] = nzalloc(x_size, new_rows, "icc-obuf");

    if (uimg.color_form != CFM_SGF && uimg.color_form != CFM_SEPLANE) {
    register char	*pr = obuf[0],	*pg = obuf[1],	*pb = obuf[2];

	switch (uimg.in_type) {
	register int	col;
	case PPM:
	case PNM:
		for (row=0; row < rows; row++)	{
		register pixel	*pP = xel24[row];
		    for (col=0; col < cols; col++, pP++)
			pr[col] = PPM_GETR(*pP),
			pg[col] = PPM_GETG(*pP),
			pb[col] = PPM_GETB(*pP);
		pr+=r_size,	pg+=r_size,	pb+=r_size;
		}
		free(xel24);
		break;
	default:
		if (uimg.channels==3)
		    if (uimg.color_form == CFM_ILL ||
			uimg.in_type==TiFF && uimg.color_form != CFM_ILC) {
			byte*	scan[3];
				scan[2] = (byte*)uimg.src - cols;
				for (row=0; row < rows; row++)	{
					scan[0] = scan[2] + cols;
					scan[1] = scan[0] + cols;
					scan[2] = scan[1] + cols;
					memcpy(pr, scan[0], cols);
					memcpy(pg, scan[1], cols);
					memcpy(pb, scan[2], cols);
					pr+=r_size, pg+=r_size, pb+=r_size;
				}
		    }
		    else {
			register byte*	ibp = (byte*) uimg.src;
			for (row=rows; row--;pr+=r_size, pg+=r_size, pb+=r_size)
			    for (col=0; col < cols; col++)
				pr[col] = *ibp++,
				pg[col] = *ibp++,
				pb[col] = *ibp++;
			}
		else	{
		register byte	*cmap[3], *bp = (byte *)uimg.src;
			cmap[0] = reg_cmap[0];
			cmap[1] = reg_cmap[1];
			cmap[2] = reg_cmap[2];
			if (cmap[0] == NULL)
				rlemap_to_regmap(cmap, &rle_dflt_hdr);
			for (row=rows; row--; bp+=col, pr+=r_size, pg+=r_size, pb+=r_size)
			    for (col=0; col < cols; col++)
				pr[col] = cmap[0][bp[col]],
				pg[col] = cmap[1][bp[col]],
				pb[col] = cmap[2][bp[col]];
		}
	}
    }
    else while (cnt--)	{
	register char	*po = obuf[cnt], *ps = (char*)uimg.src + cols*rows*cnt;
	for (row=rows; row--; po+=r_size, ps+=cols)
		memcpy(po, ps, cols);
    }
    free(uimg.src);

    if (gamma) {
	LKT*	lkt = zalloc(MaxColors, sizeof(*lkt), "icc-lkt");
	hist = nzalloc(MaxColors, sizeof(*hist), "icc-hist");
	row = new_rows * x_size;
	for (cnt=icc_hd.img_hd.planes; cnt--;)
		apply_gamma(obuf[cnt], row, lkt);
    }
    if (rot) {
	cnt = icc_hd.img_hd.planes;
	obuf[cnt] = nzalloc(x_size, new_rows, "icc-rot");
	while (cnt--)
		rotate90(obuf[cnt], obuf[cnt+1], new_rows, x_size, 1, rot+1);
    }
    for (cnt=0, rot = rot!=0; cnt<icc_hd.img_hd.planes; cnt++)
	if (fwrite(obuf[cnt + rot], new_rows, x_size, out_fp) != x_size)
		syserr("write data");
    for (cnt = icc_hd.img_hd.planes + rot; cnt--;)
	free(obuf[cnt]);
exit(0);
}

apply_gamma(bp, size, lkt)
register byte*	bp;
register LKT*	lkt;
{
register maxval = histogram_calc(bp, size, hist);
	eta_curve(lkt, gamma, maxval, Etype);
	for (maxval=size; maxval--;)
		bp[maxval] = lkt[bp[maxval]];
}
