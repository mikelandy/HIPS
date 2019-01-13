/*	Copyright (c) 1992 Jens Michael Carstensen, IMSOR

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 * frarithmetic.c - perform arithmetic functions on the input sequence
 *		using a file sequence or a constant.
 *	   	Sequences must be in byte, short, integer, float, double, 
 *		complex, or double complex format.
 *
 * This program has been made to work with the HIPS-2 software.
 *
 * usage:	fradd	( file | - constant ) < iseq > oseq
 * 		frsub	( file | - constant ) < iseq > oseq
 * 		frmul	( file | - constant ) < iseq > oseq
 * 		frdiv	( file | - constant ) < iseq > oseq
 * 		frmin	( file | - constant ) < iseq > oseq
 * 		frmax	( file | - constant ) < iseq > oseq
 * 		frdif	( file | - constant ) < iseq > oseq
 *
 * to load:	cc -o fradd frarithmetic.c -lhips
 * 
 */

#define	Byte		unsigned char

#define FRADD		1
#define FRSUB		2
#define FRMUL		3
#define FRDIV		4
#define FRMIN		5
#define FRMAX		6
#define FRDIF		7

#include <string.h>
#include <hipl_format.h>
#include <stdio.h>
#include <math.h>

int main (argc,argv)
int argc;
char **argv;
{
	struct header 	hd, fhd;
	FILE		*filep;
	int 		f,i,nrows,ncols,npix,pixfmt,pixsize,nr,noread=0;
	int		inum, frop;
	double		fnum;
	Byte		*bpic,*bfpic,*bp,*bfp;
	short		*spic,*sfpic,*sp,*sfp;
	int 		*ipic,*ifpic,*ip,*ifp;
	float 		*fpic,*ffpic,*fp,*ffp;
	double 		*dpic,*dfpic,*dp,*dfp;

	Progname = strsave(*argv);
	if (argc<2) perr(HE_MSG,"argument missing");

        if (strcmp(argv[0], "fradd") == 0) frop=FRADD;
        if (strcmp(argv[0], "frsub") == 0) frop=FRSUB;
        if (strcmp(argv[0], "frmul") == 0) frop=FRMUL;
        if (strcmp(argv[0], "frdiv") == 0) frop=FRDIV;
        if (strcmp(argv[0], "frmin") == 0) frop=FRMIN;
        if (strcmp(argv[0], "frmax") == 0) frop=FRMAX;
        if (strcmp(argv[0], "frdif") == 0) frop=FRDIF;

	read_header(&hd);
	nrows=hd.orows;
	ncols=hd.ocols;
	npix =nrows*ncols;
	pixfmt=hd.pixel_format;
	if (pixfmt == PFCOMPLEX || pixfmt == PFDBLCOM) npix *= 2;

	if (argc>2) {
	if (pixfmt == PFBYTE || pixfmt == PFSHORT || pixfmt == PFINT)
		inum=atoi(argv[2]);
	else 
		fnum=atof(argv[2]);
	noread=1;
	}

	if (noread == 0) {
	if ((filep = fopen(argv[1],"r")) == NULL)
		perr(HE_MSG,"can't open file : %s",argv[1]);
	fread_header(filep,&fhd,argv[1]);
	if (hd.orows != fhd.orows || hd.ocols != fhd.ocols)
		perr(HE_MSG,"images must be of equal size");
	if (pixfmt != fhd.pixel_format)
		perr(HE_MSG,"images must be of equal type");
	if (fhd.num_frame != hd.num_frame && fhd.num_frame != 1)
		perr(HE_MSG,"bad number of frames in file");
	nr=fhd.num_frame;
	}
	else nr=0;

	update_header(&hd,argc,argv);
	write_header(&hd);

	switch (pixfmt) {
	case PFBYTE:
		pixsize=sizeof(Byte);
		bpic = (Byte *) halloc(npix,pixsize);
		hd.image=(Byte *) bpic;
		if (nr) {
			bfpic = (Byte *) halloc(npix,pixsize);
			fhd.image=(Byte *) bfpic;
		}
		for (f=0;f<hd.num_frame;f++) {
			read_image(&hd,f);
			if (f<nr) fread_image(filep,&fhd,f,argv[1]); 
			switch (frop) {
			case FRADD:
				if (noread)
					for (i=0, bp=bpic;i<npix;i++)
						*bp++ += inum;
				else
					for (i=0, bp=bpic, bfp=bfpic;i<npix;i++)
						*bp++ += *bfp++;
				break;
			case FRSUB: case FRDIF:
				if (noread)
					for (i=0, bp=bpic;i<npix;i++) {
						*bp = ABS((int)*bp-inum);
						bp++;
					}
				else
					for (i=0, bp=bpic, bfp=bfpic;i<npix;i++)
						{*bp = ABS((int)(*bp)-(int)(*bfp)); bp++; bfp++;}
				break;
			case FRMUL:
				if (noread)
					for (i=0, bp=bpic;i<npix;i++)
						*bp++ *= inum;
				else
					for (i=0, bp=bpic, bfp=bfpic;i<npix;i++)
						*bp++ *= *bfp++;
				break;
			case FRDIV:
				if (noread)
					for (i=0, bp=bpic;i<npix;i++)
						*bp++ /= inum;
				else
					for (i=0, bp=bpic, bfp=bfpic;i<npix;i++)
						*bp++ /= *bfp++;
				break;
			case FRMIN:
				if (noread)
					for (i=0, bp=bpic;i<npix;i++) {
						*bp = MIN(*bp,inum);
						bp++;
					}
				else
					for (i=0,bp=bpic,bfp=bfpic;i<npix;i++) {
						*bp = MIN(*bp,*bfp);
						bfp++; bp++;
					}
				break;
			case FRMAX:
				if (noread)
					for (i=0, bp=bpic;i<npix;i++) {
						*bp = MAX(*bp,inum);
						bp++;
					}
				else
					for (i=0,bp=bpic,bfp=bfpic;i<npix;i++) {
						*bp = MAX(*bp,*bfp);
						bfp++; bp++;
					}
				break;
			}
			write_image(&hd,f);
		}
		break;
	case PFSHORT:
		pixsize=sizeof(short int);
		spic = (short int *) halloc(npix,pixsize);
		hd.image=(Byte *) spic;
		if (nr) {
			sfpic = (short int *) halloc(npix,pixsize);
			fhd.image=(Byte *) sfpic;
		}
		for (f=0;f<hd.num_frame;f++) {
			read_image(&hd,f);
			if (f<nr) fread_image(filep,&fhd,f,argv[1]); 
			switch (frop) {
			case FRADD:
				if (noread)
					for (i=0, sp=spic;i<npix;i++)
						*sp++ += inum;
				else
					for (i=0, sp=spic, sfp=sfpic;i<npix;i++)
						*sp++ += *sfp++;
				break;
			case FRSUB:
				if (noread)
					for (i=0, sp=spic;i<npix;i++)
						*sp++ -= inum;
				else
					for (i=0, sp=spic, sfp=sfpic;i<npix;i++)
						*sp++ -= *sfp++;
				break;
			case FRDIF:
				if (noread)
					for (i=0, sp=spic;i<npix;i++) {
						*sp = ABS(*sp-inum);
						sp++;
					}
				else
					for (i=0,sp=spic,sfp=sfpic;i<npix;i++) {
						*sp = ABS(*sp-*sfp);
						sfp++; sp++;
					}
				break;
			case FRMUL:
				if (noread)
					for (i=0, sp=spic;i<npix;i++)
						*sp++ *= inum;
				else
					for (i=0, sp=spic, sfp=sfpic;i<npix;i++)
						*sp++ *= *sfp++;
				break;
			case FRDIV:
				if (noread)
					for (i=0, sp=spic;i<npix;i++)
						*sp++ /= inum;
				else
					for (i=0, sp=spic, sfp=sfpic;i<npix;i++)
						*sp++ /= *sfp++;
				break;
			case FRMIN:
				if (noread)
					for (i=0, sp=spic;i<npix;i++) {
						*sp = MIN(*sp,inum);
						sp++;
					}
				else
					for (i=0,sp=spic,sfp=sfpic;i<npix;i++) {
						*sp = MIN(*sp,*sfp);
						sfp++; sp++;
					}
				break;
			case FRMAX:
				if (noread)
					for (i=0, sp=spic;i<npix;i++) {
						*sp = MAX(*sp,inum);
						sp++;
					}
				else
					for (i=0,sp=spic,sfp=sfpic;i<npix;i++) {
						*sp = MAX(*sp,*sfp);
						sfp++; sp++;
					} 
				break;
			}
			write_image(&hd,f);
		}
		break;
	case PFINT:
		pixsize=sizeof(int);
		ipic = (int *) halloc(npix,pixsize);
		hd.image=(Byte *) ipic;
		if (nr) {
			ifpic = (int *) halloc(npix,pixsize);
			fhd.image=(Byte *) ifpic;
		}
		for (f=0;f<hd.num_frame;f++) {
			read_image(&hd,f);
			if (f<nr) fread_image(filep,&fhd,f,argv[1]); 
			switch (frop) {
			case FRADD:
				if (noread)
					for (i=0, ip=ipic;i<npix;i++)
						*ip++ += inum;
				else
					for (i=0, ip=ipic, ifp=ifpic;i<npix;i++)
						*ip++ += *ifp++;
				break;
			case FRSUB:
				if (noread)
					for (i=0, ip=ipic;i<npix;i++)
						*ip++ -= inum;
				else
					for (i=0, ip=ipic, ifp=ifpic;i<npix;i++)
						*ip++ -= *ifp++;
				break;
			case FRDIF:
				if (noread)
					for (i=0, ip=ipic;i<npix;i++) {
						*ip = ABS(*ip-inum);
						ip++;
					}
				else
					for (i=0,ip=ipic,ifp=ifpic;i<npix;i++) {
						*ip = ABS(*ip-*ifp);
						ifp++; ip++;
					}
				break;
			case FRMUL:
				if (noread)
					for (i=0, ip=ipic;i<npix;i++)
						*ip++ *= inum;
				else
					for (i=0, ip=ipic, ifp=ifpic;i<npix;i++)
						*ip++ *= *ifp++;
				break;
			case FRDIV:
				if (noread)
					for (i=0, ip=ipic;i<npix;i++)
						*ip++ /= inum;
				else
					for (i=0, ip=ipic, ifp=ifpic;i<npix;i++)
						*ip++ /= *ifp++;
				break;
			case FRMIN:
				if (noread)
					for (i=0, ip=ipic;i<npix;i++) {
						*ip = MIN(*ip,inum);
						ip++;
					}
				else
					for (i=0,ip=ipic,ifp=ifpic;i<npix;i++) {
						*ip = MIN(*ip,*ifp);
						ifp++; ip++;
					}
				break;
			case FRMAX:
				if (noread)
					for (i=0, ip=ipic;i<npix;i++) {
						*ip = MAX(*ip,inum);
						ip++;
					}
				else
					for (i=0,ip=ipic,ifp=ifpic;i<npix;i++) {
						*ip = MAX(*ip,*ifp);
						ifp++; ip++;
					}
				break;
			}
			write_image(&hd,f);
		}
		break;
	case PFFLOAT: case PFCOMPLEX:
		pixsize=sizeof(float);
		fpic = (float *) halloc(npix,pixsize);
		hd.image=(Byte *) fpic;
		if (nr) {
			ffpic = (float *) halloc(npix,pixsize);
			fhd.image=(Byte *) ffpic;
		}
		for (f=0;f<hd.num_frame;f++) {
			read_image(&hd,f);
			if (f<nr) fread_image(filep,&fhd,f,argv[1]); 
			switch (frop) {
			case FRADD:
				if (noread)
					for (i=0, fp=fpic;i<npix;i++)
						*fp++ += fnum;
				else
					for (i=0, fp=fpic, ffp=ffpic;i<npix;i++)
						*fp++ += *ffp++;
				break;
			case FRSUB:
				if (noread)
					for (i=0, fp=fpic;i<npix;i++)
						*fp++ -= fnum;
				else
					for (i=0, fp=fpic, ffp=ffpic;i<npix;i++)
						*fp++ -= *ffp++;
				break;
			case FRDIF:
				if (noread)
					for (i=0, fp=fpic;i<npix;i++) {
						*fp = ABS(*fp-fnum);
						fp++;
					}
				else
					for (i=0,fp=fpic,ffp=ffpic;i<npix;i++) {
						*fp = ABS(*fp-*ffp);
						ffp++; fp++;
					}
				break;
			case FRMUL:
				if (noread)
					for (i=0, fp=fpic;i<npix;i++)
						*fp++ *= fnum;
				else
					for (i=0, fp=fpic, ffp=ffpic;i<npix;i++)
						*fp++ *= *ffp++;
				break;
			case FRDIV:
				if (noread)
					for (i=0, fp=fpic;i<npix;i++)
						*fp++ /= fnum;
				else
					for (i=0, fp=fpic, ffp=ffpic;i<npix;i++)
						*fp++ /= *ffp++;
				break;
			case FRMIN:
				if (noread)
					for (i=0, fp=fpic;i<npix;i++) {
						*fp = MIN(*fp,fnum);
						fp++;
					}
				else
					for (i=0,fp=fpic,ffp=ffpic;i<npix;i++) {
						*fp = MIN(*fp,*ffp);
						ffp++; fp++;
					}
				break;
			case FRMAX:
				if (noread)
					for (i=0, fp=fpic;i<npix;i++) {
						*fp = MAX(*fp,fnum);
						fp++;
					}
				else
					for (i=0,fp=fpic,ffp=ffpic;i<npix;i++) {
						*fp = MAX(*fp,*ffp);
						ffp++; fp++;
					}
				break;
			}
			write_image(&hd,f);
		}
		break;
	case PFDOUBLE: case PFDBLCOM:
		pixsize=sizeof(double);
		dpic = (double *) halloc(npix,pixsize);
		hd.image=(Byte *) dpic;
		if (nr) {
			dfpic = (double *) halloc(npix,pixsize);
			fhd.image=(Byte *) dfpic;
		}
		for (f=0;f<hd.num_frame;f++) {
			read_image(&hd,f);
			if (f<nr) fread_image(filep,&fhd,f,argv[1]); 
			switch (frop) {
			case FRADD:
				if (noread)
					for (i=0, dp=dpic;i<npix;i++)
						*dp++ += fnum;
				else
					for (i=0, dp=dpic, dfp=dfpic;i<npix;i++)
						*dp++ += *dfp++;
				break;
			case FRSUB:
				if (noread)
					for (i=0, dp=dpic;i<npix;i++)
						*dp++ -= fnum;
				else
					for (i=0, dp=dpic, dfp=dfpic;i<npix;i++)
						*dp++ -= *dfp++;
				break;
			case FRDIF:
				if (noread)
					for (i=0, dp=dpic;i<npix;i++) {
						*dp = ABS(*dp-fnum);
						dp++;
					}
				else
					for (i=0,dp=dpic,dfp=dfpic;i<npix;i++) {
						*dp = ABS(*dp-*dfp);
						dfp++; dp++;
					}
				break;
			case FRMUL:
				if (noread)
					for (i=0, dp=dpic;i<npix;i++)
						*dp++ *= fnum;
				else
					for (i=0, dp=dpic, dfp=dfpic;i<npix;i++)
						*dp++ *= *dfp++;
				break;
			case FRDIV:
				if (noread)
					for (i=0, dp=dpic;i<npix;i++)
						*dp++ /= fnum;
				else
					for (i=0, dp=dpic, dfp=dfpic;i<npix;i++)
						*dp++ /= *dfp++;
				break;
			case FRMIN:
				if (noread)
					for (i=0, dp=dpic;i<npix;i++) {
						*dp = MIN(*dp,fnum);
						dp++;
					}
				else
					for (i=0,dp=dpic,dfp=dfpic;i<npix;i++) {
						*dp = MIN(*dp,*dfp);
						dfp++; dp++;
					}
				break;
			case FRMAX:
				if (noread)
					for (i=0, dp=dpic;i<npix;i++) {
						*dp = MAX(*dp,fnum);
						dp++;
					}
				else
					for (i=0,dp=dpic,dfp=dfpic;i<npix;i++) {
						*dp = MAX(*dp,*dfp);
						dfp++; dp++;
					}
				break;
			}
			write_image(&hd,f);
		}
		break;
	} /* End switch */
	return(0);
}
