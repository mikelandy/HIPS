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
 * to load:	cc -o to_hjpeg to_hjpeg.c -hipsh -lhips
 *
 * AUTHOR:	Jin Guojun - 11/12/92
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] =
{
    {"c", {LASTFLAG}, 0, {{PTBOOLEAN, "FALSE"}, LASTPARAMETER}},
    {"o", {LASTFLAG}, 0, {{PTBOOLEAN, "FALSE"}, LASTPARAMETER}},
    {"Q", {LASTFLAG}, 1, {{PTINT, "75", "Q-factor"}, LASTPARAMETER}},
    LASTFLAG};

main(argc, argv)
int       argc;
char    **argv;

{
int	oform, qfact;
h_boolean	oflag, ccflag, cflag;
int	rows, cols, frames, f, numcolor, pad;
int	xstream[2], xjpeg[2];
struct header	hd;
sframe_header	fhd;
Filename	filename;
FILE	*fp;
byte	*fakec;

    Progname = strsave(*argv);
    parseargs(argc, argv, flagfmt, &ccflag, &oflag, &qfact, FFONE, &filename);

    fp = hfopenr(filename);
/*
    fread_hdr_a(fp, &hd, filename);
*/
    cflag = hjpeg_uimg_init(fp, &hd, 1, 1);
    frames = hd.num_frame;
    numcolor = hd.numcolor;

    xstream[0] = 1;
    xstream[1] = 1;
    setparam(&hd, "stream-info", PFINT, 2, xstream);
    xjpeg[0] = qfact;
    xjpeg[1] = -1;		/* place to store data size later */
    setparam(&hd, "JPEG-info", PFINT, 2, xjpeg);
    write_headeru(&hd, argc, argv);
    ccflag &= !cflag;
    check_compress_header(stdout, &hd, 1, cflag | ccflag);

    alloc_image(&hd);
    if (ccflag && !oflag)	/* fake color image for gray-scale */
	fakec = (byte *) malloc(hd.cols * hd.rows * 3);
    else
	fakec = hd.image;

    for (f = 0; f < frames * numcolor; f++) {
#ifdef OLD
	fread_image(fp, &hd, f, filename);
#else
/*
	if (fread(hd.image, hd.sizeimage, 1, fp) != 1)
	    return (perr(HE_READFRFILE, f, filename));
*/
	hjpeg_read_image(fakec);
#endif
	if (!oflag) {
	    if (ccflag) {
		register byte *cp = fakec, *sp = hd.image;
		register int r = hd.rows, w = hd.cols;
		while (r--) {
		    memcpy(cp, sp, w);
		    cp += w;
		    memcpy(cp, sp, w);
		    cp += w;
		    memcpy(cp, sp, w);
		    cp += w;
		    sp += w;
		}
	    }
	    (int) hd.image ^= (int) fakec ^= (int) hd.image ^= (int) fakec;
	    if (compress_jpeg_frame(&hd, &fhd) < 0)
		return (perr(HE_WRITEFRFILE, f, filename));
	    /*
	     * this routine points hd->image to the compressed image, and
	     * sets the size of the compressed frame in fhd
	     */
	    fhd.frame = f;
	    pad = 8 - (fhd.size % 8);
	    fprintf(stderr, "padding by %d bytes \n", pad);
	    fhd.size += pad;
	    if (fwrite(&fhd, sizeof(sframe_header), 1, stdout) != 1)
		return (perr(HE_WRITEFRFILE, f, filename));
	    if (fwrite(hd.image, fhd.size, 1, stdout) != 1)
		return (perr(HE_WRITEFRFILE, f, filename));
	    (int) hd.image ^= (int) fakec ^= (int) hd.image ^= (int) fakec;

#define NEED			/* this is needed for compatibility with the
				 * Parallax card */
#ifdef NEED
	    while (pad--)
		fputc(0, stdout);
#endif
	} else
	    write_image(&hd, f);
    }
    write_jpeg_eof(stdout);
    fclose(stdout);
    free(hd.image);
    if (cflag)
	free(fakec);
    return (0);
}
