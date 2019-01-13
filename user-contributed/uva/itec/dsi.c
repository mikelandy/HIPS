/*	PROGRAM
 *		ds (RGB)
 *
 *	PURPOSE
 *		to display the frame buffer with specific LUTs
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
#include "image.h"

ds_itec(lutc)
	char lutc;
{

	itecinit(STD);

	RGB->LUTADDR = 0;

	switch (lutc)  {
		case 'i':
			/* set output LUTs to identity */
			RGB->LUTSEL = 0x3f;
			break;
		case 'm':
			/* user defined spectrum 12 */
		        RGB->LUTSEL = 0x70;
			break;
		case 'n':
			/* set output LUTs to negative */
			RGB->LUTSEL = 0x3e;
			break;
		case 'c':
			/* set output LUTs to spectrum */
			RGB->LUTSEL = 0x3d;
			break;
		case 'r':
			/* set output LUTs to red */
			RGB->LUTSEL = 0x3c;
			break;
		case 'g':
			/* set output LUTs to green */
			RGB->LUTSEL = 0x3b;
			break;
		case 'b':
			/* set output LUTs to blue */
			RGB->LUTSEL = 0x3a;
			break;
		case '0':
			/* user defined spectrum 0 */
			RGB->LUTSEL = 0x30;
			break;
		case '1':
			/* user defined spectrum 1 */
			RGB->LUTSEL = 0x31;
			break;
		case '2':
			/* user defined spectrum 2 */
			RGB->LUTSEL = 0x32;
			break;
		case '3':
			/* user defined spectrum 3 */
			RGB->LUTSEL = 0x33;
			break;
		case '4':
			/* user defined spectrum 4 */
			RGB->LUTSEL = 0x34;
			break;
		case '5':
			/* user defined spectrum 5 */
			RGB->LUTSEL = 0x35;
			break;
		case '6':
			/* user defined spectrum 6 */
			RGB->LUTSEL = 0x36;
			break;
		case '7':
			/* user defined spectrum 7 */
			RGB->LUTSEL = 0x37;
			break;
		case '8':
			/* user defined spectrum 8 */
			RGB->LUTSEL = 0x38;
			break;
		case '9':
			/* user defined spectrum 9 */
			RGB->LUTSEL = 0x39;
			break;
		default:
			printf("unknown option: %c\n", lutc);
			printf("usage: display [i|n|c|r|g|b|0..9]\n");
			break;
	}
}
