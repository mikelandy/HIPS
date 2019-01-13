/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * seeheader.c - print the header of a frame sequence
 *
 * Usage:	seeheader [-p] [-a] < frame
 *
 * Load:	cc -o seeheader seeheader.c -lhips
 *
 * Michael Landy - 2/4/82
 * HIPS 2 - msl - 1/6/91
 *
 * The -p option allows seeheader to be used in a pipe, sending the
 * original sequence to the standard output, and the text output to stderr.
 * The -a flag causes entire extended parameter arrays to be printed.  By
 * default at most 5 values are printed. If the -s flag is specified, then
 * per-frame headers are output if the file is a stream file.
 */

#include <hipl_format.h>
#include <stdio.h>

static Flag_Format flagfmt[] =
{
    {"p",
     {LASTFLAG}, 0,
     {
	 {PTBOOLEAN, "FALSE"}, LASTPARAMETER}},
    {"a",
     {LASTFLAG}, 0,
     {
	 {PTBOOLEAN, "FALSE"}, LASTPARAMETER}},
    {"s",
     {LASTFLAG}, 0,
     {
	 {PTBOOLEAN, "FALSE"}, LASTPARAMETER}},
    LASTFLAG};

/*
#define DEBUG
*/

main(argc, argv)
    int       argc;
    char    **argv;

{
    struct header hd;
    int       c, i, skip, pad;
    h_boolean   pflag, aflag, sflag;
    Filename  filename;
    FILE     *fp;

    Progname = strsave(*argv);
    parseargs(argc, argv, flagfmt, &pflag, &aflag, &sflag, FFONE, &filename);
    fp = hfopenr(filename);
    if (pflag)
	fread_hdr_a(fp, &hd, filename);
    else
	fread_header(fp, &hd, filename);
    fprintf(stderr, "%s", formatheaderc(&hd, aflag));
#ifdef DEBUG
    fprintf(stderr, "after format_header, fp now at: %d \n", (int) ftell(fp));
#endif
    {
	stream_info sinfo;
	sframe_header fhd;

	if (sflag) {
#ifdef NEED
	    check_stream_header(&hd, FALSE);
#endif
	    if (get_sinfo(&sinfo, 0) && sinfo.stream) {
		if (hd.numcolor == 3)
		    hd.num_frame /= 3;
		if (sinfo.comp_type == 1)
		    hread_jmain_header(fp);
#ifdef DEBUG
		fprintf(stderr, "after read_main header, fp now at: %d \n",
			(int) ftell(fp));
#endif

		for (i = 0; i < hd.num_frame; i++) {
		    if (sinfo.comp_type == 1 && i > 0)
			hread_jscan_header(fp);
#ifdef DEBUG
		    fprintf(stderr, "after read scan header, fp now at: %d \n",
			    (int) ftell(fp));
#endif
		    if (read_stream_header(fp, &hd, &fhd, i) == HIPS_ERROR)
			break;

		    fprintf(stderr, "frame %d, size: %d, time code %d:%d:%d:%d \n",
			    fhd.frame, fhd.size, fhd.tc.hr, fhd.tc.mn,
			    fhd.tc.sc, fhd.tc.fr);

		    fseek(fp, fhd.size, 1);	/* skip over JPEG data */
		}
	    }
	}
    }
    if (!pflag)
	return (0);
    write_header(&hd);
    if (hd.sizeimage) {
	for (i = 0; i < hd.num_frame; i++) {
	    fread_image(fp, &hd, i, filename);
	    write_image(&hd, i);
	}
    } else {
	while ((c = getc(fp)) != EOF)
	    putchar(c);
    }
    return (0);
}
