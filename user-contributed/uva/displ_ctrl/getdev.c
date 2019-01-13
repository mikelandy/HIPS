/*
 *	SUB-PROCEDURE
 *		getdev()
 *
 *	PURPOSE
 *		to get the device from the environment and return the
 *		first letter (in CAPS) to the calling program.
 *
 * Charles Carman (BME dept. UVA) - 3/18/86
 */
#include <stdio.h>
#include <ctype.h>
#define IMGDEV_H
#include "device.h"

getdev()
{
	char *getenv();
	char *env_val;

	if ((env_val = getenv(ENV_DISP_NAME)) == NULL) {
		fprintf(stderr,
	"Display device not specified!  Use: setenv DSPDEV <ITEC | LEX>\n");
		return(0);
	}
	return(toupper(env_val[0]));
}

