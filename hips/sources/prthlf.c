/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * prthlf.c - print halftones on the Anadex
 *
 * Usage:	prthlf [-s shift]    < sequence
 *		       [-l label]
 *		       [-lf label [startframe]]
 *		       [-f labelfile]
 *		       [-ff labelfile [startframe]]
 *		       [-o]
 *
 * defaults:	shift = 3
 *
 * to load:	cc -o prthlf prthlf.c -lhips
 *
 * Michael Landy - 2/10/82
 * HIPS 2 - msl - 8/2/91
 *
 * This program reads a sequence of byte-formatted frames, and
 * outputs them as halftones on the Anadex.  It uses the Anadex
 * graphics mode, presenting each pixel as a 4x4 array of dots
 * where each dot may be overstruck once, yielding a gray scale
 * of 0-31.  The pixels can be shifted, and the default shift is
 * three, thus converting a 0-255 gray scale to 0-31.
 * The maximum image size is 128 x 128.
 * The printer is assumed to be at the top of page when
 * the program begins, and it resets top of form to be the same
 * when it exits. The -l switches allows for a single line label
 * to be placed below each frame of the sequence.  The -f switch
 * allows for several lines of text from a file to be printed below
 * the frame.  The -lf and -ff switches act like -l and -f, respectively,
 * and add a line giving the frame number.  An optional starting frame
 * number may be specified which defaults to 0.  The -o switch converts code 0
 * to 128 in order to print black on white drawings without overstriking.
 */

#include <hipl_format.h>
#include <stdio.h>

static Flag_Format flagfmt[] = {
	{"s",{LASTFLAG},1,{{PTINT,"3","shift"},LASTPARAMETER}},
	{"l",{"lf","f","ff",LASTFLAG},1,{{PTBOOLEAN,"FALSE"},
		{PTSTRING," ","label"},LASTPARAMETER}},
	{"lf",{"l","f","ff",LASTFLAG},1,{{PTBOOLEAN,"FALSE"},
		{PTSTRING," ","label"},{PTINT,"0","startframe"},LASTPARAMETER}},
	{"f",{"l","lf","ff",LASTFLAG},1,{{PTBOOLEAN,"FALSE"},
		{PTSTRING,"","labelfile"},LASTPARAMETER}},
	{"ff",{"l","lf","f",LASTFLAG},1,{{PTBOOLEAN,"FALSE"},
		{PTSTRING,"","labelfile"},{PTINT,"0","startframe"},
		LASTPARAMETER}},
	{"o",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	LASTFLAG};
int types[] = {PFBYTE,LASTTYPE};

char *greys[] = {
	"\100\100\100\100",
	"\100\110\100\100",
	"\100\110\120\100",
	"\140\110\120\100",
	"\140\110\120\104",
	"\150\110\120\104",
	"\150\110\120\124",
	"\150\150\120\124",
	"\150\150\124\124",
	"\150\150\124\134",
	"\170\150\124\134",
	"\170\154\124\134",
	"\170\154\164\134",
	"\174\154\164\134",
	"\174\154\164\174",
	"\174\174\174\174"
};

int main(argc,argv)

int argc;
char **argv;

{
	int shift,frame,row,col,i,out(),lncnt,c,framedelta,method;
	char buf[128];
	byte *p;
	FILE *fp,*fp2;
	char *label;
	h_boolean lflag,lfflag,fflag,ffflag,oflag;
	struct header hd,hdp;
	Filename filename,labelfile;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&shift,&lflag,&label,&lfflag,&label,
		&framedelta,&fflag,&labelfile,&ffflag,&labelfile,&framedelta,
		&oflag,FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	method = fset_conversion(&hd,&hdp,types,filename);
	if (hd.orows > 128 || hd.ocols > 128) {
		sprintf(buf,"image dimension of %d x %d is too large",hd.orows,
			hd.ocols);
		perr(HE_MSG,buf);
	}
	printf("\34");
	for (i=0;i<128;i++)
		buf[i] = 0;
	for (frame=0;frame<hdp.num_frame;frame++) {
		fread_imagec(fp,&hd,&hdp,method,frame,filename);
		for (i=(792-hd.orows*4)/2;i>8;i-=9) {
			putchar('\100');
			putchar('9');
		}
		if (i>0) {
			putchar('\100');
			putchar('0' + (char) i);
		}
		p = hdp.image + hd.orows*hd.ocols;
		for (row=0;row<hd.orows;row++) {
			for (col=0;col<hd.ocols;col++) {
				buf[col] = *--p;
				if (oflag && buf[col] == 0)
					buf[col] = 128;
				buf[col] = 037 - ((buf[col] >> shift) & 037);
			}
			if (out(buf,hd.ocols)) {
				putchar('0');
				out(buf,hd.ocols);
			}
			putchar('4');
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
			for (i=0;i<(80-strlen(label))/2;i++)
				putchar(' ');
			for (i=0;i<strlen(label);i++)
				putchar(label[i]);
			lncnt = 0;
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
		for (i=((720-hd.rows*4)/2)-lncnt*12;i>8;i-=9) {
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

int out(buf,cols)

int cols;
char buf[];

{
	int first, last, count, i, j;

	for (first=0;first<cols;first++)
		if (buf[first]) break;
	if (first == cols) {
		putchar('\100');
		return(0);
	}
	for (i=first;i<cols;i++)
		if(buf[i])
			last = i;
	i = (588 - cols*4)/2 + first*4;
	printf(";%d%d%d",i/100,(i/10)%10,(i)%10);
	count = 0;
	for (i=first;i<=last;i++) {
		if((j = buf[i])>15) {
			j = 15;
			count++;
			buf[i] -= 16;
		}
		else
			buf[i] = 0;
		printf("%s",greys[j]);
	}
	return(count);
}
