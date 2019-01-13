/*
 * vaximgld.c - Program to download an image sequence into Adage memory
 * 
 * This program downloads all images from the specified file to the frame
 * buffer, packing the images into the frame buffer as closely as possible.  
 * Immediately after the images in the frame buffer it loads a blank frame
 * and a cuespot frame both equal in size to the other images downloaded.
 * 
 * 03-31-87 PH  Created using msls routine mov14
 *
 * usage:	vaximgld  [-b] [-c[val]] [-B] [-C] [-R] 
 *				[-l[0123]] startframe movie_file 
 *
 *		-u describe usage of vaximgld
 *		-b adds a blank frame at the end.
 *		-c adds a cue spot (with given lut value) after the blank frame
 *		-B acts as if the movie is binary for frame placement
 *		-C avoids crossing 1024 column boundaries
 *		-R avoids crossing 1024 row boundaries
 *		-l uses lores mode, and optionally specifies the byte
 *			number (which defaults to zero)
 *
 * Mike Landy - 1/20/87
 */

#include <stdio.h>
#include <hipl_format.h>
#include <graphics/ik_const.h>

char Usage[] = "usage:	vaximgld  [-b] [-c[val]] [-B] [-C] [-R]\n\
			[-l[0123]] startframe movie_file\n\
	-u describe usage of vaximgld\n\
	-b adds a blank frame at the end\n\
 	-c[val] adds a cue spot (with given lut value) after the blank frame\n\
	-B acts as if the movie is binary for frame placement\n\
	-C avoids crossing 1024 column boundaries\n\
	-R avoids crossing 1024 row boundaries\n\
	-l uses lores mode, and optionally specifies the byte\n\
		number (which defaults to zero)\n";

char *errors[] = {
	"can't open file",				/* 1 */
	"frame must be in byte format",			/* 2 */
	"binary movies must be LSBFIRST",		/* 3 */
	"can't fit sequence in Adage memory",		/* 4 */
	"not enough core",				/* 5 */
	"can't allocate input frame core",		/* 6 */
	"error reading primary input",			/* 7 */
	"lores images must not be bit-packed"};		/* 8 */


main(argc,argv)

int argc;
char *argv[];

{
	int startframe = 0;
	int Binary = 0;
	int Row = 0;
	int Col = 0;
	int cuesw = 0;
	int cueval = 228;
	int blanksw = 0;
	int lores = 0;
	int byteno = 0;
	int currarg,i;
	char tmp[100];

	Progname = strsave(*argv);
	Ik_openn();
	Ik_set_mode( SET_32_BIT_MODE );

	currarg = 1;
	while (currarg < argc) {
	    if (argv[currarg][0]=='-') {
		switch (argv[currarg][1]) {
		case 'D':	break;
		case 'c':	cuesw++;
				blanksw++;
				if (argv[currarg][2] != '\0' )
					cueval = atoi( &(argv[currarg][2]) );
				break;
		case 'b':	blanksw++;
				break;
		case 'B':	Binary++;
				break;
		case 'R':	Row++;
				break;
		case 'C':	Col++;
				break;
		case 'u':	printf( "%s", Usage );
				return; 
		case 'l':	lores++;
				if (argv[currarg][2] != '\0' )
					byteno = atoi(&(argv[currarg][2]));
				break;
		default:	sprintf(tmp,"unrecognized option %1s",
					&argv[currarg][1]);
				perr(HE_MSG,tmp);
		}
	    }
	    currarg++;
	}
	if (byteno < 0 || byteno > 3)
		perr(HE_MSG,"invalid lores byte number");
	startframe = atoi(argv[argc-2]);
	
	if (lores)
		i=loadimgl(cuesw,cueval,blanksw,startframe,
			byteno,argv[argc-1]);
	else
		i=loadimg(cuesw,cueval,blanksw,Binary,Col,Row,
			startframe,argv[argc-1]);

	Ik_close();
	if (i)
		perr(HE_MSG,errors[i-1]);
	exit(0);
}
