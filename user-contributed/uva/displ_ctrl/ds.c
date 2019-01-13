/*	PROGRAM
 *		ds (RGB)
 *
 *	PURPOSE
 *		to display the frame buffer with specific LUTs
 *
 *	SYNOPSIS
 *		ds [i|n|m|c|r|g|b|0-9]
 *	-  input arguments:
 *	   'i' is the identity transform 
 *         'n' is the negative/inverse trans
 *	   'm' is the
 *	   'c' displays a resister color code spectrum.
 *	   'r','g','b' display red, green, or blue respectively
 *         '0' - '9' are free for user-defined transforms
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
#include <stdio.h>
main(argc, argv)
	int argc;
	char *argv[];
{

	int lexflg;
	char devc, lutc;

	if ((devc = getdev()) == 0)
		exit(1);

	lexflg = (devc == 'L') ? 1 : 0;
	lutc = (argc == 1) ? 'i' : *argv[1];

	if (lexflg) {
		if (lutc >= '0' && lutc <= '9') {
			fprintf(stderr,"Use: lutmake file | wrlut\n");
			exit(2);
		}
		ds_lex(lutc);
	} else
		ds_itec(lutc);
}

