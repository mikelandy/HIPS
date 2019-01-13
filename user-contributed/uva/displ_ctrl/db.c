/*
 *	test lexidata overlay text
 */
#include <stdio.h>
#include <lexioctl.h>

main(argc, argv)
	int argc;
	char **argv;
{
	int colmax, rowmax;
	short err, chan, buff = 0;
	short shrtstr[16];

	if (getdev() != 'L') {
		fprintf(stderr,"db: Device not the Lexidata\n");
		exit(1);
	}
	getsiz(&colmax, &rowmax);

	if (argc == 2) buff = atoi(argv[1]);
	if (buff != 0) {
		if (colmax > 1024) buff = 0x0010;
		else buff = 0x0011;
	}

	dsopn_(&err, &chan);
	dsbuff_(&buff);
	dscls_();
}
