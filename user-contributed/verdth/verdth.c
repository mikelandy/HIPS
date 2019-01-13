/*	Copyright (c) 1982 Michael Landy, Yoav Cohen, and George Sperling

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 * verdth.c - print halftones with 64 grey level dither on the Versatec
 *
 * Usage:	verdth [-n] < sequence
 *
 * to load:	cc -o verdth verdth.c -lhips
 *
 * Michael Landy - 2/24/83
 *
 * This program reads a sequence of byte-formatted frames, and
 * outputs them as halftones on the Versatec.  It uses the Versatec
 * plot mode, presenting each pixel as an array of dots, using a
 * 16x16 dither array, yielding a gray scale of 0-255.
 * The maximum image size is 2048 x 2048. Smaller images will be
 * blown up to fill the 2048 x 2048 area as much as possible.
 * If -n is specified, the enlargement factor is fixed at 4, filling a page
 * with 512x512, and centering anything smaller.
 */

#include <hipl_format.h>
#include <stdio.h>
#include <sys/vcmd.h>

int dither[] = {
  0, 128,  32, 160,   8, 136,  40, 168,    2, 130,  34, 162,  10, 138,  42, 170,
192,  64, 224,  96, 200,  72, 232, 104,  194,  66, 226,  98, 202,  74, 234, 106,
 48, 176,  16, 144,  56, 184,  24, 152,   50, 178,  18, 146,  58, 186,  26, 154,
240, 112, 208,  80, 248, 120, 216,  88,  242, 114, 210,  82, 250, 122, 218,  90,
 12, 140,  44, 172,   4, 132,  36, 164,   14, 142,  46, 174,   6, 134,  38, 166,
204,  76, 236, 108, 196,  68, 228, 100,  206,  78, 238, 110, 198,  70, 230, 102,
 60, 188,  28, 156,  52, 180,  20, 148,   62, 190,  30, 158,  54, 182,  22, 150,
252, 124, 220,  92, 244, 116, 212,  84,  254, 126, 222,  94, 246, 118, 214,  86,

  3, 131,  35, 163,  11, 139,  43, 171,    1, 129,  33, 161,   9, 137,  41, 169,
195,  67, 227,  99, 203,  75, 235, 107,  193,  65, 225,  97, 201,  73, 233, 105,
 51, 179,  19, 147,  59, 187,  27, 155,   49, 177,  17, 145,  57, 185,  25, 153,
243, 115, 211,  83, 251, 123, 219,  91,  241, 113, 209,  81, 249, 121, 217,  89,
 15, 143,  47, 175,   7, 135,  39, 167,   13, 141,  45, 173,   5, 133,  37, 165,
207,  79, 239, 111, 199,  71, 231, 103,  205,  77, 237, 109, 197,  69, 229, 101,
 63, 191,  31, 159,  55, 183,  23, 151,   61, 189,  29, 157,  53, 181,  21, 149,
255, 127, 223,  95, 247, 119, 215,  87,  253, 125, 221,  93, 245, 117, 213,  85};

int nflag = 0;

#define WAIT_TIME	15	/* max minutes to wait if versatec busy */

int	plotcom[] =	{ VPLOT, 0, 0};

char	*tmpfilenm = "/tmp/VdthXXXXXX";

main(argc,argv)

int argc;
char **argv;

{
	struct header hd;
	int shift, frame, row, col, i, ocol, orow, nrow, ncol, enl, val, tf;
	int brow, bcol, startrow, startcol, maxn;
	char *pic,*p,*filename,c,*mktemp(),*tfname,tmp[100];
	int f;
	int time,sec;
	char obuf[264];

	Progname = strsave(*argv);
	while (--argc > 0) {
		argv++;
		if (argv[0][0] == '-') {
			switch(argv[0][1]) {

			case 'D':
				continue;
			case 'n':
				nflag++;
				continue;
			}
		}
	}
	read_header(&hd);
	if (hd.pixel_format != PFBYTE)
		perr(HE_MSG,"image pixel format must be unpacked bytes");
	nrow = hd.orows;
	ncol = hd.ocols;
	maxn = nrow > ncol ? nrow : ncol;
	if (maxn > 2048) {
		sprintf(tmp,"image dimension of %d x %d is too large",
			hd.orows,hd.ocols);
		perr(HE_MSG,tmp);
	}
	for (i=0;i<12;i++) {
		enl = 1 << i;
		if ((ncol*enl*2 > 2048) || (nrow*enl*2 > 1700))
			break;
	}
	if (nflag && enl>4)
		enl = 4;
	brow = nrow * enl;
	bcol = ncol * enl;
	startrow = (1700 - brow)/2;
	startcol = (2048 - bcol)/2;
	pic = (char *) halloc(hd.orows*hd.ocols,sizeof (char));
	tfname = mktemp(tmpfilenm);
	close(creat(tfname,0600));
	for (frame=0;frame<hd.num_frame;frame++) {

		time = 0;
		sec = 0;
		while ((f=open("/dev/va0", 1)) < 0)  {
			if ((sec/300)*300 == sec)
				printf("verdth: waiting for versatec %d minutes\n",time);
			sleep(10);
			sec +=10;
			time = sec/60;
			if (time > WAIT_TIME)  {
				unlink(tfname);
				perr(HE_MSG,"giving up on versatec");
			}
		}
		if( ioctl(f, VSETSTATE, plotcom) != 0 ) {
			unlink(tfname);
			perr(HE_MSG,"ioctl failure vp");
		}
		if (fread(pic,hd.orows*hd.ocols*sizeof(char),1,stdin) != 1) {
			unlink(tfname);
			perr(HE_MSG,"unexpected end-of-file");
		}
		if((tf = open(tfname,1)) < 0) {
			unlink(tfname);
			perr(HE_MSG,"can't open temp file");
			exit(1);
		}
		p = pic + hd.orows*hd.ocols - 1;
		orow = 0;
		for (row=startrow;row<brow+startrow;row++) {
			ocol = 0;
			for (col=startcol;col<bcol+startcol;col++) {
				val = *p & 0377;
				if (val < dither[(row%16)*16+(col%16)])
					obuf[col/8] |= 1<<(7-(col%8));
				if ((++ocol)%enl==0)
					p--;
			}
			write(tf, (char *)obuf, sizeof(obuf));
			for (i=0;i<264;i++)
				obuf[i] = 0;
			if ((++orow % enl) != 0)
				p += hd.ocols;
		}
		close(tf);
		if((tf = open(tfname,0)) < 0) {
			unlink(tfname);
			perr(HE_MSG,"can't open temp file");
		}
		for (i=0;i<264;i++)
			obuf[i] = 0;
		nice(-20);
		for (row=0;row<startrow;row++)
			write(f, (char *)obuf, sizeof(obuf));
		for (row=startrow;row<brow+startrow;row++) {
			read(tf, (char *)obuf, sizeof(obuf));
			write(f, (char *)obuf, sizeof(obuf));
		}
		for (i=0;i<264;i++)
			obuf[i] = 0;
		for (row=brow+startrow;row<2700;row++)
			write(f, (char *)obuf, sizeof(obuf));
		nice(20);
		close(tf);
		close(f);
	}
	unlink(tfname);
	return(0);
}
