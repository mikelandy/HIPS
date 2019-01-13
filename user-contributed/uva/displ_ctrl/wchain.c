/*	PROGRAM
 *		wrchain
 *
 *	PURPOSE
 *		Reconstruct chain coded outlining on Image Technology 
 *		frame buffers and the Lexidata.
 *
 *	SYNOPSIS
 *		wrline [a-d] [0..255] 
 *
 *			[a-d]	 the quadrant position
 *			[0..255] the intensity at which to display 
 *				 the outline the default is 255.
 *
 *		a chain coded file is expected from standard input.
 *
 *	AUTHOR
 *		Thao Le 
 *		for
 *		Merickel Imaging Lab
 *		Biomedical Engineering
 *		University of Virginia
 *		Charlottesville, Va.  22903
 *
 *	REVISIONS
 *		5/13/86		Allowed for standard input. 
 *		Stuart Ware	Used Itec library routines.
 *		Chuck Carman	Added Lex output
 */
#include <stdio.h>
#include <hipl_format.h>
#include <image.h>

#define	I_COLMAX 512
#define I_ROWMAX 480

int CC_DIR[2][8] = {
	{ 1,  1,  0, -1, -1, -1, 0, 1 },
	{ 0, -1, -1, -1,  0,  1, 1, 1 }
};

main(argc,argv)
	int argc;
	char *argv[];
{
	struct header hd;
	int colmax, colcenter, rowmax, rowcenter;
	int lex_flg, quad_flg;
	unsigned short pixval;
	short tr, lc;

	Progname = strsave(*argv);
	hd.cols = hd.rows = hd.ocols = hd.orows = 0;
	read_header(&hd);
	if (hd.pixel_format != PFCHAIN) {
		fprintf(stderr,"wrchain: input must be chain code data\n");
		exit(1);
	}

	switch (getdev()) {
	case 'I':
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
		lex_flg = 1;
		getsiz(&colmax,&rowmax);
		break;
	default:
		fprintf(stderr,"wrchain: unknown device\n");
		exit(2);
	}
	colcenter = colmax / 2;
	rowcenter = rowmax / 2;

	/*set flags to decode the input parameters*/
	tr = lc = 0;
	pixval = 255;
	quad_flg = 0;
	for (argc--; argc > 0; argc--) {
		switch (getquad(argv[argc],&lc,&tr,colcenter,rowcenter)) {
		case 0: /* arg begins with a '-' */
			break;
		case 1: /* arg defined a quadrant */
			quad_flg++;
			break;
		case 2: /* arg begins with an number */
			pixval = (short) atoi(argv[argc]);
			break;
		}
	}
	if (quad_flg == 0) {
		lc = (colmax - hd.ocols) / 2;
		tr = (rowmax - hd.orows) / 2;
	}

	if (lex_flg)
		wrchn_lex(tr,lc,pixval);
	else
		wrchn_itc(tr,lc,pixval);

	return(0);
 }
