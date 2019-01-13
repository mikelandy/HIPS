/*
 *	PROGRAM
 *		initlex
 *
 *	PURPOSE
 *		to initialize the Lexidata
 */
#include <lexioctl.h>

main()
{
	short err, chan, varb = -1, start = 0, end = 255;

	/* open the I/O channel to the lexidata */
	dsopn_(&err, &chan);

	/* initialize the lexidata */
	dspld_(&varb);
	dschan_(&varb,&varb,&varb);

	/* initialize the luts for the standard bw mapping */
	dsllu_(&start,&start,&end,&end);

	/* close the I/O channel */
	dscls_();
}

