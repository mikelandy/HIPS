/*
 *	PROGRAM
 *		drln
 *
 *	SYNTAX
 *		drln (begx, begy, endx, endy, color, gray)
 *
 *	PURPOSE
 *		to draw a line (using Bressenham' algorithm)
 *		from (begx, begy) to (endx, endy) with gray value
 *
 *	Merickel Imaging Lab
 *	Biomedical Engineering
 *	University of Virginia
 *	Charlottesville, VA 22908
 */
#include <image.sh>

drln(xb, yb, xe, ye, color, gray)
	char color;
	unsigned char gray;
	unsigned short xb, yb, xe, ye;
{
	unsigned short xoff, yoff, sgrey;
	int dx, dy, incx, incy, incr1, incr2;
	register int i, d, end, inc;

	sgrey = (gray << 8) + gray;

	swab(&FBSW->PAN, &xoff, 2);
	swab(&FBSW->SCROLL, &yoff, 2);
	xoff += xb;
	yoff += yb;
	swab(&xoff, &FBSW->X, 2);
	swab(&yoff, &FBSW->Y, 2);
	FBCW->X = FBSW->X;
	FBCW->Y = FBSW->Y;
	
	switch (color) {
		case RED:
		case CHAR:
			FBSW->MASK = 0xffff;
			break;
		case BLUE:
			FBC->MASKLO = FBS->MASKHI = 0xff;
			break;
		case GREEN:
			FBC->MASKLO = FBS->MASKLO = 0xff;
			break;
		case BW:
		case COLOR:
		case SHORT:
		default:
			FBC->MASKLO = 0;
			FBSW->MASK = 0;
			break;
	}

	dx = xe - xb;
	if (dx < 0) {
		incx = -1;
		dx *= incx;
	} else incx = 1;
	dy = ye - yb;
	if (dy < 0) {
		incy = -1;
		dy *= incy;
	} else incy = 1;

	if (dx >= dy) {
		FBC->FBCTRHI = FBS->FBCTRHI = (incx == 1) ? INCXWR : DECXWR;
		FBSW->PIXEL = sgrey;
		FBC->PIXELLO = gray;
		incr1 = dy << 1;
		d = incr1 - dx;
		incr2 = (dy - dx) << 1;
		inc = incy;
		for (i=0, end=dx; i<end; i++) {
			if (d < 0)
				d += incr1;
			else {
				d += incr2;
				if (inc == 1) {
					FBS->YLO++;
					if (FBS->YLO == 0)
						FBS->YHI++;
				} else {
					if (FBS->YLO == 0)
						FBS->YHI--;
					FBS->YLO--;
				}
				FBCW->Y = FBSW->Y;
			}
			FBSW->PIXEL = sgrey;
			FBC->PIXELLO = gray;
		}
	} else {
		FBC->FBCTRHI = FBS->FBCTRHI = (incy == 1) ? INCYWR : DECYWR;
		FBSW->PIXEL = sgrey;
		FBC->PIXELLO = gray;
		incr1 = dx << 1;
		d = incr1 - dy;
		incr2 = (dx - dy) << 1;
		inc = incx;
		for (i=0, end=dy; i<end; i++) {
			if (d < 0)
				d += incr1;
			else {
				d += incr2;
				if (inc == 1) {
					FBS->XLO++;
					if (FBS->XLO == 0)
						FBS->XHI++;
				} else {
					if (FBS->XLO == 0)
						FBS->XHI--;
					FBS->XLO--;
				}
				FBCW->X = FBSW->X;
			}
			FBSW->PIXEL = sgrey;
			FBC->PIXELLO = gray;
		}
	}
	FBC->FBCTRHI = FBS->FBCTRHI = 0;
	FBC->MASKLO = 0;
	FBSW->MASK = 0;
}
