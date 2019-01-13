/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * suntohips.c - convert a single Sun image to HIPS format
 *
 * usage:	suntohips < in > out
 *
 * Suntohips converts a Sun raster image to HIPS format.  A single frame in Sun
 * format is converted.  Depending on the raster depth and format, the
 * resulting image will be either MSBF bit-packed, byte, RBG, RGBZ, BGR or
 * ZBGR format.  The Sun lookup tables are stored in parameter cmap.  Binary
 * images without colormaps are inverted so that white maps to 1.
 *
 * to compile:	cc -o suntohips suntohips.c -lhipsh -lhips
 *
 * Mike Landy - 6/12/87
 * HIPS 2 - 1/8/91
 * added 24-bit/32-bit images and got rid of pixrect dependency - msl - 2/9/93
 * BGR/ZBGR/etc. - msl - 5/24/93
 */

#include <stdio.h>
#include <hipl_format.h>

#define	RT_STANDARD	1
#define	RT_BYTE_ENCODED	2
#define	RT_FORMAT_RGB	3

#define	RMT_NONE	0
#define	RMT_EQUAL_RGB	1

static Flag_Format flagfmt[] = {LASTFLAG};
void swbytes();

int main(argc,argv)

int argc;
char **argv;

{
	register int r;
	register byte *p;
	int rows,cols,depth,icb,ocb,i,n,m,cmaplen,ichar;
	struct header hd;
	Filename filename;
	FILE *fp;
	byte cmap[768];
	int hdr[8];
	h_boolean ENCflag=FALSE,RGBflag=FALSE,CMAPflag=FALSE;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,FFONE,&filename);
	fp = hfopenr(filename);
	if (fread(hdr,sizeof(int)*8,1,fp) != 1)
		perr(HE_MSG,"error reading Sun raster header");
	if (hdr[0] == 0x956aa659) {
		for (i=1;i<8;i++)
			swbytes((char *) (hdr+i));
	}
	else if (hdr[0] != 0x59a66a95)
		perr(HE_MSG,"input is not a Sun raster file");
	rows = hdr[2];
	cols = hdr[1];
	depth = hdr[3];
	if (!(depth == 1 || depth == 8 || depth == 24 || depth == 32))
		perr(HE_MSG,"nonstandard pixel depth");
	if (hdr[5] == RT_STANDARD)
		;	/* nothing to do here */
	else if (hdr[5] == RT_BYTE_ENCODED)
		ENCflag = TRUE;
	else if (hdr[5] == RT_FORMAT_RGB)
		RGBflag = TRUE;
	else
		perr(HE_MSG,"can't handle this type of Sun raster file");
	if (hdr[6] == RMT_EQUAL_RGB)
		CMAPflag = TRUE;
	else if (hdr[6] != RMT_NONE)
		perr(HE_MSG,"can't handle this type of raster color map");
	cmaplen = hdr[7];
	if (CMAPflag && (cmaplen < 1 || cmaplen > 768 || ((cmaplen%3) != 0)))
		perr(HE_MSG,"unreasonable color map length");
	if (depth==1) {
		init_header(&hd,"","",1,"",rows,cols,PFMSBF,1,"");
		ocb = (cols+7)/8;
	}
	else if (depth==8) {
		init_header(&hd,"","",1,"",rows,cols,PFBYTE,1,"");
		ocb = cols;
	}
	else if (depth == 24) {
		init_header(&hd,"","",1,"",rows,cols,RGBflag ? PFRGB : PFBGR,
			1,"");
		ocb = cols*3;
	}
	else {
		init_header(&hd,"","",1,"",rows,cols,RGBflag ? PFRGBZ : PFZBGR,
			1,"");
		ocb = cols*4;
	}
	if (CMAPflag && cmaplen > 0) {
		perr(HE_IMSG,"raster colormap will be placed in image header");
		if (fread(cmap,sizeof(byte)*cmaplen,1,fp) != 1)
			perr(HE_MSG,"error reading Sun raster color map");
		setparam(&hd,"cmap",PFBYTE,cmaplen,cmap);
	}
	write_headeru(&hd,argc,argv);
	icb = (ocb+1) & ~1;
	if (icb != ocb) {
		if (depth == 1) {
			setsize(&hd,rows,cols+8);
			setroi(&hd,0,0,rows,cols);
			alloc_imagez(&hd);
		}
		else if (depth == 8) {
			setsize(&hd,rows,cols+1);
			setroi(&hd,0,0,rows,cols);
			alloc_imagez(&hd);
		}
		else
			hd.image = halloc(icb*rows,sizeof(byte));
	}
	else
		alloc_image(&hd);
	if (ENCflag) {
		p = hd.image;
		while ((ichar = getc(fp)) != EOF) {
			if (ichar != 128)
				*p++ = ichar;
			else {
				if ((n = getc(fp)) == EOF)
					perr(HE_MSG,"nonsense encoded file");
				if (n == 0)
					*p++ = 128;
				else {
					if ((m = getc(fp)) == EOF)
						perr(HE_MSG,
						    "nonsense encoded file");
					for (i=0;i<=n;i++)
						*p++ = m;
				}
			}
		}
	}
	else {
		if (fread(hd.image,rows*icb,1,fp) != 1)
			perr(HE_READFRFILE,0,filename);
	}
	if (!CMAPflag && depth == 1)
		h_neg(&hd,&hd);
	if (icb == ocb)
		write_image(&hd,0);
	else if (depth == 24) {
		p = hd.image;
		for (r=0;r<rows;r++) {
			if (fwrite(p,ocb*sizeof(byte),1,stdout) != 1)
				perr(HE_WRITEFR,1);
			p += icb;
		}
	}
	else
		write_roi(&hd,0);
	exit(0);
}

void swbytes(p)

char *p;

{
	char savep;

	savep = p[0];
	p[0] = p[3];
	p[3] = savep;
	savep = p[1];
	p[1] = p[2];
	p[2] = savep;
}
