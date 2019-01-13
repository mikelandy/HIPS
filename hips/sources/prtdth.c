/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * prtdth.c - print halftones with 64 grey level dither on the Anadex
 *
 * Usage:	prtdth [-s shift]    < sequence
 *		       [-l label]
 *		       [-lf label [startframe]]
 *		       [-f labelfile]
 *		       [-ff labelfile [startframe]]
 *		       [-i 2 | -i 4]
 *
 * defaults:	shift = 2
 *
 * to load:	cc -o prtdth prtdth.c -lhips
 *
 * Michael Landy - 5/5/82
 * Hips 2 - msl - 8/2/91
 *
 * This program reads a sequence of byte-formatted frames, and
 * outputs them as halftones on the Anadex.  It uses the Anadex
 * graphics mode, presenting each pixel as a single dot, using an
 * 8x8 dither array, yielding a gray scale of 0-63.
 * The pixels can be shifted, and the default shift is
 * two, thus converting a 0-255 gray scale to 0-63.
 * The maximum image size is 512 x 512.
 * The printer is assumed to be at the top of page when
 * the program begins, and it resets top of form to be the same
 * when it exits. The -l switches allows for a single line label
 * to be placed below each frame of the sequence. The -f switch
 * allows for several lines of text from a file to be printed below
 * the frame. The -lf and -ff switches act like -l and -f, respectively,
 * and add a line giving the frame number.  An optional starting frame
 * number may be specified following the -lf or -ff, which defaults to 0.
 * The -i switches use interpolation to print smaller images at 2 or 4 times
 * their normal size.
 *
 * pixel formats handled directly: BYTE
 */

#include <hipl_format.h>
#include <stdio.h>

static Flag_Format flagfmt[] = {
	{"s",{LASTFLAG},1,{{PTINT,"2","shift"},LASTPARAMETER}},
	{"l",{"lf","f","ff",LASTFLAG},1,{{PTBOOLEAN,"FALSE"},
		{PTSTRING," ","label"},LASTPARAMETER}},
	{"lf",{"l","f","ff",LASTFLAG},1,{{PTBOOLEAN,"FALSE"},
		{PTSTRING," ","label"},{PTINT,"0","startframe"},LASTPARAMETER}},
	{"f",{"l","lf","ff",LASTFLAG},1,{{PTBOOLEAN,"FALSE"},
		{PTSTRING,"","labelfile"},LASTPARAMETER}},
	{"ff",{"l","lf","f",LASTFLAG},1,{{PTBOOLEAN,"FALSE"},
		{PTSTRING,"","labelfile"},{PTINT,"0","startframe"},
		LASTPARAMETER}},
	{"i",{LASTFLAG},1,{{PTINT,"1","enlargement"},LASTPARAMETER}},
	LASTFLAG};
int types[] = {PFBYTE,LASTTYPE};

int dither[] = {
	 1, 32,  8, 40,  2, 34, 10, 42,
	48, 16, 56, 24, 50, 18, 58, 26,
	12, 44,  4, 36, 14, 46,  6, 38,
	60, 28, 52, 20, 62, 30, 54, 22,
	 3, 35, 11, 43,  1, 33,  9, 41,
	51, 19, 59, 27, 49, 17, 57, 25,
	15, 47,  7, 39, 13, 45,  5, 37,
	63, 31, 55, 23, 61, 29, 53, 21};

int scale2[] = {

	4,0,0,0,  2,2,0,0,

	2,0,2,0,  1,1,1,1};

int scale4[] = {

	16, 0, 0, 0,   12, 4, 0, 0,    8, 8, 0, 0,    4,12, 0, 0,

	12, 0, 4, 0,    9, 3, 3, 1,    6, 6, 2, 2,    3, 9, 1, 3,

	 8, 0, 8, 0,    6, 2, 6, 2,    4, 4, 4, 4,    2, 6, 2, 6,

	 4, 0,12, 0,    3, 1, 9, 3,    2, 2, 6, 6,    1, 3, 3, 9};
void out();

int main(argc,argv)

int argc;
char **argv;

{
	int shift,frame,row,col,i,orow,nrow,ncol,enl,val,framedelta;
	int valr,vald,valrd,mul,mulr,muld,mulrd,dr,dc,div,lncnt,c,method;
	char buf[512];
	byte *p;
	FILE *fp,*fp2;
	char *label;
	h_boolean lflag,lfflag,fflag,ffflag;
	struct header hd,hdp;
	Filename filename,labelfile;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&shift,&lflag,&label,&lfflag,&label,
		&framedelta,&fflag,&labelfile,&ffflag,&labelfile,&framedelta,
		&enl,FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	method = fset_conversion(&hd,&hdp,types,filename);
	if (enl != 1 && enl != 2 && enl != 4)
		perr(HE_MSG,"-i argument must be 2 or 4");
	nrow = hd.orows * enl;
	ncol = hd.ocols * enl;
	if (nrow > 512 || ncol > 512) {
		sprintf(buf,"image dimension of %d x %d is too large",
			hd.orows,hd.ocols);
		perr(HE_MSG,buf);
	}
	for (i=0;i<512;i++)
		buf[i] = 0;
	printf("\34");
	for (frame=0;frame<hdp.num_frame;frame++) {
		fread_imagec(fp,&hd,&hdp,method,frame,filename);
		for (i=(792-nrow)/2;i>8;i-=9) {
			putchar('\100');
			putchar('9');
		}
		if (i>0) {
			putchar('\100');
			putchar('0' + (char) i);
		}
		p = hdp.image + hd.orows*hd.ocols;
		orow = 0;
		for (row=0;row<nrow;row++) {
			for (col=0;col<ncol;col++) {
				if (enl == 1)
					val = *--p;
				else {
				   if (col % enl == 0)
					--p;
				   val = *p;
				   valr = (col>=ncol-4)?val : *(p-1);
				   vald = (row>=nrow-4)?val:*(p-hd.ocols);
				   valrd=(row>=nrow-4)?valr:
				      ((col>=ncol-4)?vald:*(p-hd.ocols-1));
				   dr = row % enl;
				   dc = col % enl;
				   mul = (enl == 2) ? scale2[dr*8+dc*4] :
					scale4[dr*16+dc*4];
				   mulr = (enl == 2) ? scale2[dr*8+dc*4+1] :
					scale4[dr*16+dc*4+1];
				   muld = (enl == 2) ? scale2[dr*8+dc*4+2] :
					scale4[dr*16+dc*4+2];
				   mulrd = (enl == 2) ? scale2[dr*8+dc*4+3] :
					scale4[dr*16+dc*4+3];
				   div = (enl == 2) ? 4 : 16;
				   val = (mul*val+mulr*valr+muld*vald+
					mulrd*valrd)/div;
				}
				val = val >> shift;
				val = 63 - val;
				if (val >= dither[8*(row%8)+(col%8)])
					buf[col] |= 040 >> orow;
			}
			if ((row % enl) != enl-1)
				p += hd.ocols;
			if (orow==5) {
				out(buf,ncol);
				putchar('6');
				for (i=0;i<ncol;i++)
					buf[i] = 0;
				orow = 0;
			}
			else
				orow++;
		}
		if (orow>0) {
			out(buf,ncol);
			printf("%d",orow);
			for (i=0;i<ncol;i++)
				buf[i] = 0;
		}
		for (i=0;i<4;i++) {
			putchar('\100');
			putchar('9');
		}
		printf("\35\22");
		if (fflag || ffflag) {
			fp2 = ffopen(labelfile,"r");
			lncnt = 0;
			while ((c = getc(fp2)) != EOF) {
				putchar(c);
				if (c == '\n')
					lncnt++;
			}
			fclose(fp2);
		}
		else {
			lncnt = 0;
			for (i=0;i<(80-strlen(label))/2;i++)
				putchar(' ');
			for (i=0;i<strlen(label);i++)
				putchar(label[i]);
		}
		if (lfflag || ffflag) {
			putchar('\n');putchar('\n');
			for (i=0;i<35;i++)
				putchar(' ');
			if (frame+framedelta < 100)
				putchar(' ');
			printf("frame %d",frame+framedelta);
			lncnt += 2;
		}
		printf("\27\34");
		for (i=((716-nrow)/2)-lncnt*12;i>8;i-=9) {
			putchar('\100');
			putchar('9');
		}
		if (i>0) {
			putchar('\100');
			putchar('0' + (char) i);
		}
	}
	printf("\35\33\64\60\66\66");
	return(0);
}

void out(buf,cols)

int cols;
char buf[];

{
	int first,last,i;

	for (first=0;first<cols;first++)
		if (buf[first]) break;
	if (first == cols) {
		putchar('\100');
		return;
	}
	for (i=first;i<cols;i++)
		if(buf[i])
			last = i;
	i = (588 - cols)/2 + first;
	printf(";%d%d%d",i/100,(i/10)%10,(i)%10);
	for (i=first;i<=last;i++)
		putchar(buf[i] | '\100');
}
