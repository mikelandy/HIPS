/* 	PROGRAM
 *		grab (RGB)
 *
 *	PURPOSE
 *		to continuously grab and display frames
 *
 *	SYNOPSIS
 *		grab
 *		-  the frame buffer is put into continuous
 *		      grab mode using the phase locked loop
 *		-  the write protect is turned off for frame buffer
 *
 *	SYNTAX
 *		grab
 *
 *	AUTHOR 
 *		Charles Carman
 *		for
 *		Merickel Imaging Lab
 *		Biomedical Engineering
 *		University of Virginia
 *		Charlottesville, VA  22903
 *
 */
#include <image.h>

main()
{
	/* initiallize the itec boards */
	itecinit(FBINIT);

	/* turn off the write protect on the frame buffer */
	unmask(RED);

	/* turn on the phase locked loop */
	RGB->RGBCTRL = 0x04;

	/* start grabbing frames */
	FBC->FBCTRHI = 0xc0;
}

