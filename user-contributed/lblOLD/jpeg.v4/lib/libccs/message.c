/*	message.c
#
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

This software is copyright (C) by the Lawrence Berkeley Laboratory.
Permission is granted to reproduce this software for non-commercial
purposes provided that this notice is left intact.

It is acknowledged that the U.S. Government has rights to this software
under Contract DE-AC03-765F00098 between the U.S.  Department of Energy
and the University of California.

This software is provided as a professional and academic contribution
for joint exchange. Thus, it is experimental, and is provided ``as is'',
with no warranties of any kind whatsoever, no support, no promise of
updates, or printed documentation. By using this software, you
acknowledge that the Lawrence Berkeley Laboratory and Regents of the
University of California shall have no liability with respect to the
infringement of other copyrights by any part of this software.

For further information about this notice, contact William Johnston,
Bld. 50B, Rm. 2239, Lawrence Berkeley Laboratory, Berkeley, CA, 94720.
(wejohnston@lbl.gov)

For further information about this software, contact:
	Jin Guojun
	Bld. 50B, Rm. 2275, Lawrence Berkeley Laboratory, Berkeley, CA, 94720.
	g_jin@lbl.gov

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% AUTHOR:	Jin Guojun - LBL	10/1/90
*/

#include "stdef.h"

/* only init_header() uses this
char	*desc_massage(char*	s)
{
int	i, beg=1, len=strlen(s);

for (i=0; i < len; i++)	{
    if (beg && s[i] == '.' && *(s+i+1) == '\n')	{
	len -= 2;	i--;
	memcpy(s+i, s+i+2, len-i);
    }	else	beg = (s[i]=='\n');
}
return	s;
}	*/

#define	vset	va_list	ap;	va_start(ap)

#ifdef	TC_Need
#	include <stdarg.h>

message(const char *vform, ...)
{
#ifdef	va_list
vset;
vfprintf(stderr, vform, ap);
#else
vfprintf(stderr, vform, ...);
#endif
return	fflush(stderr);
}

#else

#ifndef	NO_V_LIST

#	include <varargs.h>

message(va_alist)
va_list	va_alist;
{
char*	vf;
vset;
vf = va_arg(ap, char*);
vfprintf(stderr, vf, ap);
return	fflush(stderr);
}

#else

message(form, a0, a1, a2, a3, a4, a5, a6, ap)
{
return	fprintf(stderr, (char*)form, a0, a1, a2, a3, a4, a5, a6, ap);
}

#endif

#endif	TC_Need
