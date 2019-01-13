/*	PROCEDURES 
 *		dohome
 *		dozoom
 *		zoomed
 *
 *	PURPOSE
 *		dohome
 *			homes the image.
 *		dozoom
 *			performs a hardware zoom of image, using (x,y) to 
 *			specify the upper left corner for view box.
 *		zoomed
 *			return: TRUE if frame buffers are in hardware zoom
 *				FALSE if frame buffers are not zoomed.
 *	SYNTAX
 *		dohome(homex)
 *		dozoom(x,y)
 *		zoomed()
 *
 *	AUTHOR
 *		Chuck Carmen / Stuart Ware
 *		for 
 *		Merickel Imaging Lab
 *		Biomedical Engineering
 *		Univerity of Virginia
 *		Charlottesville, Virginia 22903
 *
 */		
#include "image.sh"

dohome(homex)
unsigned short homex;
{
	swab(&homex, &FBSW->PAN, 2);
	FBCW->PAN = FBSW->PAN;

	FBSW->SCROLL = 0;
	FBCW->SCROLL = 0;

	FBS->FBCTRLO = 0;
	FBC->FBCTRLO = 0;
}

unsigned short dozoom (x, y)
short x, y; 
{
	unsigned short homex;

	swab(&FBSW->PAN, &homex, 2);
	homex %= COLCENTER;
	x += homex;
	swab (&x, &FBSW->PAN, 2);
	swab (&y, &FBSW->SCROLL, 2);
	FBCW->PAN = FBSW->PAN;
	FBCW->SCROLL = FBSW->SCROLL;
	FBS->FBCTRLO = 0x06;
	FBC->FBCTRLO = 0x06;
	return(homex);
}

zoomed()
{
	return(((FBS->FBCTRLO & 0x06) == 0x06) ? 1 : 0);
}
