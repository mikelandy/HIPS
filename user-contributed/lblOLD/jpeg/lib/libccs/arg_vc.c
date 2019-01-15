/*	ARG_VC . C
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
%		arguments - argc & argv - handler
%	return 0, to both routines, means failure.
%
%	must include <stdlib.h>
%
% AUTHOR:	Jin Guojun - LBL	04/01/91
*/

#include <math.h>
#include <stdlib.h>
#include "imagedef.h"

isdecimal(s, f)
register	f;
{
	switch (s) {
	case '+':
	case '-':	f = isfloat(f);	break;
	case '.':	f = isdigit(f);	break;
	default:	f = isfloat(s);
	}
return	f;
}

avset(ac, av, i, c, s)
register char**	av;
register int	*i, *c, s;
{
register int	cur=av[*i][*c], curnotnum= !s && !isdecimal(cur, av[*i][*c+1]),
		nextisnum;
    if (*i+1 < ac) {
	nextisnum = isdecimal(*av[*i+1], av[*i+1][1]);
	if (!cur && (nextisnum | s) || curnotnum && nextisnum)
		++*i,  *c ^= *c;
	else
s_only:		if (curnotnum)	return	0;
    }
    else	goto	s_only;
return	av[*i][*c];
}

double
arget(ac, av, i, c)
register int	ac, *i, *c;
register char**	av;
{
if (avset(ac, av, i, c, 0))	return	atof(av[*i] + *c);
return	0;
}
