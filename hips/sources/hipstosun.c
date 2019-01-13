/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * hipstosun.c - convert a single hips image to Sun format
 *
 * usage:	hipstosun [-e] < in > out
 *
 * Hipstosun converts a hips image to Sun format.  Only a single frame
 * sequence is permitted, either in MSBF, byte, RBG, RGBZ, BGR or ZBGR format.
 * If parameter cmap is set, it is stored in the output
 * lookup table.  Otherwise, no lookup table is included in the output.
 * MSBF images are output as 1-bit
 * deep.  BYTE images are output as 8-bits deep.  RGB images are output as
 * 24 bits deep in RGB raster format, and RGBZ images are output as 32 bits
 * deep in RGB raster format.  BGR images are output as standard 24-bit deep
 * images, and ZBGR images are output as standard 32-bit deep images. The -e
 * flag encodes the image using RLE encoding, and requires BGR or ZBGR images
 * for color input.  Binary images without colormaps are inverted so that 1
 * displays as white.
 *
 * to compile:	cc -o hipstosun hipstosun.c -lhipsh -lhips -lm
 *
 * Mike Landy - 6/12/87
 * HIPS 2 - 1/8/91
 * removed pixrect dependencies, added RGB/RGBZ - msl - 2/10/93
 * BGR/ZBGR/etc. - msl - 5/24/93
 */

#include <stdio.h>
#include <hipl_format.h>

#define	RT_STANDARD	1
#define	RT_BYTE_ENCODED	2
#define	RT_FORMAT_RGB	3

#define	RMT_NONE	0
#define	RMT_EQUAL_RGB	1

static Flag_Format flagfmt[] = {
	{"e",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	LASTFLAG};
int types[] = {PFMSBF,PFBYTE,PFRGB,PFRGBZ,PFBGR,PFZBGR,LASTTYPE};
int types2[] = {PFMSBF,PFBYTE,PFBGR,PFZBGR,LASTTYPE};

int main(argc,argv)

int argc;
char **argv;

{
	int nrows,ncols,icb,ocb,method,i,j,hdr[8],depth,cmapl,len;
	int imagelen;
	struct header hd,hdp;
	Filename filename;
	FILE *fp;
	h_boolean Eflag;
	byte *cmap,*p,*pp,*pc,pixel,*endp,*imgbuf,*outimg;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&Eflag,FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	method = fset_conversion(&hd,&hdp,Eflag ? types2 : types,filename);
	if (hdp.num_frame != 1)
		perr(HE_MSG,"number of frames must be 1");
	fread_imagec(fp,&hd,&hdp,method,0,filename);
	clearroi(&hdp);
	nrows = hd.orows;
	ncols = hd.ocols;
	if (hdp.pixel_format == PFMSBF) {
		icb = (ncols+7)/8;
		depth = 1;
	}
	else if (hdp.pixel_format == PFBYTE) {
		icb = ncols;
		depth = 8;
	}
	else if (hdp.pixel_format == PFRGB || hdp.pixel_format == PFBGR) {
		icb = ncols*3;
		depth = 24;
	}
	else {
		icb = ncols*4;
		depth = 32;
	}
	ocb = (icb + 1) & ~1;
	hdr[0] = 0x59a66a95;
	hdr[1] = ncols;
	hdr[2] = nrows;
	hdr[3] = depth;
	hdr[4] = ocb*nrows;
	if (Eflag)
		hdr[5] = RT_BYTE_ENCODED;
	else if (hdp.pixel_format == PFRGB || hdp.pixel_format == PFRGBZ)
		hdr[5] = RT_FORMAT_RGB;
	else
		hdr[5] = RT_STANDARD;
	if (findparam(&hd,"cmap") != NULLPAR) {
		hdr[6] = RMT_EQUAL_RGB;
		cmapl = 768;
		getparam(&hd,"cmap",PFBYTE,&cmapl,&cmap);
		if (cmapl % 3)
			perr(HE_MSG,"colormap length is not a multiple of 3");
		perr(HE_IMSG,"colormap will be placed in Sun raster file");
		hdr[7] = cmapl;
	}
	else {
		hdr[6] = RMT_NONE;
		hdr[7] = cmapl = 0;
		if (depth == 1)
			h_neg(&hdp,&hdp);
	}
	if (icb != ocb) {
		pp = imgbuf = halloc(ocb*nrows,sizeof(byte));
		p = hdp.image;
		for (i=0;i<nrows;i++) {
			for (j=0;j<icb;j++)
				*pp++ = *p++;
			*pp++ = 0;
		}
	}
	else
		imgbuf = hdp.image;
	if (Eflag) {
		outimg = pc = hmalloc(3*ocb*nrows/2);
		p = imgbuf;
		endp = p + ocb*nrows;
		len = 0;
		while (p < endp) {
			pixel = *p;
			pp = p;
			for (i=1;i<256;i++) {
				if (++pp >= endp)
					break;
				if (*pp != pixel)
					break;
			}
			if (i < 3) {
				if (pixel == 128) {
					*pc++ = 128;
					*pc++ = 0;
					len += 2;
				}
				else {
					*pc++ = pixel;
					len++;
				}
				p++;
			}
			else {
				*pc++ = 128;
				*pc++ = i-1;
				*pc++ = pixel;
				p += i;
				len += 3;
			}
		}
		imagelen = hdr[4] = len;
	}
	else {
		outimg = imgbuf;
		imagelen = ocb*nrows;
	}
	if (fwrite(hdr,sizeof(int)*8,1,stdout) != 1)
		perr(HE_MSG,"error writing header");
	if (cmapl) {
		if (fwrite(cmap,sizeof(byte)*cmapl,1,stdout) != 1)
			perr(HE_MSG,"error writing colormap");
	}
	if (fwrite(outimg,sizeof(byte)*imagelen,1,stdout) != 1)
		perr(HE_MSG,"error writing colormap");
	exit(0);
}
