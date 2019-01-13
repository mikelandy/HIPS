
/* 
 *	PROCEDURES
 *		rdpt
 *		
 *	PURPOSE
 * 		read a point from the red, green, or blue frame buffers.
 *
 *	SYNTAX
 *		xxrdpt(x,y)
 *		where xx is the abbreviated color name (ie. red is rd)
 *			x,y    : location in frame buffer.
 *
 *	AUTHOR
 *		Stuart Ware
 *		for 
 *		Merickel Imaging Labs
 *		Biomedical Engineering
 *		University of Virginia
 * 		Charlottesville, Virginia 22903
 *
 */
#include "image.sh"

char blrdpt(x,y)	/* READ POINT FROM BLUE FRAME BUFFER */
unsigned short x;
unsigned short y;
{
	unsigned short xoff, yoff;
	
	swab(&FBCW->PAN,&xoff,2);
	swab(&FBCW->SCROLL,&yoff,2);
	x += xoff;
	y += yoff;
	swab(&x,&FBBW->X,2);
	swab(&y,&FBBW->Y,2);
	return(FBB->PIXELLO);
}


char rdrdpt(x,y)	/* READ POINT FROM RED FRAME BUFFER */
unsigned short x;
unsigned short y;
{
	unsigned short xoff, yoff;
	
	swab(&FBCW->PAN,&xoff,2);
	swab(&FBCW->SCROLL,&yoff,2);
	x += xoff;
	y += yoff;
	swab(&x,&FBRW->X,2);
	swab(&y,&FBRW->Y,2);
	return(FBR->PIXELLO);
}


char grrdpt(x,y)	/* READ POINT FROM GREEN FRAME BUFFER */
unsigned short x;
unsigned short y;
{
	unsigned short xoff, yoff;
	
	swab(&FBCW->PAN,&xoff,2);
	swab(&FBCW->SCROLL,&yoff,2);
	x += xoff;
	y += yoff;
	swab(&x,&FBGW->X,2);
	swab(&y,&FBGW->Y,2);
	return(FBG->PIXELHI);
}
