/*
 *	PROGRAM
 *		zoom (RGB configuration)
 *
 *	PURPOSE 
 *		enlarge part of the displayed image by reducing the
 *		amount of the image displayed.
 *		the ITEC boards have a hardware zoom of only a factor of 2
 *		the Lexidata has a hardware zoom of up to a factor of 16
 *
 *	SYNOPSIS 
 *		zoom [abcd] [zoom factor]
 *      -  letters indicate a quadrant
 *      -  no argument defaults to center of current display
 *	-  zoom factor is magnification factor number (default = 2)
 *
 *	AUTHOR
 *		Charles Carman
 *		for
 * 		Merickel Imaging Lab
 *		Biomedical Engineering
 *		University of Virginia
 *		Charlottesville, VA   22903	
 *		
 */
#include <stdio.h>
#include <image.h>
#include <lexioctl.h>

#define I_COLMAX COLMAX
#define I_ROWMAX ROWMAX

main(argc, argv)
int argc;
char *argv[];
{
	int i, lexflg, quadflg, devc;
	int colmax, rowmax;
	short x, y, pwr, err, chan;

	if ((devc = getdev()) == 0) 
		exit(1);
	switch (devc) {
	case 'I':
		lexflg = 0;
		colmax = I_COLMAX;
		rowmax = I_ROWMAX;
		break;
	case 'L':
		lexflg = 1;
		getsiz(&colmax,&rowmax);
		break;
	default:
		exit(2);
	}

	quadflg=0;
	x=0;
	y=0;
	pwr=2;

	for (i=argc-1; i>0; --i) {
		switch (getquad(argv[i],&x,&y,colmax/2,rowmax/2)) {
		case 0:
			fprintf(stderr,"Syntax: zoom [abcd] [zoom factor]\n");
			exit(2);
			break;
		case 1:
			quadflg++;
			if (pwr == 2) pwr = 1 << strlen(argv[i]);
			break;
		case 2:
			pwr = (short) atoi(argv[i]);
			break;
		}
	}

	if (!quadflg) {
		x = (colmax / 2) - (colmax / (1 << pwr));
		y = (rowmax / 2) - (rowmax / (1 << pwr));
	}

	if (lexflg) {
		/* x must be a multiple of 20 (see manual) */
		x = (x / 20) * 20;
		dsopn_(&err, &chan);
		dszom_(&x,&y,&pwr);
		dscls_();
	} else {
		/* pwr is ignored, will always be 2 */
		itecinit(STD);
		dozoom (x, y);
	}
}

