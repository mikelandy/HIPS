/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * prteps.c - print HIPS images on an Epson (e.g. LQ800)
 *
 * Usage:	prteps [-d]    < sequence
 *		       [-l label]
 *		       [-lf label [startframe]]
 *		       [-f labelfile]
 *		       [-ff labelfile [startframe]]
 *		       [-e factor]
 *
 * to load:	cc -o prteps prteps.c -lhips
 *
 * Michael Landy - 6/27/88
 * HIPS 2 - msl - 8/2/91
 *
 * This program reads a sequence of byte-formatted frames, and
 * outputs them on an Epson printer.  It uses Epson graphics mode.  It
 * operates in two modes.  The default mode outputs a dot for all nonzero
 * pixels.  The dither mode (-d) dithers the image.  The probability of a dot
 * being printed is proportional to the blackness of a pixel (i.e. is 
 * 1 - (pixel/255)).  The -l switches allows for a single line label
 * to be placed below each frame of the sequence. The -f switch
 * allows for several lines of text from a file to be printed below
 * the frame. The -lf and -ff switches act like -l and -f, respectively,
 * and add a line giving the frame number.  An optional starting frame
 * number may be specified which defaults to 0.  The -e switch enlarges the
 * image by pixel replication.
 */

#include <hipl_format.h>
#include <stdio.h>

static Flag_Format flagfmt[] = {
	{"d",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"l",{"lf","f","ff",LASTFLAG},1,{{PTBOOLEAN,"FALSE"},
		{PTSTRING," ","label"},LASTPARAMETER}},
	{"lf",{"l","f","ff",LASTFLAG},1,{{PTBOOLEAN,"FALSE"},
		{PTSTRING," ","label"},{PTINT,"0","startframe"},LASTPARAMETER}},
	{"f",{"l","lf","ff",LASTFLAG},1,{{PTBOOLEAN,"FALSE"},
		{PTSTRING,"","labelfile"},LASTPARAMETER}},
	{"ff",{"l","lf","f",LASTFLAG},1,{{PTBOOLEAN,"FALSE"},
		{PTSTRING,"","labelfile"},{PTINT,"0","startframe"},
		LASTPARAMETER}},
	{"e",{LASTFLAG},1,{{PTINT,"1","factor"},LASTPARAMETER}},
	LASTFLAG};
int types[] = {PFBYTE,LASTTYPE};

int main(argc,argv)

int argc;
char **argv;

{
	char *oline[3],*op,*p0,*p1,*p2,method;
	unsigned char c1,c2,c3,val,bit;
	byte *p;
	int nrow,ncol,norow,nocol,frame,i,j,nbl,nlf,bitno,byteno;
	int r,rcopy,c,ccopy,framedelta,enlarge;
	FILE *fp,*fp2;
	char *label,msg[100];
	h_boolean lflag,lfflag,fflag,ffflag,dflag;
	struct header hd,hdp;
	Filename filename,labelfile;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&dflag,&lflag,&label,&lfflag,&label,
		&framedelta,&fflag,&labelfile,&ffflag,&labelfile,&framedelta,
		&enlarge,FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	method = fset_conversion(&hd,&hdp,types,filename);
	nrow = hd.orows;
	ncol = hd.ocols;
	norow = nrow * enlarge;
	nocol = ncol * enlarge;
	if (norow > 1800 || nocol > 1440) {
		sprintf(msg,"output image dimension of %d x %d is too large",
			norow,nocol);
		perr(HE_MSG,msg);
	}
	nbl = (80 - (nocol/18))/2;
	nlf = (66 - (norow/30))/2;
	oline[0] = (char *) halloc(nocol,sizeof(char));
	oline[1] = (char *) halloc(nocol,sizeof(char));
	oline[2] = (char *) halloc(nocol,sizeof(char));
	c1 = nocol/256;
	c2 = nocol % 256;
	for (frame=0;frame<hdp.num_frame;frame++) {
		fread_imagec(fp,&hd,&hdp,method,frame,filename);
		for (i=0;i<nlf;i++)
			printf("\n");
		printf("\0333");
		c3 = 24;
		putchar(c3);
		bitno = 7; bit = 1<<bitno; byteno = 0;
#ifdef ULORIG
		p = hdp.image;
#else
		p = hdp.image + (nrow-1)*ncol;
#endif
		for (i=0;i<3;i++) {
			op = oline[0];
			for (j=0;j<nocol;j++)
				*op++ = 0;
		}
		op = oline[0];
		for (r=0;r<nrow;r++) {
		    for (rcopy=0;rcopy<enlarge;rcopy++) {
			for (c=0;c<ncol;c++) {
			    val = *p++;
			    for (ccopy=0;ccopy<enlarge;ccopy++) {
				if (dflag) {
					i = 255.*H__RANDOM() /
						(017777777777 + 1.0);
					if (val <= i)
						*op |= bit;
				}
				else if (val == 0)
					*op |= bit;
				op++;
			    }
			}
			op -= nocol;
			p -= ncol;
			if (--bitno < 0) {
			    if (++byteno > 2) {
				for (i=0;i<nbl;i++)
					putchar(' ');
				printf("\033*\047");
				putchar(c2);
				putchar(c1);
				p0 = oline[0];
				p1 = oline[1];
				p2 = oline[2];
				for (i=0;i<nocol;i++) {
					putchar(*p0);
					putchar(*p1);
					putchar(*p2);
					*p0++ = 0;
					*p1++ = 0;
					*p2++ = 0;
				}
				putchar('\n');
				byteno = 0;
			    }
			    bitno = 7;
			    op = oline[byteno];
			}
			bit = 1<<bitno;
		    }
#ifdef ULORIG
		    p += ncol;
#else
		    p -= ncol;
#endif
		}
		if (byteno != 0 || bitno != 7) {
			for (i=0;i<nbl;i++)
				putchar(' ');
			printf("\033*\047");
			putchar(c2);
			putchar(c1);
			p0 = oline[0];
			p1 = oline[1];
			p2 = oline[2];
			for (i=0;i<nocol;i++) {
				putchar(*p0++);
				putchar(*p1++);
				putchar(*p2++);
			}
			putchar('\n');
		}
		printf("\0333");
		c3 = 30;
		putchar(c3);
		printf("\n\n\n");
		if (fflag || ffflag) {
			fp2 = ffopen(filename,"r");
			while ((c = getc(fp2)) != EOF)
				putchar(c);
			fclose(fp2);
		}
		else if (lflag || lfflag) {
			j = (80 - strlen(label))/2;
			for (i=0;i<j;i++)
				putchar(' ');
			printf("%s",label);
		}
		if (ffflag || lfflag) {
			putchar('\n');putchar('\n');
			for (i=0;i<35;i++)
				putchar(' ');
			if (frame+framedelta < 100)
				putchar(' ');
			printf("Frame %d",frame+framedelta);
		}
		printf("\014");
	}
	return(0);
}
