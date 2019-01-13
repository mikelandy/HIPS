/*	Copyright (c) 1982 Mickael Landy, Yoav Cohen, and George Sperling

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to 
insure its reliability.    */

/*
 * rframe.c - read a frame from a display device
 *
 * Usage: 
 *   rframe [-p left_col [top_row]] [-s cols [rows]] [a..d] [-rgbC] > seq
 *
 * Defaults:	centered on the Itec display
 *
 * Load:	cc -O -o ../bin/rframe rframe.c ../itec/rframi.o \
 *			../lex/rframl.o -lhips -llexf -lI
 *
 * Michael Landy/ Yoav Cohen - 2/4/82
 * Charles Carman (BME dept. UVA) - 10/22/86
 *
 * (left_col, top_row) specifies the screen position for the
 * frame coordinate (0,0).  Effective off-screen coordinates are lost,
 * so there is no wraparound.
 *
 * (cols, rows) specifies the size of the image to be read.
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
	int left_col, top_row, rows, cols, frames;
	int colmax, colcenter, rowmax, rowcenter;
	int lex_flg, lc_flg, pos_flg, siz_flg;
	int quad_flg = 0, err_flg = 0, pd_flg = 0;
	int i;
	short slc, str;
	char tmpc, color = BW, dev_nm[16];
	struct header hd;

	Progname = strsave(*argv);
	left_col = top_row = 0;
	cols = rows = 0;
	slc = str = 0;
	frames = 1;

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
		strcpy(dev_nm,"Itec");
		break;
	case 'L':
		/* set up the variables for the lexidata */
		lex_flg = 1;
		getsiz(&colmax,&rowmax);
		strcpy(dev_nm,"Lexidata");
		break;
	default:
		exit();
		break;
	}
	colcenter = colmax / 2;
	rowcenter = rowmax / 2;

	for (i=1; i < argc; i++) {
		switch (getquad(argv[i],&slc,&str,colcenter,rowcenter)) {
		case 0:		/* next arg begins with an alpha char */
			tmpc = argv[i][1];
			switch (tmpc) {
			case 'p':
				pd_flg++;
				pos_flg++;
				siz_flg = 0;
				lc_flg = 1;
				break;
			case 's':
				siz_flg++; 
				pos_flg = 0;
				lc_flg = 1;
				break;
			case 'h':
				syntax();
				break;
			case 'b':
			case 'g':
			case 'r':
			case 'C':
				color = tmpc;
				break;
			default:
			fprintf(stderr,"%s: unrecognixed argument.\n",argv[0]);
				syntax();
				break;
			}
			break;
		case 1:		/* getquad found a legal quadrant */
			quad_flg++;
			rows = rowmax >> strlen(argv[i]);
			cols = colmax >> strlen(argv[i]);
			break;
		case 2:		/* next arg begins with a number */
			if (pos_flg) {
				if (lc_flg) {
					left_col = atoi(argv[i]);
					lc_flg = 0;
				} else {
					top_row = atoi(argv[i]);
					pos_flg = 0;
				}
			}
			if (siz_flg) {
				if (lc_flg) {
					cols = atoi(argv[i]);
					lc_flg = 0;
				} else {
					rows = atoi(argv[i]);
					siz_flg = 0;
				}
			}
			break;
		default:
			break;
		}
	}
	if (rows == 0) rows = rowmax;
	if (cols == 0) cols = colmax;
	if (!quad_flg && !pd_flg) {
		left_col = (colmax - cols) / 2;
		top_row = (rowmax - rows) / 2;
	}
	if (quad_flg) {
		left_col = slc;
		top_row = str;
	} else {
		slc = left_col;
		str = top_row;
	}

	if (left_col + cols > colmax) {
		fprintf(stderr,"specified image has too many columns\n");
		err_flg++;
	}
	if (top_row + rows > rowmax) {
		fprintf(stderr,"specified image has too many rows\n");
		err_flg++;
	}
	if (err_flg) exit(2);

fprintf(stderr,"rframe: -p %d %d, -s %d %d\n",slc,str,cols,rows);

	if (!lex_flg && color == COLOR ) frames = 3;

	init_header(&hd,dev_nm,"",frames,"",rows,cols,PFBYTE,1,"");
	update_header(&hd,argc,argv);
	write_header(&hd);

	if (lex_flg)
		rframl(hd.cols,hd.rows,left_col,top_row);
	else if (color == COLOR) {
		rframi(hd.cols,hd.rows,left_col,top_row,RED);
		rframi(hd.cols,hd.rows,left_col,top_row,GREEN);
		rframi(hd.cols,hd.rows,left_col,top_row,BLUE);
	} else 
		rframi(hd.cols,hd.rows,left_col,top_row,color);

	return(0);
}

syntax()
{
fprintf(stderr,
  "rframe [-p left_col [top_row]] [-s cols [rows]] [a..d] [-rgbC] > seq\n");
	exit(1);
}

