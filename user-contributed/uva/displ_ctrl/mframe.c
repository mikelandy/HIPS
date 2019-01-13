/*	Copyright (c) 1982 Mickael Landy, Yoav Cohen, and George Sperling

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to 
insure its reliability.    */

/*
 * mframe.c - modify a frame on an output display device
 *
 * Usage: wframe [initialrow [initialcolumn]] [a..d] [-rgbC] [-t thresh] < seq
 *
 * Defaults:	centered on the display
 *
 * Load:	cc -O -o ../bin/mframe mframe.c itec/mframi.o \
 *			lex/mframl.o -lhips -llexf -lI
 *
 * Michael Landy/ Yoav Cohen - 2/4/82
 * Charles Carman (BME dept. UVA) - 9/23/86
 *
 * (initialrow, initialcolumn) specifies the screen position for the
 * frame coordinate (0,0).  Effective off-screen coordinates are lost,
 * so there is no wraparound.
 *
 * [a..d] are quadrant designations which can be iterated for more precise
 * positioning of the image.
 *
 * [-rgbC] are color designations to specify which frame buffer the images
 * will be written into. [-C] indicates that the sequence will be written 
 * into the red, green, and blue frame buffers in that order.
 *
 * [-t thresh] is a threshold below which the pixel is ignored, and above
 * which the pixel value is written into the display device.
 */

#include <stdio.h>
#include <hipl_format.h>
#include <image.h>

#define I_COLMAX 512
#define I_ROWMAX 480

main(argc,argv)
	int argc;
	char **argv;
{
	int left_col, top_row;
	int colmax, colcenter, rowmax, rowcenter;
	int frames = 1, thresh = 0, quad_flg, lc_flg, lex_flg;
	int i;
	short slc, str;
	char color = BW;
	struct header hd;

	Progname = strsave(*argv);
	left_col = top_row = 0;
	slc = str = 0;

	read_header(&hd);
	if (hd.pixel_format != PFBYTE) {
		fprintf(stderr,"wframe: frame must be in byte format\n");
		exit(1);
	}

	switch(getdev()) {
	case 'I':
		/* set up the variables for the itec boards */
		lex_flg = 0;
		itecinit(STD);
		colmax = I_COLMAX;
		rowmax = I_ROWMAX;
		if (zoomed()) {
			colmax /= 2;
			rowmax /= 2;
		}
		break;
	case 'L':
		/* set up the variables for the lexidata */
		lex_flg = 1;
		getsiz(&colmax,&rowmax);
		break;
	default:
		exit();
		break;
	}
	colcenter = colmax / 2;
	rowcenter = rowmax / 2;

	lc_flg = quad_flg = 0;
	for (i=1; i<argc; i++) {
		switch (getquad(argv[i],&slc,&str,colcenter,rowcenter)) {
		case 0:		/* next arg begins with an alpha char */
			if (argv[i][1] == 't')
				thresh = atoi(argv[++i]);
			else
				color = argv[i][1];
			break;
		case 1:		/* getquad found a legal quadrant */
			quad_flg++;
			break;
		case 2:		/* next arg begins with a number */
			if (lc_flg == 0) {
				top_row = atoi(argv[i]);
				lc_flg++;
			} else
				left_col = atoi(argv[i]);
			break;
		default:
			break;
		}
	}
	if (quad_flg == 0 && lc_flg == 0) {
		left_col = (colmax - hd.ocols) / 2;
		top_row = (rowmax - hd.orows) / 2;
	}
	if (quad_flg) {
		left_col = slc;
		top_row = str;
	} else {
		slc = left_col;
		str = top_row;
	}

	frames = hd.num_frame;
	if (!lex_flg && color == COLOR) {
		if (frames % 3 == 0) frames /= 3;
		else {
			fprintf(stderr,"input is NOT a COLOR image\n");
			exit(1);
		}
	}

	for (i=0; i<frames; i++) {
		if (i != 0) {
			left_col += hd.ocols;
			if (left_col + hd.ocols >= colmax) {
				left_col = slc;
				top_row += hd.orows;
				if (top_row >= rowmax) {
		  fprintf(stderr,"%s: display full; %d images not displayed\n",
					argv[0],frames-i);
					exit(1);
				}
			}
		} /* end of if */

		if (lex_flg)
			mframl(hd.ocols,hd.orows,left_col,top_row,thresh);
		else if (color == COLOR) {
			mframi(hd.ocols,hd.orows,left_col,top_row,thresh,RED);
			mframi(hd.ocols,hd.orows,left_col,top_row,thresh,GREEN);
			mframi(hd.ocols,hd.orows,left_col,top_row,thresh,BLUE);
		} else 
			mframi(hd.ocols,hd.orows,left_col,top_row,thresh,color);

	} /* end of frames for loop */
	return(0);
}

