/*
 * Copyright (c) 1992	Jin Guojun
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 *
 * to_hjpeg - convert an image to HIPS-JPEG format
 *
 * usage:	to_hjpeg < idata > oseq
 *
 * to load:	cc -o to_jfif to_jfif.c -lhips
 *
 * AUTHOR:	Jin Guojun - 11/12/92
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] =
{
    {"o", {LASTFLAG}, 0, {{PTBOOLEAN, "FALSE"}, LASTPARAMETER}},
    LASTFLAG};

#define	M_EOI	0xD9
#define	M_SOS	0xDA


main(argc, argv)
    int       argc;
    char    **argv;

{
    int       oform;
    h_boolean   oflag;
    int       rows, cols, frames, f, numcolor, pad, ch1;
    int       xstream[2], xjpeg[2];
    struct header hd;
    sframe_header fhd;
    Filename  filename;
    FILE     *fp;

    Progname = strsave(*argv);
    parseargs(argc, argv, flagfmt, &oflag, FFONE, &filename);

    fp = hfopenr(filename);
    fread_header(fp, &hd, filename);
    alloc_image(&hd);


    while(1) {
        ch1 = fgetc(fp);
        fputc(ch1,stdout);
	if (ch1 == 0xff)	{
		ch1 = fgetc(fp);
		fputc(ch1,stdout);
		if (ch1 == M_SOS)	break;
	}
    }
    fgetc(fp);
    fputc(0, stdout);
    ch1 = fgetc(fp);
    fputc(ch1, stdout);
    fprintf(stderr, "jfif size %d \n", (int)ch1);
    ch1-=2;
    while (ch1--)	fputc(fgetc(fp) , stdout);

    fread(&fhd, sizeof(sframe_header), 1, fp);
    fprintf(stderr, "frame %d, size %d \n", fhd.frame, fhd.size);

    /* this will do 1st frame only */
	if (fread(hd.image, fhd.size, 1, fp) != 1)
	    return (perr(HE_READFRFILE, f, filename));
	if (fwrite(hd.image, fhd.size, 1, stdout) != 1)
	    return (perr(HE_READFRFILE, f, filename));

    write_jpeg_eof(stdout);
    fclose(stdout);
    return (0);
}
