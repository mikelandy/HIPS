/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * defines to handle hardware problems with the statements:
 *
 * char *buf;
 * float f;
 *
 * ...
 *
 * f = (float *) buf;
 *
 * On some machines this causes a bus error if the buf pointer is not fullword aligned.
 */

#ifdef FASTCOPY

#define getfloat(f,bp)  f = *(bp)
#define storefloat(bp,f)  *(bp) = (f)
#define copyfloat(bp1,bp2)  *(bp1) = *(bp2)

#else

#define getfloat(f,fp)  {		\
	float tmp;			\
	register char *ptmp = (char *) &tmp;\
	register char *pbuf = (char *) (fp);\
					\
	*ptmp++ = *pbuf++;		\
	*ptmp++ = *pbuf++;		\
	*ptmp++ = *pbuf++;		\
	*ptmp = *pbuf;			\
	f = tmp; }

#define storefloat(fp,f)  {		\
	float tmp = (f);		\
	register char *ptmp = (char *) &tmp;\
	register char *pbuf = (char *) (fp);\
					\
	*pbuf++ = *ptmp++;		\
	*pbuf++ = *ptmp++;		\
	*pbuf++ = *ptmp++;		\
	*pbuf = *ptmp; }

#define copyfloat(dp,fp)  {		\
	float tmp;			\
	register char *ptmp = (char *) &tmp;\
	register char *pbuf = (char *) (fp);\
					\
	*ptmp++ = *pbuf++;		\
	*ptmp++ = *pbuf++;		\
	*ptmp++ = *pbuf++;		\
	*ptmp = *pbuf;			\
	*dp = tmp; }

#endif
