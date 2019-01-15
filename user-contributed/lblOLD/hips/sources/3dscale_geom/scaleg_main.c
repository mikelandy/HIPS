/* scaleg_main:	main program for 3D image scaling
#
% This program is a hack from the 'zoom' program by Paul Heckbert, UC Berkeley
% The zoom program was modified to work with the Hips file format.
%	-Brian Tierney, LBL  9/89
%
%   terminology:
%	x = colums
%	y = rows
%
% see additional comments in scale_g.c and pic.3
%
% Modified:	by Jin Guojun
%		6/15/91	for 3-dimensional handling
%		8/15/92	for handling color
%		12/6/94	fixed a bug -	Line 285
*/

#include <math.h>

#include "hips.h"
#include "simple.h"
#include "pic.h"
#include "filt.h"
#include "scaleg.h"
#include "scanline.h"

#define FILTER_DEFAULT "triangle"
#define WINDOW_DEFAULT "blackman"

U_IMAGE	uimg;
arg_fmt_list_string	arg_fmts[] =	{
	{"-#", "%f", 0, 1, 1,
		"image will be shrinked or enlarged # times (-5.0 ~ +5.0)\n\
	-# means '-real_number', such as: -1.1, --4.75, ..."},
	{"-blur", "%g %g %g", -1., 3, 1,
		"blur factor: > 1 is blurry, < 1 is sharp"},
	{"-c", "%b", True, 1, 0, "output color (RLE) image"},
	{"-D", "%d %d %d", 0, 3, 2,
		"number of cols, rows, [frames] in file dest file"},
	{"-d[c][r][f]", "%d %d %d %d", 0, 1, 1,
		"number of cols, [rows], [frames] in destination file\n\
			(default = source size)"},
	{"-filt", "%s %s %s", No, 3, 0, "\n\
	filter name in x, y, and z (default = triangle)\n\
		\"-filt '?'\" prints a filter catalog"},
	{"-s",  "%d %d %d %d", 0, 4, 4, "source box (row col #rows #cols)"},
	{"-supp", "%g %g %g", -1, 3, 1, "filter support radius"},
	{"-window", "%s %s", 0, 2, 1,
		"window an IIR filter (default = blackman)"},
	{"-xy", "%b", True, 1, 0, "filter x before y"},
	{"-yx", "%b", False,1, 0, "filter y before x"},
	{"-debug", "%i", 0, 1, 1, "print filter coefficients"},
	{"-v", "%d", 1, 1, 0, "verbose {2 exit}"},
	{"-plain", "%b", True, 1, 0, "\bdisable filter coercion"},
	{"-keep0", "%b", True, 1, 0, "\bkeep zeros in xfilter"},
	{"-i", "%b", True, 1, 0, "integer scale"},
	{" [ < ] input [ > | ] output\n", NULL, 0, 0, 0, "end of usage"},
	NULL	};


main(ac, av)
int	ac;
char	**av;
{
char	*xfiltname=FILTER_DEFAULT, *yfiltname=0, *zfiltname=0,
	*xwindowname=0, *ywindowname=0, **fl;
int	nocoerce, square, intscale, keepzeros, i, v_pic_c=0;
float	scale_factor;
double	xsupp, ysupp = -1., zsupp = -1.,
	xblur, yblur = -1., zblur = -1.;
Pic	*apic, *bpic;
Window_box	a,	/* src window */
		b;	/* dest window */
Filt	*xfilt, *yfilt, *zfilt, xf, yf, zf;

a.x0 = a.y0 = a.z0 = a.x1 = a.y1 = a.z1 = a.nx = a.ny = a.nz = 0;
b.x0 = b.y0 = b.z0 = b.x1 = b.y1 = b.z1 = b.nx = b.ny = b.nz = 0;
square = intscale = 0;
nocoerce = keepzeros = 0;

p_use_pound_sign(True);
    if ((i=parse_argus(&fl, ac, av, arg_fmts, &scale_factor,
	&xblur, &yblur, &zblur,
	&uimg.color_dpy,
	&b.nx, &b.ny, &b.nz,
	&b.nx, &b.nx, &b.ny, &b.nz,
	&xfiltname, &yfiltname, &zfiltname,
	&a.x0, &a.y0, &a.nx, &a.ny,
	&xsupp, &ysupp, &zsupp,
	&xwindowname, &ywindowname,
	&zoom_xy, &zoom_xy,
	&zoom_debug, &v_pic_c,
	&nocoerce, &keepzeros,
	&intscale)) < 0)	exit(i);

    if (v_pic_c)	{
	pic_catalog();
	if (v_pic_c > 1)	exit(0);
    }
    if (str_eq(xfiltname, "?"))
	filt_catalog(),	exit(0);

    if (fabs(scale_factor) > 5.)
info:	parse_usage(arg_fmts),	exit(-1);

format_init(&uimg, IMAGE_INIT_TYPE, uimg.color_dpy ? RLE : HIPS, -1, *av, "M5-3");

    if (i && !(in_fp=freopen(uimg.name=fl[0], "rb", stdin))	)
	syserr("can't open %s", uimg.name);
    io_test(fileno(in_fp), goto	info);

    zoom_coerce = !nocoerce;
    zoom_trimzeros = !keepzeros;

	/* for I/O files */
	apic = pic_open("stdin", "r");
	bpic = pic_open("stdout", "w");

	/* check input image size */
    if (a.x0 > uimg.width || a.y0 > uimg.height ||
	a.nx > uimg.width || a.ny > uimg.height)
	prgmerr(1, "\n Error: input image is not that big. \n\n");
    /*
     * set defaults
     */
    if (a.nx == 0)
	a.nx = uimg.width;
    if (a.ny == 0)
	a.ny = uimg.height;
    if (!a.nz)
	a.nz = uimg.frames;
    /* want square pixels, so scale x or y to the appr. value */
    if (b.nx == 0 && b.ny == 0) {
	b.nx = a.nx;
	b.ny = a.ny;
    }
    if (b.nx == 0)
	b.nx = (int) ((b.ny * a.nx) / (float) a.ny + .5);
    if (b.ny == 0)
	b.ny = (int) ((b.nx * a.ny) / (float) a.nx + .5);
    if (!b.nz)
	if (a.nz == 1 || uimg.color_dpy)
		b.nz = a.nz;
	else if (b.nx == b.ny)
		b.nz = b.nx;
	else	b.nz = (b.nx * a.nz) / (float) a.nx + .5;

    if (scale_factor > 0)	{
	b.nx = a.nx * scale_factor;
	b.ny = a.ny * scale_factor;
	if (!uimg.color_dpy)	b.nz = a.nz * scale_factor;	/* no 3D color */
    }
    else if (scale_factor < 0)	{
	scale_factor = -scale_factor;
	b.nx = a.nx / scale_factor;
	b.ny = a.ny / scale_factor;
	if (!uimg.color_dpy)	b.nz = a.nz / scale_factor;
    }
    if (!b.nz)
	b.nz++;
    if (b.nx > XMAX || b.ny > YMAX)
	prgmerr(2, "\ndestination file must be smaller than %d by %d \n\n",
		XMAX, YMAX);

    uimg.width = b.nx;
    uimg.height = b.ny;		/* must do this after call to zoom */
    uimg.frames = b.nz;
    if (uimg.o_type == HIPS)	uimg.update_header = -1,
	(*uimg.header_handle)(HEADER_WRITE, &uimg, ac, av, True);

    /* sets x1 and y1 values */
    window_box_set_max(&a);
    window_box_set_max(&b);

    if (!yfiltname)
	yfiltname = xfiltname;
    if (!zfiltname)
	zfiltname = xfiltname;
    xfilt = filt_find(xfiltname);
    yfilt = filt_find(yfiltname);
    zfilt = filt_find(zfiltname);
    if (!xfilt | !yfilt | !zfilt)
	prgmerr(1, "can't find filters %s or %s or %s\n",
		xfiltname, yfiltname, zfiltname);
	if (v_pic_c)	message("XF %s, YF %s, ZF %s\n",
		xfilt->name, yfilt->name, zfiltname);
    /* copy the filters before modifying them */
    xf = *xfilt;
    xfilt = &xf;
    yf = *yfilt;
    yfilt = &yf;
    zf = *zfilt;
    zfilt = &zf;
    if (xsupp >= 0.)
	xfilt->supp = xsupp;
    if (xsupp >= 0. && ysupp < 0.)
	ysupp = xsupp;
    if (ysupp >= 0.)
	yfilt->supp = ysupp;
    if (xsupp >= 0. && zsupp < 0.)
	zsupp = xsupp;
    if (zsupp >= 0.)
	zfilt->supp = zsupp;
    if (xblur >= 0.)
	xfilt->blur = xblur;
    if (xblur >= 0. && yblur < 0.)
	yblur = xblur;
    if (yblur >= 0.)
	yfilt->blur = yblur;
    if (xblur >= 0. && zblur < 0.)
	zblur = xblur;
    if (zblur >= 0.)
	zfilt->blur = zblur;

    if (!ywindowname)
	ywindowname = xwindowname;
    if (xwindowname || xfilt->windowme) {
	if (!xwindowname)
	    xwindowname = WINDOW_DEFAULT;
	xfilt = filt_window(xfilt, xwindowname);
    }
    if (ywindowname || yfilt->windowme) {
	if (!ywindowname)
	    ywindowname = WINDOW_DEFAULT;
	yfilt = filt_window(yfilt, ywindowname);
    }
    if (xfilt->printproc) {
	mesg("xfilt: ");
	filt_print_client(xfilt);
    }
    if (yfilt->printproc) {
	mesg("yfilt: ");
	filt_print_client(yfilt);
    }
    /* process each of the frames */
    if (a.nz == b.nz)
	for (i=0; i < uimg.frames; i++) {
	Hips	*save_a = apic->data, *save_b = bpic->data;
	int	rf_size = save_a->dx * save_a->dy, wf_size;
		if (v_pic_c && uimg.frames > 1)
			msg("\r	processsing frame %d ...", i);
		if (load_hips_frame(&uimg, apic->data) < rf_size)	break;
		zoom_opt(apic, &a, bpic, &b, xfilt, yfilt, square, intscale);
		if (isColorImage(uimg.color_form))	{
			wf_size = save_b->dx * save_b->dy;
			save_a->ibuf += rf_size;
			save_b->ibuf += wf_size;
			zoom_opt(apic, &a, bpic, &b, xfilt, yfilt, square, intscale);
			save_a->ibuf += rf_size;
			save_b->ibuf += wf_size;
			zoom_opt(apic, &a, bpic, &b, xfilt, yfilt, square, intscale);
			save_a->ibuf -= rf_size << 1;
			save_b->ibuf -= wf_size << 1;
			if (uimg.in_type==RLE)	i = -1;
		}
		store_hips_frame(bpic->data);
		if (i < 0)
		if ((*uimg.header_handle)(HEADER_READ, &uimg, 0, Yes, 0))
			break;
		else if (uimg.dpy_channels == 3)
			uimg.color_form = CFM_SEPLANE,	uimg.frames = 1;
    }	else	{
	int	j, z, line, n=MIN(8, a.nz), nb = b.nx*b.ny;
	Mapping	m;
	Filtpar	az;
	Pixel1	*fbuf = ZALLOC(8, nb, "fbuf"),
		*itmp, *ibuf, *buf3d;
	Pixel4	*accum = NZALLOC(sizeof(*accum), nb, "accum");
	Weighttab	zweight;
	Hips	*hp = bpic->data;

	m.sz = (double)b.nz / a.nz;
	m.tz = b.z0 - .5 - m.sz*(a.z0 - .5);
	az.scale = zfilt->blur*MAX(1., 1./m.sz);
	az.supp = MAX(.5, az.scale*zfilt->supp);
	az.wid = ceil(2.*az.supp);
	m.uz = b.z0 - m.sz*(a.z0 - .5) - m.tz;
	zweight.weight = ZALLOC(sizeof(*zweight.weight), az.wid, "zweight");

	for (i=0; i<n; i++)	{	/* first 8 - 2D frames	*/
		load_hips_frame(&uimg, apic->data);
		hp->ibuf = fbuf+nb*i;
		zoom_opt(apic, &a, bpic, &b, xfilt, yfilt, square, intscale);
	}
	hp->ibuf = buf3d = NZALLOC(sizeof(*buf3d), nb, "buf3d");
	for (j=0; j<b.nz; j++)	{
	    make_weighttab(b.z0+j, MAP(j, m.sz, m.uz),	/* increase zw.i1 */
				zfilt, &az, a.nz, 0, &zweight);
	    while (zweight.i1 >= i && i < a.nz)	{
		load_hips_frame(&uimg, apic->data);
		hp->ibuf = fbuf+nb*(i&7);	/* prepare more 2D Fs */
		zoom_opt(apic, &a, bpic, &b, xfilt, yfilt, square, intscale);
		i++;
	    }
	    hp->ibuf = buf3d;
	    for (line=0; line<b.ny; line++)	{	/* process 3D	*/
	    register Pixel4	*ap=accum;
		bzero(ap, b.nx<<2);
		for (z=b.nx; z--;)
	/*		--*ap++;	*/
			*ap++ = (1 << CHANBITS) - 1;
		itmp = fbuf + line*b.nx;
		for (z=zweight.i0; z<zweight.i1; z++){
		register short	weight = zweight.weight[z-zweight.i0];
		    ap = accum;
		    ibuf = (z&7) * nb + itmp;
		    for (n=b.nx; n--;)
			*ap++ += *ibuf++ * weight;
		}
		ibuf = hp->ibuf + b.nx*line;
		ap = accum;
		for (n=b.nx; n--;)
			*ibuf++ = *ap++ >> CHANBITS+6; /* scale needs	*/
	    }
	store_hips_frame(bpic->data);
	}
    }

message("%s : done\n", Progname);
return (0);
}
