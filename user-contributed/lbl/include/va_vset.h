/*	VA_VSET . H
#
%	Copyright (c)	Jin Guojun
%
%	Virtual Variable List - C should have a standard?
%
% AUTHOR:	Jin Guojun - LBL	10/1/90
*/

#if !defined	NO_V_LIST & !defined	TC_Need

#ifndef	VA_VSET_VVL
#define	VA_VSET_VVL

#define	vvset	va_list	ap;	va_start

#ifdef	USE_STDARG

#include <stdarg.h>
#if defined	SOLARIS & !defined	__STDC__
#define	vset(vfmt)	vvset(ap)	/* silly faked stdarg.h == varargs.h */
#else
#define	vset(vfmt)	vvset(ap, vfmt)
#endif

#else

#include <varargs.h>
#define	vset(vfmt)	vvset(ap)	/* does any compiler don't like it ? */

#endif

#endif
#endif
