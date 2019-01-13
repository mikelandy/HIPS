/*
 * hfant.c - Perform spacial transforms on images. (should be "remap")
 *
 * Author:	John W. Peterson
 * 		Computer Science Dept.
 * 		University of Utah
 * Date:	Wed Jun 25 1986
 * Copyright (c) 1986 John W. Peterson
 *
 *  this version for use with hips files:  Brian Tierney;  LBL
 *    input and output arrays changed to be 2-d arrays.
 *
 */

/*
 * This program performs spatial transforms on images.  For full
 * details, consult the paper:
 *      Fant, Karl M. "A Nonaliasing, Real-Time, Spatial Transform
 *                     Technique", IEEE CG&A, January 1986, p. 71
 *
 */

#include <stdio.h>
#include <math.h>
#include <hipl_format.h>

#define PI 3.141592
/* #define DEBUG  */

#define and &&
#define or ||			/* C readability */

#define Calloc(a,b) (b *) calloc((unsigned)(a), sizeof(b))
#define Fread(a,b,c,d) fread((char *)(a), b, (int)(c), d)
#define Fwrite(a,b,c,d) fwrite((char *)(a), b, (int)(c), d)

#define H_PASS 0
#define V_PASS 1

/* Conversion macros */

#define FRAC(x) ((x) - ((int) (x)))
#define ROUND(x) ((int)((x) + 0.5))
#define DEG(x) ((x) * PI / 180.0)


#define MIN(x,y) ((x) < (y) ? (x) : (y))
#define MAX(x,y) ((x) > (y) ? (x) : (y))

typedef unsigned char u_char;

typedef struct point {
    float     x, y;
}         point;

typedef struct glob_stuff {
    int       xmin, xmax;
    int       ymin, ymax;
}         gl;

u_char  **in_rast;
u_char  **out_rast;

int       const_ind;		/* Constant index */
int       pass;			/* Which pass we're on (h or v) */
gl        inglob, outglob;
int       array_width;
int       outlinewidth, array_lines;
int       X_origin, Y_origin;
int       vpassonlyflag;	/* If true, we only need the vertical pass */

float     xscale, yscale, angle;
int       scaleflag, angleflag;
int       verboseflag;		/* Be chatty (Fant can be slow) */
int       originflag;		/* Center picture on given orig instead of
				 * center */


main(argc, argv)
    int       argc;
    char     *argv[];
{
    int       i;
    point     p[5];		/* "5" so we can use 1-4 indices Fant does. */
    struct header hd;		/* hips header */

    xscale = 1.0;
    yscale = 1.0;
    scaleflag = 0;
    angleflag = 0;
    angle = 0.0;
    verboseflag = 0;		/* set to off by default */
    originflag = 0;
    vpassonlyflag = 0;

    Progname = strsave(*argv);
    parse_args(argc, argv);

    angle = angle * (-1.);	/* to make default clock-wise rotation */
    if (fabs(angle) > 45.0)
	fprintf(stderr,
	"hfant: Warning: angle larger than 45 degrees, image will blur.\n");

    if ((xscale == 1.0) and(angle == 0.0)) {
	vpassonlyflag = 1;
	if (verboseflag)
	    fprintf(stderr, "hfant: Only performing vertical pass\n");
    }
    /* read hips header */
    read_header(&hd);
    if (hd.pixel_format != PFBYTE) {
	fprintf(stderr, "hfant: pixel format must be byte\n");
	exit(1);
    }
    inglob.xmin = inglob.ymin = 0;
    inglob.xmax = hd.ocols - 1;	/* index val, not size */
    inglob.ymax = hd.orows - 1;

    outglob = inglob;

    /*
     * To define the output rectangle, we start with a set of points defined
     * by the original image, and then rotate and scale them as desired.
     */
    p[1].x = inglob.xmin;
    p[1].y = inglob.ymin;
    p[2].x = inglob.xmax;
    p[2].y = inglob.ymin;
    p[3].x = inglob.xmax;
    p[3].y = inglob.ymax;
    p[4].x = inglob.xmin;
    p[4].y = inglob.ymax;

    xform_points(p, xscale, yscale, angle);

    /* Make sure the output image is large enough */

    outglob.xmin = MAX(0, MIN((int) p[1].x, (int) p[4].x));
    outglob.ymin = MAX(0, MIN((int) p[1].y, (int) p[2].y));

    outglob.xmax = MAX(ROUND(p[2].x), ROUND(p[3].x));
    outglob.ymax = MAX(ROUND(p[3].y), ROUND(p[4].y));

    /*
     * Need to grab the largest dimensions so the buffers will hold the
     * picture.  The arrays for storing the pictures extend from 0 to
     * outglob.xmax in width and from outglob.ymin to ymax in height.  The
     * reason X goes from 0 to xmax is because rle_getrow returns the entire
     * row when the image is read in.
     */
    array_width = MAX(outglob.xmax, inglob.xmax) + 1;
    array_lines = MAX(outglob.ymax - outglob.ymin,
		      inglob.ymax - inglob.ymin) + 1;
    outlinewidth = outglob.xmax - outglob.xmin + 1;

    hd.cols = hd.ocols = outglob.xmax + 1;
    hd.rows = hd.orows = outglob.ymax + 1;
    update_header(&hd, argc, argv);	/* hips header */
    write_header(&hd);

    /*
     * Since the array begins at ymin, the four output corner points must be
     * translated to this coordinate system.
     */
    for (i = 1; i <= 4; i++)
	p[i].y -= MIN(inglob.ymin, outglob.ymin);

    alloc_arrays();

    for (i = 0; i < hd.num_frame; i++) {	/* process each frame */
	if (hd.num_frame > 1)
	    fprintf(stderr, "\n processing frame %d...", i);
	clear_raster(in_rast);
	getraster(in_rast);
#ifdef DEBUG
	fprintf(stderr, "input image: ");
	dumpraster(in_rast);
#endif

	xform_image(p);

#ifdef DEBUG
	fprintf(stderr, "output image: ");
	dumpraster(out_rast);
#endif
	putraster(out_rast);
    }				/* for each frame  */

    exit(0);
}

/*
 * This transforms the image.
 * The result image is based on the points p, using linear
 * interpolation per scanline.
 */
xform_image(p)
    point    *p;
{
    float     real_outpos, sizefac, delta;
    u_char  **tmprast;
    int       ystart, yend;
    int       xinlen, yinlen;

    xinlen = inglob.xmax - inglob.xmin + 1;
    yinlen = inglob.ymax - inglob.ymin + 1;

    /* Vertical pass */
    if (verboseflag)
	fprintf(stderr, "\n performing vertical pass...");
    pass = V_PASS;
    clear_raster(out_rast);

    real_outpos = p[1].y;

    sizefac = (p[4].y - p[1].y) / (yinlen - 1);
    delta = (p[2].y - p[1].y) / xinlen;

    for (const_ind = inglob.xmin; const_ind <= inglob.xmax; const_ind++) {
	interp_row(sizefac, 0, real_outpos,
		   inglob.ymax - inglob.ymin + 1,
		   array_lines - 1);
	real_outpos += delta;
    }

    if (!vpassonlyflag) {
	/* Flip buffers */
	tmprast = in_rast;
	in_rast = out_rast;
	out_rast = tmprast;

	/* Horizontal pass */
	if (verboseflag)
	    fprintf(stderr, "\n performing horizontal pass...");
	pass = H_PASS;
	clear_raster(out_rast);

	real_outpos = (((p[2].y - p[4].y) * (p[1].x - p[4].x))
		       / (p[1].y - p[4].y)) + p[4].x;
	sizefac = (p[2].x - real_outpos) / (xinlen - 1);
	delta = (p[4].x - real_outpos)
	    / ((float) ((int) p[4].y) - ((int) p[2].y) + 1.0);

	/* If we're moving backwards, start at p1 (needs more thought...) */
	if (delta < 0)
	    real_outpos = p[1].x;

	ystart = MIN((int) p[2].y, (int) p[1].y);
	yend = MAX((int) p[4].y, (int) p[3].y);

	if (ystart < 0) {	/* Ensure start isn't negative */
	    real_outpos += delta * abs(ystart);
	    ystart = 0;
	}
	for (const_ind = ystart; const_ind < yend; const_ind++) {
	    interp_row(sizefac, inglob.xmin, real_outpos,
		       inglob.xmax,
		       outglob.xmax);
	    real_outpos += delta;
	}
    }
}

/**************************************************/
/*
 * Transform the points p according to xscale, yscale and angle.
 * Rotation is done first, this allows the separate scaling factors to
 * be used to adjust aspect ratios.  Note the image quality of the
 * resulting transform degrades sharply if the angle is > 45 degrees.
 */
xform_points(p, xscal, yscal, ang)
    point    *p;
    float     xscal, yscal, ang;
{
    float     s, c, xoff, yoff;
    float     tmp;
    int       i;

    /* Sleazy - should build real matrix */

    c = cos(DEG(ang));
    s = sin(DEG(ang));
    if (!originflag) {
	xoff = ((float) (inglob.xmax - inglob.xmin) / 2.0);
	yoff = ((float) (inglob.ymax - inglob.ymin) / 2.0);
    } else {
	xoff = X_origin;
	yoff = Y_origin;
    }
    if (verboseflag)
	fprintf(stderr, "Output rectangle:\n");

    for (i = 1; i <= 4; i++) {
	p[i].x -= xoff;		/* translate to origin */
	p[i].y -= yoff;

	tmp = p[i].x * c + p[i].y * s;	/* Rotate... */
	p[i].y = -p[i].x * s + p[i].y * c;
	p[i].x = tmp;

	p[i].x *= xscal;	/* Scale */
	p[i].y *= yscal;

	p[i].x += (xoff);	/* translate back from origin */
	p[i].y += (yoff);

	if (verboseflag)
	    fprintf(stderr, "  %4.1f\t%4.1f\n", p[i].x, p[i].y);
    }
}

/*
 * Interpolate a row or column pixels.  This is a straightforward
 * (floating point) implementation of the paper in the algorithm.
 * Sizefac is the amount the row is scaled by, ras_strt is the
 * position to start in the input raster, and real_outpos is the place
 * (in floating point coordinates) to output pixels.  The algorithm
 * loops until it's read pixels up to inmax or output up to outmax.
 */
interp_row(sizefac, ras_strt, real_outpos, inmax, outmax)
    float     sizefac;
    int       ras_strt;
    float     real_outpos;
    int       inmax, outmax;
{
    float     inoff, inseg, outseg, accum, insfac, pv;
    int       inptr, outptr;
    float     getpxl();

    insfac = 1.0 / sizefac;

    if (real_outpos > 0.0) {
	inseg = 1.0;
	outseg = insfac * (1.0 - FRAC(real_outpos));
	outptr = (int) real_outpos;
	inptr = ras_strt;
    } else {			/* Must clip */
	inoff = -real_outpos * insfac;
	inseg = 1.0 - FRAC(inoff);
	outseg = insfac;
	inptr = ras_strt + ((int) inoff);
	outptr = 0;
    }
    accum = 0.0;

    while ((inptr < inmax) and(outptr < outmax)) {
	pv = getpxl(inptr, inseg);

	if (outseg > inseg) {
	    accum += pv * inseg;
	    outseg -= inseg;
	    inseg = 1.0;
	    inptr++;
	} else {
	    accum += pv * outseg;
	    inseg -= outseg;
	    outseg = insfac;
	    putpxl(outptr, accum * sizefac);
	    outptr++;
	    accum = 0.0;
	}
    }

    /* If we ran out of input, output partially completed pixel */
    if (outptr <= outmax)
	putpxl(outptr, accum * sizefac);
}

/**************************************************/
/*
 * Grab inseg's worth of pixels from the current input raster.  If inseg
 * is less than one, we perform linear interpolation (Fant's "expansion
 * smoothing") between adjacent pixels.
 *
 */
float
getpxl(index, inseg)
    int       index;
    float     inseg;
{
    float     result;
    float     find1, find2;

    if (pass == V_PASS) {
	find1 = (float) in_rast[index][const_ind];
	find2 = (float) in_rast[index + 1][const_ind];
	if (index < inglob.ymax)
	    result = inseg * find1 + (1.0 - inseg) * find2;
	else
	    result = inseg * find1;
    } else {
	find1 = (float) in_rast[const_ind][index];
	find2 = (float) in_rast[const_ind][index + 1];
	if (index < inglob.xmax)
	    result = inseg * find1 + (1.0 - inseg) * find2;
	else
	    result = inseg * find1;
    }
    return (result);
}

/**************************************************/
/*
 * Write a pixel into the current output channel.
 */
putpxl(index, pxlval)
    int       index;
    float     pxlval;
{
    pxlval = ROUND(pxlval);
    if (pxlval > 255) {
	fprintf(stderr, "pixel overflow: %f at %d %d\n",
		pxlval, ((pass == V_PASS) ? const_ind : index),
		((pass == V_PASS) ? index : const_ind));
	pxlval = 255;
    }
    if (pass == V_PASS)
	out_rast[index][const_ind] = (int) pxlval;
    else
	out_rast[const_ind][index] = (int) pxlval;

}

/**************************************************/
getraster(ras_ptr)
    u_char  **ras_ptr;
{
    /*
     * Note: input image is set to be the same size as the output image, and
     * the lines are padded with zeros
     */

    int       len, i;

    len = (inglob.xmax + 1) * sizeof(u_char);
    for (i = 0; i <= inglob.ymax; i++)
	if (fread(ras_ptr[i], len,1,stdin) != 1)
	    perr(HE_MSG,"error during read");

}

/**************************************************/
/* Write out the image */
putraster(ras_ptr)
    u_char  **ras_ptr;
{
    int       len, i;

/*
    len = ((outglob.xmax+1) * (outglob.ymax+1)) * sizeof(u_char);
    if (fwrite(ras_ptr[0], len,1,stdout) != 1)
	perr(HE_MSG,"error during write");
*/

    len = (outglob.xmax + 1) * sizeof(u_char);
    for (i = 0; i <= outglob.ymax; i++)
	if (fwrite(ras_ptr[i], len,1,stdout) != 1)
	    perr(HE_MSG,"error during write");

}

/**************************************************/
/* Clear the raster image */
clear_raster(ras_ptr)
    u_char  **ras_ptr;
{
    bzero(ras_ptr[0], array_width * array_lines);
}

/**************************************************/
alloc_arrays()
{
    int       i, xsize, ysize;
    /* make both arrays the size of the output array */

    xsize = array_width;
    ysize = array_lines;

    if ((in_rast = Calloc(ysize, u_char *)) == NULL)
	perror("calloc error: in_rast ");
    if ((in_rast[0] = Calloc(xsize * ysize, u_char)) == NULL)
	perror("calloc error: in_rast[] ");

    for (i = 1; i < ysize; i++)
	in_rast[i] = *in_rast + xsize * i;

    if ((out_rast = Calloc(ysize, u_char *)) == NULL)
	perror("calloc error: out_rast ");
    if ((out_rast[0] = Calloc(ysize * xsize, u_char)) == NULL)
	perror("calloc error: out_rast[] ");

    for (i = 1; i < ysize; i++)
	out_rast[i] = *out_rast + xsize * i;

}

/**************************************************/
/*
 * Dump out a raster (used for debugging).
 */
dumpraster(ras_ptrs)
    u_char  **ras_ptrs;
{
    int       j, i;
    for (i = 0; i < array_lines; i++) {
	for (j = 0; j < array_width; j++) {
	    fprintf(stderr, "%2x", ras_ptrs[i][j]);
	    fprintf(stderr, " ");
	}
	fprintf(stderr, "\n");
    }
}

/****************************************************************/
parse_args(argc, argv)
    int       argc;
    char     *argv[];
{
    /*
     * sets the following vars: scaleflag, xscale, yscale, angleflag, angle,
     * verboseflag, originflag, X_origin, Y_origin
     */


    /* Interpret options  */
    while (--argc > 0 && (*++argv)[0] == '-') {
	char     *s;
	for (s = argv[0] + 1; *s; s++)
	    switch (*s) {
	    case 's':
		if (argc < 3)
		    usageterm();
		sscanf(*++argv, "%f", &xscale);
		sscanf(*++argv, "%f", &yscale);
		scaleflag++;
		argc -= 2;
		break;
	    case 'v':
		verboseflag++;
		break;
	    case 'a':
		if (argc < 2)
		    usageterm();
		sscanf(*++argv, "%f %f", &angle);
		angleflag++;
		argc--;
		break;
	    case 'o':
		if (argc < 3)
		    usageterm();
		sscanf(*++argv, "%d", &X_origin);
		sscanf(*++argv, "%d", &Y_origin);
		originflag++;
		argc -= 2;
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

usageterm()
{

    fprintf(stderr, "Usage: hfant [-s xscale yscale] [-v] [-a angle] [-o xoff yoff ] \n");
    fprintf(stderr, " Options: [-s rr rr]  Scale the image by this amount \n");
    fprintf(stderr, "          [-v]  verbose mode \n");
    fprintf(stderr, "          [-a rr] angle to rotate image (clockwise) \n");
    fprintf(stderr, "          [-o rr rr] image rotated around this point \n\n");
    exit(0);
}
