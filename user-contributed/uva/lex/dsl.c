/*	PROGRAM
 *		ds_lex 
 *
 *	PURPOSE
 *		to load the LUT of the lexidata with a file defined
 *			in the directory /usr/spool/images/lut
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
#include <stdio.h>

ds_lex(lutc)
	char lutc;
{
	char lutfile[20];

	switch (lutc) {
	case 'i':
	case 'n':
	case 'c':
	case 'r':
	case 'g':
	case 'b':
		break;
	default:
		fprintf(stderr,"Unrecognized argument %c\n",lutc);
		exit(1);
	}

	sprintf(lutfile,"/usr/spool/images/lut/lut%c",lutc);

	freopen(lutfile,"r",stdin);
	execlp("wrlut","wrlut",0);
}

