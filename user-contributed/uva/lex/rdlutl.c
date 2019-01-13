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

rdlut_lex(color)
	char color;
{
	int rnum, i;
	short err, chan, begin, numv = 256;
	short sbuf[256];
	char cbuf[256];
	register int j;
	register short *spb;
	register char *cpb;

	switch (color) {
	case RED:
		begin = 256;
		break;
	case GREEN:
		begin = 512;
		break;
	case BLUE:
		begin = 768;
		break;
	case BW:
		begin = 256;
		break;
	case COLOR:
		begin = 768;
		break;
	}
	rnum = (color == COLOR) ? 3 : 1;
	
	dsopn_(&err, &chan);

	for (i=0; i<rnum; i++, begin-=numv) {
		dslrd_(&begin,&numv,sbuf);
		for (j=0,spb=sbuf,cpb=cbuf; j<256; j++,spb++,cpb++)
			*cpb = *spb;
		fwrite(cbuf, 256,1,stdout);
	}

	dscls_();
}

