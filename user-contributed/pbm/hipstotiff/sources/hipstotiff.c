/*
 * Copyright (c) 2019 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * hipstotiff.c - convert HIPS images and sequences to TIFF format
 *
 * usage:	hipstotiff [-o outfilebase] < iseq [> oseq]
 *
 * Hipstotiff converts hips sequences to TIFF format. TIFF is a single-image
 * format. Each image in the sequence is stored in a separate file
 * (outfilebase.1.tif, outfilebase.2.tif, ..., where the base filename defaults
 * to tiffoutfile). If there is only one frame, the ".1" is omitted. All pixel
 * formats handled by TIFF format are handled appropriately, including RGB,
 * but not files with 3 separate color bands
 *
 * pixel formats handled directly: BYTE, SBYTE, SHORT, USHORT, INT, UINT,
 * FLOAT, DOUBLE, RGB
 *
 * to load: cc -o hipstotiff hipstotiff.c -lhips -ltiff
 *
 * Mike Landy - 1/11/19
 */

#include <stdio.h>
#include <hipl_format.h>
#include <string.h>
#include <tiff.h>
#include <tiffio.h>

#include "calc_header.h"

static Flag_Format flagfmt[] = {
	{"o",{LASTFLAG},1,{{PTFILENAME,"tiffoutfile","output file basename"},
		LASTPARAMETER}},
	LASTFLAG};

int types[] = {PFBYTE,PFSBYTE,PFSHORT,PFUSHORT,PFINT,PFUINT,PFFLOAT,
	PFDOUBLE,PFRGB,LASTTYPE};

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd,hdp;
	Filename filename,filenamebase;
	h_boolean oflag;
	int method,fr,f,rows,cols,linebytes,bpp,nbands,r;
	char tempstring[400];
	FILE *fp,*outfile;
	TIFF *out;
	byte *linep;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&filenamebase,FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	if (hd.numcolor != 1 && hd.numcolor != 3)
		perr(HE_MSG,"number of colors must be 1 or 3");
	method = fset_conversion(&hd,&hdp,types,filename);
	fr = hdp.num_frame;
	rows = hdp.orows;
	cols = hdp.ocols;
	linebytes = cols*hsizepix(hdp.pixel_format);
	if (hdp.pixel_format == PFRGB) {
		nbands = 3;
		bpp = 8;
	}
	else {
		nbands = 1;
		bpp = 8*hsizepix(hdp.pixel_format);
	}
	for (f=0;f<fr;f++) {
		if (fr == 1)
			sprintf(tempstring,"%s.tif",filenamebase);
		else if (fr < 10)
			sprintf(tempstring,"%s.%d.tif",filenamebase,f+1);
		else if (fr < 100)
			sprintf(tempstring,"%s.%02d.tif",filenamebase,f+1);
		else
			sprintf(tempstring,"%s.%03d.tif",filenamebase,f+1);
		out = TIFFOpen(tempstring,"w");
		if (out == NULL)
			perr(HE_OPEN,tempstring);
		TIFFSetField(out,TIFFTAG_IMAGEWIDTH,cols);
		TIFFSetField(out,TIFFTAG_IMAGELENGTH,rows);
#ifdef ULORIG
		TIFFSetField(out,TIFFTAG_ORIENTATION,ORIENTATION_TOPLEFT);
#else
		TIFFSetField(out,TIFFTAG_ORIENTATION,ORIENTATION_BOTLEFT);
#endif
		TIFFSetField(out,TIFFTAG_SAMPLESPERPIXEL,nbands);
		TIFFSetField(out,TIFFTAG_BITSPERSAMPLE,bpp);
		TIFFSetField(out,TIFFTAG_FILLORDER,FILLORDER_LSB2MSB); /* guess */
		TIFFSetField(out,TIFFTAG_PLANARCONFIG,PLANARCONFIG_CONTIG);
		if (hdp.pixel_format == PFRGB)
			TIFFSetField(out,TIFFTAG_PHOTOMETRIC,PHOTOMETRIC_RGB);
		else
			TIFFSetField(out,TIFFTAG_PHOTOMETRIC,PHOTOMETRIC_MINISBLACK);
		switch (hdp.pixel_format) {
		case PFBYTE:
		case PFUSHORT:
		case PFUINT:
		case PFRGB:
			TIFFSetField(out, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT);
			break;
		case PFSBYTE:
		case PFSHORT:
		case PFINT:
			TIFFSetField(out, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_INT);
			break;
		case PFFLOAT:
		case PFDOUBLE:
			TIFFSetField(out, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP);
			break;
		default:
			perr(HE_MSG,"pixel format is wrong internally");
		}
		TIFFSetField(out, TIFFTAG_COMPRESSION,
			(uint16) COMPRESSION_PACKBITS);
		fread_imagec(fp,&hd,&hdp,method,f,filename);
		linep = hdp.image;
		for (r=0;r<rows;r++) {
			/* Swap bytes??? */
			if (TIFFWriteScanline(out,linep,r,0) < 0) 
				perr(HE_MSG,"scanline write error");
			linep += linebytes;
		}
		TIFFClose(out);
	}
	return(0);
}
