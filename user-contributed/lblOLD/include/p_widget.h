/*	Panel_WIDGET . H
#
%	Copyright (c)	Jin Guojun
%
% Author:	Jin Guojun, LBL -	1/1/94
*/

#ifndef	P_WIDGET_H
#define	P_WIDGET_H

#ifndef	PANEL_H
typedef	XID	PWID_t;
#endif

typedef	struct	{
	XEvent	*ep;	/* incoming event pointer	*/
	int	etype,	/* callback defined event type	*/
		argu0;	/* a single integer parameter	*/
	cookie_t	CooKie;	/* any event data (value or pointer)	*/
} pw_event;

typedef	struct	pw_tree	{
	struct	PW_Stack	{
		cookie_t	pw_ID,
				user_param;
		StdInterface	*callback;
		struct pw_tree	*leaf;
		pw_event	pw_e;
	}	**PW_stack;	/* dp is slow, but easy to reallocate	*/
	int	n_PW_PWs, n_PW_PSs;
} pw_manager_t;

#define	IS_PanelWidget(pw)	\
		(PW_BUTTON <= ((AnyParts *)pw)->pw_type && \
		((AnyParts *)pw)->pw_type <= PW_SLIDER)
#define	IS_PW_Parent(pw)	\
		(PW_SLIDER < ((AnyParts *)pw)->pw_type || \
		((AnyParts *)pw)->pw_type < PW_BUTTON)

#define	pw_parent(pw)	((AnyParts *)pw)->pan

#define	PW_MASK		0xFFFFFFFFFFFFFFFEL
#define	PW_MAGIC	0xA55A699669960000L

#define	PW_ACCESSORies	(PW_MAGIC | 0x00)
	/* data passed in is not static, and can be freed as desctrcution. */
#define	PW_FREEMEM		0x01	/* So, odd number is reserved	*/
#define	PW_BUTTON	(PW_MAGIC | 0x02)
#define	PW_PRESSBUTTON	(PW_MAGIC | 0x04)
#define	PW_POPMENU	(PW_MAGIC | 0x08)
#define	PW_POPMENU_FM	(PW_MAGIC | 0x08 | PW_FREEMEM)	/* destrcut menu */
#define	PW_SCROLLBAR	(PW_MAGIC | 0x10)
#define	PW_SLIDER	(PW_MAGIC | 0x20)

#define	PW_HORIZONTAL	0
#define	PW_VERTICAL	1
#define	PW_

#define	PW_BUTTONPRESS	1
#define	PW_MENUPOP	2
#define	PW_SLIDING	4


#define	PW_FreeMem	1
#define	PW_Orientation	5
#define	PW_BgColor	6
#define	PW_FgColor	7
#define	PW_InfoStr	8
#define	PW_BaseHeight	9
#define	PW_BaseWidth	10
#define	PW_Height	11
#define	PW_Width	12
#define	PW_MaxHeight	13
#define	PW_MaxWidth	14
#define	PW_Hidden	15
#define	PW_Label	16
#define	PW_Lists	17
#define	PW_Masks	18
#define	PW_Maximum	19
#define	PW_Minimum	20
#define	PW_Number	25
#define	PW_Origin	26
#define	PW_Parent	27
#define	PW_PosXYZ	28
#define	PW_Scale	29
#define	PW_Title	30
#define	PW_ValueA	31
#define	PW_ValueB	32

#ifdef	DOT_V_LIST
#define	ARGU1(a, b)	a, b
#define	ARGU2(a, b, c)	a, b, c
#else
#define	ARGU1(a, b)
#define	ARGU2(a, b, c)
#endif

extern	Button*	PW_CreateButton(ARGU1(Panel* pan, ...));
extern	Panel*	PW_CreatePanel(ARGU1(WinAttribute* parent, ...));
extern	PopMenu* PW_CreatePopMenu(ARGU2(Panel* pan, WinAttribute* parent, ...));
extern	PressButton*	PW_CreatePressButton(ARGU1(Panel* pan, ...));

#endif
