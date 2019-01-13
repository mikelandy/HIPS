/*
 * Copyright (c) 1992	Jin Guojun
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 *
 * to_stream - convert an image to HIPS-JPEG format
 *
 * usage:	to_stream < idata > oseq
 *
 * to load:	cc -o to_stream to_stream.c -lscs5 -lccs -hipsh -lhips -ljpeg
 *
 * AUTHOR:	Jin Guojun - 11/12/92
 */

#include <stdio.h>
#include "header.def"
#include "imagedef.h"

static Flag_Format flagfmt[] =
{
    {"c", {LASTFLAG}, 0, {{PTBOOLEAN, "FALSE"}, LASTPARAMETER}},
/*  {"f", {LASTFLAG}, 1, {{PTINT, "30", "frames / sec."}, LASTPARAMETER}}, */
    {"n", {LASTFLAG}, 1, {{PTINT, "99999999", "number frames"}, LASTPARAMETER}},
    {"j", {LASTFLAG}, 0, {{PTBOOLEAN, "FALSE"}, LASTPARAMETER}},
    {"p", {LASTFLAG}, 0, {{PTINT, "0", "pad to 2**"}, LASTPARAMETER}},
    {"Q", {LASTFLAG}, 1, {{PTINT, "75", "Q-factor"}, LASTPARAMETER}},
    LASTFLAG};


main(argc, argv)
int	argc;
char**	argv;
{
int	oform, qfact, pflag = -1;
h_boolean	jflag, ciflag, ccflag, coflag;
int	frames, f, pad;
sframe_header	fhd;
Filename	filename;
byte	*fakec;
extern	U_IMAGE	uimg;

    Progname = *argv;
    parseargs(argc, argv, flagfmt, &coflag, &frames, &jflag, &pflag, &qfact,
	FFONE, &filename);

    jpeg_uimg_init(hfopenr(filename), HIPS, 0, coflag);
    reset_pipe_read(&uimg);
    if ((*uimg.header_handle)(HEADER_READ, &uimg, 0, 0, Yes) < 0)
	prgmerr('h', "Unknown image type");
    if (frames > hhd.num_frame)
	frames = hhd.num_frame / hhd.numcolor;
    else	hhd.num_frame = frames * hhd.numcolor;
/*
    ciflag = hjpeg_color(&hhd);
*/
    ciflag = isColorImage(uimg.in_color);
    ccflag = coflag & !ciflag;

    if (is_stream_image(0))	/* stream input	*/
	if ((pad=is_comp_image(0)) == 1)	{	/* compress type for reading */
		if (jflag)	prgmerr('j', "s_jpeg to s_jpeg - Why ?");
#	ifndef	USE_JPEG_STREAM
		stream_jpeg_rinit(in_fp, get_sinfo_q(0), ciflag | coflag);
#	endif
		clearparam(&hhd, "JPEG-info");
	} else if (pad= !jflag)
		prgmerr(0, "Warning: stream to stream - waste time");

    set_stream_param(&hhd, get_stream_header_sline(hhd.ocols, hips_channels(&hhd)),
		jflag, -1, qfact, -1 /* no data_size */, 1);

    if (!jflag && !pad)
	if (coflag/* && hhd.numcolor == 1*/)	{
	    if (hhd.pixel_format != PFRGB)
		hhd.sizeimage *= hips_channels(&hhd);
	    if (ciflag & pad)	{
		hhd.pixel_format = PFRGB;
		hhd.num_frame /= hhd.numcolor,	hhd.numcolor = 1;
	    }
	} else	{
		hhd.pixel_format = PFBYTE;
		hhd.num_frame /= hhd.numcolor,	hhd.numcolor = 1;
	}
    else if (pad)	hhd.sizeimage *= hips_channels(&hhd);

    write_headeru(&hhd, argc, argv);
/*
	(*uimg.header_handle)(HEADER_WRITE, &uimg, argc, argv, NULL);
*/
    fakec = (byte *) NZALLOC(uimg.width * uimg.height + 320, 3, "fakec");
    uimg.dest = fakec;
    if (jflag)
	hhjpeg_winit(stdout, &hhd, get_sinfo_q(1), ciflag | coflag);

	if (ccflag)
		hhd.image = (byte *) NZALLOC(uimg.width * uimg.height, 3, "image");
	if (pflag < 0)	pflag = 0;
	if (pflag)	{
		for (pad=0, pflag=(pflag<<1)-1; pflag>>=1; pad++);
		pflag = 1 << pad;
	}

    for (f = 0; f < frames; f++) {
	if (stream_jpeg_read_image(fakec, &hhd, f) < 1)
		prgmerr('r', "stream_jpeg_read_image");
	if (ccflag) {
		register byte *sp = fakec, *cp = hhd.image;
		register int r = uimg.height, w = uimg.width;
		while (r--) {
		    memcpy(cp, sp, w);
		    cp += w;
		    memcpy(cp, sp, w);
		    cp += w;
		    memcpy(cp, sp, w);
		    cp += w;
		    sp += w;
		}
	} else	hhd.image = fakec;

	if (jflag) {
	    if (stream_jpeg_write_frame(hhd.image, &fhd) < 0)
		return (perr(HE_WRITEFRFILE, f, filename));
	    /*
	     * this routine points hhd->image to the compressed image, and
	     * sets the size of the compressed frame in fhd
	     */
	    fhd.frame = f;
	    if (pflag)	{
		pad = pflag - (fhd.size % pflag);
		fprintf(stderr, "padding by %d bytes \n", pad);
		fhd.size += pad;
	    }
	    if (write_stream_header(stdout, &hhd, &fhd, f) < 0
	    	|| fwrite(hhd.image, fhd.size, 1, stdout) != 1)
		return (perr(HE_WRITEFRFILE, f, filename));

	} else	write_image(&hhd, f);
    }

if (jflag)
	stream_jpeg_write_term();

fclose(stdout);
free(fakec);
if (ccflag)
	free(hhd.image);
return (0);
}
