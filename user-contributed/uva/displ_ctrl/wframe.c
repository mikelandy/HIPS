/*	Copyright (c) 1982 Mickael Landy, Yoav Cohen, and George Sperling

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to 
insure its reliability.    */

/*
 * wframe.c - write a frame on an output display device
 *
 * Usage:	wframe [initialrow [initialcolumn]] [a..d] [-rgbC] < sequence
 *
 * Defaults:	centered on the Itec display
 *
 * Load:	cc -O -o ../bin/wframe wframe.c ../itec/wframi.o \
 *			../lex/wframl.o -lhips -llexf -lI
 *
 * Michael Landy/ Yoav Cohen - 2/4/82
 * Charles Carman (BME dept. UVA) - 3/18/86
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
	unsigned size;
	int left_col, top_row, frames;
	int colmax, colcenter, rowmax, rowcenter;
	int lc_flg, quad_flg, lex_flg;
	int i;
	short slc, str;
	char *rbuf, *gbuf, *bbuf, *buf, *malloc();
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
	size = hd.orows * hd.ocols;

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
	for (argc--; argc > 0; argc--) {
		switch (getquad(argv[argc],&slc,&str,colcenter,rowcenter)) {
		case 0:		/* next arg begins with an alpha char */
			color = argv[argc][1];
			break;
		case 1:		/* getquad found a legal quadrant */
			quad_flg++;
			break;
		case 2:		/* next arg begins with a number */
			if (lc_flg == 0) {
				left_col = atoi(argv[argc]);
				lc_flg++;
			} else
				top_row = atoi(argv[argc]);
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
		rbuf = malloc(size);
		gbuf = malloc(size);
		bbuf = malloc(size);
		if (rbuf == NULL || gbuf == NULL || bbuf == NULL) {
			fprintf(stderr,"wframe: error allocating buffers\n");
			exit(1);
		}
		if (frames % 3 == 0) frames /= 3;
		else {
			fprintf(stderr,"Input NOT a color image\n");
			exit(2);
		}
	} else
		if ((buf = malloc(size)) == NULL) {
			fprintf(stderr,"wframe: error allocating buffers\n");
			exit(1);
		}

	for (i=0; i<frames; i++) {
		if (i != 0) {
			left_col += hd.ocols;
			if (left_col + hd.ocols > colmax) {
				left_col = slc;
				top_row += hd.orows;
				if (top_row >= rowmax) {
		  fprintf(stderr,"%s: display full; %d images not displayed\n",
					argv[0],frames-i);
					exit(1);
				}
			}
		} /* end of if */

		if (!lex_flg && color == COLOR) {
			fread(rbuf,size,1,stdin);
			fread(gbuf,size,1,stdin);
			fread(bbuf,size,1,stdin);
		} else
			fread(buf,size,1,stdin);

		if (lex_flg)
			wframl(hd.ocols,hd.orows,left_col,top_row,buf);
		else if (color == COLOR)
		    wframi_c(hd.ocols,hd.orows,left_col,top_row,rbuf,gbuf,bbuf);
		else if (color == BW)
			wframi_b(hd.ocols,hd.orows,left_col,top_row,buf);
		else
			wframi_a(hd.ocols,hd.orows,left_col,top_row,color,buf);

	} /* end of frames for loop */
	return(0);
}

