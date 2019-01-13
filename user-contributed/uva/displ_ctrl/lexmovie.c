/*	Copyright (c) 1982 Mickael Landy, Yoav Cohen, and George Sperling

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to 
insure its reliability.    */

/*
 * lexmovie.c - write a sequence on the Lexidata, and display the frames
 *		as a movie.
 *
 * Usage:	lexmovie [frames/second [#frames rows]] < sequence
 *
 * Defaults:	the first frame goes in the top left corner
 *		the display is zoomed so that a frame fills the display
 *		the frames sequence at 6 per second
 *
 * Load:	cc -O -o lexmovie lexmovie.c ../lex/wframl.o -lhips -llexf -lI
 *
 * Michael Landy/ Yoav Cohen - 2/4/82
 * Charles Carman (BME dept. UVA) - 3/18/86
 * Charles Carman (BME dept. UVA) - 5/19/88
 */

#include <stdio.h>
#include <hipl_format.h>

main(argc,argv)
	int argc;
	char **argv;
{
	unsigned size;
	int colmax, rowmax, left_col, top_row, upd_flg;
	int i, frames;
	short err, chan;
	short sx, sy, xsiz, ysiz, pwr, repcnt, npic, xmax;
	char *buf, *malloc();
	struct header hd;

	Progname = strsave(*argv);
	upd_flg = 0;
	repcnt = 10;
	if (argc > 1) {
		if (argv[1][0] == '-') {
fprintf(stderr,"Syntax: lexmovie [frames/sec [#frames rows]]\n");
			exit(1);
		}
		i = atoi(argv[1]);
		repcnt = 60 / i;
		if (argc > 2) {
			upd_flg = 1;
			frames = atoi(argv[2]);
			ysiz = atoi(argv[3]);
		}
	}

	if (getdev() != 'L') {
		fprintf(stderr,"lexmovie: display device not the Lexidata\n");
		exit(1);
	}

	if (!upd_flg) {
		read_header(&hd);
		if (hd.pixel_format != PFBYTE) {
		fprintf(stderr,"lexmovie: sequence must be in byte format\n");
			exit(1);
		}
		size = hd.orows * hd.ocols;
		frames = hd.num_frame;
	}

	/* set up the variables for the lexidata */
	getsiz(&colmax,&rowmax);
	if (!upd_flg) ysiz = rowmax / (int)(rowmax / hd.orows);
	xsiz = 5 * ysiz / 4;
	if (upd_flg) xmax = xsiz * (int)(rowmax / ysiz);
	npic = frames;

	if (!upd_flg) {
		if ((buf = malloc(size)) == NULL) {
			fprintf(stderr,"lexmovie: error allocating buffer\n");
			exit(1);
		}

		xmax = 0;
		left_col = top_row = 0;
		for (i=0; i<frames; i++) {
			if (i != 0) {
				left_col += xsiz;
				if (left_col + xsiz > colmax) {
					if (xmax == 0) xmax = left_col - 1;
					left_col = 0;
					top_row += ysiz;
					if (top_row + ysiz > rowmax) {
		  fprintf(stderr,"%s: display full; %d images not displayed\n",
					argv[0],frames-i);
						exit(1);
					}
				}
			} /* end of if */

			fread(buf,size,1,stdin);
			wframl(hd.ocols,hd.orows,left_col,top_row,buf);
		} /* end of frames for loop */
		free(buf);
	}

	/* zoom to the first frame */
	dsopn_(&err, &chan);
	sx = sy = 0;
	pwr = rowmax / ysiz;
	dszom_(&sx, &sy, &pwr);

	/* start the movie */
	if (xmax == 0) xmax = npic * xsiz - 1;
	dsmov_(&ysiz, &xmax, &npic, &repcnt);
	dscls_();

	return(0);
}

