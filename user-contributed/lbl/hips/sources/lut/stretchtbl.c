/*	Copyright (c) 1989 Michael Landy

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 * stretchtbl.c - stretch or compress the range of gray-levels
 *		  in a byte-formatted frame.
 *
 * usage: stretchtbl [ exp1 [ boundary [ exp2 ]]] < oldframe > tbl_name
 *
 * defaults:  exp1:  2., boundary: .5, exp2: 1/exp1.
 *
 * to load: cc -o stretchtbl stretchtbl.c -lhips -lm
 *
 * Yoav Cohen 2/19/82
 */

/*
 * exponentiation bug fixed - WEJohnston 9/89
 *
 *  added support for short images and
 *  modified to use look-up table for byte and short images:
 *     Brian Tierney, LBL 10/90
 */

#include <hipl_format.h>
#include <math.h>
#include <stdio.h>

unsigned char *pic;
short    *spic;

#define MAXSHORT 32768
#define UNDEFFLOAT -1.0

/* scaling values */
double    midpt = UNDEFFLOAT;
double    expt1, expt2;
short     mval;			/* max value in a short image */
int       midfract = 0;

main(argc, argv)
    int       argc;
    char    **argv;

{
    struct header hd;
    int       rc, form, maxmid;

    Progname = strsave(*argv);
    read_header(&hd);
    form = hd.pixel_format;
    if (argv[argc - 1][0] == '-')
	argc--;
    expt1 = 2.;
    if (argc > 1)
	expt1 = atof(argv[1]);
    if (argc > 2) {
	midpt = atof(argv[2]);
	if (midpt < 1.0)
	    midfract = 1;
	if (form == PFBYTE)
	    maxmid = 255;
	else
	    maxmid = MAXSHORT;
	if (midpt <= 0 || midpt >= maxmid)
	    perr(HE_MSG, "boundary must be between 0 and 255");
    }
    fprintf(stderr, "here\n");
    expt2 = 1. / expt1;
    fprintf(stderr, "we\n");
    if (argc > 3)
	expt2 = atof(argv[3]);
    if (midpt == UNDEFFLOAT) {
	midfract = 1;
	midpt = .5;
    }
    rc = hd.orows * hd.ocols;
    if (form == PFBYTE) {
	pic = (unsigned char *) halloc(rc, sizeof(unsigned char));
	if (fread(pic, rc * sizeof(unsigned char), 1, stdin) != 1 )
	    perr(HE_MSG, "error during read");
	get_maxb(rc);
	printf("%d\n", mval);
	printf("%d\n", form);
	fprintf(stderr, "go\n");
	make_byte_lut();
    } else if (form == PFSHORT) {
	spic = (short *) halloc(rc, sizeof(short));
	if (fread(spic, rc * sizeof(unsigned short), 1, stdin) != 1 )
	    perr(HE_MSG, "error during read");
	get_maxs(rc);
	printf("%d\n", mval);
	form = PFSHORT;
	printf("%d\n", form);
	make_short_lut();
    } else
	perr(HE_MSG, "input format must be byte or short format");
    return (0);
}

make_byte_lut()
{
    int       i;
    double    newpt, s1, s2, dtmp, dstmp;
    unsigned char entry;

    if (midfract)
	midpt = midpt * mval;
    fprintf(stderr, "stretchtbl: midpoint = %f\n", midpt);
    s1 = pow(midpt, (1. - expt1));
    s2 = pow((255. - midpt), 1. - expt2);

    for (i = 1; i < 256; i++) {
	dtmp = i;
	if (dtmp == 0.)
	    newpt = 0.;
	else if (dtmp <= midpt)
	    newpt = s1 * pow(dtmp, expt1);
	else {
	    dstmp = dtmp - midpt;
	    if (dstmp < 0)
		newpt = midpt;
	    else
		newpt = midpt + pow(dstmp, expt2) * s2;
	}
	entry = (unsigned char) (newpt + 0.5);
	printf("%d\n", entry);
    }
}

get_maxb(size)
    int       size;
{
    register int i;
    unsigned char *ppic;

    mval = 0;
    ppic = pic;
    for (i = 0; i < size; i++) {
	if (*ppic > mval)
	    mval = *ppic;
	ppic++;
    }
    fprintf(stderr, "Maximum value is: %d \n", mval);
}

get_maxs(size)
    int       size;
{
    register int i;
    short    *pspic;

    mval = 0;
    pspic = spic;
    for (i = 0; i < size; i++) {
	if (*pspic > mval)
	    mval = *pspic;
	pspic++;
    }
    fprintf(stderr, "Maximum value is: %d \n", mval);
}

make_short_lut()
{
    int       i;
    double    newpt, endpt_scale1, endpt_scale2, dtmp, dstmp;
    short     entry;

    if (midfract)
	midpt = midpt * mval;
    fprintf(stderr, "stretchtbl: midpoint = %f\n", midpt);
    endpt_scale1 = pow(midpt, (1. - expt1));
    endpt_scale2 = pow(((float) MAXSHORT - midpt), 1. - expt2);

    for (i = 1; i <= mval; i++) {
	dtmp = i;
	if (dtmp == 0.)
	    newpt = 0.;
	else if (dtmp <= midpt)
	    newpt = endpt_scale1 * pow(dtmp, expt1);
	else {
	    dstmp = dtmp - midpt;
	    newpt = endpt_scale2 * pow(dstmp, expt2) + midpt;
	}
	entry = (short) (newpt + 0.5);
	if (entry < 0)
	    entry = 0;
	printf("%d\n", entry);
    }
}
