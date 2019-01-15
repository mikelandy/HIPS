/*	Copyright (c) 1989 Michael Landy

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 * powertbl.c - raise to a power and normalize each pixel of a frame.
 *
 * usage: powertbl [power] < frame > table_name
 *
 * default power:  .5
 *
 * For byte images, pixels are renormalized to lie between 0 and 255.  For
 * short, integer, and float images, the output is a float image and no
 * renormalization is performed.
 *
 * to load: cc -o powertbl powertbl.c -lhips -lm
 *
 * Yoav Cohen 2/16/82
 * added int/float - Mike Landy - 3/16/89
 *
 *  modified to use look-up table for byte and short images:
 *     Brian Tierney, LBL 10/90
 *
 *  modified to generate look-up table for any type of image, but not
 *  apply the table to the image.  Therefore, this is to be used in
 *  conjunction with mapapply to generate an image:
 *     Bryan Skene, LBL 1/29/91
 *
 */

#include <hipl_format.h>
#include <math.h>
#include <stdio.h>


#define MAXSHORT  32768

main(argc, argv)
    int       argc;
    char     *argv[];

{
    double    power;
    struct header hd;
    int       form;

    Progname = strsave(*argv);
    if (argc == 1 || argv[1][0] == '-')
	power = .5;
    else
	power = atof(argv[1]);
    read_header(&hd);
    form = hd.pixel_format;
    switch (form) {
    case PFBYTE:
	printf("256\n");
	printf("%d\n", form);
	make_byte_lut(power);
	break;
    case PFSHORT:
	printf("32768\n");
	form = PFFLOAT;
	printf("%d\n", form);
	make_short_lut(power);
	break;
    default:
	perr(HE_MSG, "input format must be byte or short");
    }
    return (0);
}

make_byte_lut(power)
    double    power;

{
    int       i;
    unsigned char entry;

    for (i = 0; i < 256; i++) {	/* re-normalize to 0 to 255 */
	entry = (unsigned char)
	    (255. * (pow((double) (i & 0377) / 255., power)) + 0.5);
	printf("%d\n", entry);
    }
}

make_short_lut(power)
    double    power;

{
    int       i;
    float     entry;

    for (i = 0; i < MAXSHORT; i++) {
	entry = (float) (pow((double) i, power));
	printf("%f\n", entry);
    }
}
