/* PWidget_CTRL . C
#
%	Panel Widget CONTROLs
%
%	Copyright (c)	Jin Guojun -	All rights reserved
%
% Author:	Jin Guojun - LBL	1/1/94
*/

#include "panel.h"


static pw_manager_t	pw_cell;

#define	pw_stack	pw_cell.PW_stack
#define	n_pws	pw_cell.n_PW_PWs	/* number of Panel Widgets in stack */
#define	n_pw_ss	pw_cell.n_PW_PSs	/* number of Pointer Stack allocated */
#define	leaf_stack	leaf->PW_stack
#define	leaf_n_pws	leaf->n_PW_PWs
#define	leaf_n_pw_ss	leaf->n_PW_PSs
#define	my_stack	my_mp->PW_stack
#define	my_n_pws	my_mp->n_PW_PWs
#define	my_n_pw_ss	my_mp->n_PW_PSs

/*	deep recursive control	*/
#ifdef	PW_DEEPLEVEL_RECURSIVE	/* what to do with AddCallback() ?	*/
#define	which_n_pws	me ? me->n_PW_PWs : n_pws
#define	which_n_pw_ss	me ? me->n_PW_PSs : n_pw_ss
#define	which_stack	me ? me->PW_stack : pw_stack
#else
#define	which_n_pws	n_pws
#define	which_n_pw_ss	n_pw_ss
#define	which_stack	pw_stack
#endif



PW_AddCallback (cookie_t pw_id, cookie_t e_type, StdInterface *callback,
		cookie_t user_param, int sys_param)
{
int	i = n_pws;
register struct PW_Stack **	pw_sp = pw_stack;
	if (IS_PanelWidget(pw_id))	{	/* has a parent	*/
	cookie_t	pid = pw_parent(pw_id);
	    while (i--)
		if (pid == pw_sp[i]->pw_ID)	{	/* two levels	*/
		register int	n = pw_sp[i]->leaf_n_pws;
			pw_sp = pw_sp[i]->leaf_stack;
			i = n;
			goto	nextlvl;
		}
	    i = n_pws;	/* parent only	*/
	}
nextlvl:
	while (i--)	if (pw_id == pw_sp[i]->pw_ID)	{
		pw_sp[i]->callback = callback;
		pw_sp[i]->user_param = user_param;
		pw_sp[i]->pw_e.etype = e_type;
		pw_sp[i]->pw_e.argu0 = sys_param;
		break;
	}
return	i;	/* EOF (-1)	means failure	*/
}

cookie_t
PW_AddWidget(cookie_t	pw_id, pw_manager_t*	me)
{
int	i;
register pw_manager_t*	my_mp = me ? me : &pw_cell;
    if (pw_id)	{	/* security check	*/
	if (
#ifdef	TWOLEVEL_WIDGET	/* be more deeply without checking !me	*/
	    !me &&
#endif
		IS_PanelWidget(pw_id))	{
	register struct PW_Stack**	pw_sp = my_stack;
	cookie_t	prnt = pw_parent(pw_id);
	    for (i = my_n_pws; i--;)
		if (pw_sp[i]->pw_ID == prnt)	{
			if (!pw_stack[i]->leaf)
				pw_stack[i]->leaf = ZALLOC((MType)1,
						sizeof(*my_mp), "leaf");
			return	PW_AddWidget(pw_id, pw_sp[i]->leaf);
		}
	}
	if (my_n_pws >= my_n_pw_ss)
		if (!verify_buffer_size(&my_stack, -SIZEOF(*my_stack),
				++my_n_pw_ss))
			return	(cookie_t) False;
	my_stack[my_n_pws] = ZALLOC((MType)1, sizeof(*my_stack[0]), "my_stack");
	return	my_stack[my_n_pws++]->pw_ID = pw_id;
    }
return	0;
}

PW_RemoveWidget(cookie_t	pw_id, pw_manager_t*	me, int	i)
{
register pw_manager_t*	my_mp = me ? me : &pw_cell;
struct PW_Stack	**pw_sp = my_stack;
int	n, j = (i < 1 || !me) ? my_n_pws : i;
	while (j--)	{
	    if (pw_sp[j]->pw_ID == pw_id)	{	/* found it in root */
		if (my_mp=pw_sp[j]->leaf)	{
			for (n = my_n_pws; n--;)
#	ifdef	TWOLEVEL_WIDGET
				CFREE(my_stack[n]->pw_ID),
				CFREE(my_stack[n]);
#	else	/* remove sub-tree completely	*/
				PW_RemoveWidget(my_stack[n]->pw_ID,
						my_mp, n + 1);
#	endif
			CFREE(my_stack);	/* free stack holder	*/
		/* my_mp === pw_sp[j]->leaf; CFREEnNULL(pw_sp[j]->leaf) is
				equivalent to following lines, but slow	*/
			CFREE(my_mp);		/* free node	*/
			pw_sp[j]->leaf = NULL;	/* make my_mp clear	*/
		}
		my_mp = me ? me : &pw_cell;	/* reset node	*/
		CFREE(pw_sp[j]);	/* free sibling node	*/
		if (j != --my_n_pws)	/* shift others down	*/
			memcpy(pw_sp + j, pw_sp + j + 1,
				sizeof(pw_sp[0]) * (my_n_pws - j));
		break;

	    }
	/* is Faster on removing leaf here, but slow whole.
	Since Remove parent is frequently used, we could put
	removing leaf routine after while loop with another for() loop	*/
	    if (pw_sp[j]->leaf && IS_PanelWidget(pw_id) && /* tree search */
		(n=PW_RemoveWidget(pw_id, pw_sp[j]->leaf, 0)) >= 0)	{
		return	n;	/* found one node; don't free root	*/
	    }
	}
CFREE(pw_id);	/* free root	*/
return	j;	/* -1 == nothing is removed	*/
}

PW_ShowNode(AnyParts* app, pw_manager_t	*me, int	sgl)
{
pw_manager_t*	my_mp = me ? me : &pw_cell;
register struct PW_Stack**	pw_sp = my_stack;
int	i = (sgl != EOF && me) ? sgl	/* me is loaded	*/ : my_n_pws - 1;
    do	{
	AnyParts* app = (AnyParts*)pw_sp[i]->pw_ID;
	switch (app->pw_type)	{
	case PW_BUTTON:
		DrawButton(app);	break;
	case PW_PRESSBUTTON:
		DrawPressButton(app);	break;
	case PW_SLIDER:
		DrawSlider(app);
	}
	if (my_stack[i]->leaf)	/* draw all childen	*/
		PW_ShowNode(app, my_stack[i]->leaf, EOF);
    } while (i-- && sgl == EOF);
}

PW_ClickHandler(Image*	img, XEvent *xep, pw_manager_t*	my_mp)
{
    if (my_mp)	{	/* must be checked and point to current leaf	*/
	int	i = my_n_pws;
	struct PW_Stack	**pw_sp = my_stack;
	while (i--)
	    if (pw_sp[i]->callback)
		switch (((AnyParts*)pw_sp[i]->pw_ID)->pw_type)	{
		case PW_BUTTON:
		    if (OnButton(pw_sp[i]->pw_ID, xep) > 0)
			goto	all_hd;
			break;
		case PW_PRESSBUTTON:
		    if (ButtonPressed(pw_sp[i]->pw_ID, xep))	{
all_hd:			pw_sp[i]->pw_e.CooKie = (cookie_t) img;
			return	(*pw_sp[i]->callback)(pw_sp[i]->pw_ID,
				pw_sp[i]->user_param, &pw_sp[i]->pw_e);
		    }
			break;
		case PW_POPMENU:
			break;
		case PW_SLIDER:
		case PW_SCROLLBAR:
		default:
			return	prgmerr(0, "not ready");
		}
    }
return	False;
}

general_PW_manager(AnyWindow *ewin /* event win */, XEvent *xep, cookie_t any)
{
pw_manager_t*	my_mp = &pw_cell;
int	i = my_n_pws;
    while (i--)	{
	AnyParts* app = (AnyParts *) my_stack[i]->pw_ID;
	/* user panel has no cb, but exposure	*/
	Window	w = (app->pw_type == PW_POPMENU || IS_PW_Parent(app)) ?
		((AnyWindow *)app)->win : app->pan->win;
	if (w == xep->xany.window)	{
	    switch ((int) xep->type) {
	    case Expose:
		PW_ShowNode(app, my_mp, i);	break;
	    case ButtonPress:
		if (PW_ClickHandler(ewin, xep, my_stack[i]->leaf) < 0)
			return	False;
	    }
	    return	True;
	}
    }
return	False;
}

void
PW_DestroyPanel(Panel *p)
{
	if (!p)	return;
	XFreeGC(p->dpy, p->gc);
	XDestroyWindow(p->dpy, p->win);
	if (p->name)	CFREE(p->name);
	PW_RemoveWidget(p, NULL, 0);
}

PW_Destroy(AnyParts*	app)
{
if (!app)	return	EOF;

if (app->pw_type != PW_BUTTON)
	CFREEnNULL(app->name);

switch (app->pw_type & PW_MASK)	{
case	PW_BUTTON:	{
	register Button*	b = (Button*) app;
		CFREE(b->color);	CFREE(b->bw);
	}	break;
case	PW_POPMENU:
	if (app->pw_type & PW_FREEMEM)	CFREE(((PopMenu*)app)->menu);
	PW_DestroyPanel(app);
	return	0;
case	PW_PRESSBUTTON:
	break;
case	PW_SCROLLBAR:
#ifdef	SCROLLBAR_on_CANVAS
	{
	register ScrollBar*	sb = (ScrollBar *) app;
		XDestroyWindow(sb->awin->dpy, sb->h_swin);
		XDestroyWindow(sb->awin->dpy, sb->v_swin);
	}
#endif
	break;
case	PW_SLIDER:	{
	register Slider*	s = (Slider*)app;
		CFREE(s->info);	CFREE(s->scolor);
		CFREE(s->sx);	CFREE(s->sy);
	}	break;
default:
	return	prgmerr(0, "window %u is not a panel widget", app->valid);
}
PW_RemoveWidget(app, NULL, 0);	/* general routine	*/
return	0;
}
