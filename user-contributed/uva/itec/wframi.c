/*
 * wframi.c - write a frame on the Itec boards
 *
 * Charles Carman 9/23/86
 */

#include <stdio.h>
#include "image.sh"

wframi_a(cols,rows,left_col,top_row,color,buf)
int cols, rows, left_col, top_row;
char color;
unsigned char *buf;
{
	register unsigned char *pb;
	int i, right_col, bottom_row;
	register int j;
	short scol, srow;

	itecinit(STD);

	switch(color) {
	case RED:
		swab(&FBCW->PAN,&scol,2);
		swab(&FBCW->SCROLL,&srow,2);
		FBR->FBCTRHI = INCXWR;
		break;
	case GREEN:
	case BLUE:
		swab(&FBSW->PAN,&scol,2);
		swab(&FBSW->SCROLL,&srow,2);
		FBS->FBCTRHI = INCXWR;
		break;
	}

	scol += left_col;
	srow += top_row;
	right_col = left_col + cols; bottom_row = top_row + rows;

	unmask(color);

	pb = buf;
	for (i=top_row;i<bottom_row;i++,srow++) 
	{
		switch(color) {
		case RED:
			swab(&srow,&FBCW->Y,2);
			swab(&scol,&FBCW->X,2);
			for (j=0; j<cols; j++, pb++) {
				if (*pb == WHITE) *pb = WHITE-1;
				FBR->PIXELLO = *pb;
			}
			break;
		case GREEN:
			swab(&srow,&FBSW->Y,2);
			swab(&scol,&FBSW->X,2);
			for (j=0; j<cols; j++, pb++) {
				if (*pb == WHITE) *pb = WHITE-1;
				FBG->PIXELHI = *pb;
			}
			break;
		case BLUE:
			swab(&srow,&FBSW->Y,2);
			swab(&scol,&FBSW->X,2);
			for (j=0; j<cols; j++, pb++) {
				if (*pb == WHITE) *pb = WHITE-1;
				FBB->PIXELLO = *pb;
			}
			break;
		}
	}

	switch(color) {
	case RED:
		FBR->FBCTRHI = 0;
		break;
	case GREEN:
	case BLUE:
		FBS->FBCTRHI = 0;
		break;
	}

	mask(BW);
}

wframi_b(cols,rows,left_col,top_row,buf)
int cols, rows, left_col, top_row;
unsigned char *buf;
{
	register unsigned char *pb;
	int i, right_col, bottom_row;
	register int j;
	short scol, srow;

	itecinit(STD);
	unmask(BW);

	swab(&FBSW->PAN,&scol,2);
	swab(&FBSW->SCROLL,&srow,2);
	FBC->FBCTRHI = INCXWR;
	FBS->FBCTRHI = INCXWR;

	scol += left_col;
	srow += top_row;
	right_col = left_col + cols; bottom_row = top_row + rows;

	pb = buf;
	for (i=top_row;i<bottom_row;i++,srow++) 
	{
		swab(&srow,&FBCW->Y,2);
		swab(&scol,&FBCW->X,2);
		FBSW->Y = FBCW->Y;
		FBSW->X = FBCW->X;
		for (j=0; j<cols; j++, pb++) {
			if (*pb == WHITE) *pb = WHITE-1;
			FBC->PIXELLO = *pb;
			FBSW->PIXEL = (*pb << 8) + *pb;
		}
	}

	FBC->FBCTRHI = 0;
	FBS->FBCTRHI = 0;

	mask(BW);
}

wframi_c(cols,rows,left_col,top_row,rbuf,gbuf,bbuf)
int cols, rows, left_col, top_row;
unsigned char *rbuf, *gbuf, *bbuf;
{
	register unsigned char *prb, *pgb, *pbb;
	register int j;
	int i, right_col, bottom_row;
	short scol, srow;

	itecinit(STD);
	unmask(BW);

	swab(&FBSW->PAN,&scol,2);
	swab(&FBSW->SCROLL,&srow,2);
	FBC->FBCTRHI = INCXWR;
	FBS->FBCTRHI = INCXWR;

	scol += left_col;
	srow += top_row;
	right_col = left_col + cols; bottom_row = top_row + rows;

	prb = rbuf; pgb = gbuf; pbb = bbuf;
	for (i=top_row;i<bottom_row;i++,srow++) 
	{
		swab(&srow,&FBCW->Y,2);
		swab(&scol,&FBCW->X,2);
		FBSW->Y = FBCW->Y;
		FBSW->X = FBCW->X;
		for (j=0; j<cols; j++) {
			FBC->PIXELLO = *prb++;
			FBG->PIXELHI = *pgb++;
			FBB->PIXELLO = *pbb++;
		}
	}

	FBC->FBCTRHI = 0;
	FBS->FBCTRHI = 0;

	mask(BW);
}
