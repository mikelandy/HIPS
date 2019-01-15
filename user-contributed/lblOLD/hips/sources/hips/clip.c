/*	Copyright (c) 1982 Michael Landy, Yoav Cohen, and George Sperling

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/* clip.c - limit an image to a range of grey vlaues
 *		  in a byte-formatted frame.
 *
 * usage: clip [low_limit high_limit [new_low_val [new_high_val]] <oldframe >newframe
 *
 * defaults:  low_limit=0 high_limit=255 [new_low_val [new_high_val]]
 *
 * to load: cc -o clip clip.c -lhipl -lm
 *
 * Yoav Cohen 2/19/82
 */

#include <hipl_format.h>
#include <math.h>
#include <stdio.h>

main(argc, argv)
    int       argc;
    char     *argv[];
{
    int       f, rc, i;
    struct header hd;
    unsigned char *pic, *p, low, high, new_low_val, new_high_val, pixel;

    Progname = strsave(*argv);
    if (argv[argc - 1][0] == '-')
	argc--;

    low = 0;
    high = 255;

    if (argc > 1)
	low = atoi(argv[1]);
    if (argc > 2)
	high = atoi(argv[2]);
    if (argc > 3)
	new_low_val = atoi(argv[3]);
    if (argc > 4)
	new_high_val = atoi(argv[4]);

    read_header(&hd);
    if (hd.pixel_format != PFBYTE)
	perr(HE_MSG, "image pixel format must be bytes");
    update_header(&hd, argc, argv);
    write_header(&hd);


    rc = hd.orows * hd.ocols;

    pic = (unsigned char *) halloc(rc, sizeof(unsigned char));

    for (f = 0; f < hd.num_frame; f++) {
#ifdef OLD
	if (fread(pic, rc * sizeof(unsigned char), 1, stdin) != 1)
	    perr(HE_MSG, "error during read");
#else
	hd.image = pic;
	if (read_image(hd, f) == HIPS_ERROR)
	    return (HIPS_ERROR);
#endif
	p = pic;
	for (i = 0; i < rc; i++) {
	    pixel = *p;
	    if (pixel > high)
		*p++ = new_high_val;
	    else if (pixel < low)
		*p++ = new_low_val;
	    else
		*p++;
	}
#ifdef OLD
	if (fwrite(pic, rc * sizeof(char), 1, stdout) != 1)
	    perr(HE_MSG, "error during write");
#else
	if (write_image(hd, f) == HIPS_ERROR)
	    return (HIPS_ERROR);
#endif
    }
    return (0);
}
