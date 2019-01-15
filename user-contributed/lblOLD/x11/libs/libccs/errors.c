/*	ERRORS . C
#
%	Copyright (c)	Jin Guojun -	All rights reserved
%
%	Error handler for all ARCHs
%
% AUTHOR:	Jin Guojun - LBL	10/1/90
*/

#include <errno.h>
#include "header.def"	/*	to find HIPS2_HF for Progname	*/

#if	defined sparc || !defined	__STDC__
extern	int	sys_nerr;
extern	char	*sys_errlist[];
#endif

char	*Mversion, *io_test_msg[]={"stdin", "stdout", "stderr", "aux", "?"};

#ifndef	HIPS2_HF
char	*Progname;
int	hipserrlev, hipserrprt;
#endif

#define	Real_STR(str, subs)	str ? str : subs
#define	MainMsg	Real_STR(Progname, "?"), Real_STR(Mversion, "NoVer")
#define	vermsg	message("%s [%s] : ",	MainMsg)

#define	Error_Body(virtual)	vermsg;	\
	vfprintf(stderr, (char*) vfmt, virtual);	error_mesg();
#define	HowFatal()	if (fatal > 0)	exit(fatal);	return	~fatal


void	error_mesg()
{
if (errno > 0)	{
	msg(";	Error<%d>", errno);
	message(" %s", errno < sys_nerr ? sys_errlist[errno] : "?");
}
mesg("\n");	fflush(stderr);
}


#ifndef	TC_Need

#ifndef	NO_V_LIST

#include "va_vset.h"

#ifdef	SHOW_WARNINGS
VType	/*	system error for va_list handling compilers	*/
#endif

#ifdef	DOT_V_LIST
syserr(char* vfmt, ...)
#else
syserr(vfmt, va_alist)
va_list	va_alist;
#endif
{
vset(vfmt);
	Error_Body(ap);
exit(errno);
}

#ifdef	DOT_V_LIST
prgmerr(bool fatal, char* vfmt, ...)
#else
prgmerr(fatal, vfmt, va_alist)
va_list	va_alist;
#endif
{
vset(vfmt);
	Error_Body(ap);
HowFatal();
}

#else	/* for stupid compilers that can't handle virtual lists	*/

#define	argus	a1,a2,a3,a4,a5,a6

syserr(s, argus)
char *s;
{
vermsg;	message(s, argus);
error_mesg();
exit(errno);
}

prgmerr(fatal, fmt, argus)
char	*fmt;
{
vermsg;	message(fmt, argus);
	error_mesg();
HowFatal();
}
#endif	/*NO_V_LIST*/

#else	FOR TURBO_C
#include <stdarg.h>

syserr(char* vfmt, ...)
{
	Error_Body(...);
exit(errno);
}

prgmerr(bool fatal, char* vfmt, ...)
{
	message("%s [%s] : %s : ", MainMsg, strerror(errno));
	vfprintf(stderr, vfmt, ...);
HowFatal();
}

#endif	/*TC_Need*/
