
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
 * portabletohips - Convert from various portable formats to HIPS format
 *
 * Usage:	portabletohips [-f frames] < portablefile > hipsfile
 *
 * Portabletohips converts from a variety of Jef Poskanzer's portable formats
 * to HIPS format.  This allows one to then use Poskanzer's routines to
 * convert from a variety of other standard image formats to HIPS format.
 * This program only accepts input files in the standard input.  Here are the
 * conversions that it performs:
 *
 *	Input format			Output pixel format
 *
 *	PBM (raw or plain)		MSBF packed
 *	PGM (raw or plain)		byte, short or integer
 *	PPM (raw or plain)		RGB
 *
 * For plain PGM formats, the maximum value in the sequence determines the
 * output pixel format (so that no clipping is ever required).  For raw PBM
 * format, the code assumes that each row begins on a byte boundary (which is
 * not stated explicitly in the PBM documentation but is true of the PBM
 * library code). For all PPM formats, only maximum values no greater than 255
 * are accepted.
 *
 * Hipstoportable allows one to collapse a multiframe sequence into a single
 * portable frame (with successive frames one above the next).  With the -f
 * switch, the single portable frame is treated as if it needs to be
 * `uncollapsed', resulting in a multiframe HIPS sequence.  Thus, the
 * specified number of frames must be an even divisor of the number of rows in
 * the portable input image.
 *
 * Michael Landy - 4/17/95
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
#define	PFRGB		35	/* RGBRGBRGB bytes */

int max,rows,cols,oformat,frames=1;

char usage[] =
	"Usage: portabletohips [-f frames] < portableimage > hipssequence";
int ourgetint(),hpub_wnocr(),hpub_dfprintf();
void hpub_wrthdr(),ptoh_b(),ptoh_g(),ptoh_p(),ptoh_b_r(),ptoh_g_r(),ptoh_p_r();
void hpub_fwrthdr(),hpub_perr();

int main(argc,argv)

int argc;
char **argv;

{
	int i,format;

	for (i=1;i<argc;i++) {
		if (argv[i][0] == '-') {
			switch(argv[i][1]) {
			case 'f':	if (++i >= argc) {
						fprintf(stderr,"%s\n",usage);
						exit(1);
					}
					frames = atoi(argv[i]);
					continue;
			default:	fprintf(stderr,"%s\n",usage);
					exit(1);
			}
		}
		else {
			fprintf(stderr,"%s\n",usage);
			exit(1);
		}
	}
	if ((getchar() != 'P') || ((format = getchar()) < '1') ||
	    format > '6') {
		fprintf(stderr,
		  "portabletohips: input sequence is not in portable format\n");
		exit(1);
	}
	cols = ourgetint();
	rows = ourgetint();
	if (rows%frames) {
		fprintf(stderr,"portabletohips: specified number of %s\n",
			"frames must evenly divide number of input rows\n");
		exit(1);
	}
	if (format != '1' && format != '4')
		max = ourgetint();
	switch(format) {
	case '1':	hpub_wrthdr(PFMSBF,rows/frames,cols,frames,1,argc,argv);
			ptoh_b();
			break;
	case '2':	if (max < 256)
				oformat = PFBYTE;
			else if (max < 16384)
				oformat = PFSHORT;
			else
				oformat = PFINT;
			hpub_wrthdr(oformat,rows/frames,cols,frames,1,
				argc,argv);
			ptoh_g();
			break;
	case '3':	if (max > 255) {
				fprintf(stderr,
			    "portabletohips: RGB with pixel values > 255\n");
				exit(1);
			}
			hpub_wrthdr(PFRGB,rows/frames,cols,frames,1,argc,argv);
			ptoh_p();
			break;
	case '4':	hpub_wrthdr(PFMSBF,rows/frames,cols,frames,1,argc,argv);
			ptoh_b_r();
			break;
	case '5':	hpub_wrthdr(PFBYTE,rows/frames,cols,frames,1,argc,argv);
			ptoh_g_r();
			break;
	case '6':	hpub_wrthdr(PFRGB,rows/frames,cols,frames,1,argc,argv);
			ptoh_p_r();
			break;
	}
	return(0);
}

/*
 * hpub_wrthdr.c - HIPS image header write (public domain version, slightly
 *			modified)
 *
 * Michael Landy - 9/30/92
 */

void hpub_wrthdr(format,rows,cols,frames,colors,argc,argv)

int format,rows,cols,frames,colors,argc;
char **argv;

{
	hpub_fwrthdr(stdout,format,rows,cols,frames,colors,argc,argv);
}

void hpub_fwrthdr(fp,format,rows,cols,frames,colors,argc,argv)

FILE *fp;
int format,rows,cols,frames,colors,argc;
char **argv;

{
	int i,j;
	char hist[200];

	fprintf(fp,"HIPS\n");
	i = 5;
	i += hpub_wnocr(fp,"");	/* onm */
	i += hpub_wnocr(fp,""); /* snm */
	i += hpub_dfprintf(fp,frames);
	i += hpub_wnocr(fp,"");
	i += hpub_dfprintf(fp,rows);	/* orows */
	i += hpub_dfprintf(fp,cols);	/* ocols */
	i += hpub_dfprintf(fp,rows);
	i += hpub_dfprintf(fp,cols);
	i += hpub_dfprintf(fp,0);	/* frow */
	i += hpub_dfprintf(fp,0);	/* fcol */
	i += hpub_dfprintf(fp,format);
	i += hpub_dfprintf(fp,colors);
	i += hpub_dfprintf(fp,1);	/* szhist */
	i += 1;
	if (fwrite("\n",1,1,fp) != 1)
		hpub_perr("error writing hips header");
	strcpy(hist,argv[0]);
	for (j=1;j<argc;j++) {
		strcat(hist," ");
		strcat(hist,argv[j]);
	}
	strcat(hist,"\n");
	j = strlen(hist);
	i += hpub_dfprintf(fp,j);	/* szdesc */
	i += j;
	if (fwrite(hist,j,1,fp) != 1)
		hpub_perr("error writing hips header");
	i += hpub_dfprintf(fp,0);	/* nparam */
	i += 2;
	while ((i & 03) != 0) {		/* pad binary area size line with
					   blanks so that entire header
					   comes out to be an even multiple
					   of 4 bytes - the binary area itself
					   is guaranteed to be an even
					   multiple because each individual
					   entry is padded */
		putc(' ',fp);
		i++;
	}
	fprintf(fp,"0\n");
}

int hpub_wnocr(fp,s)

char *s;
FILE *fp;

{
	char *t;
	int i;

	t = s;
	i = 0;
	while (*t != '\n' && *t != '\0') {
		putc(*t++,fp);
		i++;
	}
	putc('\n',fp);
	return(i+1);
}

int hpub_dfprintf(fp,i)

FILE *fp;
int i;

{
	char s[30];
	int j;

	sprintf(s,"%d\n",i);
	j = strlen(s);
	if (fwrite(s,j,1,fp) != 1)
		hpub_perr("error during header write");
	return(j);
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

void ptoh_b() {
	int i,j,sz,bitpos,val;
	unsigned char *img,*p;

	sz = rows*((int) ((cols+7)/8))*sizeof(unsigned char);
	if ((img = (unsigned char *) malloc(sz)) == 0) {
		fprintf(stderr,
			"portabletohips: ptoh_b: can't allocate memory\n");
		exit(1);
	}
	p = img;
	for (i=0;i<rows;i++) {
		bitpos = 1<<7;
		val = 0;
		for (j=0;j<cols;j++) {
			if (ourgetint())
				val |= bitpos;
			bitpos = bitpos>>1;
			if (bitpos == 0) {
				*p++ = val;
				val = 0;
				bitpos = 1<<7;
			}
		}
		if (bitpos < (1<<7))
			*p++ = val;
	}
	if (fwrite(img,sz,1,stdout) != 1) {
		fprintf(stderr,
			"portabletohips: ptoh_b: error during write\n");
		exit(1);
	}
}

void ptoh_g() {
	int i,j,*p_i,sz;
	unsigned char *img,*p_b;
	short *p_s;

	if (oformat == PFBYTE)
		sz = rows*cols*sizeof(unsigned char);
	else if (oformat == PFSHORT)
		sz = rows*cols*sizeof(short);
	else
		sz = rows*cols*sizeof(int);
	if ((img = (unsigned char *) malloc(sz)) == 0) {
		fprintf(stderr,
			"portabletohips: ptoh_g: can't allocate memory\n");
		exit(1);
	}
	if (oformat == PFBYTE) {
		p_b = img;
		for (i=0;i<rows;i++)
			for (j=0;j<cols;j++)
				*p_b++ = ourgetint();
	}
	else if (oformat == PFSHORT) {
		p_s = (short *) img;
		for (i=0;i<rows;i++)
			for (j=0;j<cols;j++)
				*p_s++ = ourgetint();
	} else {
		p_i = (int *) img;
		for (i=0;i<rows;i++)
			for (j=0;j<cols;j++)
				*p_i++ = ourgetint();
	}
	if (fwrite(img,sz,1,stdout) != 1) {
		fprintf(stderr,
			"portabletohips: ptoh_g: error during write\n");
		exit(1);
	}
}

void ptoh_p() {
	int i,j,sz;
	unsigned char *img,*p;

	cols *= 3;
	sz = rows*cols*sizeof(unsigned char);
	if ((img = (unsigned char *) malloc(sz)) == 0) {
		fprintf(stderr,
			"portabletohips: ptoh_p: can't allocate memory\n");
		exit(1);
	}
	p = img;
	for (i=0;i<rows;i++)
		for (j=0;j<cols;j++)
			*p++ = ourgetint();
	if (fwrite(img,sz,1,stdout) != 1) {
		fprintf(stderr,
			"portabletohips: ptoh_p: error during write\n");
		exit(1);
	}
}

void ptoh_b_r() {
	unsigned char *img;
	int sz;
	
	sz = rows*((int) ((cols+7)/8))*sizeof(unsigned char);

	if ((img = (unsigned char *) malloc(sz)) == 0) {
		fprintf(stderr,
			"portabletohips: ptoh_b_r: can't allocate memory\n");
		exit(1);
	}
	if (fread(img,sz,1,stdin) != 1) {
		fprintf(stderr,
			"portabletohips: ptoh_b_r: error during read\n");
		exit(1);
	}
	if (fwrite(img,sz,1,stdout) != 1) {
		fprintf(stderr,
			"portabletohips: ptoh_b_r: error during write\n");
		exit(1);
	}
}

void ptoh_g_r() {
	unsigned char *img;
	int sz;
	
	sz = rows*cols*sizeof(unsigned char);

	if ((img = (unsigned char *) malloc(sz)) == 0) {
		fprintf(stderr,
			"portabletohips: ptoh_g_r: can't allocate memory\n");
		exit(1);
	}
	if (fread(img,sz,1,stdin) != 1) {
		fprintf(stderr,
			"portabletohips: ptoh_g_r: error during read\n");
		exit(1);
	}
	if (fwrite(img,sz,1,stdout) != 1) {
		fprintf(stderr,
			"portabletohips: ptoh_g_r: error during write\n");
		exit(1);
	}
}

void ptoh_p_r() {
	unsigned char *img;
	int sz;
	
	sz = 3*rows*cols*sizeof(unsigned char);

	if ((img = (unsigned char *) malloc(sz)) == 0) {
		fprintf(stderr,
			"portabletohips: ptoh_p_r: can't allocate memory\n");
		exit(1);
	}
	if (fread(img,sz,1,stdin) != 1) {
		fprintf(stderr,
			"portabletohips: ptoh_p_r: error during read\n");
		exit(1);
	}
	if (fwrite(img,sz,1,stdout) != 1) {
		fprintf(stderr,
			"portabletohips: ptoh_p_r: error during write\n");
		exit(1);
	}
}

char ourgetc()

{
	int c;

	if ((c = getchar()) == EOF) {
		fprintf(stderr,"portabletohips: read error\n");
		exit(1);
	}
	if (c == '#') {
		do {
			if ((c = getchar()) == EOF) {
				fprintf(stderr,"portabletohips: read error\n");
				exit(1);
			}
		} while (c != '\n');
	}
	return (char) c;
}

int ourgetint()

{
	char c;
	int i;

	do {
		c = ourgetc();
	} while (c == ' ' || c == '\t' || c == '\n');

	if (c < '0' || c > '9') {
		fprintf(stderr,
			"portabletohips: integer expected but not found\n");
		exit(1);
	}
	i = 0;
	do {
		i = i*10 + c - '0';
		c = ourgetc();
	} while (c >= '0' && c <= '9');
	return i;
}
