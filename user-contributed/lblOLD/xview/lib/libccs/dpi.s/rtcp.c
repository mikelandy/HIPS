/*	RTCP . C
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
	Bld. 50B, Rm. 2275, Lawrence Berkeley Laboratory, Berkeley, CA, 94720
	g_jin@lbl.gov

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% Author:	Jin Guojun - LBL	3/1/93
*/

#define	USE_RTP
#include "net_need.h"

#ifdef	USE_STDARG
#include <stdarg.h>
#else
#include <varargs.h>
#endif

typedef	struct  {
	int	type;
	VType   **list;
	} pointer_pair;


#ifdef	USE_STDARG
rtcp_getoptions(VType*	buf, int num_options, ...)
#else
rtcp_getoptions(VType*	buf, int num_options, va_list	va_alist)
#endif
{
rtp_hdr_t *rtp=(rtp_hdr_t *)buf;
rtpopthdr_t *opt=(rtpopthdr_t *)(rtp+1);
pointer_pair*	rtp_opts=NULL;
va_list	ap;

    if (rtp->rh_op)	{
	register int	i;
	if (num_options)	{
		rtp_opts = ZALLOC(num_options, sizeof(pointer_pair), "rtp_opt");
#ifdef	USE_STDARG
		va_start(ap, num_options);
#else
		va_start(ap);
#endif
		for (i=0; i<num_options; i++)	{
			rtp_opts[i].type = va_arg(ap, int);
			rtp_opts[i].list = va_arg(ap, VType*);
		}
		va_end(ap);
	}
	do	{
	    for (i=num_options; i--;)	/* if request option, set it */
		if (opt->roh_type == rtp_opts[i].type)	{
			*rtp_opts[i].list = (VType*)opt;
			break;
		}
		/*	(opt->roh_optlen << 2) - sizeof(rtp_hdr_t)	*/
	    i = opt->roh_fin;	/* skip to next option.	*/
	    opt = (rtpopthdr_t *)((long*)opt + opt->roh_optlen);
	} while (!i);
	if (rtp_opts)	free(rtp_opts);
    }

return	(int) opt - (int) buf;	/* header length in bytes, not words */
}


rtcp_setoption(rtpopthdr_t* opt, char *o_mesg, int mesg_len, int opt_type,
		bool	fin)
{

	opt->roh_fin = fin;
	opt->roh_type = opt_type;
	opt->roh_optlen = (mesg_len >> 2) + 2;	/* length in long words	*/
	memcpy(opt + 1, o_mesg, mesg_len);
}
