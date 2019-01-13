#include <stdio.h>

short int wrarray[16] = { 2, 9 };

main(argc, argv)
	int argc;
	char *argv[];
	{
	short int zero = 0;
	short int two = 2;
	short int eleven = 11;
	short int err, chan;
	short int retarray[1];

	if (getdev() != 'L')
		{
		fprintf(stderr,"Your display device is not the Lexidata!\n");
		exit(0);
		}

	if (argc > 2)
		{
		fprintf(stderr,"Syntax: lexconfig [config number]\n");
		exit(0);
		}
		
	if (argc == 2) wrarray[1] = (short int)(atoi(argv[1]));

	switch (wrarray[1]) {
	case 0:
	case 1:
	case 3:
	case 4:
	case 9:
		break;
	default:
		fprintf(stderr,"Available configurations are: 0,1,3,4,9\n");
		exit(1);
	}

	dsopn_(&err, &chan);
	dsesc_(wrarray, &two, retarray, &zero);
	sleep(2);

	if (wrarray[1] == 4) {
		wrarray[0] = 32;
		wrarray[1] = 1;
		wrarray[2] = 8;
		wrarray[3] = 1;
		wrarray[4] = 2;
		wrarray[5] = 3;
		wrarray[6] = 4;
		wrarray[7] = 5;
		wrarray[8] = 6;
		wrarray[9] = 7;
		wrarray[10] = 1;
		dsesc_(wrarray,&eleven,retarray,&zero);
		sleep(2);
	}
	dscls_();
}
