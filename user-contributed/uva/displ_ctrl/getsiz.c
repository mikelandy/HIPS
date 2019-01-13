/*
 *	SUB-PROCEDURE
 *		getsiz()
 *
 *	PURPOSE
 *		to get the current size of the lexidata display
 *
 * Charles Carman (BME dept. UVA) - 12/10/87
 */
#include <stdio.h>
#define IMGDEV_H
#include "device.h"

getsiz(x, y)
	int *x, *y;
{
	short int err, chan;
	short int curs_no = 2, sx = 0, sy = 0;

	dsopn_(&err, &chan);
	dsgcp_(&curs_no, &sx, &sy);
	sleep((unsigned)1);
	dscls_();

	*x = sx * 2;
	*y = sy * 2;

	if (*x > 1280) {
		fprintf(stderr,
"getsiz: error reading from the Lexidata\n\nPlease init the Lexidata\n");
		exit(1);
	}
}
