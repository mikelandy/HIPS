/*	Copyright (c) 1993 Jens Michael Carstensen

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 * hips2tga - converts a hips-image to tga file format
 *
 * Input image must be in byte or rgb format,
 *
 * usage:	hips2tga <seq >tga-image
 *
 * to load:	cc -o hips2tga hips2tga.c -lhips 
 *
 */

#include <stdio.h>
#include <hipl_format.h>

#define Byte        unsigned char

int main(argc,argv)
int argc;
char *argv[];
{
	Byte		*pic;
	Byte		header[18];
	int		i,j,nrows,ncols,npix,im_type;
	struct header 	hd;

	Progname = strsave(*argv);
	read_header (&hd);
	nrows=hd.orows;
	ncols=hd.ocols;
	npix = hd.orows*hd.ocols;

	switch (hd.pixel_format) {
	case PFBYTE:
		im_type=3;
		break;
	case PFRGB:
		im_type=2;
		break;
	default:
		perr(HE_MSG,"bad pixel format");
	}

	header[0]=0;
	header[1]=0;
	header[2]=im_type;
	header[3]=0; /* CMAP spec */
	header[4]=0;
	header[5]=0;
	header[6]=0;
	header[7]=0;
	header[8]=0; /* Image spec */
	header[9]=0;
	header[10]=0;
	header[11]=0;
	header[12]=ncols%256;
	header[13]=ncols/256;
	header[14]=nrows%256;
	header[15]=nrows/256;
	header[16]=hd.sizepix*8;
	header[17]=0x30;

	if (fwrite(&header[0],18,1,stdout) != 1) 
		perr(HE_MSG,"error during write");


	pic = (Byte *) halloc(npix*hd.sizepix,sizeof(Byte));
	if (fread(pic,1,npix*hd.sizepix,stdin) != npix*hd.sizepix)
	      perr(HE_MSG,"error during read");
	if (fwrite(pic,1,npix*hd.sizepix,stdout) != npix*hd.sizepix)
	      perr(HE_MSG,"error during write");

	return(0);
}
