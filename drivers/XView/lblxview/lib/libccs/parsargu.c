/*	ParseARGUment . C
#
%	Copyright (c)	Jin Guojun - All rights reserved
%
% Restrictions:
%	<1>	with extension mode:
%			(1) only one variable can be passed
%			(2) format string can not be reused
%	<2>	For fast processing:
%		If same flag is given more than once, only the first one
%		on the list is used, and w/o warning.
%
% Author:	Jin Guojun - LBL	10/01/92
*/

#include <math.h>
#include "va_vset.h"
#include "stdef.h"

#ifndef	msg
#define	msg(f, s)	fprintf(stderr, f, s)
#endif
#ifndef	LONG_BITS
#define	LONG_BITS	32
#endif
#ifdef	USE_STRING_ARGU_DEF
#define	USAD	/* acronym	*/
#endif
#define	isARGU_B(a)	(a<ARGU_BOOL || a >= ARGU_BORDEF)


static	int	use_pounds, find_input_type();

void
p_use_pound_sign(bool y_or_n)
{
	use_pounds = y_or_n;
}

void
free_arg_fmt_list(int num_list, register arg_fmt_lists*	fmt_list)
{
register int	i, n;
	for (n=num_list; n--; fmt_list++)	{
		i = fmt_list->extended_flags;
		while (i--)
			CFREE(fmt_list->extend[i].v),
			CFREE(fmt_list->extend[i].fls);
		if (fmt_list->extended_flags)
			CFREE(fmt_list->extend);
		CFREE(fmt_list->v);
	}
	CFREE(fmt_list - num_list);
}

#if	defined	GCC_NWrtStr && defined	ReAlloc_NWrtStr

# define	ReAlloc_NWrtStrBuf
# undef	GCC_NWrtStr
char*	rebuild_str(arg_fmt_list_string* fmt)
{
char	*cp;
	if ((cp=index(fmt->flag, '[')) && index(cp, ']'))	{
		cp = str_save(fmt->flag);
		fmt->flag = cp;
	}
return	fmt->flag;
}

#endif

arg_fmt_lists*
build_arg_fmt_list(int	*retn, arg_fmt_list_string* fmt, va_list ap)
{
int	tBE, na=1;
register int	n, ne;
register arg_fmt_lists*	aflp;
char	*cp, *cpr, *fmtp;
#define	nv	ne

#ifdef	GCC_NWrtStr
int	gne;
#define	SEP_SBRACKET(p)
#else
#define	SEP_SBRACKET(p)	*p = No
#endif

    while ((++fmt)->flag)	na++;
    if (retn)	*retn = na;
    aflp = ZALLOC((MType)na, SIZEOF(*aflp), "arg_fmt_list");

    for (fmt-=na, n=0; n<na; fmt++, n++)	{
	aflp[n].fls = fmt;
	fmtp = fmt->in_fmt;
	aflp[n].v = ZALLOC((MType)(tBE=fmt->num_vars), SIZEOF(convs_s), "aconvs");
	for (nv=aflp[n].ext_level = 0; nv < tBE; nv++)
		aflp[n].u_l_s = find_input_type(&fmtp, fmt->def_val,
			(nv - (nv && aflp[n].v->type < ARGU_BOOL)) < fmt->min_inps,
			aflp[n].v+nv, &ap);
#ifdef	GCC_NWrtStr
	gne=0;
#endif
	if (nv)	{	/*	if not comment, build extensions	*/
#if	defined	ReAlloc_NWrtStrBuf
	cp = rebuild_str(fmt);
#else
	cp = fmt->flag;
#endif
	    tBE = isARGU_B(aflp[n].v->type);
	    while (cpr = strchr(cp, '['))	{
		register arg_fmt_lists*	eaflp;
		SEP_SBRACKET(cpr);
#ifdef	GCC_NWrtStr
		if (!gne)	gne = cpr - cp;
#endif
		cp = ++cpr;
		ne = aflp[n].extended_flags++;
		verify_buffer_size(&aflp[n].extend, -ne-1, SIZEOF(*eaflp), "exaflp");
		eaflp = aflp[n].extend + ne;
		eaflp->ext_level = ++ne;
		eaflp->extended_flags = 0;
		eaflp->fls = ZALLOC((MType)1, SIZEOF(*eaflp->fls), "efls");
		eaflp->fls->flag = cp;
		if (!(cpr = strchr(cp, ']')))	return	(arg_fmt_lists*)
			prgmerr(-1, "extension not match %s", cp);
		SEP_SBRACKET(cpr);
		{	register MType	num_ext = *cp != '#';
		eaflp->flag_len = num_ext ? strlen(cp) : 0;
		num_ext &= tBE;	/* # ext is a special type, not ext.	*/
		cp = ++cpr;
		eaflp->v = ZALLOC((num_ext + 1), SIZEOF(convs_s), "econvs");
		eaflp->fls->num_vars = num_ext + 1;
		eaflp->u_l_s = find_input_type(&fmtp, eaflp->fls->def_val =
			aflp[n].fls->def_val, No, eaflp->v + num_ext, &ap);
		if (num_ext)	*eaflp->v = *aflp[n].v;
		}
	    }
	}
#ifdef	GCC_NWrtStr
	if (gne)	ne = gne;	else
#endif
	ne = strlen(fmt->flag);
	aflp[n].flag_len = ne - (fmt->flag[ne-1] == '#');/* for -# */
    }
return	aflp;
}


static
/*	return	0 for OK; otherwise for unknown type	*/
find_input_type(fmtp, dvsp, setv, v, ap)
register char**	fmtp;
#ifdef	USAD
	char*
#else
	float
#endif
		dvsp;
bool	setv;
convs_s	*v;
va_list	*ap;
{
static argu_type	last_arg_t=ARGU_NONE;
argu_type	arg_t, et;
register float	dv =
#ifdef	USAD
		atof
#endif
			(dvsp);

	while (**fmtp && *(*fmtp)++ != '%');
	v->p.v_p = va_arg(*ap, void*);

	et = (*fmtp)[1];
	/* user should pass a valid LIST buf either NULL or allocated,
	*	and we don't want to touch it here	*/
	if (last_arg_t == ARGU_LIST)	setv = 0;

	switch (arg_t = **fmtp)	{
	case 'c':	if (setv) *v->p.c_p = dv;	break;
	case 'd':	arg_t = ARGU_INT;	goto	set_int;
	case 's':	if (et == '+')	{	arg_t++;/* ARGU_STRCPY	*/
			 if ((*fmtp)[2] == '+')	arg_t++;/* ARGU_strcat	*/ }
#if	(LONG_BITS > 32)	/* long == void* ?	*/
		goto	set_64b;
#else	(LONG_BITS == 32)
		goto	set_int;
#endif
	case 'X':	arg_t = ARGU_HEX;
	case 'x':
	case '-': case '+':if (et==ARGU_SHORT)	goto	set_sht;
	set_int:
	case 'i':	if (setv) *v->p.i_p = dv;	break;
	case 'S':	arg_t = ARGU_SHORT;
	set_sht:
	case 'h':	if (setv) *v->p.s_p = dv;	break;
	case 'f':	if (setv) *v->p.f_p = dv;	break;
	case 'D':	arg_t = ARGU_DOUBLE;
	set_64b:
	case 'g':	if (setv) *v->p.d_p = dv;	break;
	case '%':	arg_t = ARGU_ADDPARAM;	/* flag = "?-*"	*/
			if (et == '+')	arg_t++;	/* more argus */
			break;
	case 'b':	case '?':
#ifndef	UNCONDITION_BOOL_ARGU_SET	/* make more flexible	*/
		if (setv)
#endif
			*v->p.b_p = !dv;	/* who else ???	*/
	case 'N': case 'E': case 'B':	case '*':
	case '&':	case '|':	case '^':
	case '#':	case '!':	case '~':	break;
	default:if (isdigit(**fmtp))
			arg_t = (argu_type) atoi(*fmtp);
		else	arg_t = last_arg_t;
	}
	while (*(++*fmtp) && **fmtp != '%');
	v->type = last_arg_t = arg_t;
return	arg_t==ARGU_NONE ? arg_t : et;	/* extended integer type	*/
}

get_args(int nflags, arg_fmt_lists* fmt_list,
	int argc, char* *argv, int *n, int f)
{
#ifdef	_DEBUG_
argu_type	p_t;	/* parent type	*/
#else
#define	p_t	cmn
#endif
char	*val;
float	scale;	/* make things a little bit slower, but may be useful	*/
int	dbl_arg, e_dv, noarg = ARGU_NONE,	p_list, s_list;

#define	NUMber()	NS_TSET(n, &f, 0)
#define	NextArg()	NS_TSET(n, &f, 1)

    while (nflags--)	{
	register arg_fmt_lists* aflp = fmt_list+nflags;
	register int	cmn = aflp->flag_len, nvs;
	if (cmn)	{
	    if (strncmp(val=argv[*n]+f, aflp->fls->flag, cmn) ||
		((e_dv=aflp->extended_flags) && strlen(val) > cmn && /* extension */
		((noarg=get_args(e_dv, aflp->extend, argc, argv, n, cmn)) ||
			!(f=noarg) /* ret 0 -> rewind f to begin of next str */)
	/* "-#" */	|| cmn==1 && *aflp->fls->flag=='-' && !isfloat(val[cmn])))
		continue;	/* maybe faster than	return	0;	*/
            f += cmn;
	} else	continue;

	e_dv =
#ifdef	USAD
		atoi
#endif
			(aflp->fls->def_val);

	p_t = aflp->v->type;
	dbl_arg = isARGU_B(p_t);
	if (p_t < ARGU_BNEG)
		*aflp->v->p.a_p = p_t;

#define	advf	f = strlen(argv[*n])
#define	vp	aflp->v->p
#define	doSHORT	if (aflp->u_l_s==ARGU_SHORT)
	p_list = 0;	s_list = e_dv;
	scale = 0;
	switch (p_t)	{	/*	process ARGU_Bxxxx	*/
	case ARGU_BDEF:	doSHORT	*vp.s_p = e_dv;
			else	*vp.i_p = e_dv;	break;
	case ARGU_BANDEF:	doSHORT	*vp.s_p &= e_dv;
				else	*vp.i_p &= e_dv;	break;
	case ARGU_BORDEF:	doSHORT	*vp.s_p |= e_dv;
				else	*vp.i_p |= e_dv;	break;
	case ARGU_BXORDEF:	doSHORT	*vp.s_p ^= e_dv;
				else	*vp.i_p ^= e_dv;	break;
	case ARGU_BEXTYPE:
		if (cmn=aflp->ext_level)	{
			val = aflp->fls->in_fmt;
			while (cmn--)	while (*val && *++val == '%');
			*vp.i_p = *val;
		}
		break;
	case ARGU_BEXT:
		if (aflp->ext_level)
			*vp.i_p = *aflp->fls->flag;
		break;
	case ARGU_BNUM:
	case ARGU_BNEGI:
		*vp.b_p = NUMber();
		if (p_t == ARGU_BNEGI)
	case ARGU_BNEG:	*vp.b_p = !*vp.b_p;
		break;
	case ARGU_BPLUS:
		++*vp.b_p;	break;
	case ARGU_BMINUS:
		--*vp.b_p;	break;
	case ARGU_SCALE:	scale = *vp.f_p;
		break;
	case ARGU_LIST:	p_list++;
	}

	for (p_t=aflp->v[nvs=dbl_arg].type; nvs < aflp->fls->num_vars; nvs++) {
#ifndef	TC_Need
	register	/* TurboC bug? cant handle single word union	*/
#endif			/* some stupid compiler don't like auto-init	*/
		convs_p	cvp;
		cvp = aflp->v[nvs].p;
		if (p_list)	/* LIST uses array DB pointer instead of ptr */
			verify_buffer_size(aflp->v[nvs].p.v_p, -s_list, p_list,
				"argu_list"),
			cvp.s_p = *aflp->v[nvs].p.stp + (p_list - 1) * s_list
				+ aflp->fls->sscan;	/* field offset	*/
	switch (aflp->v[nvs].type)	{	/* for input argus. was p_t */
	case ARGU_BOOL:
		*cvp.b_p =
#ifdef	USAD
			strcmp(aflp->fls->def_val, "False") != 0;
#else
			aflp->fls->def_val;
#endif
		break;
	case ARGU_ADDPARAMS:	/* can be simpler ?	*/
	case ARGU_ADDPARAM:
	case ARGU_STRCAT:
	case ARGU_STRCPY:
	case ARGU_STRING:
		if (p_t >= ARGU_ADDPARAM)	/* add paramter flag; no argus control	*/
			strcat(cvp.c_p, argv[*n] + 1),
			strcat(cvp.c_p, " "),	advf;

		if (!NextArg())	return	f;	/* end of argv list	*/

		switch (p_t)	{
		case ARGU_ADDPARAMS:
#ifndef	TWO_ARGU_ADDPARAM
		case ARGU_ADDPARAM:
#endif
		case ARGU_STRCAT:
			strcat(cvp.c_p, argv[*n]); /* add paramter element */
			if (p_t != ARGU_STRCAT)	strcat(cvp.c_p, " ");
			break;
		case ARGU_STRCPY:
			strcpy(cvp.c_p, argv[*n]);
			break;
		case ARGU_STRING:
			if (!f && *argv[*n] == '-')	continue;
			/* cvp.stp == *vp.i_p; but != *vp.c_p */
			*cvp.stp = argv[*n] + f;
		}
		advf;
		break;
	default:
		if (p_list || !aflp->fls->sscan)	{
		double	vd;
			if (!NUMber())
				if ((e_dv=nvs-aflp->fls->min_inps) < dbl_arg) {
					if (!p_list && isdigit(*argv[*n]))
						NextArg();
					return	e_dv+p_list-1;
				} else	if (p_t != aflp->v[nvs].type)
					continue;	/* no defaults	*/
				else	{
#ifdef	USAD
					val = aflp->fls->def_val;
#else
					val = NULL;
					vd = aflp->fls->def_val;
#endif
				}
			else	val = argv[*n] + f,	advf;	/* fast process	*/

			if (val)	vd = atof(val);
			if (scale)	vd *= scale;
			e_dv = vd;

			switch (aflp->v[nvs].type)	{
			case ARGU_HEX:	if (val) {
				sscanf(val, "%x", cvp.i_p);	break;
			}
			case ARGU_INT:	*cvp.i_p = e_dv;	break;
			case ARGU_SHORT:*cvp.s_p = e_dv;	break;
			case ARGU_FLOAT:*cvp.f_p = vd;	break;
			case ARGU_DOUBLE:*cvp.d_p = vd;	break;
			default:
			    if (nvs+1 < aflp->fls->min_inps)	return	EOF;
			}
		} else	if (NUMber())	{
		void *tempp[8];	/* max = 8 argus for a scanning "set"	*/
		    for (nvs = MIN(8, aflp->fls->num_vars); nvs-- > dbl_arg; )
			tempp[nvs-dbl_arg] = aflp->v[nvs].p.v_p;
		    sscanf(argv[*n] + f, aflp->fls->in_fmt + (dbl_arg << 1),
				tempp[0], tempp[1],
				tempp[2], tempp[3], tempp[4],
				tempp[5], tempp[6], tempp[7]);
		    advf;	goto	pend;
		}
	}
	 if (p_list)
		*vp.i_p = p_list++,
		nvs--;	/* keep go through list	*/
	}
pend:
	NextArg();
/*	noarg = abs(noarg) - *n;	if (noarg)	noarg = *n;	*/
	noarg = f;
	if (aflp->ext_level && !aflp->extended_flags)	break;
    }
return	noarg;
}


/*	Independent session. If can be in another file	*/
parserr(arg_fmt_lists*	fmt, int items, int vless, int argerr, char**	argv)
{
int	i;
register char*	flagp=argv[argerr];

	if (vless=abs(vless)-1)
		prgmerr(0, "option %s less %d argu\n", flagp, vless);
	else	prgmerr(0, "wrong option [%d] %s\n", argerr, flagp);
#ifndef	GCC_NWrtStr
	for (;items-- && *(flagp=fmt[items].fls->flag);)
	    if (fmt[items].fls->num_vars)	/* don't change comments */
		/*	recover the extensions	*/
		for (i=fmt[items].extended_flags; i--;)	{
			while(*++flagp);	*flagp = '[';
			while(*++flagp);	*flagp = ']';
		}
#endif
	parse_usage(fmt->fls);
}

parse_usage(register arg_fmt_list_string*	fmt)	/* public entry	*/
{
int	i, tBE;
char	msgfmt[128];
register char*	flagp;

    for (; flagp=fmt->flag; fmt++)	{
	msg("%s", fmt->flag);
	if (flagp=fmt->in_fmt)
	for (i=0, tBE=isARGU_B(flagp[1]); i<fmt->num_vars || *++flagp; i++) {
	register char	*finfo, c;
		while (*flagp && *flagp++ != '%');
		if (!*flagp)	continue;
		c = *flagp;
	    if (use_pounds && c > ARGU_CHAR && c < ARGU_STRING)
		finfo = "#";
	    else switch (c)	{
		default:while (flagp[1] && flagp[1] != Space)	flagp++;
			if (fmt->num_vars > 1)
				finfo = "";
			else
		case 'b':	finfo = "\t";	break;
		case 'd': case 'i':	finfo = "INTEGER";	break;
		case 'f':	finfo = "FLOAT";	break;
		case 'g': case 'D':	finfo = "DOUBLE";	break;
		case 'h':	finfo = "SHORT";	break;
		case 'c': case 's':	finfo = "STRING";	break;
		case 'X': case 'x':	finfo = "Hex";	break;
		}
	    msg(i-tBE<fmt->min_inps || *finfo=='\t'? "  %s" : " [%s]", finfo);
	}
	if (flagp=fmt->info)	{
		while (iscntrl(*flagp))	msg("%c", *flagp++);
		if (strlen(flagp) > sizeof(msgfmt) - 8)
			msg("\t{ %s }\n", flagp);
		else	sprintf(msgfmt, "\t{ %s }\n", flagp),
			msg(msgfmt, fmt->def_val);
	} else	mesg("\t{ < ? > }\n");
    }
}

parse_argus(	/* public entry	*/
	char**	flistp[],
	int	argc,
	char**	argv,
	arg_fmt_list_string*	fmt,
	va_list	va_alist	)
{
int	i, nf=0, nvars, v;
#ifdef	MUST_START_VA
arg_fmt_lists*	fmt_list;
vset(fmt);
	fmt_list = build_arg_fmt_list(&nvars, fmt, ap);
#else
arg_fmt_lists*	fmt_list = build_arg_fmt_list(&nvars, fmt, (va_list)&va_alist);
#endif

if (flistp)	*flistp = NULL;

    for (i=0; ++i < argc;)	{
retry:	v = get_args(nvars, fmt_list, argc, argv, &i, 0);
	if (v > 0 && !argv[i][v])	continue;
	else if (flistp	&& *argv[i] != '-' && *argv[i] != '+' &&
			!(ispunct(*argv[i]) && argv[i][1] == '-'))	{
		verify_buffer_size(flistp, -SIZEOF(int), nf+1, "nf");
		(*flistp)[nf++] = argv[i];	/*	add_file_list	*/
	} else if (v < 0)	{
		nf = -i;
		parserr(fmt_list, nvars, v, i, argv);
		break;
	} else 	if (!v)	goto	retry;	/*	fast process result	*/
    }
free_arg_fmt_list(nvars, fmt_list);
return	nf;
}
