/*	zalloc.c - memory management
#
%	Copyright (c)	Jin Guojun -	All Rights Reserved
%
% Author:	Jin Guojun - LBL	1/1/91
*/

#include "stdef.h"

#ifdef	SHARED_CCS_LIB	/* for compiling in dynamic - shared library	*/
int	_debug;
#else
int	debug;
#endif

static int	memory_used;


#ifndef	LOWER_HEAP_BOUND
#define	LOWER_HEAP_BOUND	11900	/* for Sun only XXX	*/
#endif
#ifndef	HIGHER_HEAP_BOUND
#define	HIGHER_HEAP_BOUND	0x300000	/* was 2750000	*/
#endif
#define	impossible_heap(s)	\
		((int)s < LOWER_HEAP_BOUND || (int)s > HIGHER_HEAP_BOUND)

#ifdef	TC_Need
# include	<stdlib.h>
#else
# ifndef	__STDC__
#	include	<malloc.h>
# endif
# ifndef	coreleft
#	include <sys/resource.h>

MType
coreleft()
{
struct	rlimit	rl;
	getrlimit(RLIMIT_DATA, &rl);
return	rl.rlim_max;
}
# endif
#endif


char*	check_mesg(char *msgp)	/* XXX	*/
{
char	*addr_sp;
if (!msgp
#ifdef	sparc	/*	cc doesn't know how to make a call	*/
	|| impossible_heap(msgp)
#endif
	)
	addr_sp = malloc(16),	sprintf(addr_sp, "0x%x", msgp),	msgp = addr_sp;
return	msgp;
}

#if	defined _DEBUG_ | defined MEMORY_TRACE	/* for non-standard C MM */

static	int	segs;

# ifdef	BIN_TREE_SEARCH
mem_list_p	mem_list[];	/* for fast searching	*/
# else
mem_list_p	mem_list;
# endif

/*	enable tracing memory usage and checking for memory leak.	*/
#define	MEM_LIST

#ifdef	MEMORY_TRACE	/* disabling leakage checking	*/

#undef	_DEBUG_
#define	MEM_MANAGE	/* for non-standard C MM to manage memory usage	*/
#ifndef	ccs_mat_check
# define	ccs_mat_check(s)
#endif

#else	/* _DEBUG_	*/

/* check to see if any memory allocation table (header) has been clobbered;
*	otherwise, it won't work; but do we care?
*	static ?	*/

void	ccs_mat_check(char *msgp)
{
register mem_list_p	mlp=mem_list;
register int	i = segs, bs;
    while (i--)	{
	bs = pointer_buffer_size(mlp[i].maddr);
	if (bs != mlp[i].msize
#if	defined	sgi | defined SOLARIS	/* they change mats somehow	*/
		&& bs-2 != mlp[i].msize
#endif
					) {
	register int	j=segs, pp=0;	/* set break point HERE	*/
	register VType	*mp, *p = NULL;	/* use p may be faster?	*/
		while (j--)	if ((mp=mlp[j].maddr) > p &&
				mp < mlp[i].maddr)	p = mp, pp = j;

		msgp = pp ? mlp[pp].r_name : check_mesg(msgp);
message(!p ? "Other MM intruder [%d] %X %u before %s[%d] <%X %u : %u>\n"
	: "suspect point [%d] %X (rsize = %u) near %s[%d] <%X %u : %u>\n",
			pp, p, mlp[pp].rsize, msgp,
			i, mlp[i].maddr, mlp[i].rsize, bs);
	}
    }
}

#endif	/* MEMORY_TRACE	*/
 
#define	In_Mem_List(i, p)	\
	for (i=segs; i > 0; i--)	if (p==mem_list[i].maddr)	break

mem_list_p
in_mem_list(VType *p)
{
register int	i;
	In_Mem_List(i, p);
return	i < 0 ? NULL : mem_list + i;
}

/* add or delete memory allocation information into or from kernel records */
rec_mem_list(VType *p, MType msize, char* name)
{
register int	ss;
if (msize)	{
	ss = (++segs + 1) * sizeof(*mem_list);
#ifdef	BSD4	/*	Any one else?	*/
	if (!mem_list)
		mem_list = (mem_list_p) malloc(ss);
	else
#endif
		mem_list = (mem_list_p) realloc(mem_list, ss);
	mem_list[0].maddr = mem_list;
	mem_list[0].msize = pointer_buffer_size(mem_list);
	mem_list[0].rsize = ss;
	mem_list[0].r_name = "mmaster";
	ss = segs;
	mem_list[ss].maddr = p;
	mem_list[ss].msize = pointer_buffer_size(p);
	memory_used += (mem_list[ss].rsize = msize);
	mem_list[ss].r_name = name;
	mem_list[ss].stack_n = ss;	/* fancy report	*/
} else	{
	In_Mem_List(ss, p);
	if (ss > 0)	{
	register int	fancys = mem_list[ss].stack_n;
		memory_used -= mem_list[ss].rsize;
		if (segs > ss)
			mem_list[ss] = mem_list[segs];
		segs--;
		ss = fancys;
	} /* else, p is not registered (allocated) by ccs	*/
}
return	ss;
}

#define	add_mem_list	rec_mem_list
#define	del_mem_list(p)	rec_mem_list(p, 0, NULL	/* irix only? */)

#endif	/* _DEBUG_ | MEMORY_TRACE	*/


VType*	/* record and report all memory been allocated	*/
core_trace(MType mleft, VType *p, MType i, MType j, char *ms)
{
static int	m, n;	/* debug only	*/
register int	total = i * j;
char	*s = check_mesg(ms);

#ifdef	MEM_MANAGE
	n = add_mem_list(p, total, s);
#endif

if (DEBUG2OH)	{
#if	defined	_DEBUG_
	ccs_mat_check("core_trace");
	n = add_mem_list(p, total, s);
#elif	!defined	MEM_MANAGE	/* slow but accurate !	*/
	memory_used += (total = pointer_buffer_size(p));
#endif
message("[%2d] %x core [locat %6lX] - %-6lu (%lu * %4lu)\t{REC = %lu} for %s\n",
	n,  m==mleft ? (MType)&mleft : mleft, p, total, j, i, memory_used, s);
	m = mleft;
}
if (!p)	prgmerr(ms, "no enough core available for %s -> %lxH x %lxH = %lu",
		s, i, j, total);
return	p;
}

void	ccs_free(VType *p)
{
if (p)	{
register int	n=-1;
#ifdef	MEM_MANAGE
	n = del_mem_list(p);	/* [n <= 0] means p is not allocted by ccs */
#endif
    if (DEBUG2OH)	{
	register int	ps;
#if	defined	_DEBUG_
	register mem_list_p	mlp = in_mem_list(p);
	if (mlp)
#endif
		ps = pointer_buffer_size(p),
#if	defined	_DEBUG_
		ps = mlp->rsize;	/* mlp->msize is word aligned	*/
	ccs_mat_check("before free");
	n = del_mem_list(p);
#elif	!defined	MEM_MANAGE
	memory_used -= ps;
#endif
	message("release memory [%d] %X %u\t{ REC = %lu }\n",
		n, p, ps, memory_used);
    }
    free(p);
}
else	DEBUGMESSAGE("free NULL at %X\n", &p);
}

#define	MemDef()	(i, j, ms)	register MType	i, j;	char*	ms;

VType*
zalloc	MemDef()
{
	return	ZALLOC(i, j, ms);
}

VType*
nzalloc	MemDef()
{
	return	NZALLOC(i, j, ms);
}


/* allocate a continous 2D array, and assign each row point in order	*/
VType **
alloc_2d_array(colsize, rows)
{
register char	**p = NZALLOC(rows, sizeof(*p), "2d-point");
register int	i=1;
	p[0] = NZALLOC(rows, colsize, "2d-array");
	for (; i<rows; i++)
		p[i] = p[i-1] + colsize;
	return	(VType **) p;
}

void
free_2d_array(VType	**p)
{
	CFREE(p[0]);
	CFREE(p);
}

/* allocate rows pointers, then allocate cols * size for each row pointer
% 	to form a 2D discrete array.	This method is not good for general
%	purpose, but may be good when only some frgaments are left.
*/
VType **
alloc_2d_discrete(cols, rows, size)
{
register VType	**p = NZALLOC(rows, sizeof(*p), "2d-rows");
register int	i=0;
	for (; i<rows; i++)
		p[i] = NZALLOC(cols, size, "2d-cols");
	return	p;
}

void
free_2d_discrete(VType	**p, register int	rows)
{
	while (rows--)	/* can be got by pointer_buffer_size()	*/
		CFREE(p[rows]);
	CFREE(p);
}

p_buffer_size(VType *p)	/* return the block size allocated for pointer *p */
{
#ifdef	MEMORY_TRACE
register mem_list_p	mlp = in_mem_list(p);
	return	(mlp ? mlp->rsize : 0);
#else
	return	pointer_buffer_size(p);
#endif
}

/*
%	For IBM/PC, it returns the last pointer position.
%	For other machine, it may be used for block link.
*/
last_pointer_pos(VType	*p)
{
	return	*((int *)p - 1);
}

/*	when function	returns	NULL -- fails;	-1 -- no change;
%	otherwise,	returns	valid address.
*/
VType*
verify_buffer_size(VType **pp, int bsize, int bytes, char *name)
{
register int	b = abs(bsize), vsize = b * bytes, pbs;
	if (*pp) if ((pbs=pointer_buffer_size(*pp)) < vsize)
		if (bsize > 0)
			CFREE(*pp);
		else	{
#ifdef	MEM_LIST
			del_mem_list(*pp);
#endif
			*pp = (VType*) realloc(*pp, vsize);
#ifdef	MEM_LIST
			add_mem_list(*pp, vsize - pbs, name);
#else	/* simple trace	*/
			if (DEBUG2OH)
				memory_used += vsize - pbs;
#endif
			return	*pp;
		}
	else	return	(VType *) -1;	/* No changes	*/
return	*pp = NZALLOC(b, bytes, name);
}

