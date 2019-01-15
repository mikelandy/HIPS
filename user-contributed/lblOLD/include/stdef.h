/*	stdef.h
#
%	Copyright (c)	Jin Guojun --	All Rights Reserved
%
%	Definitions for common types, arch dependencies, NULL, and errno
%
% AUTHOR:	Jin Guojun - LBL	10/01/1990
*/

#ifndef _STDDEF
#define _STDDEF

#if __STDC__
#define _Cdecl
#else
#define _Cdecl	cdecl
#endif

#if	!defined FILE & !defined __STDIO_DEF_
#include <stdio.h>
#endif
#ifndef	SEEK_SET
#define	SEEK_SET	0
#define	SEEK_CUR	1
#define	SEEK_END	2
#endif
#define	SEEK_GETB	-1
#define	SEEK_UGETB	-2
#define	SEEK_PEEK	-3


extern	FILE*	zreopen(/* char* name, int* status, char* ztmp */);


#if	defined TC_Need & !defined _PTRDIFF_T & !defined sgi
#define _PTRDIFF_T
#if	defined(__LARGE__) | defined(__HUGE__) | defined(__COMPACT__)
typedef long	ptrdiff_t;
#else
typedef int	ptrdiff_t;
#endif
#endif
#ifndef NULL
#if defined(__TINY__) | defined(__SMALL__) | defined(__MEDIUM__)
#define NULL	0
#else
#define NULL	0L
#endif

extern	int	_Cdecl errno;

#else

extern	int	errno;

#endif	/*	TC_Need	*/


#ifdef	NO_CCSECL
# define	msg(s, a)	fprintf(stderr, s, a)
# define	message(s,a,b)	fprintf(stderr, s, a, b)
#else
# define	msg	message
#endif
#ifndef	mesg
#define	mesg(s)		fprintf(stderr, s)
#endif

#ifndef	VType
#define	VType	void
#endif
typedef	void*	VPointer;

#ifndef	PtrCAST
#define	PtrCAST	(VType *)
#endif

#define	swif_h_alist	job, img, ac, av, assist
#define	swif_h_alist_def	\
	U_IMAGE	*img;	\
	cookie_t ac;	\
	char	*av[];	\
	VType	*assist

#ifndef	MIN
# define MIN(a, b)	((a < b) ? a : b)
#endif

#define	at_least_1(v)		if (! v)	v += 1
#define	at_least_n(v, n)	if (v < n)	v = n
/*	macros for swapping. Which is faster?	*/
#define	swap_by_xor(a, b)	{	a ^= b;	b ^= a;	a ^= b;	}
#define	swap_by_tmp(a, b, tm)	{	tm = a;	a = b;	b = tm;	}
#define	swap_by_reg(a, b, tp)	{	register tp	t;	\
					t = a;	a = b;	b = t;	}

#ifndef	isfloat
#include <ctype.h>
#define	isfloat(c)	(isdigit(c) || '+'<=c && c<='.' || toupper(c)=='E')
#endif

#define	Stx	2
#define	Heart	3
#define	Diamond	4
#define	Club	5
#define	Spade	6
#define	Ack	6
#define	Bel	7
#define	BS      8
#define	Tab     9
#define	LF      10
#define	VT      11
#define	FmFd    12
#define	CR      13
#define	SO	14
#define	SI	15
#define	Esc	0x1B
#define	DC2	0x12
#define	DC4	0x14
#define	NAk	0x15
#define	CTRL_Y	0x19
#define	Space	0x20
#define	Del	0x7F
#define	GT_EQL	242
#define	ST_EQL	243
#define	RootS	251

#define	ByteMax	256

#ifndef	TRUE
#define	TRUE	1
#define	FALSE	0
#endif
#define	Yes	1
#define	No	0

#ifndef	True
#define	True	1
#define	False	0
#endif


#ifdef	HIPS2_HF
#define	BYTE_DEFINED
#endif
#ifndef	BYTE_DEFINED
typedef unsigned char	byte;
#endif

#ifdef	COMMON_TOOL
# ifndef	FITS_IMAGE
#    define	FITS_IMAGE
# endif
# ifndef	RLE_IMAGE
#    define	RLE_IMAGE
# endif
#endif

/*	standard TYPE definitions	*/
#ifndef	MType
# if	(INT_BITS == 16)
#   define	MType	long
# else
#   define	MType	int
# endif
#endif

#ifndef	cmap_t
#define	cmap_t	byte
#endif

typedef int		bool;
typedef unsigned int	Pointer_Mem;
typedef unsigned int	word;

#if	(INT_BITS == POINTER_BITS)
#define	cookie	int
#elif	(LONG_BITS == POINTER_BITS)	/* OK, we accept it for osf ?	*/
#define	cookie	long
#else
#define	cookie	void*	/* This will cause many warnings	*/
#endif

typedef	cookie	cookie_t;	/* if cookie is defined somewhere else	*/

#if	(LONG_BITS > 32 && INT_BITS == 32)
typedef	unsigned int	longword;
#define	long_32	int	/* for C standard problems on data exchange	*/
#else
typedef	unsigned long	longword;
#define	long_32	long
#endif

typedef	short		coord_t;

#ifndef	SIZEOF
#define	SIZEOF	(MType)sizeof
#endif

/*	definition for display attributes.	*/
#define	NoDpyB	0
#define	UnderLn	1
#define	Normal	7
#define	Reverse	0x70
#define	Blink	0x80
#define	HighLi	8

#define	ScrollUp	6
#define	ScrollDown	7

/*	The foot of a letter is lower than y0 in XDrawString()	*/
#define	XStringBaseHigh	4

#ifdef	__cplusplus
extern	"C"	{
#endif

typedef	struct	{
	coord_t	x0, y0,	/*	upper left corner position	*/
		cs, ls;	/*	Colunms and Lines		*/
	} Range;

typedef	struct	{
	coord_t	x, y,		/* x & y are margin's offsets		*/
		width, height;	/* screen display area height & width	*/
	} Offset;


typedef	union	{
	struct	{
		char	line_w, fill, join, style;
		short	angle1, angle2;
	} draw;
	struct	{
		char	font_id, fw, fh, ascent,
			*content;
	} text;
}	si_union;

typedef	struct	{
		char	r, g, b, a;
	}	color_channel;

typedef	union	{
	long_32	index;
	color_channel	v;
	} color_union;

typedef	struct	{
	coord_t	x0, y0, w, h;
	long_32	color;
color_channel	v;
	char	e_type, hidden, cols, rows;
	long_32	reserved;
	si_union	elem;
	} superimpose_elems;

typedef	struct	{
	short	stack_len, n_elems;
	superimpose_elems*	stack;
	} superimpose_stack;

#define	which_si_elem(isText, Elem)	(isText=Elem<0) ? Elem=~Elem : Elem--

#ifndef	QSType
#define	QSType	MType
#endif

typedef	struct	{	/* for QuickSort */
	QSType	value;	/*	sort dependent value	*/
	int	QsIndex;	/* can be any pointer	*/
	} QSCell;

typedef	struct	{
	char	*base,	/* buffer pointer; shouldn't be changed	*/
		*ptr;	/* data pointer	*/
	MType	bsize,	/* buffer size	*/
		dlen;	/* data size	*/
	int	offset,	/* header length	*/
		flags;	/* buffer ID	*/
	} BUFFER;

#define	BUFFER_MAGIC	0x9669A5A5	/* need to fit short and long	*/
#define	beof(bp)	((bp)->ptr - ((bp)->base + (bp)->offset) >= (bp)->dlen)


#define	SoftInterface	cookie_t	/* int is not OK	*/
typedef	SoftInterface	TableInterface;
typedef	SoftInterface	StdInterface();

typedef	struct	{
StdInterface	*open,
		*close,
		*op,	/* read, write, ...	*/
		*seek,
		*eof;
	} com_io_sw_t;

typedef	struct	{
	FILE	*fp;
	com_io_sw_t	*bio;
	} io_content;

extern	com_io_sw_t	isys_bio[];

BUFFER*	buffer_create();
void	buffer_close();
int	buffer_seek(), buffer_read(), buffer_write(),
	pipe_read(), peekbyte();

/*	for old Image io compatibility	*/
#define	IN_FP	i.fp
#define	OUT_FP	o.fp
#define	i_read	i.bio->op
#define	i_write	o.bio->op
#define	r_seek	i.bio->seek
#define	w_seek	o.bio->seek
#define	r_eof	i.bio->eof
#define	w_eof	o.bio->eof
#define	r_open	i.bio->open
#define	w_open	o.bio->open
#define	r_close	i.bio->close
#define	w_close	o.bio->close

typedef	struct	{
	StdInterface	*handle;	/* handler routine pointer	*/
	cookie_t	pc1, pc2, pc3;	/* pointer parameters	*/
	int	pi1, pi2, pi3, pi4;	/* value parameters	*/
	} eX_thread;		/* event headler thread	*/

typedef	enum	{
	ARGU_NONE = -1,
	ARGU_B0,	/* #0, the first # for "%0-#"	*/
	ARGU_B1,
	ARGU_B2,
	ARGU_B3,
	ARGU_B4,
	ARGU_B5,
	ARGU_B6,
	ARGU_B7,
	ARGU_B8,
	ARGU_B9,
	ARGU_B10,	/* #10, maybe the top # for %0	*/
	ARGU_B14 = 14,	/* the last possible # for "%0"	*/
	ARGU_BNEG =	'!',	/* reserve 0F - 1F for other Bxxx	*/
	ARGU_BNEGI =	'~',	/* 0 if input is set; otherwise 1.	*/
	ARGU_BNUM =	'#',	/*	True if input with number	*/
	ARGU_BPLUS =	'+',
	ARGU_BMINUS =	'-',
	ARGU_SCALE =	'*',
	ARGU_LIST =	'?',
	ARGU_BANDEF =	'&',	/* logic operation with default value	*/
	ARGU_BORDEF =	'|',
	ARGU_BXORDEF =	'^',
	ARGU_BEXTYPE =	'B',	/*	return	extended argument type	*/
	ARGU_BEXT =	'E',	/*	return	extension value	*/
	ARGU_BDEF =	'N',	/*	reset default value	*/
	ARGU_BOOL =	'b',
	ARGU_CHAR =	'c',
	ARGU_SHORT =	'h',	/* not in numerical order !!	*/
	ARGU_HEX =	'x',
	ARGU_INT =	'i',
	ARGU_FLOAT =	'f',
	ARGU_DOUBLE =	'g',
	ARGU_STRING =	's',	/* place argv[?] to given pointer address */
	ARGU_STRCPY,	/* "s+" copy argv[?] content to given array space */
	ARGU_STRCAT,	/* "s++" cat argv[?] content to given array space */
	ARGU_ADDPARAM =	126,	/* ADDPARAMS only used with TWO_ARGU_ADDPARAM */
	ARGU_ADDPARAMS	/* flag should be "?-*" ;  ? is any single character */
} argu_type;

typedef	union	{
    argu_type	a;
	bool	b;
	char	c;
	short	s;
	int	i;
	float	f;
	double	d;
} convs_v;

typedef	union	{
    argu_type	*a_p;
	bool	*b_p;
	char	*c_p,
	*	*stp;
	short	*s_p;
	int	*i_p;
	float	*f_p;
	double	*d_p;
	void	*v_p;
} convs_p;

typedef	struct	{
	argu_type	type;
	convs_p	p;
	convs_v	v;
} convs_s;

#ifndef	HLIB	/* for backward compatibility!	*/
#define	convs	convs_s
#endif

typedef	struct	{
	char	*flag,		/* argument flag	*/
		*in_fmt;	/* input format.	*/
#ifdef	USE_STRING_ARGU_DEF
	char*	def_val;	/* no defined variable can be passed	*/
#else
	float	def_val;	/* no double value for default	*/
#endif
	int	num_vars,
		min_inps;	/* min input(s) for above variables	*/
	char	*info;
	bool	sscan;		/* this is the offset for LIST	*/
} arg_fmt_list_string;

typedef	struct	e_afls	{
	arg_fmt_list_string	*fls;
	int	flag_len,
		u_l_s,
		extended_flags,
		ext_level;	/* root=0, 1st_ext=1, 2nd_ext=2, ...	*/
	convs_s	*v;
	struct	e_afls	*extend;
}	arg_fmt_lists;

typedef struct	ml	{	/* for any list	*/
	VType*	maddr;
	MType	msize,		/* real allocated size	*/
		rsize;		/* required size	*/
	char*	r_name;		/* point to caller's name	*/
	int	stack_n,	/* real stack number	*/
		reserved;
#ifdef	BIN_TREE_SEARCH
	struct	ml	*prve, *next;
#endif
} *mem_list_p;

extern	mem_list_p	in_mem_list(/* if p is in memory management list */);

#ifdef	__cplusplus
}
#endif

#ifdef	WINDOW_VER

#	include <dos.h>
typedef	union	REGS	Regset;
#	include <keycode.h>
#	define	WaitKey()	while (!kbhit())

#endif

extern	bool	_debug, debug;
bool	prgmerr(
#ifdef	DOT_V_LIST
		int, char*, ...
#endif
				);

#ifdef	HAVE_STDLIB_H
#include <stdlib.h>	/* for int.s != pointer	*/
#endif

#include <string.h>
double	arget();	/* <math.h> for atof()	*/
float	consum_time();
VType	*nzalloc(), *zalloc(), *core_trace();

#define	Loop	while (1)

#define	repeat_8(ops)	{ops;}{ops;}{ops;}{ops;}{ops;}{ops;}{ops;}{ops;}
#define	spread_8(sw, ops, fval)	\
	switch (sw)	{	\
	case 7:	{ops;}	\
	case 6:	{ops;}	\
	case 5:	{ops;}	\
	case 4:	{ops;}	\
	case 3:	{ops;}	\
	case 2:	{ops;}	\
	case 1:	{ops;}	\
	default: fval;	\
	}
#define	unroll8_fwd(init, count, upper, ops)	{	\
	for(init; count+8 <= upper; count += 8) {	\
		repeat_8(ops)	}		\
	spread_8(upper-count, ops, count=upper)	\
	}
#define	unroll8_bwd(init, count, ops)	{	\
	for(init; count >= 8; count -= 8) {	\
		repeat_8(ops)	}		\
	spread_8(count, ops, count=0)	}

#ifndef	DEBUGLEVEL
#ifdef	SHARED_CCS_LIB
#define	DEBUGLEVEL	_debug
#else
#define	DEBUGLEVEL	debug
#endif
#endif
#define	DEBUGANY	DEBUGLEVEL

#define	DEBUG_L2	(DEBUGLEVEL==2)
#define	DEBUG_L3	(DEBUGLEVEL==3)
#define	DEBUG2OH	(DEBUGLEVEL > 1)
#define	DEBUG3OH	(DEBUGLEVEL > 2)
#define	DEBUGMESSAGE	if (DEBUGANY) message
#define	DEBUG2MESSAGE	if (DEBUG2OH) message

#define	NS_TSET(r, c, type)	avset(argc, argv, r, c, type)

#define	Alloc_Mem(ahow, i, j, ms)	\
	PtrCAST((i)&&(j) ? core_trace(coreleft(), ahow, i, j, ms) : 0)
#define	NZALLOC(i, j, ms)	Alloc_Mem(malloc((i) * (j)), i, j, ms)
#define	ZALLOC(i, j, ms)	Alloc_Mem(calloc(i, j), i, j, ms)

#if	defined	_DEBUG_ | defined USE_CCS_MEMMM
#define	CFREE	ccs_free
#else
#define	CFREE	free
#endif
/*	Macro to release core and set pointer to NULL	*/
#define	CFREEnNULL(p)	CFREE(p), p = NULL

#if	defined	__osf__	/* a very interesting method	*/
# ifdef	__osf1v2_
#   define	pointer_buffer_size(p)	( ((1 << ((int *)p)[-2]) - 1) << 4 )
# else	/* OSF1/V3 or later	*/
#   define	pointer_buffer_size(p)	(-32 - *((int*)(p) - 2))
# endif
#elif	defined	MEMORY_TRACE	/*	ultrix | BSD(i)	*/
# define	pointer_buffer_size(p)	p_buffer_size(p)	/* use func. */
#else
# if	defined	sgi || defined SOLARIS
# define	BUFFER_SIZE_OFFSET_LENGTH	1
# else
# define	BUFFER_SIZE_OFFSET_LENGTH	(sizeof(int) << 1)
# endif
#define	pointer_buffer_size(p)	( *((int *)(p) - 2) - BUFFER_SIZE_OFFSET_LENGTH)
#endif

#define	pointer_last_block(p)	( *((int *)(p) - 1) )

#ifndef	str_save
# ifdef	__osf__
#define	str_save(s)	strcpy(zalloc((MType)(strlen(s)+1), (MType)1, s), s)
# else
#define	str_save(s)	strcpy(ZALLOC((MType)(strlen(s)+1), (MType)1, s), s)
# endif
#endif

#if defined TC_Need & !defined _SIZE_T
#	define	_SIZE_T
typedef unsigned	size_t;
#endif

#ifdef	NO_bzero
#define	bzero(p, n)	memset(p, 0, n)
#endif
#ifdef	NO_bcopy
#define	bcopy(s,d,n)	memcpy(d,s,n)
#endif

#include <time.h>

#ifdef	USE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef	BSD4
int	fclose(), fseek();
size_t	fread(), fwrite();
#endif

/*	for reading a stream to work with fseek, fgetc, and ungetc	*/
#define	init_pipe_read(i)	set_pipe_read(i, True)
#define	reset_pipe_read(i)	set_pipe_read(i, False)
#define	pushpipe(buf,size,fp)	pipe_read(buf, -1, size, fp)

/*	timing report for performance analysis	*/
extern	struct	timeval	global_start_time, loc_start_time, end_time;
/*	used at begin of a program	*/
#define	INIT_PERFORM_TIMING()	consum_time(&global_start_time, &loc_start_time, 0)
/*	for analysing each modules	*/
#define	CONSUMED_TIME(s)	consum_time(&loc_start_time, &end_time, s)
/*	used at end of a program	*/
#define	TOTAL_CONSUMED(s)	consum_time(&global_start_time, &end_time, s)

#endif	/* _STDDEF	*/

#if	defined	MACHINE_ENDIAN
#include <machine/endian.h>
#elif	defined	DEF_ENDIAN	/* #elif must use defined !!!	*/
#include <endian.h>
#elif	defined	sgi || defined BYTE_ORDER && BYTE_ORDER == BIG_ENDIAN
#undef	LITTLE_ENDIAN
#endif

