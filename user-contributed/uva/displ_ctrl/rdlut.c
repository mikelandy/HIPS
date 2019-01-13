/*	PROGRAM
 *		rdlut
 *
 *	PURPOSE
 *		to read all of the LUTs into a file on the hard disk
 *
 *	SYNOPSIS
 *		rdlut [start lut# [end lut#]] [-rgbC] > filename 
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
	int b_flg = 0, e_flg = 0, blutn = 0, elutn = 0;
	int i, lex_flg;
	int frames, rows, cols;
	char dev_nm[16], color = COLOR;

	Progname = strsave(*argv);
	for (i=1; i<argc; i++) {
		if (argv[i][0] == '-') color = argv[i][1];
		else if (!b_flg) {
			blutn = atoi(argv[i]);
			b_flg++;
		} else {
			elutn = atoi(argv[i]);
			e_flg++;
		}
	}
	if (b_flg && !e_flg) elutn = blutn;
	if (elutn < 16) elutn++;

	if (argc > 4 || color == 'h')  {
		printf("Usage: rdlut [begin lut# [end lut#]] [-rgbC] > file\n");
		exit(1);
	}

	frames = elutn - blutn;
	cols = 256;
	rows = (color == COLOR) ? 3 : 1;

	switch (getdev()) {
	case 'I':
		lex_flg = 0;
		strcpy(dev_nm,"Itec");
		break;
	case 'L':
		lex_flg = 1;
		strcpy(dev_nm,"Lexidata");
		break;
	}

	init_header(&hd,dev_nm,"",frames,"",rows,cols,PFLUT,1,"");
	update_header(&hd,argc,argv);
	write_header(&hd);

	if (lex_flg)
		rdlut_lex(color);
	else
		rdlut_itec(blutn,elutn,color);
}

