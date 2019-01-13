/*	Copyright (c) 1990 Jens Michael Carstensen

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 * imf2hips - converts a GOP-image to a hips-image 
 *
 * Input image is in b1,u8,p8 or p16 format,
 *
 * usage:	imf2hips <imf-image >oseq
 *
 * to load:	cc -o imf2hips imf2hips.c -lhips 
 *
 * Sep. 1992 - HIPS-2 jmc
 */

#include <stdio.h>
#include <hipl_format.h>

#define Byte        unsigned char
#define Short       unsigned short
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
{   
	unsigned long id;
	unsigned long off;
	unsigned long len;
	unsigned short hgh;
	unsigned short wid;
	unsigned short typ;
};


int main(argc,argv)
int argc;
char **argv;
{
	Byte		*pic,*pic1,*ppic,rhd[512];
	int		rcol,i,npix,nrest;
	Short   	*spic,*sppic;
	struct header 	hd;
	struct gophead 	gophd;

	Progname = strsave(*argv);
	if (fread(&gophd,sizeof(struct gophead),1,stdin) != 1) 
		perr(HE_MSG,"error during read");
	nrest=512-sizeof(struct gophead);
	if (fread(rhd,1,nrest,stdin) != nrest) 
		perr(HE_MSG,"error during read");
	npix   = gophd.hgh*gophd.wid;

	if (gophd.id != image_magic)
		perr(HE_MSG,"input is not a GOP image");
	init_header(&hd,"","",1,"",gophd.hgh,gophd.wid,PFBYTE,1,"");
	switch (gophd.typ) {
	case im_b1:
		hd.pixel_format=PFMSBF;
		break;
	case im_u8:
		break;
	case im_p8:
		hd.num_frame=2;
		break;
	case im_p16:
		hd.pixel_format=PFSHORT;
		hd.num_frame=2;
		break;
	default:
		perr(HE_MSG,"image type must be b1, u8, p8 or p16");
	}
	update_header(&hd,argc,argv);
	write_header(&hd);

	switch (gophd.typ) {
	case im_b1:
		rcol = (hd.ocols+7)/8;
		pic = (Byte *) halloc(hd.orows*rcol,sizeof(Byte));
		if (fread(pic,1,hd.orows*rcol,stdin) != hd.orows*rcol)
			perr(HE_MSG,"error during read");
		if (fwrite(pic,1,hd.orows*rcol,stdout) != hd.orows*rcol)
			perr(HE_MSG,"error during write");
		break;
	case im_u8:
		pic = (Byte *) halloc(npix+1,sizeof(Byte));
		pic1 = pic;
		for (i=0;i<hd.orows;i++,pic1+=hd.ocols) {
			rcol=gophd.wid+hd.ocols%2;
			if (fread(pic1,1,rcol,stdin) != rcol)
				perr(HE_MSG,"error during read");
		}
		if (fwrite(pic,1,npix,stdout) != npix)
			perr(HE_MSG,"error during write");
		break;
	case im_p8:
		pic = (Byte *) halloc(npix,sizeof(Byte));
		ppic = (Byte *) halloc(2*npix,sizeof(Byte));
		if (fread(ppic,1,2*npix,stdin) != 2*npix)
			perr(HE_MSG,"error during read");
		for (i=0;i<npix;i++) pic[i]=ppic[2*i];
		if (fwrite(pic,1,npix,stdout) != npix)
			perr(HE_MSG,"error during write");
		for (i=0;i<npix;i++) pic[i]=ppic[2*i+1];
		if (fwrite(pic,1,npix,stdout) != npix)
			perr(HE_MSG,"error during write");
		break;
	case im_p16:
		spic = (Short *) halloc(npix,sizeof(Short));
		sppic = (Short *) halloc(2*npix,sizeof(Short));
		if (fread(sppic,sizeof(Short),2*npix,stdin) != 2*npix)
			perr(HE_MSG,"error during read");
		for (i=0;i<npix;i++) spic[i]=sppic[2*i];
		if (fwrite(spic,sizeof(Short),npix,stdout) != npix)
			perr(HE_MSG,"error during write");
		for (i=0;i<npix;i++) spic[i]=sppic[2*i+1];
		if (fwrite(spic,sizeof(Short),npix,stdout) != npix)
			perr(HE_MSG,"error during write");
		break;
	default:
		break;
	}
	return(0);
}
