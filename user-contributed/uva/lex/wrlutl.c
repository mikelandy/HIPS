/*	PROGRAM
 *		wrlut
 *
 *	PURPOSE
 *		to read each specified LUT from a file on the hard disk
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

wrlut_lex(fnum,color)
	int fnum;
	char color;
{
	int i;
	short err, chan, begin, numv = 256;
	short sbuf[256];
	unsigned char cbuf[256];
	register int j;
	register short *spb;
	register unsigned char *cpb;

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
		begin = 0;
		break;
	case COLOR:
		begin = 768;
		break;
	}
	
	dsopn_(&err, &chan);

	for (i=0; i<fnum; i++, begin-=numv) {
		fread(cbuf, 256,1,stdin);
		for (j=0,spb=sbuf,cpb=cbuf; j<256; j++,spb++,cpb++)
			*spb = *cpb;
		dslwt_(&begin,&numv,sbuf);
	}

	dscls_();
}

