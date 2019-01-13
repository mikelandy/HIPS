/*	PROGRAM
 *		wrlut
 *
 *	PURPOSE
 *		to write all of the LUTs from a file on the hard disk
 *
 *	SYNOPSIS
 *		wrlut [start lut#] [-rgbC] < filename 
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
#include <hipl_format.h>
#include <image.h>

main(argc,argv)
	int argc;
	char *argv[];
{
	struct header hd;
	int b_flg = 0, blutn = 0, elutn = 0;
	int i, lex_flg, rows;
	char color = COLOR;

	Progname = strsave(*argv);
	for (i=1; i<argc; i++) {
		if (argv[i][0] == '-') color = argv[i][1];
		else blutn = atoi(argv[i]);
	}

	if (argc > 3 || color == 'h')  {
		printf("Usage: wrlut [begin lut#] [-rgbwC] < file\n");
		exit(1);
	}

	switch (getdev()) {
	case 'I':
		lex_flg = 0;
		break;
	case 'L':
		lex_flg = 1;
		break;
	}

	read_header(&hd);
	if (hd.pixel_format != PFLUT) {
		fprintf(stderr,"Input not an LUT data file\n");
		exit(1);
	}
	rows = hd.orows;
	if (rows == 3 && color != COLOR) {
		fprintf(stderr,"The input is an RGB lut data file\n");
		exit(2);
	}
	if (rows == 1 && color == COLOR) {
		fprintf(stderr,"The input is for only a single color lut\n");
		fprintf(stderr,"Specify the color or BW\n");
		exit(3);
	}
	elutn = blutn + hd.num_frame;

	if (lex_flg)
		wrlut_lex(rows,color);
	else
		wrlut_itec(blutn,elutn,color);
}

