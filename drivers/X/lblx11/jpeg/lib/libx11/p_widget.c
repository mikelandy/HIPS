/* Panel_WIDGET . C
#
%	Copyright (c)	Jin Guojun -	All rights reserved
%
%	API to CCS-XPanel library
%
% Author:	Jin Guojun - LBL	1/1/94
*/

#include "panel.h"
#include "va_vset.h"


PopMenu*
#ifdef	DOT_V_LIST
PW_CreatePopMenu(Panel* pan, WinAttribute*	parent, ...)
#else
PW_CreatePopMenu(pan, parent, va_alist)
Panel*	pan;
WinAttribute*	parent;
va_list	va_alist;
#endif
{
char	**menu, *name=NULL;
int	code, num=0;
bool	freemenu=0;
vset(parent);

    do	{
	code = va_arg(ap, int);
	switch (code)	{
	case PW_FreeMem:
		freemenu++;
		break;
	case PW_Lists:
		menu = va_arg(ap, char**);
		num = va_arg(ap, int);
		break;
	case PW_Label:
		name = va_arg(ap, char*);
		break;
	default:if (code)
		prgmerr(0, "wrong popmenu parameter %d", code);
	}
    }	while (code);
    if (!num || !name)
	return	0;
return	(PopMenu *)
	PW_AddWidget(CreatePopMenu(pan, parent, name, menu, num, freemenu), NULL);
}

Panel	*
#ifdef	DOT_V_LIST
PW_CreatePanel(WinAttribute*	parent, ...)
#else
PW_CreatePanel(parent, va_alist)
WinAttribute*	parent;
va_list	va_alist;
#endif
{
char	*name=NULL;
int	code, x=10, y=10, w=0, h=0;
vset(parent);

    do	{
	code = va_arg(ap, int);
	switch (code)	{
	case PW_Label:
		name = va_arg(ap, char*);
		break;
	case	PW_Origin:
		x = va_arg(ap, int);
		y = va_arg(ap, int);
		break;
	case	PW_Height:
		h = va_arg(ap, int);
		break;
	case	PW_Width:
		w = va_arg(ap, int);
		break;
	default:if (code)
		prgmerr(0, "wrong panel parameter %d", code);
	}
    }	while (code);
    if (!w || !h || !name)
	return	0;
return	(Panel *)
	PW_AddWidget(CreatePanel(x, y, w, h, name, parent, NULL), NULL);
}


Button	*
#ifdef	DOT_V_LIST
PW_CreateButton(Panel* pan, ...)
#else
PW_CreateButton(pan, va_alist)
Panel*	pan;
va_list	va_alist;
#endif
{
char	**bname, *name=NULL;
int	code, x=0, y=0, numb=0, bc, pc;
vset(pan);

    do	{
	code = va_arg(ap, int);
	switch (code)	{
	case PW_Origin:
		x = va_arg(ap, int);
		y = va_arg(ap, int);
		break;
	case PW_BgColor:
		bc = va_arg(ap, int);
		break;
	case PW_FgColor:
		pc = va_arg(ap, int);
		break;
	case PW_Label:
		name = va_arg(ap, char*);
		break;
	case PW_Lists:
		bname = va_arg(ap, char**);
		numb = va_arg(ap, int);
		break;
	default:if (code)
		prgmerr(0, "wrong parameter %d", code);
	}
    }	while (code);
    if (!x || !y || !numb || !name)
	return	0;
return	(Button*)
	PW_AddWidget(CreateButton(pan, x, y, numb, name, bname, bc, pc), NULL);
}


PressButton	*
#ifdef	DOT_V_LIST
PW_CreatePressButton(Panel* pan, ...)
#else
PW_CreatePressButton(pan, va_alist)
Panel*	pan;
va_list	va_alist;
#endif
{
char	*name=NULL;
int	code, x=0, y=0, pc;
vset(pan);

    do	{
	code = va_arg(ap, int);
	switch (code)	{
	case	PW_Origin:
		x = va_arg(ap, int);
		y = va_arg(ap, int);
		break;
	case PW_FgColor:
		pc = va_arg(ap, int);
		break;
	case PW_Label:
		name = va_arg(ap, char*);
		break;
	default:if (code)
		prgmerr(0, "wrong parameter %d", code);
	}
    }	while (code);
    if (!x || !y || !name)
	return	0;
return	(PressButton*)
	PW_AddWidget(CreatePressButton(pan, x, y, name, pc), NULL);
}
