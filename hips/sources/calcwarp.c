/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * calcwarp.c - generate and execute a program which warps an image
 *
 * usage:	calcwarp [-o objectname] [-w warpcodefile | -W warpcode] [-d]
 *			[-i init-statements | -I initfilename]
 *			[-A n arg1 ... argn] [-s nr [nc]] <iseq >oseq
 *
 *	"Calcwarp" takes c-statements from the argument list (with -W) or from
 * an input file (with -w) and inserts them into
 * a "skeleton" of a program that manipulates byte-
 * formatted sequences. The resulting program is compiled and the object
 * code is then placed in the user's directory. The name of the object
 * file can be specified by the user (option -o). The "skeleton" program
 * performs a general image warp to the input image.  The output image size
 * may be specified (using -s), and defaults to the size of the input (cols
 * defaults to rows).  For
 * each output pixel, the user-specified code is executed to determine the
 * input image location from which that value should be derived.  The
 * coordinate system for both the input and output images has x and y ranging
 * from 0 (lower left pixel) to 1 (upper right pixel).  The code can be 
 * specified in the command line (-W) or in a file (-w).  Values in the output
 * image are bilinearly interpolated between neighboring values in the input
 * image.  If the input image location is outside the input image, then a
 * background value is used (set by -UL, the default is 0).  The -A switch
 * allows the user to specify arguments in the command line accessible to
 * the user-supplied code.  The -d flag is for debugging; it prints the 
 * compile line and leaves the source code in the user's directory.
 * In addition, the user may optionally (using -i or -I) specify code which
 * is executed once per image frame in advance of the loop over image pixels.
 *
 * The user can refer to the following pre-defined variables:
 *
 *	ox,oy		current output pixel coordinates
 *	x,y		corresponding input coordinates
 *	nr,nc		number of rows & columns in an input frame
 *	nor,noc		number of rows & columns in an output frame
 *	nf,ncol		number of color frames and color planes
 *	f,col,r,c	the current frame, color plane, row and column (starting
 *			from 0).
 *	depth,ndepth	the number of depth planes and current depth plane
 *	picin[i][j]	the current input frame
 *	picout[i][j]	the current output frame
 *	nargs		the number of command-line arguments
 *	args		the command-line arguments
 *	first		a global variable initialized to 1
 *
 * All of the above variables, except for x, y, ox, oy, picin, picout, and
 * args are of type "int".  x, y, ox, and oy are of type "float".
 * Picin[i][j] and picout[i][j] is of type byte (which is unsigned char). args
 * is of type "char **".  In addition to the above variables there are 5 "int"
 * variables (i1-i5) and 5 "double" variables (d1-d5) which can be freely used
 * by the user.  Additional variables can of course be defined by the user
 * within a block.  The following useful constants are also predefined:
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
 * All the legal C-statements are permitted; in addition all the "math"
 * functions can be called.  The user can specify as many statements as
 * desired, but bear in mind that these statements are inserted into the
 * busiest loop...
 *
 * The program reports on the "stderr" device when compilation is done;
 * it then "consumes" the input sequence. If an input sequence was not
 * redirected, the program reports the condition and exits.
 * The compiled program however, remains intact in the user's directory
 * for future use (under the name calcwarp.local if the option -o was not
 * specified).
 *
 * The program is not intended to serve as a tool for generating system 
 * programs, because the object code is usually a bit slower than a
 * program which is specially tailored for a particular computation.
 *
 * to load: cc -o calcwarp calcwarp.c -lhips -lm
 *
 * note that on execution the file "calcwarpp.o" needs to be in directory
 * the HIPS library directory (an installation-dependent parameter often set
 * to /usr/local/lib).  calcwarpp.o is the C-compiled version of calcwarpp.c,
 * the main program which is loaded by calcwarp.
 *
 * Michael Landy - 12/30/88
 * Hips 2 - msl - 8/12/91
 * added -i/-I - msl - 1/7/93
 * added depths - msl - 3/7/94
 */

#include <stdio.h>
#include <hipl_format.h>
#include <string.h>

#include "calc_header.h"

static Flag_Format flagfmt[] = {
	{"d",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"o",{LASTFLAG},1,{{PTFILENAME,"calcwarp.local","executablefile"},
		LASTPARAMETER}},
	{"s",{LASTFLAG},1,{{PTBOOLEAN,"FALSE"},{PTINT,"-1","rows"},
		{PTINT,"-1","cols"},LASTPARAMETER}},
	{"A",{LASTFLAG},1,{{PTLIST,"","numargs<integer> arg1 ... argn"},
		LASTPARAMETER}},
	{"W",{"w",LASTFLAG},1,{{PTBOOLEAN,"FALSE"},{PTSTRING,"","warpcode"},
		LASTPARAMETER}},
	{"w",{"W",LASTFLAG},1,{{PTBOOLEAN,"FALSE"},
		{PTFILENAME,"","warpcodefile"},LASTPARAMETER}},
	{"i",{"I",LASTFLAG},1,{{PTBOOLEAN,"FALSE"},
		{PTSTRING,"","initstatements"},LASTPARAMETER}},
	{"I",{"i",LASTFLAG},1,{{PTBOOLEAN,"FALSE"},
		{PTFILENAME,"","initstatementsfile"},LASTPARAMETER}},
	LASTFLAG};

char srhead1[] = "\
#include <hipl_format.h> \n\
#include <math.h> \n\
\n\
int first=1; \n\
extern int nargs; \n\
extern char **args; \n\
\n\
void calcwarpsr(f,nf,depth,ndepth,col,ncol,nr,nc,nor,noc,picin,picout,nlpi,nexo) \n\
\n\
int f,nf,depth,ndepth,col,ncol,nr,nc,nor,noc,nlpi,nexo; \n\
byte **picin,**picout; \n\
\n\
{ \n\
	byte *ipixpt,*opixpt,*ifr,*pofr; \n\
	double d1,d2,d3,d4,d5; \n\
	int ix,ix1,iy,iy1,i1,i2,i3,i4,i5,r,c,nr1,nc1,nor1,noc1; \n\
	float x,y,ox,oy,px,dx,py,dy,vll,vlr,vul,vur,v; \n\
\n";
char srhead2[] = "\
\n\
	nr1 = nr - 1; nc1 = nc - 1; nor1 = nor - 1; noc1 = noc - 1; \n\
	ifr = *picin; \n\
	pofr = *picout; \n\
	for (r=0;r<nor;r++) { \n\
#ifdef ULORIG \n\
		oy = 1. - (((float) r)/nor1); \n\
#else \n\
		oy = ((float) r)/nor1; \n\
#endif \n\
		for (c=0;c<noc;c++) { \n\
			ox = ((float) c)/noc1; \n\
			x = ox; \n\
			y = oy; \n";

char srtail[] = "\
\n\n\
			if (x < 0 || y < 0 || x > 1 || y > 1) { \n\
				*pofr++ = hips_lchar; \n\
				continue; \n\
			} \n\
			px = x*nc1; \n\
			ix = px; \n\
			ix1 = ix + 1; \n\
			dx = px - ix; \n\
#ifdef ULORIG \n\
			py = (1.-y)*nr1; \n\
#else \n\
			py = y*nr1; \n\
#endif \n\
			iy = py; \n\
			iy1 = iy + 1; \n\
			dy = py - iy; \n\
			if (px >= nc1) \n\
				ix1 = ix; \n\
			if (py >= nr1) \n\
				iy1 = iy; \n\
			vll = ifr[iy*nlpi + ix]; \n\
			vlr = ifr[iy*nlpi + ix1]; \n\
			vul = ifr[iy1*nlpi + ix]; \n\
			vur = ifr[iy1*nlpi + ix1]; \n\
			v = vll*(1-dx)*(1-dy) + \n\
			    vlr*dx*(1-dy) + vul*(1-dx)*dy + vur*dx*dy; \n\
			*pofr++ = v + 0.5; \n\
		} \n\
		pofr += nexo; \n\
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
char origflag[]=ORIGFLAG;
char ldlib[]=LDLIB;
char tmplib[]=TMPLIB;
int execv();

int main(argc,argv)

int argc;
char **argv;

{
	Filename filename,objectname,progname,prognamei;
	h_boolean dflag,sflag,wflag,Wflag,iflag,Iflag;
	int rows,cols,c;
	char *ccode,*ccodei;
	Listarg arguments;
	char tempstring[400],msg[150];
	char *mktemp(),template[100],*tempfile;
	FILE *srsource,*infile,*infilei;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&dflag,&objectname,&sflag,&rows,&cols,
		&arguments,&Wflag,&ccode,&wflag,&progname,
		&iflag,&ccodei,&Iflag,&prognamei,FFONE,&filename);
	if (!Wflag && !wflag)
		perr(HE_MSG,"One of -w or -W must be specified");

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
	if (wflag) {
		if ((infile=fopen(progname,"r"))==NULL)
			perr(HE_OPEN,progname);
		else while ((c=getc(infile)) != EOF)
			putc(c,srsource);
	}
	else
		fprintf(srsource,"			%s;\n",ccode);
	fprintf(srsource,"%s",srtail);

	fclose(srsource);

	strcpy(tempstring,cc);
	strcat(tempstring," -o ");
	strcat(tempstring,objectname);
	strcat(tempstring," ");
	strcat(tempstring,ldlib);
	strcat(tempstring,"/calcwarpp.o ");
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
	strcat(tempstring," ");
	strcat(tempstring,origflag);
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
