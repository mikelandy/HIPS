/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * pshalftone.c - display images as halftones on a Postscript device
 *
 * usage:	pshalftone [-h imageheight]
 * 			   [-w imagewidth]
 *			   [-l]
 *			   [-F screenfrequency]
 *			   [-A screenangle]
 *                         [-I]
 *			   [-n]
 *                         [-B] < imagefile | lpr -Ppostscript
 *
 * The -h and -w flags specify the output halftone dimensions in inches.  They
 * default to an aspect ratio equal to that of the images (assuming square
 * pixels), and if neither is specified, so that the maximum dimension is
 * 5 inches.  The screen can either be a circle screen (the default) or a
 * line screen (-l). The screen frequency (in screen cells per inch)
 * is set by -F; the screen angle in degrees is set by -A.  The
 * defaults are 60 cells per inch and 45 degrees. These defaults
 * provide 25 gray levels on a 300 DPI printer.  However, if neither -l, -A
 * nor -F is specified, then the printer's default (or default set by
 * previous PostScript code sent to the printer) is used. Grey levels are mapped
 * linearly to percent of dots painted.  HIPS routines may be used to modify
 * the mapping prior to this. This filter works for byte and bit-packed
 * (MSBFIRST) images.  Each frame is centered on a separate output page.
 *
 * For byte images, if the header includes a colormap for pseudocolor, it
 * will be used.  The header colormap will be ignored, however, if -I is
 * specified.
 *
 * For byte images, if the value of numcolor is 3, the file will be treated as
 * a true 24-bit color image and output as such.  Thus, you should only use
 * pshalftone with such images if the output device can handle 24-bit color
 * and in particular it must support the PostScript Level 2 colorimage
 * operator.  The images will be printed as 3 separate images, however, if -I
 * is specified.  24-bit color is also the result if pshalftone is applied to
 * images in PFRGB and PFRGBZ formats.
 *
 * The -B flag uses binary encoding for the image.  This results in an output
 * file which is one half the usual size.  However, most line printer
 * spoolers do not handle 8-bit data correctly, which will break this option.
 * Thus, use the -B flag only if your method of sending the resulting
 * PostScript to your output device is 8-bit safe.  This also requires a
 * printer which can handle PostScript Level 2.
 *
 * The output is Encapsulated PostScript, but includes a showpage command.
 * The -n switch eliminates the showpage command from the output.
 *
 * pixel formats handled directly: MSBF, BYTE, RGB, RGBZ
 *
 * to load:	cc -o pshalftone pshalftone.c -lhips -lm
 *
 * Michael Landy/Jeff Fookson - 1/4/87
 * Hips 2 - msl - 8/2/91
 * Color capabilities - jf/msl - 11/2/92
 * cleaned up PostScript document structuring, made EPS output - msl - 7/12/94
 */

#include <hipl_format.h>
#include <stdio.h>

static Flag_Format flagfmt[] = {
	{"h",{LASTFLAG},1,{{PTDOUBLE,"-1.","imageheight"},LASTPARAMETER}},
	{"w",{LASTFLAG},1,{{PTDOUBLE,"-1.","imagewidth"},LASTPARAMETER}},
	{"A",{LASTFLAG},1,{{PTBOOLEAN,"FALSE"},{PTDOUBLE,"45.","screenangle"},
		LASTPARAMETER}},
	{"F",{LASTFLAG},1,{{PTBOOLEAN,"FALSE"},
		{PTDOUBLE,"60.","screenfrequency"},LASTPARAMETER}},
	{"I",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}}, 
	{"B",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"l",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"n",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	LASTFLAG};
int types[] = {PFMSBF,PFBYTE,PFRGB,PFRGBZ,LASTTYPE};

char prologstuff[] = "\
%%%%EndComments\n\
%%%%BeginProlog\n\
%%%%BeginResource: pshalftone\n\
/pshalftone 50 dict def pshalftone begin\n\
/inch {72 mul}def\n\
/setimstr {/cols exch def /imstr cols string def}def\n\
/placeim {/sy exch def /sx exch def sx inch sy inch translate}def\n\
/scaleim { /ih exch def /iw exch def\n\
	   iw inch cvi ih inch cvi scale\n\
	}def\n\
/setcirc {/ang exch def /freq exch def freq ang\n\
	{dup mul exch dup mul add 1 exch sub}\n\
	setscreen}def\n\
/setline {/ang exch def /freq exch def freq ang\n\
	{pop abs 1 exch sub}\n\
	setscreen}def\n\
%%%%EndResource\n\
%%%%EndProlog\n";
long time();
char *ctime();

int main(argc,argv)

int argc;
char **argv;

{
	byte *pcmap;
	int i,j,k,r,c,f,fr,method,ncb,nex,numcol,nbytes;
	h_boolean Aflag,Fflag,lflag,packing,Iflag,Bflag,pseudo,fullcolor,nflag;
	float sx,sy;
	double sf,sa,ih,iw;
	char dateline[80];
	byte *ipr,*ipg,*ipb,*op;
	struct header hd,hdpr,hdpg,hdpb;
	Filename filename;
	FILE *fp;
	long tm;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&ih,&iw,&Aflag,&sa,&Fflag,&sf,&Iflag,
		&Bflag,&lflag,&nflag,FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	method = fset_conversion(&hd,&hdpr,types,filename);
	packing = (hdpr.pixel_format == PFMSBF);
	r = hd.rows;
	c = hd.cols;
	f = hdpr.num_frame;
	fullcolor = (hdpr.pixel_format == PFRGB || hdpr.pixel_format == PFRGBZ
		|| (hdpr.pixel_format == PFBYTE && hdpr.numcolor == 3 &&
		    !Iflag));
	pseudo = (hdpr.pixel_format == PFBYTE &&
		(findparam(&hdpr,"cmap") != NULLPAR) &&
		!Iflag);
	if (pseudo && fullcolor) {
		perr(HE_IMSG,
		 "both full and pseudocolor not supported, assuming fullcolor");
		pseudo = FALSE;
	}
	if (fullcolor && hdpr.pixel_format == PFBYTE) {
		dup_headern(&hdpr,&hdpg);
		alloc_image(&hdpg);
		dup_headern(&hdpr,&hdpb);
		alloc_image(&hdpb);
		f /= 3;
	}
	if (packing) {
		ncb = (c+7)/8;
		nex = ((hd.ocols+7)/8) - ncb;
	}
	else {
		ncb = hdpr.sizepix * c;
		nex = hdpr.sizepix * (hd.ocols - c);
	}
	if (ih < 0) {
		if (iw < 0) {
			if (r > c) {
				ih = 5.;
				iw = ih * c / r;
			}
			else {
				iw = 5.;
				ih = iw * r / c;
			}
		}
		else
			ih = iw * r / c;
	}
	else if (iw < 0)
		iw = ih * c / r;
	fprintf(stderr,"Printed image size will by %.2f by %.2f\n",ih,iw);
	fprintf(stderr,"Image contains %d rows and %d columns\n",r,c);
	if (lflag || Aflag || Fflag)
		fprintf(stderr,
		"%s screen:  %.2f cells per inch, angle %.2f degrees\n",
		lflag?"Line":"Circle",sf,sa);
	else
	    fprintf(stderr,
	    "No halftone screen specified, using printer or prior default\n");
	if (ih > 11 || iw > 8.5)
		perr(HE_IMSG,"image larger than 8.5 x 11 inches");
	sx = 4.25 - iw / 2.;
	sy = 5.5 - ih / 2.;

/* output prologue */

	printf("%%!PS-Adobe-3.0 EPSF-3.0\n");
	printf("%%%%BoundingBox: %d %d %d %d\n",
		(int) (72*sx+.5),
		(int) (72*sy+.5),
		(int) (72*(sx+iw)+.5),
		(int) (72*(sy+ih)+.5));
	printf("%%%%Creator: pshalftone\n");
	printf("%%%%Title: %s\n",filename);
	tm = time(0);
	strcpy(dateline,ctime(&tm));
	i = strlen(dateline);
	if (i > 0 && dateline[i-1] == '\n')
		dateline[i-1] = 0;
	printf("%%%%CreationDate: (%s)\n",dateline);
	printf("%%%%Pages: %d\n",f);
	printf("%s",prologstuff);


/* get colormap if pseudocolor */

	if (pseudo) {
		numcol=768;
        	getparam(&hd,"cmap",PFBYTE,&numcol,&pcmap);
		if (numcol % 3)
                        perr(HE_MSG,"colormap length not a multiple of 3");
                numcol /= 3;
	}

/* read and output single frame at a time */

	for (fr=0;fr<f;fr++) { 
		printf("%%%%Page: %d %d\n",fr+1,fr+1);
		printf("%%%%BeginPageSetup\n");
		printf("/pgsave save def\n");
		printf("%%%%EndPageSetup\n");
		if (fullcolor && hdpr.pixel_format == PFBYTE) {
			fread_imagec(fp,&hd,&hdpr,method,f,filename);
			fread_imagec(fp,&hd,&hdpg,method,f,filename);
			fread_imagec(fp,&hd,&hdpb,method,f,filename);
		}
		else
			fread_imagec(fp,&hd,&hdpr,method,f,filename);

/*set screen if necessary */

		if (lflag || Aflag || Fflag) {
			if (lflag)
				printf("%f %f setline\n",sf,sa);
			else
				printf("%f %f setcirc\n",sf,sa);
		}
		if (!(Bflag || pseudo))
			printf("%d setimstr\n",fullcolor ? c*3 : ncb);

/* position image, scale and output single page data */

		printf("%f %f placeim\n",sx,sy);
		printf("%f %f scaleim\n",iw,ih);

/* Set up color space if necessary */

		if (pseudo) {
			printf("[/Indexed /DeviceRGB %d <\n",numcol-1);
			for (i=0;i<numcol;i++) {
				printf((i == (numcol-1) || ((i+1)%8) == 0)
					? "%02x%02x%02x\n"
					: "%02x%02x%02x ",
					(int) pcmap[i],
					(int) pcmap[numcol+i],
					(int) pcmap[2*numcol+i]);
			}
			printf(">] setcolorspace\n");
		}
		else if (Bflag) {
			if (fullcolor)
				printf("/DeviceRGB setcolorspace\n");
			else
				printf("/DeviceGray setcolorspace\n");
		}

/* output image operator */

		if (Bflag || pseudo) {
			printf("<<\n/ImageType 1\n/Width %d\n/Height %d\n",c,r);
#ifdef ULORIG
			printf("/ImageMatrix [%d 0 0 %d 0 %d]\n",c,-r,r);
#else
			printf("/ImageMatrix [%d 0 0 %d 0 0]\n",c,r);
#endif
			printf("/MultipleDataSources false\n");
			if (Bflag) {
				printf("/DataSource currentfile 0 ");
				printf("(%%%%EndData) /SubFileDecode ");
				printf("filter\n");
			}
			else {
				printf("/DataSource currentfile ");
				printf("/ASCIIHexDecode filter\n");
			}
			printf("/BitsPerComponent %d\n",packing ? 1 : 8);
			if (pseudo)
				printf("/Decode [0 255]\n");
			else if (fullcolor)
				printf("/Decode [0 1 0 1 0 1]\n");
			else
				printf("/Decode [0 1]\n");
			printf(">>\n");
			if (Bflag) {
				if (fullcolor)
					nbytes = r*c*3 + 7;
				else
					nbytes = r*ncb + 7;
				printf("%%%%BeginData: %d Binary Bytes\n",
					nbytes);
			}
			else {	/* pseudo, not binary */
				nbytes = 2*r*c + ((int) ((r*c+35)/36)) + 6;
				printf("%%%%BeginData: %d Hex Bytes\n",
					nbytes);
			}
			printf("image\n");
		}
		else if (fullcolor) {
#ifdef ULORIG
			printf("%d %d 8 [%d 0 0 %d 0 %d]\n",c,r,c,-r,r);
#else
			printf("%d %d 8 [%d 0 0 %d 0 0]\n",c,r,c,r);
#endif
			printf("\t{currentfile imstr readhexstring pop}\n");
			printf("\tfalse 3\n");
			nbytes = 6*r*c + ((int) ((r*c+11)/12)) + 11;
			printf("%%%%BeginData: %d Hex Bytes\n",nbytes);
			printf("colorimage\n");
		}
		else {
#ifdef ULORIG
			printf("%d %d %d [%d 0 0 %d 0 %d]",c,r,
				packing ? 1 : 8,c,-r,r);
#else
			printf("%d %d %d [%d 0 0 %d 0 0]",c,r,
				packing ? 1 : 8,c,r);
#endif
			printf(" {currentfile imstr readhexstring pop}\n");
			nbytes = 2*r*ncb + ((int) ((r*ncb+35)/36)) + 6;
			printf("%%%%BeginData: %d Hex Bytes\n",nbytes);
			printf("image\n");
		}

/*output image*/

		if (Bflag) {
			if (fullcolor && hdpr.pixel_format == PFBYTE) {
				ipr = hdpr.firstpix;
				ipg = hdpg.firstpix;
				ipb = hdpb.firstpix;
				for (i=0;i<r;i++) {
					for (j=0;j<c;j++) {
						putchar(*ipr++);
						putchar(*ipg++);
						putchar(*ipb++);
					}
					ipr += nex;
					ipg += nex;
					ipb += nex;
				}
			}
			else if (hdpr.pixel_format == PFRGBZ) {
				op = hdpr.image;
				ipr = hdpr.firstpix;
				for (i=0;i<r;i++) {
					for (j=0;j<c;j++) {
						*op++ = *ipr++;
						*op++ = *ipr++;
						*op++ = *ipr++;
						ipr++;
					}
					ipr += nex;
				}
				fwrite(hdpr.image,3*r*c,1,stdout);
			}
			else if (nex == 0)
				fwrite(hdpr.firstpix,r*ncb,1,stdout);
			else if (hdpr.pixel_format == PFRGB) {
				op = hdpr.image;
				ipr = hdpr.firstpix;
				for (i=0;i<r;i++) {
					for (j=0;j<c;j++) {
						*op++ = *ipr++;
						*op++ = *ipr++;
						*op++ = *ipr++;
					}
					ipr += nex;
				}
				fwrite(hdpr.image,r*ncb,1,stdout);
			}
			else {
				op = hdpr.image;
				ipr = hdpr.firstpix;
				for (i=0;i<r;i++) {
					for (j=0;j<ncb;j++)
						*op++ = *ipr++;
					ipr += nex;
				}
				fwrite(hdpr.image,r*ncb,1,stdout);
			}
		}
		else {
			if (fullcolor && hdpr.pixel_format == PFBYTE) {
				ipr = hdpr.firstpix;
				ipg = hdpg.firstpix;
				ipb = hdpb.firstpix;
				k = 0;
				for (i=0;i<r;i++) {
					for (j=0;j<c;j++) {
						printf("%02x",*ipr++);
						printf("%02x",*ipg++);
						printf("%02x",*ipb++);
						if ((k++ % 12) == 11)
							printf("\n");
					}
					ipr += nex;
					ipg += nex;
					ipb += nex;
				}
				if (((r*c)%12))
					printf("\n");
			}
			else if (hdpr.pixel_format == PFRGBZ) {
				ipr = hdpr.firstpix;
				k = 0;
				for (i=0;i<r;i++) {
					for (j=0;j<c;j++) {
						printf("%02x",*ipr++);
						printf("%02x",*ipr++);
						printf("%02x",*ipr++);
						ipr++;
						if ((k++ % 12) == 11)
							printf("\n");
					}
					ipr += nex;
				}
				if (((r*c)%12))
					printf("\n");
			}
			else if (hdpr.pixel_format == PFRGB) {
				ipr = hdpr.firstpix;
				k = 0;
				for (i=0;i<r;i++) {
					for (j=0;j<c;j++) {
						printf("%02x",*ipr++);
						printf("%02x",*ipr++);
						printf("%02x",*ipr++);
						if ((k++ % 12) == 11)
							printf("\n");
					}
					ipr += nex;
				}
				if (((r*c)%12))
					printf("\n");
			}
			else {
				k = 0;
				ipr = hdpr.firstpix;
				for (i=0;i<r;i++) {
					for (j=0;j<ncb;j++) {
						printf("%02x",*ipr++);
						if ((k++ % 36) == 35)
							printf("\n");
					}
					ipr += nex;
				}
				if (((r*ncb)%36))
					printf("\n");
			}
		}
		if (Bflag)
			printf("\n");
		printf("%%%%EndData\n");

/*now showpage*/

		printf("pgsave restore\n");
		if (!nflag)
			printf("showpage\n");
	}
	printf("%%%%Trailer\nend\n");
	return(0);
}
