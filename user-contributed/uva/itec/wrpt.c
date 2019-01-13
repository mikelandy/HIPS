
/* 
 *	PROCEDURES
 *		wrpt
 *		
 *	PURPOSE
 * 		write a point to the red, green, and/or blue frame buffers.
 *
 *	SYNTAX
 *		wrpt(color,intense,x,y)
 *		xxwrpt(intense,x,y)
 *		where xx is the abbreviated color name (ie. red is rd)
 *		entering:
 *			color  : frame buffer to be written to
 *			  	   a -- all
 *				   b -- blue only
 *				   g -- green only
 *				   r -- red only
 *				   s -- blue and green 
 *			intense: intensity of point in frame buffer [0 -255]
 *			x,y    : absolute location in frame buffer.
 *
 *	AUTHOR
 *		Stuart Ware
 *		for 
 *		Merickel Imaging Labs
 *		Biomedical Engineering
 *		University of Virginia
 * 		Charlottesville, Virginia 22903
 */
#include "image.sh"

char blwrpt(intense,x,y)	/* WRITE POINT TO BLUE FRAME BUFFER */
unsigned char intense;
unsigned short x;
unsigned short y;
{
	char value;			/* value of previous blue point	*/
	unsigned short xoff, yoff;
	
	swab(&FBCW->PAN,&xoff,2);
	swab(&FBCW->SCROLL,&yoff,2);
	x += xoff;
	y += yoff;
	swab(&x,&FBBW->X,2);
	swab(&y,&FBBW->Y,2);
	value = FBB->PIXELLO;
	FBB->PIXELLO = intense;
	return(value);
}

char bwwrpt(intense,x,y)	/* WRITE POINT TO ALL FRAME BUFFERS */
unsigned char intense;
unsigned short x;
unsigned short y;
{
	char value;
	unsigned short xoff, yoff;
	
	swab(&FBCW->PAN,&xoff,2);
	swab(&FBCW->SCROLL,&yoff,2);
	x += xoff;
	y += yoff;
	swab(&x,&FBRW->X,2);
	swab(&y,&FBRW->Y,2);
	swab(&x,&FBBW->X,2);
	swab(&y,&FBBW->Y,2);
	value = FBB->PIXELLO;
	FBR->PIXELLO = intense;
	FBB->PIXELLO = intense;
	FBG->PIXELHI = intense;
	return(value);
}

char grwrpt(intense,x,y)	/* WRITE POINT TO GREEN FRAME BUFFER */
unsigned char intense;
unsigned short x;
unsigned short y;
{
	char value;
	unsigned short xoff, yoff;
	
	swab(&FBCW->PAN,&xoff,2);
	swab(&FBCW->SCROLL,&yoff,2);
	x += xoff;
	y += yoff;
	swab(&x,&FBGW->X,2);
	swab(&y,&FBGW->Y,2);
	value = FBG->PIXELHI;
	FBG->PIXELHI = intense;
	return(value);
}

char rdwrpt(intense,x,y)	/* WRITE POINT TO RED FRAME BUFFER */
unsigned char intense;
unsigned short x;
unsigned short y;
{
	char value;
	unsigned short xoff, yoff;
	
	swab(&FBCW->PAN,&xoff,2);
	swab(&FBCW->SCROLL,&yoff,2);
	x += xoff;
	y += yoff;
	swab(&x,&FBRW->X,2);
	swab(&y,&FBRW->Y,2);
	value = FBR->PIXELLO;
	FBR->PIXELLO = intense;
	return(value);
}

char wrpt(color,intense,x,y)		/* FRAME BUFFER POINT 		*/
char color;
unsigned char intense;
unsigned short x;
unsigned short y;
{
	char rdwrpt(), grwrpt(), blwrpt(), bwwrpt();

	switch (color) {
                case CHAR:
		case RED:
			return(rdwrpt(intense,x,y));
			break;
		case GREEN:
			return(grwrpt(intense,x,y));
			break;
		case BLUE:
			return(blwrpt(intense,x,y));
			break;
                default:
			return(bwwrpt(intense,x,y));
			break;
                case SHORT:
			grwrpt(intense,x,y);
			return(blwrpt(intense,x,y));
			break;
                        
	}
}
