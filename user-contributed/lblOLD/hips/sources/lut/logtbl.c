/*	Copyright (c) 1989 Michael Landy

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 * logtbl - takes log of input image.  (Output pixel = log (ipix + 1).)
 * Input image is in byte or short format,
 * output image byte for byte, floating point for short.
 *
 * usage:	logtbl [-a] < iseq > tbl_name
 *			-a parameter indicates op = log (ipix),
 *				with log(0) = -1.
 *
 * to load:	cc -o logtbl logtbl.c -lhips -lm
 *
 * Mike Landy - 5/10/82
 * Mike Landy - 5/17/85 - added float input
 * Charles Carman - 12/11/87 - added short input and -a flag
 */

#include <stdio.h>
#include <hipl_format.h>
#include <math.h>

#define MAXSHORT	32768

main(argc, argv)
    int       argc;
    char     *argv[];
{
    int       aflag = 0;
    int       i, pixform, numentries;
    double    num;
    float     entry;
    struct header hd;

    Progname = strsave(*argv);
    read_header(&hd);
    pixform = hd.pixel_format;
    if (argc == 2) {
	if (*argv[1] == '-' && argv[1][1] == 'a')
	    aflag++;
	else
	    perr(HE_MSG, "unknown argument");
    }
    if (pixform == PFBYTE)
	numentries = 256;
    else if (pixform == PFSHORT)
	numentries = MAXSHORT;
    else
	perr(HE_MSG, "pixel format must be byte or short");

    pixform = PFFLOAT;
    printf("%d\n", numentries);
    printf("%d\n", pixform);
    if (!aflag)
	printf("-1.0\n");

    for (i = 1 - aflag; i < numentries; i++) {
	if (i < 255)
	    num = (double) (aflag + i & 0xff);
	else
	    num = (double) (aflag + i);
	entry = (float) (log(num));
	printf("%f\n", entry);
    }
    return (0);
}
