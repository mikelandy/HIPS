/*	DUMMY_API . C
#
%	Copyright (c)	Jin Guojun -	All rights reserved
%
%	Two ways to link to here if user's APIs are not defined.
%
% Author:	Jin Guojun
*/

#include "imagedef.h"


static	void	(*ccs_window_open_p)();

void	u_window_api(U_IMAGE	*img)
{	if (ccs_window_open_p)	(*ccs_window_open_p)(img);	}

ccs_window_open(void (*win_create)())
{
	ccs_window_open_p = win_create;
}

