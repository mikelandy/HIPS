
/*	Copyright (c) 1989 Michael Landy

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 * mapapply.c - Apply a previously generated map to each pixel of a frame.
 *
 * usage: mapapply map_name < frame > new_frame
 *
 * For byte images, pixels are renormalized to lie between 0 and 255.  For
 * integer and float images, the output is a float image and no
 * renormalization is performed.
 *
 * to load: cc -o mapapply mapapply.c -lhips -lm
 *
 * Yoav Cohen 2/16/82
 * added int/float - Mike Landy - 3/16/89
 *
 *  modified to use look-up table for byte and short images:
 *     Brian Tierney, LBL 10/90
 *
 *  modified to generate look-up table for byte and short images, but not
 *  apply the table to the image.  Therefore, this is to be used in
 *  conjunction with mapapply to generate an image:
 *     Bryan Skene, LBL 1/29/91
 *
 */

#include <hipl_format.h>
#include <math.h>
#include <stdio.h>

#define MAXSHORT 32768

char      forms[80];

unsigned char *pic;
short    *spic;
float    *fpic;

FILE     *fp;
unsigned char byte_blut[256];
short    *short_slut;
float     byte_flut[256], *short_flut;

main(argc, argv)
    int       argc;
    char     *argv[];

{
    struct header hd;
    int       i, form, tblform, npix, numentries, bentry;
    short     sentry;
    float     fentry;
    char      mapname[80];

    Progname = strsave(*argv);
    read_header(&hd);
    form = hd.pixel_format;
    npix = hd.orows * hd.ocols;

    strcpy(mapname, argv[1]);
    fp = fopen(mapname, "r");
    fscanf(fp, "%d", &numentries);
    fscanf(fp, "%d", &tblform);

    fprintf(stderr, "number of entries in table = %d\n", numentries);
    strform(form);
    fprintf(stderr, "from : %s\n", forms);
    strform(tblform);
    fprintf(stderr, "to   : %s\n", forms);

    if (form == PFBYTE) {
	if (tblform == PFBYTE) {
	    pic = (unsigned char *) halloc(npix, sizeof(char));
	    update_header(&hd, argc, argv);
	    write_header(&hd);
	    for (i = 0; i < numentries; i++) {
		fscanf(fp, "%d\n", &bentry);
		byte_blut[i] = bentry;
	    }
	    bmapapplyb(hd.num_frame, hd.orows, hd.ocols);
	} else if (tblform == PFFLOAT) {
	    hd.pixel_format = PFFLOAT;
	    pic = (unsigned char *) halloc(npix, sizeof(char));
	    fpic = (float *) halloc(npix, sizeof(float));
	    update_header(&hd, argc, argv);
	    write_header(&hd);
	    for (i = 0; i < numentries; i++) {
		fscanf(fp, "%f\n", &fentry);
		byte_flut[i] = fentry;
	    }
	    bmapapplyf(hd.num_frame, hd.orows, hd.ocols);
	}
    } else if (form == PFSHORT) {
	if (tblform == PFSHORT) {
	    spic = (short *) halloc(npix, sizeof(short));
	    short_slut = (short *) halloc(numentries, sizeof(short));
	    update_header(&hd, argc, argv);
	    write_header(&hd);
	    fprintf(stderr, "S to S:number of entries = %d\n", numentries);
	    for (i = 0; i < numentries; i++) {
		fscanf(fp, "%f\n", &sentry);
		short_slut[i] = (short) sentry;
	    }
	    smapapplys(hd.num_frame, hd.orows, hd.ocols);
	} else if (tblform == PFFLOAT) {
	    hd.pixel_format = PFFLOAT;
	    spic = (short *) halloc(npix, sizeof(short));
	    fpic = (float *) halloc(npix, sizeof(float));
	    short_flut = (float *) halloc(numentries, sizeof(float));
	    update_header(&hd, argc, argv);
	    write_header(&hd);
	    for (i = 0; i < numentries; i++) {
		fscanf(fp, "%f\n", &fentry);
		short_flut[i] = fentry;
	    }
	    smapapplyf(hd.num_frame, hd.orows, hd.ocols);
	}
    } else
	perr(HE_MSG, "input image format must be byte or short");

    close(fp);
    return (0);
}

bmapapplyb(fr, r, c)
    int       fr, r, c;

{
    int       j, i, rc;
    unsigned char *ppic;

    rc = r * c;
    for (j = 0; j < fr; j++) {
	if (fread(pic, rc * sizeof(unsigned char), 1, stdin) != 1)
	    perr(HE_MSG, "error during read");
	ppic = pic;
	for (i = 0; i < rc; i++)
	    *ppic++ = byte_blut[*ppic];
	if (fwrite(pic, rc * sizeof(char), 1, stdout) != 1)
	    perr(HE_MSG, "error during write");
    }
    return (0);
}

bmapapplyf(fr, r, c)
    int       fr, r, c;

{
    int       j, i, rc;
    unsigned char *ppic;
    float    *pfpic;

    rc = r * c;
    for (j = 0; j < fr; j++) {
	if (fread(pic, rc * sizeof(unsigned char), 1, stdin) != 1)
	    perr(HE_MSG, "error during read");
	ppic = pic;
	pfpic = fpic;
	for (i = 0; i < rc; i++)
	    *pfpic++ = byte_flut[*ppic++];
	if (fwrite(fpic, rc * sizeof(float), 1, stdout) != 1)
	    perr(HE_MSG, "error during write");
    }
    return (0);
}


smapapplys(fr, r, c)
    int       fr, r, c;

{
    int       j, i, rc;
    short    *pspic;

    rc = r * c;
    for (j = 0; j < fr; j++) {
	if (fread(spic, rc * sizeof(short), 1, stdin) != 1)
	    perr(HE_MSG, "error during read");
	fprintf(stderr, "Ya boy!\n");
	pspic = spic;
	for (i = 0; i < rc; i++)
	    *pspic++ = short_slut[*pspic];
	if (fwrite(spic, rc * sizeof(short), 1, stdout) != 1)
	    perr(HE_MSG, "error during write");
    }
    return (0);
}

smapapplyf(fr, r, c)
    int       fr, r, c;

{
    int       j, i, rc;
    short    *pspic;
    float    *pfpic;

    rc = r * c;
    for (j = 0; j < fr; j++) {
	if (fread(spic, rc * sizeof(short), 1, stdin) != 1)
	    perr(HE_MSG, "error during read");
	pspic = spic;
	pfpic = fpic;
	for (i = 0; i < rc; i++)
	    *pfpic++ = short_flut[*pspic++];
	if (fwrite(fpic, rc * sizeof(float), 1, stdout) != 1)
	    perr(HE_MSG, "error during write");
    }
    return (0);
}

strform(formi)
    int       formi;
{
    switch (formi) {
    case PFBYTE:
	strcpy(forms, "BYTE");
	break;
    case PFSHORT:
	strcpy(forms, "SHORT");
	break;
    case PFFLOAT:
	strcpy(forms, "FLOAT");
	break;
    default:
	strcpy(forms, "UNKNOWN");
    }
}
