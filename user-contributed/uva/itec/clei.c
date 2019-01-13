/*	PROGRAM
 *		cle_itec (RGB)
 *
 *	PURPOSE
 *		-  to initialize the frame buffer and its registers
 *		-  each pixel is initialized to a value of greylevel,
 *			zero is the default 
 *
 *	AUTHOR 
 *		Chuck Carmen
 *		for
 *		Merickel Imaging Lab
 *		Biomedical Engineering
 *		University of Virginia
 *		Charlottesville, VA 22903
 */
#include "image.h"
#define STRIPE 10

cle_itec(xul,yul,cols,rows,fbset,color,full_scr)
int full_scr;
unsigned short xul, yul, cols, rows;
unsigned char color;
char fbset;
{
	unsigned short scolor;
	unsigned short x, y;
	unsigned short line;
	unsigned short r;
	register i, j;

	line = STRIPE;
	x = xul;
	y = yul;

	itecinit(STD);
	unmask(fbset);

	switch (fbset) {
	default:
	case CHAR:
	case RED:
		if (full_scr) {
			/* Initialize the byte wide fb 	*/
			fbcinit();

			/* Cause a hardware clear  	*/
			FBC->PIXELLO = color;
			FBC->FBCTRHI = 0x40;
			while (FBC->FBCTRHI & 0xc0);

			/* Remove quirky line */
		/*	for (r=0;r<ROWMAX;r++)
				rdwrpt(color,line,r); */

		} else {
			/* Do a software clear of the indicated quadrant */
			FBC->FBCTRHI = INCXWR;
			for (r=y,i=0; i<rows; i++, r++) {
				swab(&r,&FBCW->Y,2);
				swab(&x,&FBCW->X,2);
				for (j=0; j<cols; j++)
					FBC->PIXELLO = color;
			}
		}

		/* Reinitialize these registers 	*/
		FBCW->X = 0;
		FBCW->Y = 0;
		FBC->FBCTRHI = 0;

		/* Quit if byte wide clear requested */
		if (fbset==CHAR || fbset==RED)  
			break;
	case BLUE:
	case GREEN:
	case SHORT:
		scolor = (color << 8) + color;
		if (full_scr) {
			/* Initialize word wide fb		*/
			fbsinit(fbset);

			/* Cause a hardware clear		*/
			FBSW->PIXEL = scolor;	
			FBS->FBCTRHI = 0x40;
			while (FBS->FBCTRHI & 0xc0);

			/* Remove the vertical stripe 	*/
		/*	for (r=0;r<ROWMAX;r++) {
				blwrpt(color,line,r);
				grwrpt(color,line,r);
			} */
		} else {
			/* Do a software clear of the indicated quadrant */
			FBS->FBCTRHI = INCXWR;
			r = y;
			for (i=0; i<rows; i++, r++) {
				swab(&r,&FBSW->Y,2);
				swab(&x,&FBSW->X,2);
				if (fbset == BLUE)
					for (j=0; j<cols; j++)
						FBS->PIXELLO = color;
				else if (fbset == GREEN)
					for (j=0; j<cols; j++)
						FBS->PIXELHI = color;
				else
					for (j=0; j<cols; j++)
						FBSW->PIXEL = scolor;
			}
		}

		/* Reinitialize to zero 		*/
		FBSW->X = 0;
		FBSW->Y = 0;
		FBS->FBCTRHI = 0;
		break;
	}
	
	switch(fbset) {
		case RED:
		case CHAR:
			FBCW->PAN = FBSW->PAN;
			FBCW->SCROLL = FBSW->SCROLL;
			break;
		case BLUE:
		case GREEN:
		case SHORT:
			FBSW->PAN = FBCW->PAN;
			FBSW->SCROLL = FBCW->SCROLL;
			break;
		default:
			break;
	}
	
	mask(fbset);
}	
