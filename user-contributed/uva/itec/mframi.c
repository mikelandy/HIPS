/*
 * mframi.c - modify a frame on the Itec boards
 *
 * Charles Carman 3/18/86
 */

#include <stdio.h>
#include "image.sh"

mframi(cols,rows,left_col,top_row,thresh,color) 
int cols, rows, left_col, top_row, thresh;
char color;
{
	register unsigned char *pb;
	unsigned char buf[COLMAX];
	int i, right_col, bottom_row;
	register int j;
	unsigned char cval;
	short scol, srow, sval;

	itecinit(STD);
	unmask(BW);

	swab(&FBSW->PAN,&scol,2);
	swab(&FBSW->SCROLL,&srow,2);

	scol += left_col;
	srow += top_row;
	right_col = left_col + cols; bottom_row = top_row + rows;

	for (i=top_row;i<bottom_row;i++,srow++) 
	{
		pb = buf;
		for (j=left_col;j<right_col;j++) 
			*pb++ = 0377 & getchar();

		swab(&srow,&FBSW->Y,2);
		swab(&scol,&FBSW->X,2);
		FBCW->Y = FBSW->Y;
		FBCW->X = FBSW->X;

		pb = buf;
		switch(color) {
		case RED:
			FBR->FBCTRHI = INCXRD;
			for (j=0; j<cols; j++, pb++) {
				if (*pb == WHITE) *pb = WHITE - 1;
				cval = FBR->PIXELLO;
				if (*pb > thresh) FBR->PIXELLO = *pb;
			}
			break;
		case GREEN:
			FBG->FBCTRHI = INCXRD;
			for (j=0; j<cols; j++, pb++) {
				if (*pb == WHITE) *pb = WHITE - 1;
				cval = FBG->PIXELHI;
				if (*pb > thresh) FBG->PIXELHI = *pb;
			}
			break;
		case BLUE:
			FBB->FBCTRHI = INCXRD;
			for (j=0; j<cols; j++, pb++) {
				if (*pb == WHITE) *pb = WHITE - 1;
				cval = FBB->PIXELLO;
				if (*pb > thresh) FBB->PIXELLO = *pb;
			}
			break;
		default:
			FBC->FBCTRHI = INCXRD;
			FBS->FBCTRHI = INCXRD;
			for (j=0; j<cols; j++, pb++) {
				if (*pb == WHITE) *pb = WHITE - 1;
				cval = FBC->PIXELLO;
				sval = FBSW->PIXEL;
				if (*pb > thresh) {
					FBC->PIXELLO = *pb;
					FBSW->PIXEL = (*pb << 8) + *pb;
				}
			}
			break;
		}
	}

	FBC->FBCTRHI = 0;
	FBS->FBCTRHI = 0;
	mask(BW);
}
