/*	Copyright (c) 1982 Michael Landy, Yoav Cohen, and George Sperling

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 *  histoeq.c - Histogram equalization
 *
 *  Usage: histoeq [-z][-v] <  inseq > outseq
 *     [-z]  ignore zero valued pixels
 *     [-v]  verbose mode
 *
 *  to load: cc -o histoeq histoeq.c -lhips
 *
 */

/* modified by Brian Tierney, Lawrence Berkeley Laboratory, to handle
   short and int images.  ( 8/90)
 * Note: output of short and int data is scaled to 0 to NUM_BINS. So this
 * program doesn't truely properly histogram equalize short and int images,
 * but it does preserve more contrast than 255 gray levels, and runs
 * reasonably fast.
 */

#define NUM_BINS 1500

#include <hipl_format.h>
#include <stdio.h>
#include <sys/types.h>

int      *hist, *new, numbin, zcount = 0;
float     max, min, delta;

u_char   *image;
u_short  *sh_image;
u_int    *int_image;

/* global command line arguments */
int       zflag, verbose;

int main(argc, argv)
    int       argc;
    char    **argv;
{
    struct header hd;
    int       z, r, idx;
    int       rw, cl, nf, npix, i, fr;
    int       incr, zflag = 0;
    float     havg, hint;
    void      parse_args(), hist_byte(), hist_short(), hist_int();

    Progname = strsave(*argv);
    parse_args(argc, argv);

    read_header(&hd);
    if (hd.pixel_format != PFBYTE &&
	hd.pixel_format != PFSHORT &&
	hd.pixel_format != PFINT)
	perr(HE_MSG,"image must be in byte, short, or int format");

    update_header(&hd, argc, argv);
    write_header(&hd);
    rw = hd.orows;
    cl = hd.ocols;
    nf = hd.num_frame;
    npix = rw * cl;

    if (verbose)
	fprintf(stderr, "\n rows: %d,  cols: %d,  frames: %d \n", rw, cl, nf);

    if (hd.pixel_format == PFBYTE)
	numbin = 256;		/* default 256 gray level */
    else
	numbin = NUM_BINS;

    hist = (int *) halloc(numbin, sizeof(int));
    new = (int *) halloc(numbin, sizeof(int));

    if (hd.pixel_format == PFBYTE)
	image = (u_char *) halloc(npix, sizeof(u_char));
    else if (hd.pixel_format == PFSHORT)
	sh_image = (u_short *) halloc(npix, sizeof(u_short));
    else
	int_image = (u_int *) halloc(npix, sizeof(u_int));

    /* calculate the average number of pixels per bin */

    /* for each frame do histogram equalization */

    for (fr = 0; fr < nf; fr++) {
	if (nf > 1)
	    fprintf(stderr, "\n processing frame: %d ", fr);

	zcount = 0;

	/* read frame */
	if (hd.pixel_format == PFBYTE) {
	    if (fread(image, npix * sizeof(u_char),1,stdin) != 1)
		perr(HE_MSG,"error during read");
	} else if (hd.pixel_format == PFSHORT) {
	    if (fread(sh_image, npix * sizeof(u_short),1,stdin) != 1)
		perr(HE_MSG,"error during read");
	} else {
	    if (fread(int_image, npix * sizeof(u_int),1,stdin) != 1)
		perr(HE_MSG,"error during read");
	}

	/* calculate the histogram */

	if (verbose)
	    fprintf(stderr, " computing histogram... \n");

	if (hd.pixel_format == PFBYTE)
	    hist_byte(npix);
	else if (hd.pixel_format == PFSHORT)
	    hist_short(npix);
	else
	    hist_int(npix);


	/* Re-calculate the average number of pixels per bin */

	if (zflag)
	    havg = (float) (npix - zcount) / numbin;
	else
	    havg = (float) npix / numbin;

	if (verbose)
	    fprintf(stderr, " equalizing histogram... \n");

	r = 0;
	hint = 0;
	for (z = 0; z < numbin; z++) {
	    hint += hist[z];
	    incr = (int) hint / havg;
	    new[z] = r + (incr / 2);
	    r += incr;
	    hint -= incr * havg;
	}

	if (verbose)
	    fprintf(stderr, " remapping pixels... \n");

	/* histogram equalization */
	for (i = 0; i < npix; i++) {
	    if (hd.pixel_format == PFBYTE)
		image[i] = (u_char) new[image[i]];
	    else if (hd.pixel_format == PFSHORT) {
		idx = (int) (((sh_image[i] - min) / delta) + .5);
		if (idx < 0)
		    idx = 0;
		if (idx >= numbin)
		    idx = numbin - 1;
		sh_image[i] = (u_short) new[idx];
	    } else {
		idx = (int) (((int_image[i] - min) / delta) + .5);
		if (idx < 0)
		    idx = 0;
		if (idx >= numbin)
		    idx = numbin - 1;
		int_image[i] = (u_int) new[idx];
	    }
	}

	/* write new pixels */
	if (hd.pixel_format == PFBYTE) {
	    if (fwrite(image, npix * sizeof(u_char),1,stdout) != 1)
		perr(HE_MSG,"error during write");
	} else if (hd.pixel_format == PFSHORT) {
	    if (fwrite(sh_image, npix * sizeof(u_short),1,stdout) != 1)
		perr(HE_MSG,"error during write");
	} else {
	    if (fwrite(int_image, npix * sizeof(u_int),1,stdout) != 1)
		perr(HE_MSG,"error during write");
	}
    }

    return(0);
}

/**************************************************************/
void
hist_byte(size)
    int       size;
{
    int       i, idx;

    for (i = 0; i < numbin; i++)
	hist[i] = new[i] = 0;

    for (i = 0; i < size; i++) {
	idx = (int) image[i];
	if (zflag && (idx == 0))
	    zcount++;
	else
	    hist[idx]++;
    }
}

/**********************************************************************/
void
hist_short(size)
    int       size;
{
    int       i, idx;

    for (i = 0; i < numbin; i++)
	hist[i] = new[i] = 0;

    min = max = 0.;

    for (i = 0; i < size; i++) {
	if (sh_image[i] < min)
	    min = (float) sh_image[i];
	if (sh_image[i] > max)
	    max = (float) sh_image[i];
    }
    if (verbose)
	fprintf(stderr, " image min value: %d, image max value: %d \n",
		(int) min, (int) max);

    if (max < numbin)
	numbin = max + 1;

    delta = (max - min) / numbin;
    if (delta <= 0.)
	delta = .1;

    for (i = 0; i < size; i++) {
	idx = (int) (((sh_image[i] - min) / delta) + .5);
	if (zflag && (idx == 0))
	    zcount++;
	else {
	    if (idx < 0)
		idx = 0;
	    if (idx >= numbin)
		idx = numbin - 1;
	    hist[idx]++;
	}
    }
}

/*******************************************************************/
void
hist_int(size)
    int       size;
{
    int       i, idx;

    for (i = 0; i < numbin; i++)
	hist[i] = new[i] = 0;

    min = max = 0.;

    for (i = 0; i < size; i++) {
	if (int_image[i] < min)
	    min = (float) int_image[i];
	if (int_image[i] > max)
	    max = (float) int_image[i];
    }
    if (verbose)
	fprintf(stderr, " image min value: %d, image max value: %d \n",
		(int) min, (int) max);
    if (max < numbin)
	numbin = max + 1;

    delta = (max - min) / numbin;
    if (delta <= 0.)
	delta = .1;

    for (i = 0; i < size; i++) {
	idx = (int) (((int_image[i] - min) / delta) + .5);
	if (zflag && (idx == 0))
	    zcount++;
	else {
	    if (idx < 0)
		idx = 0;
	    if (idx >= numbin)
		idx = numbin - 1;
	    hist[idx]++;
	}
    }
}

/****************************************************************/
void
parse_args(argc, argv)
    int       argc;
    char     *argv[];
{
    void      usageterm();

    zflag = verbose = 0;	/* defaults = off */

    /* Interpret options  */
    while (--argc > 0 && (*++argv)[0] == '-') {
	char     *s;
	for (s = argv[0] + 1; *s; s++)
	    switch (*s) {
	    case 'z':
		zflag++;
		break;
	    case 'v':
		verbose++;
		break;
	    case 'h':
		usageterm();
		break;
	    default:
		usageterm();
		break;
	    }
    }				/* while */
}

/******************************************************/
void
usageterm()
{
    fprintf(stderr, "Usage: histoeq [-z][-v] <  inseq > outseq  \n");
    fprintf(stderr, "   [-z]  ignore zero valued pixels \n");
    fprintf(stderr, "   [-v]  verbose mode \n\n");
    exit(0);
}
