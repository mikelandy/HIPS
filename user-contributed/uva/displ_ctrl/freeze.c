/*	PROGRAM
 *		freeze (RGB)
 *
 *	PURPOSE
 *		to grab one frame and stop/freeze
 *
 *	SYNTAX
 *		freeze
 *
 *	AUTHOR
 *		Charles Carmen
 *		for
 *		Merickel Imaging Lab
 *		Biomedical Engineering
 *		University of Virginia
 *		Charlottesville, VA   22903
 *
 */
#include <image.h>

main()
{
	char temp;

	itecinit(STD);

	/* enable the phase lock loop if not already */
	temp = RGB->RGBCTRL;
	if ((temp & 0x04) == 0)  RGB->RGBCTRL = temp | 0x04;

	/* stop the current grab */
	FBC->FBCTRHI = 0x80;

	/* wait for the frame grabbing to be completed */
	while ((FBC->FBCTRHI & 0xc0) != 0)
		;

		/* put the system back on the xtal */
	RGB->RGBCTRL =  0;
	
		/* set the memory protect mask register */
	mask(RED);
}

