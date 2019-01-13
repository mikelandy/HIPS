/*	Copyright (c) 1990 Jens Michael Carstensen

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 * hips2imf - converts a hips-image to an imf-image for
 *            the GOP-302.
 *
 * Input image is in byte format,
 *
 * usage:	hips2imf <seq >imf-image
 *
 * to load:	cc -o hips2imf hips2imf.c -lhips 
 *
 * Jens Michael Carstensen - 1/29/90
 *
 * HIPS-2 - JMC Sep. 1992
 */

#include <stdio.h>
#include <hipl_format.h>

#define Byte        unsigned char
#define image_magic 0xE9ED0000

#define im_p16 1
#define im_s16 2
#define im_c16 3
#define im_p8 4
#define im_u8 5
#define im_s8 6
#define im_c8 7
#define im_x8 8
#define im_b1 9
#define im_p32 10
#define im_s32 11
#define im_c32 12

/* Header record for GOP image files */
struct gophead
{   unsigned long id;
    unsigned long off;
    unsigned long len;
    unsigned short hgh;
    unsigned short wid;
    unsigned short typ;
};


int main(argc,argv)
int argc;
char *argv[];
{
	Byte		*pic,whd[512],*row;
	int		wrcol,i,j,nrest,npix;
	struct header 	hd;
	struct gophead 	gophd;

	Progname = strsave(*argv);
	read_header (&hd);
	switch (hd.pixel_format) {
	case PFBYTE:
		if (hd.numcolor==2) gophd.typ=im_p8;
		else {
			gophd.typ = im_u8;
		}
		break;
	default:
		perr(HE_MSG,"bad pixel format");
	}
	gophd.id  = image_magic;
	gophd.off = 512;
	gophd.hgh = hd.orows;
	gophd.wid = hd.ocols;
	wrcol = hd.ocols;
	npix = hd.orows*hd.ocols;
       
	gophd.len = hd.orows*wrcol;

	if (fwrite(&gophd,sizeof(struct gophead),1,stdout) != 1) 
		perr(HE_MSG,"error during write");
	nrest=512-sizeof(struct gophead);
	if (fwrite(whd,1,nrest,stdout) != nrest) 
		perr(HE_MSG,"error during write");
	
	switch (gophd.typ) {
	case im_u8:
	  pic = (Byte *) halloc(hd.orows*wrcol+1,sizeof(Byte));
	  if (fread(pic,1,hd.orows*wrcol,stdin) != hd.orows*wrcol)
	      perr(HE_MSG,"error during read");
	  for (i=0;i<hd.orows;i++,pic+=hd.ocols) {
	       if (fwrite(pic,1,wrcol+hd.ocols%2,stdout) != wrcol+hd.ocols%2)
	           perr(HE_MSG,"error during write");
	  }  
	  break;
	case im_p8:
	  pic = (Byte *) halloc(hd.orows*wrcol*2,sizeof(Byte));
	  row = (Byte *) halloc(wrcol*2,sizeof(Byte));
	  if (fread(pic,1,hd.orows*wrcol*2,stdin) != hd.orows*wrcol*2)
	      perr(HE_MSG,"error during read");
	  for (i=0;i<hd.orows;i++,pic+=hd.ocols) {
		for (j=0;j<hd.ocols;j++) {
			row[j] = *(pic+j);
			row[j+1] = *(pic+j+npix);
		}
	       	if (fwrite(pic,1,wrcol*2,stdout) != wrcol*2)
	           perr(HE_MSG,"error during write");
	  }  

	  break;
	}
	return(0);
}
