/*	PROGRAM
 *		cle  (RGB)
 *
 *	PURPOSE
 *		-  to initialize the frame buffer and its registers
 *		-  each pixel is initialized to a value of greylevel,
 *			zero is the default 
 *		- uses the environment variable DSPDEV to specify the 
 *			display device.
 *
 *	SYNTAX
 *		cle [-csrgb] [abcd] [graylevel] \n
 *
 *	AUTHOR 
 *		Chuck Carmen
 *		for
 *		Merickel Imaging Lab
 *		Biomedical Engineering
 *		University of Virginia
 *		Charlottesville, VA 22903
 */
#include <stdio.h>
#include <image.h>

#define DASH '-'
#define I_COLMAX 512
#define I_ROWMAX 480

main(argc,argv)
int argc;
char *argv[];
{
	char fbset, devc;
        unsigned char color;
	unsigned short x, y;
	unsigned short cols, rows;
	int colmax, rowmax;
	int full_scr, lexflg, strl;
	register i;

	fbset = BW;
	color = 0;
	x = y = 0;
	full_scr = YES;

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
		fprintf(stderr,"Not a recognised device!\n");
		exit(2);
		break;
	}

	for (i=(--argc); i>0; i--) {
		switch (getquad(argv[i],&x,&y,colmax/2,rowmax/2)) {
		case 0:
			fbset = *(++argv[i]);
			break;
		case 1:
			strl = strlen(argv[i]);
			cols = colmax >> strl;
			rows = rowmax >> strl;
			full_scr = NO;
			break;
		case 2:
			color = (unsigned char) atoi(argv[i]);
			break;
		default:
			break;
		}
	}
	if (fbset == RED || fbset == GREEN || fbset == BLUE)
		full_scr = NO;

	if (lexflg)
		cle_lex(x,y,cols,rows,color,full_scr);
	else
		cle_itec(x,y,cols,rows,fbset,color,full_scr);
}

