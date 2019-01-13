/*
 *	PROCEDURES
 *		mask 
 *		unmask
 *
 *	SYNOPSIS
 *		mask   : set 
 *				the mask registers of the frame buffers
 *		unmask : clear 
 *
 *	SYNTAX 
 *		mask(color)
 *		unmask(color)
 *			char color	: frame buffer [rgbc] 
 *				  
 *
 *	Merickel Imaging Lab
 *	Biomedical Engineering
 *	University of Virginia
 *	Charlottesville, VA 22908
 */
#include <image.sh>

mask(color)
char color;
{
	switch (color) {
		case RED:
		case CHAR:
			FBC->MASKLO = 0xff;
			break;
		case BLUE:
			FBS->MASKLO = 0xff;
			break;
		case GREEN:
			FBS->MASKHI = 0xff;
			break;
		default:
			FBC->MASKLO = 0xff;
			FBSW->MASK = 0xffff;
			break;
	}
}

unmask(color)
char color;
{
	switch (color) {
		case RED:
		case CHAR:
			FBC->MASKLO = 0;
			break;
		case BLUE:
			FBS->MASKLO = 0;
			break;
		case GREEN:
			FBS->MASKHI = 0;
			break;
		default:
			FBC->MASKLO = 0;
			FBSW->MASK = 0;
			break;
	}
}
