/*
 * scaleg_main: main program for image scaling
 *
 * This program is a hack from the 'zoom' program by Paul Heckbert, UC Berkeley
 *  The zoom program was modified to work with the Hips file format.
 *  Since Hips file are not color, most (but not all) of the code for
 *  handling color has been thrown out. Input and output is alway
 *  stdin and stdout.
 *        -Brian Tierney, LBL  9/89
 *
 *   terminology:
 *       x = colums
 *       y = rows
 *
 * see additional comments in scale_g.c and pic.3
 *
 */

#include <math.h>

#include "hips.h"
#include "simple.h"
#include "pic.h"
#include "filt.h"
#include "scaleg.h"

#define FILTER_DEFAULT "triangle"
#define WINDOW_DEFAULT "blackman"

double    atof_check();

#include "usage.h"

    struct header hd;		/* global hips header */

main(ac, av)
    int       ac;
    char    **av;
{
    char     *xfiltname = FILTER_DEFAULT, *yfiltname = 0;
    char     *xwindowname = 0, *ywindowname = 0;
    int       xyflag, yxflag, doubleflag, tripleflag;
    int       nocoerce, nchan, square, intscale, keepzeros, i;
    double    xsupp = -1., ysupp = -1.;
    double    xblur = -1., yblur = -1.;
    Pic      *apic, *bpic;
    Window_box a;		/* src window */
    Window_box b;		/* dest window */

    Mapping   m;
    Filt     *xfilt, *yfilt, xf, yf;

    Progname = strsave(*av);
    a.x0 = a.y0 = a.x1 = a.y1 = a.nx = a.ny = 0;
    b.x0 = b.y0 = b.x1 = b.y1 = b.nx = b.ny = 0;
    m.sx = 0.;
    square = 0;
    intscale = 0;
    xyflag = 0;
    yxflag = 0;
    doubleflag = tripleflag = 0;
    nocoerce = 0;
    keepzeros = 0;

    for (i = 1; i < ac; i++)
	if (av[i][0] == '-')
	    if (str_eq(av[i], "-s") && ok(i + 4 < ac, "-s")) {
		a.x0 = atof_check(av[++i]);
		a.y0 = atof_check(av[++i]);
		a.nx = atof_check(av[++i]);
		a.ny = atof_check(av[++i]);
	    } else if (str_eq(av[i], "-d") && ok(i + 2 < ac, "-d")) {
		b.nx = atof_check(av[++i]);
		b.ny = atof_check(av[++i]);
	    } else if (str_eq(av[i], "-dc"))
		b.nx = atof_check(av[++i]);
	    else if (str_eq(av[i], "-dr"))
		b.ny = atof_check(av[++i]);
	    else if (str_eq(av[i], "-intscale"))
		intscale = 1;
	    else if (str_eq(av[i], "-filt") && ok(i + 1 < ac, "-filt")) {
		xfiltname = av[++i];
		if (i + 1 < ac && av[i + 1][0] != '-')
		    yfiltname = av[++i];
	    } else if (str_eq(av[i], "-supp") && ok(i + 1 < ac, "-supp")) {
		xsupp = atof_check(av[++i]);
		if (i + 1 < ac && av[i + 1][0] != '-')
		    ysupp = atof_check(av[++i]);
	    } else if (str_eq(av[i], "-blur") && ok(i + 1 < ac, "-blur")) {
		xblur = atof_check(av[++i]);
		if (i + 1 < ac && av[i + 1][0] != '-')
		    yblur = atof_check(av[++i]);
	    } else if (str_eq(av[i], "-window") && ok(i + 1 < ac, "-window")) {
		xwindowname = av[++i];
		if (i + 1 < ac && av[i + 1][0] != '-')
		    ywindowname = av[++i];
	    } else if (str_eq(av[i], "-debug") && ok(i + 1 < ac, "-debug"))
		zoom_debug = atof_check(av[++i]);
	    else if (str_eq(av[i], "-xy"))
		xyflag = 1;
	    else if (str_eq(av[i], "-yx"))
		yxflag = 1;
	    else if (str_eq(av[i], "-plain"))
		nocoerce = 1;
	    else if (str_eq(av[i], "-keep0"))
		keepzeros = 1;
	    else if (str_eq(av[i], "-dev")) {
		pic_catalog();
		exit(0);
	    } else if (str_eq(av[i], "-2")) {
		doubleflag = 1;
	    } else if (str_eq(av[i], "-3")) {
		tripleflag = 1;
	    } else if (str_eq(av[i], "-h")) {
		fputs(usage, stderr);
		exit(0);
	    } else {
		if (!str_eq(av[i], "-"))
		    fprintf(stderr, "unrecognized argument: %s\n", av[i]);
		fputs(usage, stderr);
		exit(1);
	    }

    if (str_eq(xfiltname, "?")) {
	filt_catalog();
	exit(0);
    }
    if (xyflag)
	zoom_xy = 1;
    if (yxflag)
	zoom_xy = 0;
    zoom_coerce = !nocoerce;
    zoom_trimzeros = !keepzeros;

    /* for hips files */
    apic = pic_open("stdin", "r");
    bpic = pic_open("stdout", "w");

    nchan = 1;			/* hips stuff is always 1 channel */
    /* check input image size */
    if (a.x0 > hd.ocols || a.y0 > hd.orows || a.nx > hd.ocols || a.ny > hd.orows) {
	fprintf(stderr, "\n Error: input image is not that big. \n\n");
	exit(-1);
    }
    /*
     * set defaults
     */
    if (a.nx == 0)
	a.nx = hd.ocols;
    if (a.ny == 0)
	a.ny = hd.orows;
    /* want square pixels, so scale x or y to the appr. value */
    if (b.nx == 0 && b.ny == 0) {
	b.nx = a.nx;
	b.ny = a.ny;
    }
    if (b.nx == 0)
	b.nx = (int) ((b.ny * a.nx) / (float) a.ny + .5);
    if (b.ny == 0)
	b.ny = (int) ((b.nx * a.ny) / (float) a.nx + .5);

    if (doubleflag) {
	b.nx = a.nx * 2;
	b.ny = a.ny * 2;
    }
    if (tripleflag) {
	b.nx = a.nx * 3;
	b.ny = a.ny * 3;
    }
    if (b.nx > XMAX || b.ny > YMAX) {
	fprintf(stderr, "\nError: destination file must be smaller than %d by %d \n\n",
		XMAX, YMAX);
	exit(-1);
    }

    hd.ocols = hd.cols = b.nx;
    hd.orows = hd.rows = b.ny;		/* must do this after call to zoom */
    update_header(&hd, ac, av);
    write_header(&hd);

    /* sets x1 and y1 values */
    window_box_set_max(&a);
    window_box_set_max(&b);

    if (!yfiltname)
	yfiltname = xfiltname;
    xfilt = filt_find(xfiltname);
    yfilt = filt_find(yfiltname);
    if (!xfilt || !yfilt) {
	fprintf(stderr, "can't find filters %s and %s\n",
		xfiltname, yfiltname);
	exit(1);
    }
    /* copy the filters before modifying them */
    xf = *xfilt;
    xfilt = &xf;
    yf = *yfilt;
    yfilt = &yf;
    if (xsupp >= 0.)
	xfilt->supp = xsupp;
    if (xsupp >= 0. && ysupp < 0.)
	ysupp = xsupp;
    if (ysupp >= 0.)
	yfilt->supp = ysupp;
    if (xblur >= 0.)
	xfilt->blur = xblur;
    if (xblur >= 0. && yblur < 0.)
	yblur = xblur;
    if (yblur >= 0.)
	yfilt->blur = yblur;

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
	fprintf(stderr, "xfilt: ");
	filt_print_client(xfilt);
    }
    if (yfilt->printproc) {
	fprintf(stderr, "yfilt: ");
	filt_print_client(yfilt);
    }
    /* process each of the frames */
    for (i = 0; i < hd.num_frame; i++) {
	if (hd.num_frame > 1)
	    fprintf(stderr, "\n processsing frame %d...", i);
	load_hips_frame(apic->data);
	zoom_opt(apic, &a, bpic, &b, xfilt, yfilt, square, intscale);
	store_hips_frame(bpic->data);
    }
    fprintf(stderr, "\n\n");
    return (0);
}

/************************************************/
ok(enough, option)
    int       enough;
    char     *option;
{
    if (!enough) {
	fprintf(stderr, "insufficient args to %s\n", option);
	exit(1);
    }
    return 1;
}

/************************************************/
/* atof_check: ascii to float conversion with checking */

double
atof_check(str)
    char     *str;
{
    char     *s;

    for (s = str; *s; s++)
	if (strchr("0123456789.+-eE", *s) == NULL) {
	    fprintf(stderr, "expected numeric argument, not %s\n", str);
	    exit(1);
	}
    return atof(str);
}
