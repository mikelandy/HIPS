/*	PROGRAM
 *		rdlut
 *
 *	PURPOSE
 *		to read each specified LUT into a file on the hard disk
 *
 *	AUTHOR
 *		Chuck Carmen
 *		for
 *		Merickel Imaging Lab
 *		Biomedical Engineering
 *		University of Virginia
 *		Charlottesville, VA   22903 
 *
 */
#include <stdio.h>
#include "image.sh"

rdlut_itec(blutn, elutn, color)
	int blutn, elutn;
	char color;
{
	register int i, j, k;
	int bcol, ecol;
	char buf[256], *bp;

	switch (color) {
	case RED:
		bcol = 0;
		break;
	case GREEN:
		bcol = 1;
		break;
	case BLUE:
		bcol = 2;
		break;
	case BW:
		bcol = 0;
		break;
	case COLOR:
		bcol = 0;
		break;
	}
	ecol = (color == COLOR) ? bcol + 3 : bcol + 1;

	itecinit(STD);

	RGB->RGBCTRL = 0;
	
	for (k=blutn; k<elutn; k++) {
		for (j=bcol; j<ecol; j++)  {
			RGB->LUTSEL = j * 16 + k;
			bp = buf;
			for (i=0; i<256; i++)  {
				RGB->LUTADDR = i;
				*(bp++) = RGB->LUTDATA;
			}
			fwrite(buf, 256,1,stdout);
		}
	}

	RGB->LUTSEL = 0x3f;
}

