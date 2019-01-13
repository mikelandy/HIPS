/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * conv_extrn.c - constants required by the conversion routines
 *
 * Michael Landy - 1/4/91
 */

#include <hipl_format.h>

#ifdef MSBFVERSION		/* set the prevailing packing mode */
int hips_packing = MSBF_PACKING;
#else
int hips_packing = LSBF_PACKING;
#endif
byte hips_lchar = 0;	/* value substituted for a 0 bit (MSBF/LSBF) */
byte hips_hchar = 255;	/* value substituted for a 1 bit (MSBF/LSBF) */
int hips_cplxtor = CPLX_MAG;	/* for complex/double complex to single
					valued formats, take complex
					magnitude by default */
int hips_rtocplx = CPLX_RVI0;	/* for single valued formats to complex
					valued formats, set the real part to
					the input and the imaginary part to
					zero by default */
char *Progname;
int hipserrno;
char hipserr[200];
int hipserrlev = HEL_ERROR;	/* default: print&die for all errors */
int hipserrprt = HEL_INFORM;	/* default: print&return for all others */
int Image_border = 2;		/* border pixels required by pyramid routines */
h_boolean hips_convback = FALSE;	/* if TRUE, convert back to the input fmt */
h_boolean hips_fullhist = TRUE;	/* if TRUE, preserve full seq history */
h_boolean hips_fulldesc = TRUE;	/* if TRUE, preserve full seq description */
h_boolean hips_fullxpar = FALSE;	/* if TRUE, preserve full extended pars */
int hips_lclip = 0;		/* number of underflows */
int hips_hclip = 0;		/* number of overflows */
int hips_zdiv = 0;		/* number of divisions by zero */
h_boolean hips_oldhdr = FALSE;	/* TRUE if just read a HIPS-1 header */
