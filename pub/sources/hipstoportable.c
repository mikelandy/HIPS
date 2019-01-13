
/*
 * This file is considered to be public domain software.  We hereby give
 * permission for anyone to make any use of this code, including copying the
 * code, including it with freely distributed software, including it with
 * commercially available software, and including it in ftp-able code.
 * We do not assert that this software is completely bug-free (although we hope
 * it is), and we do not support the software (officially) in any way.  The
 * intention is to make it possible for people to read and write standard HIPS
 * formatted image sequences, and write conversion programs to other formats,
 * without owning a license for HIPS-proper.  However, we do require that all
 * distributed copies of these source files include the following copyright
 * notice.
 *
 ******************************************************************************
 *
 * Copyright (c) 1995 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 *
 ******************************************************************************
 */

/*
 * hipstoportable - Convert from HIPS format to various portable formats
 *
 * Usage:	hipstoportable [-p] [-c] < hipsfile > portablefile
 *
 * Hipstoportable converts from HIPS format to a variety of Jef Poskanzer's
 * portable formats.  This allows one to then use Poskanzer's routines to
 * convert to a variety of other standard image formats.  This program does
 * no automatic conversion between different HIPS image formats.  This program
 * only accepts input files in the standard input.  Here are the conversions
 * that it performs:
 *
 *	Input pixel format		Output format
 *
 *	MSBF packed			PBM (raw or plain)
 *	byte				PGM (raw or plain)
 *	short				PGM (plain only)
 *	int				PGM (plain only)
 *	RGB				PPM (raw or plain)
 *
 * For short and int formats, values less than zero are clipped (and a message
 * to that effect is produced).  For MSBF format, the code assumes that each
 * row begins on a byte boundary (which is not stated explicitly in the PBM
 * documentation but is true of the PBM library code).
 *
 * Switches:
 *	-p	Force the use of plain (not raw) format in the output.  This
 *		is basically an ASCII format, and hence is vastly slower to
 *		work with in subsequent processing
 *
 *	-c	Collapse a multiple frame HIPS image into a single frame on
 *		the output (with numberofrows*numberofframes rows).  The
 *		default is to output only the first frame of a HIPS sequence,
 *		omitting the others.
 *
 * Michael Landy - 4/13/95
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Pixel Format Codes (raster formats only)
 */

#define	PFBYTE		0	/* Bytes interpreted as unsigned integers */
#define PFSHORT		1	/* Short integers (2 bytes) */
#define PFINT		2	/* Integers (4 bytes) */
#define	PFMSBF		30	/* packed, most-significant-bit first */
#define	PFLSBF		31	/* packed, least-significant-bit first */
#define	PFRGB		35	/* RGBRGBRGB bytes */

#define	LINELENGTH 200		/* max characters per line in header vars */

#define MSBFIRST 1	/* HIPS-1 bit_packing value for PFMSBF */
#define LSBFIRST 2	/* HIPS-1 bit_packing value for PFLSBF */

int format,rows,cols,frames,colors,pflag=0,cflag=0;
char usage[] = "Usage: hipstoportable [-c] [-p] < hipsfile > portablefile\n";
void hpub_rdhdr(),htop_b(),htop_s(),htop_i(),htop_mp(),htop_rgb(),hpub_frdhdr();
void hpub_fgets(),hpub_frdoldhdr(),hpub_perr(),hpub_swallownl();

int main(argc,argv)

int argc;
char **argv;

{
	int i;

	for (i=1;i<argc;i++) {
		if (argv[i][0] == '-') {
			switch(argv[i][1]) {
			case 'c':	cflag++; continue;
			case 'p':	pflag++; continue;
			default:	fprintf(stderr,"%s\n",usage);
					exit(1);
			}
		}
		else {
			fprintf(stderr,"%s\n",usage);
			exit(1);
		}
	}
	hpub_rdhdr(&format,&rows,&cols,&frames,&colors);
	if (cflag)
		rows *= frames;
	switch(format) {
	case PFBYTE:	htop_b(); break;
	case PFSHORT:	htop_s(); break;
	case PFINT:	htop_i(); break;
	case PFMSBF:	htop_mp(); break;
	case PFRGB:	htop_rgb(); break;
	default:	fprintf(stderr,
				"hipstoportable: illegal input format\n");
			exit(1);
	}
	return(0);
}

/*
 * hpub_rdhdr.c - HIPS Picture Format Header read (public domain version)
 *
 * Michael Landy - 9/30/92
 */

void hpub_rdhdr(format,rows,cols,frames,colors)

int *format,*rows,*cols,*frames,*colors;

{
	hpub_frdhdr(stdin,format,rows,cols,frames,colors);
}

void hpub_frdhdr(fp,format,rows,cols,frames,colors)

FILE *fp;
int *format,*rows,*cols,*frames,*colors;

{
	char inp[LINELENGTH],*buf;
	int i,sz,np;

	hpub_fgets(inp,LINELENGTH,fp);	/* magic */
	if (strcmp(inp,"HIPS\n") != 0) {
		hpub_frdoldhdr(fp,format,rows,cols,frames,colors);
		return;
	}
	hpub_fgets(inp,LINELENGTH,fp);	/* onm */
	hpub_fgets(inp,LINELENGTH,fp);	/* snm */
	if (fscanf(fp,"%d",frames) != 1)
		hpub_perr("error reading number of frames");
	hpub_swallownl(fp);
	hpub_fgets(inp,LINELENGTH,fp);	/* odt */
	if (fscanf(fp,"%d",rows) != 1)
		hpub_perr("error reading number of rows");
	if (fscanf(fp,"%d",cols) != 1)
		hpub_perr("error reading number of cols");
	if (fscanf(fp,"%d",&i) != 1)	/* roirows */
		hpub_perr("error reading header");
	if (fscanf(fp,"%d",&i) != 1)	/* roicols */
		hpub_perr("error reading header");
	if (fscanf(fp,"%d",&i) != 1)	/* frow */
		hpub_perr("error reading header");
	if (fscanf(fp,"%d",&i) != 1)	/* fcol */
		hpub_perr("error reading header");
	if (fscanf(fp,"%d",format) != 1)
		hpub_perr("error reading pixel format");
	if (fscanf(fp,"%d",colors) != 1)
		hpub_perr("error reading number of colors");
	if (fscanf(fp,"%d",&sz) != 1)	/* szhist */
		hpub_perr("error reading header");
	hpub_swallownl(fp);
	if (sz) {
		if ((buf = (char *) malloc(sz)) == (char *) 0)
			hpub_perr("error allocating memory to read header");
		if (fread(buf,sz,1,fp) != 1)
			hpub_perr("error reading header");
		free(buf);
	}
	if (fscanf(fp,"%d",&sz) != 1)	/* szdesc */
		hpub_perr("error reading header");
	hpub_swallownl(fp);
	if (sz) {
		if ((buf = (char *) malloc(sz)) == (char *) 0)
			hpub_perr("error allocating memory to read header");
		if (fread(buf,sz,1,fp) != 1)
			hpub_perr("error reading header");
		free(buf);
	}
	if (fscanf(fp,"%d",&np) != 1)	/* nparam */
		hpub_perr("error reading header");
	hpub_swallownl(fp);
	for (i=0;i<np;i++)
		hpub_fgets(inp,LINELENGTH,fp);
	if (fscanf(fp,"%d",&sz) != 1)
		hpub_perr("error reading header");
	hpub_swallownl(fp);
	if (sz) {
		if ((buf = (char *) malloc(sz)) == (char *) 0)
			hpub_perr("error allocating memory to read header");
		if (fread(buf,sz,1,fp) != 1)
			hpub_perr("error reading header");
		free(buf);
	}
}

/*
 * hpub_frdoldhdr - read old (HIPS-1) format header (public domain version)
 *
 * Michael Landy - 9/30/92
 */

void hpub_frdoldhdr(fp,format,rows,cols,frames,colors)

FILE *fp;
int *format,*rows,*cols,*frames,*colors;

{
	char inp[LINELENGTH];
	int i,bpck;

	/* onm already read */
	hpub_fgets(inp,LINELENGTH,fp);	/* snm */
	if (fscanf(fp,"%d",frames) != 1)
		hpub_perr("error reading number of frames");
	hpub_swallownl(fp);
	hpub_fgets(inp,LINELENGTH,fp);	/* odt */
	if (fscanf(fp,"%d",rows) != 1)
		hpub_perr("error reading number of rows");
	if (fscanf(fp,"%d",cols) != 1)
		hpub_perr("error reading number of cols");
	if (fscanf(fp,"%d",&i) != 1)	/* bpp */
		hpub_perr("error reading header");
	if (fscanf(fp,"%d",&bpck) != 1)	/* bpck */
		hpub_perr("error reading header");
	if (fscanf(fp,"%d",format) != 1)
		hpub_perr("error reading pixel format");
	hpub_swallownl(fp);
	if (*format == PFBYTE && bpck == MSBFIRST)
		*format = PFMSBF;
	else if (*format == PFBYTE && bpck == LSBFIRST)
		*format = PFLSBF;
	*colors = 1;
	hpub_fgets(inp,LINELENGTH,fp);	/* hist */
	while(inp[strlen(inp)-3] == '|')
		hpub_fgets(inp,LINELENGTH,fp);
	while (1) {	/* desc */
		hpub_fgets(inp,LINELENGTH,fp);
		if (strcmp(inp,".\n") == 0)
			return;
	}
}

void hpub_swallownl(fp)

FILE *fp;

{
	int i;

	while ((i = getc(fp)) != '\n') {
		if (i == EOF)
			hpub_perr("error reading input end-of-line");
	}
}

void hpub_fgets(s,n,fp)

char *s;
int n;
FILE *fp;

{
	int i;

	fgets(s,n,fp);
	i=strlen(s);
	if (i==0 || s[i-1]!='\n')
		hpub_perr("error reading input line");
}

/*
 * hpub_perr.c - error printer
 */

void hpub_perr(s)

char *s;

{
	fprintf(stderr,"Hipspub routines: %s\n",s);
	exit(1);
}

void htop_b()

{
	unsigned char *img,*p;
	int i,j,max,nout;

	printf(pflag ? "P2\n" : "P5\n");
	printf("%d %d\n",cols,rows);
	if ((img = (unsigned char *) malloc(rows*cols*sizeof(unsigned char)))
		== 0) {
			fprintf(stderr,
			    "hipstoportable: htop_b: can't allocate memory\n");
			exit(1);
	}
	if (fread(img,rows*cols*sizeof(unsigned char),1,stdin) != 1) {
		fprintf(stderr,"hipstoportable: htop_b: error during read\n");
		exit(1);
	}
	p = img;
	max = *img;
	for (i=0;i<rows;i++) {
		for (j=0;j<cols;j++) {
			if (*p > max)
				max = *p;
			p++;
		}
	}
	printf("%d\n",max);
	if (pflag) {
		p = img;
		for (i=0;i<rows;i++) {
			nout = 0;
			for (j=0;j<cols;j++) {
				if (nout > 66)
					nout = printf("\n%d",*p++);
				else if (nout)
					nout += printf(" %d",*p++);
				else
					nout += printf("%d",*p++);
			}
			printf("\n");
		}
	}
	else {
		if (fwrite(img,rows*cols*sizeof(unsigned char),1,stdout) != 1) {
			fprintf(stderr,
				"hipstoportable: htop_b: error during write\n");
			exit(1);
		}
	}
}

void htop_s()

{
	short *img,*p;
	int i,j,max,nout,nclip=0;

	printf("P2\n");
	printf("%d %d\n",cols,rows);
	if ((img = (short *) malloc(rows*cols*sizeof(short))) == 0) {
			fprintf(stderr,
			    "hipstoportable: htop_s: can't allocate memory\n");
			exit(1);
	}
	if (fread(img,rows*cols*sizeof(short),1,stdin) != 1) {
		fprintf(stderr,"hipstoportable: htop_s: error during read\n");
		exit(1);
	}
	p = img;
	max = *img;
	for (i=0;i<rows;i++) {
		for (j=0;j<cols;j++) {
			if (*p > max)
				max = *p;
			p++;
		}
	}
	printf("%d\n",max);
	p = img;
	for (i=0;i<rows;i++) {
		nout = 0;
		for (j=0;j<cols;j++) {
			if (*p < 0) {
				*p = 0;
				nclip++;
			}
			if (nout > 66)
				nout = printf("\n%d",*p++);
			else if (nout)
				nout += printf(" %d",*p++);
			else
				nout += printf("%d",*p++);
		}
		printf("\n");
	}
	if (nclip)
		fprintf(stderr,
			"hipstoportable: %d negative values set to zero\n",nclip);
}

void htop_i()

{
	int *img,*p,i,j,max,nout,nclip=0;

	printf("P2\n");
	printf("%d %d\n",cols,rows);
	if ((img = (int *) malloc(rows*cols*sizeof(int))) == 0) {
			fprintf(stderr,
			    "hipstoportable: htop_i: can't allocate memory\n");
			exit(1);
	}
	if (fread(img,rows*cols*sizeof(int),1,stdin) != 1) {
		fprintf(stderr,"hipstoportable: htop_i: error during read\n");
		exit(1);
	}
	p = img;
	max = *img;
	for (i=0;i<rows;i++) {
		for (j=0;j<cols;j++) {
			if (*p > max)
				max = *p;
			p++;
		}
	}
	printf("%d\n",max);
	p = img;
	for (i=0;i<rows;i++) {
		nout = 0;
		for (j=0;j<cols;j++) {
			if (*p < 0) {
				*p = 0;
				nclip++;
			}
			if (nout > 66)
				nout = printf("\n%d",*p++);
			else if (nout)
				nout += printf(" %d",*p++);
			else
				nout += printf("%d",*p++);
		}
		printf("\n");
	}
	if (nclip)
		fprintf(stderr,
			"hipstoportable: %d negative values set to zero\n",nclip);
}

void htop_mp()

{
	unsigned char *img,*p;
	int i,j,nout,nrc8,bitpos;

	printf(pflag ? "P1\n" : "P4\n");
	printf("%d %d\n",cols,rows);
	nrc8 = rows * ((int) ((cols+7)/8));
	if ((img = (unsigned char *) malloc(nrc8*sizeof(unsigned char)))
		== 0) {
			fprintf(stderr,
			    "hipstoportable: htop_mp: can't allocate memory\n");
			exit(1);
	}
	if (fread(img,nrc8*sizeof(unsigned char),1,stdin) != 1) {
		fprintf(stderr,"hipstoportable: htop_mp: error during read\n");
		exit(1);
	}
	if (pflag) {
		p = img;
		for (i=0;i<rows;i++) {
			nout = 0;
			bitpos = 1<<7;
			for (j=0;j<cols;j++) {
				if (nout)
					nout += printf(
						(*p & bitpos) ? " 1" : " 0");
				else
					nout += printf(
						(*p & bitpos) ? "1" : "0");
				if (nout > 68) {
					printf("\n");
					nout = 0;
				}
				bitpos = bitpos >> 1;
				if (bitpos == 0) {
					bitpos = 1<<7;
					p++;
				}
			}
			printf("\n");
			if (bitpos != 1<<7)
				p++;
		}
	}
	else {
		if (fwrite(img,nrc8*sizeof(unsigned char),1,stdout) != 1) {
			fprintf(stderr,
			    "hipstoportable: htop_mp: error during write\n");
			exit(1);
		}
	}
}

void htop_rgb()

{
	unsigned char *img,*p;
	int i,j,max,nout;

	printf(pflag ? "P3\n" : "P6\n");
	printf("%d %d\n",cols,rows);
	cols *= 3;
	if ((img = (unsigned char *) malloc(rows*cols*sizeof(unsigned char)))
		== 0) {
			fprintf(stderr,
			  "hipstoportable: htop_rgb: can't allocate memory\n");
			exit(1);
	}
	if (fread(img,rows*cols*sizeof(unsigned char),1,stdin) != 1) {
		fprintf(stderr,"hipstoportable: htop_rgb: error during read\n");
		exit(1);
	}
	p = img;
	max = *img;
	for (i=0;i<rows;i++) {
		for (j=0;j<cols;j++) {
			if (*p > max)
				max = *p;
			p++;
		}
	}
	printf("%d\n",max);
	if (pflag) {
		p = img;
		for (i=0;i<rows;i++) {
			nout = 0;
			for (j=0;j<cols;j++) {
				if (nout > 66)
					nout = printf("\n%d",*p++);
				else if (nout)
					nout += printf(" %d",*p++);
				else
					nout += printf("%d",*p++);
			}
			printf("\n");
		}
	}
	else {
		if (fwrite(img,rows*cols*sizeof(unsigned char),1,stdout) != 1) {
			fprintf(stderr,
			    "hipstoportable: htop_rgb: error during write\n");
			exit(1);
		}
	}
}
