/*	rotate90 . c
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

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
% AUTHOR:	Jin, Guojun - LBL
*/

#include "imagedef.h"

rotate90(ibuf, obuf, newr, newc, pix_size, rflag)
VType	*ibuf, *obuf;
{
int	isize = newr * newc;
register int	i, j;
	if (pix_size == 1) {
	register byte	*ibp=(byte*)ibuf, *obp;
	    if (!rflag)
		for (i=0; i<newc;) {
			obp = (byte*)obuf + isize-newc + i++;
			for (j=0; j<newr; j++, obp-=newc)
				*obp = *ibp++;
		}
	    else for (i=newc; i--;) {
			obp = (byte*)obuf + i;
			for (j=0; j<newr; j++, obp+=newc)
				*obp = *ibp++;
		}
	}
	else if (pix_size == 2) {
	register short	*sibp=(short*)ibuf, *sobp;
	    if (!rflag)
		for (i=0; i<newc;) {
			sobp = (short*)obuf + isize-newc + i++;
			for (j=0; j<newr; j++, sobp-=newc)
				*sobp = *sibp++;
		}
	    else for (i=newc; i--;) {
			sobp = (short *)obuf + i;
			for (j=0; j<newr; j++, sobp+=newc)
				*sobp = *sibp++;
		}
	}
	else if (pix_size == 4) {
	register long	*ibp=(long*)ibuf, *obp;
	    if (!rflag)
		for (i=0; i<newc;) {
			obp = (long*)obuf + isize-newc + i++;
			for (j=0; j<newr; j++, obp-=newc)
				*obp = *ibp++;
		}
	    else for (i=newc; i--;) {
			obp = (long*)obuf + i;
			for (j=0; j<newr; j++, obp+=newc)
				*obp = *ibp++;
		}
	}
	else	return	EOF;
return	0;
}
