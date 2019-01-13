/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * fcalccomb.c - generate and execute a program which manipulates multiple
 *	float-formatted sequences according to user-defined instructions.
 *
 * usage:	fcalccomb [-d] [-o objectname] [-A n arg1 ... argn]
 *			 [-s "statements" | -F filename]
 *			 [-i "init-statements" | -I initfilename]
 *				iseq0 iseq1 ... > oseq
 *
 * Fcalccomb takes c-statements from the argument list (with -s) or from an
 * input file (with -F) and inserts them into a "skeleton" of a program that
 * manipulates float-formatted sequences. The resulting program is compiled and
 * the object code is then placed in the user's directory. The name of the
 * object file can be specified by the user (option -o). The "skeleton" program
 * goes over all pixels of the input sequences (columns within rows within
 * frames) and executes for each pixel the user-supplied C-statements.
 * In addition, the user may optionally (using -i or -I) specify code which
 * is executed once per image frame in advance of the loop over image pixels.
 * The -A switch allows the user to specify arguments in the command line
 * accessible to the user-supplied code.  The -d switch (debug) prints out the
 * compilation command before executing it, and leaves the source code in the
 * user's directory.
 *
 * In his/her statements, the user can refer to the following pre-defined
 * variables:
 *
 *	nseq		the number of input sequences specified
 *	nr,nc		number of rows & columns in a frame.
 *	nf,ncol		number of color frames and color planes
 *	f,col,r,c	the current frame, color plane, row and column (starting
 *			from 0).
 *	depth,ndepth	the number of depth planes and current depth plane
 *	ipix(seq),opix	the gray-level of the current input & output pixel.
 *			The input sequences are numbered from 0 through
 *			nseq-1.  Note the parentheses rather than braces on
 *			ipix (this is a macro).
 *	picin[sq][i][j]	the current frame. (ipix(sq)=picin[sq][r][c]).  If the
 *			current pixel is not modified by the user-supplied
 *			instructions, it is set to the current input pixel:
 *			opix=ipix[0].	
 *	picout[i][j]	the current output frame
 *	nargs		the number of command-line arguments
 *	args		the command-line arguments
 *	first		a global variable initialized to 1
 *
 * picin[sq][i][j], picout[i][j], ipix(sq) and opix are of type float.
 * args is of type "char **".  All other variables are ints.
 * In addition to the above variables there are 10 "int" variables (i1-i10)
 * and 10 "double" variables (d1-d10) which can be freely used by the user.
 * Additional variables can of course be defined by the user within a block.
 * The following useful constants are also predefined:
 *
 *	H_E		e (base of the natural logarithms)
 *	H_LOG2E		Log to the base 2 of e
 *	H_LOG10E	Log to the base 10 of e
 *	H_LN2		Natural logarithm of 2
 *	H_LN10		Natural logarithm of 10
 *	H_PI		pi
 *	H_PI_2		pi/2
 *	H_PI_4		pi/4
 *	H_ONE_PI	1/pi
 *	H_TWO_PI	2/pi
 *	H_TWO_SQRTPI	2/sqrt(pi)
 *	H_180_PI	180/pi (radians to degrees)
 *	H_SQRT2		sqrt(2)
 *	H_SQRT1_2	sqrt(1/2)
 *	H_SQRT3OVER2	sqrt(3/2)
 *  
 * All legal C-statements are permitted; in addition all the "math"
 * functions can be called.
 * The user can specify as many statements as desired, but bear in mind
 * that these statements are inserted into the busiest loop...
 *
 * The program reports on the "stderr" device when compilation is done;
 * it then "consumes" the input sequence. If an input sequence was not
 * redirected, the program reports the condition and exits.
 * The compiled program however, remains intact in the user's directory
 * for future use (under the name fcalccomb.local if the option -o was not
 * specified).
 *
 * The program is not intended to serve as a tool for generating system 
 * programs, because the object code is usually a bit slower than a
 * program which is specially tailored for a particular computation.
 *
 * to load: cc -o fcalccomb fcalccomb.c -lhips -lm
 *
 * note that on execution the file "fcalccombp.o" needs to be in
 * the HIPS library directory (an installation-dependent parameter often set
 * to /usr/local/lib).  fcalccombp.o is the C-compiled version of fcalccombp.c,
 * the main program which is loaded by fcalccomb.
 *
 * Michael Landy - 1/4/93
 * added depths - msl - 3/7/94
 */

#include <stdio.h>
#include <hipl_format.h>
#include <string.h>

#include "calc_header.h"

static Flag_Format flagfmt[] = {
	{"d",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"o",{LASTFLAG},1,{{PTFILENAME,"fcalccomb.local","executablefile"},
		LASTPARAMETER}},
	{"A",{LASTFLAG},1,{{PTLIST,"","numargs<integer> arg1 ... argn"},
		LASTPARAMETER}},
	{"s",{"F",LASTFLAG},1,{{PTBOOLEAN,"FALSE"},{PTSTRING,"","statements"},
		LASTPARAMETER}},
	{"F",{"s",LASTFLAG},1,{{PTBOOLEAN,"FALSE"},
		{PTFILENAME,"","statementsfile"},LASTPARAMETER}},
	{"i",{"I",LASTFLAG},1,{{PTBOOLEAN,"FALSE"},
		{PTSTRING,"","initstatements"},
		LASTPARAMETER}},
	{"I",{"i",LASTFLAG},1,{{PTBOOLEAN,"FALSE"},
		{PTFILENAME,"","initstatementsfile"},LASTPARAMETER}},
	LASTFLAG};

char srhead1[] = "\
#include <hipl_format.h> \n\
#include <math.h> \n\
#define	ipix(i)	(*(ipixpt[(i)])) \n\
\n\
int first=1; \n\
extern int nargs; \n\
extern char **args; \n\
extern float **ipixpt; \n\
\n\
void fcalccombsr(nseq,f,nf,depth,ndepth,col,ncol,nr,nc,picin,picout,nexi,nexo) \n\
\n\
int nseq,f,nf,depth,ndepth,col,ncol,nr,nc,*nexi,nexo; \n\
float ***picin,**picout; \n\
\n\
{ \n\
	float *opixpt,opix; \n\
	int seqnum,i1,i2,i3,i4,i5,i6,i7,i8,i9,i10; \n\
	double d1,d2,d3,d4,d5,d6,d7,d8,d9,d10; \n\
	register int r1,c1,r,c; \n\
\n";
char srhead2[] = "\
\n\
	for (seqnum=0;seqnum<nseq;seqnum++) \n\
		ipixpt[seqnum] = *picin[seqnum]; \n\
	opixpt = *picout; \n\
	for(r1=r=0;r1<nr;r = ++r1) {\n\
		for(c1=c=0;c1<nc;c = ++c1) { \n\
			opix = ipix(0); \n\n";

char srtail[] = "\
\n\n\
			for (seqnum=0;seqnum<nseq;seqnum++) \n\
				ipixpt[seqnum]++; \n\
			*opixpt++ = opix; \n\
		} \n\
		for (seqnum=0;seqnum<nseq;seqnum++) \n\
			ipixpt[seqnum] += nexi[seqnum]; \n\
		opixpt += nexo;\n\
	} \n\
}\n";

char cc[]=CC;
char cflags[]=CFLAGS;
char ldflags1[]=LDFLGS1;
char ldflags2[]=LDFLGS2;
char ldflags3[]=LDFLGS3;
char ldflags4[]=LDFLGS4;
char ldflags5[]=LDFLGS5;
char ldflags6[]=LDFLGS6;
char ldlib[]=LDLIB;
char tmplib[]=TMPLIB;
int execv();

int main(argc,argv)

int argc;
char **argv;

{
	Filename filelist,objectname,progname,prognamei;
	h_boolean dflag,sflag,Fflag,iflag,Iflag;
	int nseq,c;
	char *ccode,*ccodei;
	Listarg arguments;
	char tempstring[400],msg[150];
	char *mktemp(),template[100],*tempfile;
	FILE *srsource,*infile,*infilei;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&dflag,&objectname,&arguments,&sflag,
		&ccode,&Fflag,&progname,&iflag,&ccodei,&Iflag,&prognamei,
		FFLIST,&nseq,&filelist);
	if (!Fflag && !sflag)
		perr(HE_MSG,"One of -s or -F must be specified");

/* write source file, compile and overlay */
	if (dflag)
		template[0] = '\0';
	else {
		strcpy(template,tmplib);
		strcat(template,"/");
	}
	strcat(template,"caXXXXXX");
	tempfile=mktemp(template);
	strcat(tempfile,".c");
	if ((srsource=fopen(tempfile,"w"))==NULL)
		perr(HE_OPEN,tempfile);
	fprintf(srsource,"%s",srhead1);
	if (Iflag) {
		if ((infilei=fopen(prognamei,"r"))==NULL)
			perr(HE_OPEN,prognamei);
		else while ((c=getc(infilei)) != EOF)
			putc(c,srsource);
	}
	else if (iflag)
		fprintf(srsource,"	%s;\n",ccodei);
	fprintf(srsource,"%s",srhead2);
	if (Fflag) {
		if ((infile=fopen(progname,"r"))==NULL)
			perr(HE_OPEN,progname);
		else while ((c=getc(infile)) != EOF)
			putc(c,srsource);
	}
	else
		fprintf(srsource,"			%s;\n",ccode);
	fprintf(srsource,"%s",srtail);

	fclose(srsource);

	strcpy(tempstring,CC);
	strcat(tempstring," -o ");
	strcat(tempstring,objectname);
	strcat(tempstring," ");
	strcat(tempstring,ldlib);
	strcat(tempstring,"/fcalccombp.o ");
	strcat(tempstring,tempfile);
	strcat(tempstring," ");
	strcat(tempstring,cflags);
	strcat(tempstring," ");
	strcat(tempstring,ldflags1);
	strcat(tempstring," ");
	strcat(tempstring,ldflags2);
	strcat(tempstring," ");
	strcat(tempstring,ldflags3);
	strcat(tempstring," ");
	strcat(tempstring,ldflags4);
	strcat(tempstring," ");
	strcat(tempstring,ldflags5);
	strcat(tempstring," ");
	strcat(tempstring,ldflags6);
	strcat(tempstring," -lm");

	if (dflag)
		fprintf(stderr,"%s\n",tempstring);

	if (system(tempstring) >= 127) {
		sprintf(msg,"cannot compile, source file `%s' not removed",
			template);
		perr(HE_MSG,msg);
	}

	strcpy(tempstring,"rm -f ");
	strcat(tempstring,tempfile);
/*	tempfile[9]='o';
	strcat(tempstring," ");
	strcat(tempstring,tempfile); */

	if (!dflag)
	    if (system(tempstring))
		perr(HE_IMSG,"cannot remove temporary file");

	perr(HE_IMSG,"compilation done");
	execv(objectname,argv);
	perr(HE_MSG,"execv failed");
}
