/*
 * This file is considered to be public domain software.  We hereby give
 * permission for anyone to make any use of this code, including copying the
 * code, including it with freely distributed software, including it with
 * commercially available software, and including it in ftp-able code.
 * We do not assert that this software is completely bug-free (although we hope
 * it is), and we do not support the software (officially) in any way.  The
 * intention is to make it possible for people to read and write standard HIPS
 * formatted image sequences, and write conversion programs to other formats,
 * without owning a license for HIPS-proper.  However, we do require that all
 * distributed copies of these source files include the following copyright
 * notice.
 *
 ******************************************************************************
 *
 * Copyright (c) 1995 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 *
 ******************************************************************************
 */

/*
 * hpub_xparam.c - HIPS public-domain header extended parameter management
 *
 * Michael Landy 11/1/95
 */

#include <hipspub.h>
void hpub_perr();
int hpub_checkname();

void hpub_initparams(xpar)

struct hpub_xparlist *xpar;

{
	xpar->numparam = 0;
	xpar->params = HP_NULLPAR;
}

void hpub_setparamb(xpar,name,value)

struct hpub_xparlist *xpar;
char *name;
unsigned char value;

{
	struct hpub_extpar *xp;

	xp = hpub_xpar_findalloc(xpar,name);
	xp->format = PFBYTE;
	xp->count = 1;
	xp->val.v_b = value;
}

void hpub_setparamc(xpar,name,value)

struct hpub_xparlist *xpar;
char *name;
unsigned char value;

{
	struct hpub_extpar *xp;

	xp = hpub_xpar_findalloc(xpar,name);
	xp->format = PFASCII;
	xp->count = 1;
	xp->val.v_b = value;
}

void hpub_setparams(xpar,name,value)

struct hpub_xparlist *xpar;
char *name;
short value;

{
	struct hpub_extpar *xp;

	xp = hpub_xpar_findalloc(xpar,name);
	xp->format = PFSHORT;
	xp->count = 1;
	xp->val.v_s = value;
}

void hpub_setparami(xpar,name,value)

struct hpub_xparlist *xpar;
char *name;
int value;

{
	struct hpub_extpar *xp;

	xp = hpub_xpar_findalloc(xpar,name);
	xp->format = PFINT;
	xp->count = 1;
	xp->val.v_i = value;
}

void hpub_setparamf(xpar,name,value)

struct hpub_xparlist *xpar;
char *name;
float value;

{
	struct hpub_extpar *xp;

	xp = hpub_xpar_findalloc(xpar,name);
	xp->format = PFFLOAT;
	xp->count = 1;
	xp->val.v_f = value;
}

void hpub_setparamb2(xpar,name,count,value)

struct hpub_xparlist *xpar;
char *name;
int count;
unsigned char *value;

{
	struct hpub_extpar *xp;

	xp = hpub_xpar_findalloc(xpar,name);
	xp->format = PFBYTE;
	xp->count = count;
	xp->val.v_pb = value;
}

void hpub_setparamc2(xpar,name,count,value)

struct hpub_xparlist *xpar;
char *name;
int count;
unsigned char *value;

{
	struct hpub_extpar *xp;

	xp = hpub_xpar_findalloc(xpar,name);
	xp->format = PFASCII;
	xp->count = count;
	xp->val.v_pb = value;
}

void hpub_setparams2(xpar,name,count,value)

struct hpub_xparlist *xpar;
char *name;
int count;
short *value;

{
	struct hpub_extpar *xp;

	xp = hpub_xpar_findalloc(xpar,name);
	xp->format = PFSHORT;
	xp->count = count;
	xp->val.v_ps = value;
}

void hpub_setparami2(xpar,name,count,value)

struct hpub_xparlist *xpar;
char *name;
int count,*value;

{
	struct hpub_extpar *xp;

	xp = hpub_xpar_findalloc(xpar,name);
	xp->format = PFINT;
	xp->count = count;
	xp->val.v_pi = value;
}

void hpub_setparamf2(xpar,name,count,value)

struct hpub_xparlist *xpar;
char *name;
int count;
float *value;

{
	struct hpub_extpar *xp;

	xp = hpub_xpar_findalloc(xpar,name);
	xp->format = PFFLOAT;
	xp->count = count;
	xp->val.v_pf = value;
}

struct hpub_extpar *hpub_xpar_findalloc(xpar,name)

struct hpub_xparlist *xpar;
char *name;

{
	struct hpub_extpar *xp;

	if (hpub_checkname(name))
		hpub_perr("invalid parameter name");
	if ((xp = hpub_findparam(xpar,name)) == HP_NULLPAR) {
		if ((xp = (struct hpub_extpar *)
			malloc(sizeof(struct hpub_extpar))) == HP_NULLPAR)
				hpub_perr("can't allocate parameter memory");
		xp->name = hpub_strsave(name);
		xp->nextp = xpar->params;
		xpar->params = xp;
		xpar->numparam++;
	}
	return(xp);
}

unsigned char hpub_getparamb(xpar,name)

struct hpub_xparlist *xpar;
char *name;

{
	struct hpub_extpar *xp;

	xp = hpub_checkfind(xpar,name);
	if (xp->count != 1 || xp->format != PFBYTE)
		hpub_perr("invalid parameter count or format");
	return(xp->val.v_b);
}

unsigned char hpub_getparamc(xpar,name)

struct hpub_xparlist *xpar;
char *name;

{
	struct hpub_extpar *xp;

	xp = hpub_checkfind(xpar,name);
	if (xp->count != 1 || xp->format != PFASCII)
		hpub_perr("invalid parameter count or format");
	return(xp->val.v_b);
}

short hpub_getparams(xpar,name)

struct hpub_xparlist *xpar;
char *name;

{
	struct hpub_extpar *xp;

	xp = hpub_checkfind(xpar,name);
	if (xp->count != 1 || xp->format != PFSHORT)
		hpub_perr("invalid parameter count or format");
	return(xp->val.v_s);
}

int hpub_getparami(xpar,name)

struct hpub_xparlist *xpar;
char *name;

{
	struct hpub_extpar *xp;

	xp = hpub_checkfind(xpar,name);
	if (xp->count != 1 || xp->format != PFINT)
		hpub_perr("invalid parameter count or format");
	return(xp->val.v_i);
}

float hpub_getparamf(xpar,name)

struct hpub_xparlist *xpar;
char *name;

{
	struct hpub_extpar *xp;

	xp = hpub_checkfind(xpar,name);
	if (xp->count != 1 || xp->format != PFFLOAT)
		hpub_perr("invalid parameter count or format");
	return(xp->val.v_f);
}

unsigned char *hpub_getparamb2(xpar,name,count)

struct hpub_xparlist *xpar;
char *name;
int *count;

{
	struct hpub_extpar *xp;

	xp = hpub_checkfind(xpar,name);
	if (xp->count == 1 || xp->format != PFBYTE)
		hpub_perr("invalid parameter count or format");
	*count = xp->count;
	return(xp->val.v_pb);
}
unsigned char *hpub_getparamc2(xpar,name,count)

struct hpub_xparlist *xpar;
char *name;
int *count;

{
	struct hpub_extpar *xp;

	xp = hpub_checkfind(xpar,name);
	if (xp->count == 1 || xp->format != PFASCII)
		hpub_perr("invalid parameter count or format");
	*count = xp->count;
	return(xp->val.v_pb);
}

short *hpub_getparams2(xpar,name,count)

struct hpub_xparlist *xpar;
char *name;
int *count;

{
	struct hpub_extpar *xp;

	xp = hpub_checkfind(xpar,name);
	if (xp->count == 1 || xp->format != PFSHORT)
		hpub_perr("invalid parameter count or format");
	*count = xp->count;
	return(xp->val.v_ps);
}

int *hpub_getparami2(xpar,name,count)

struct hpub_xparlist *xpar;
char *name;
int *count;

{
	struct hpub_extpar *xp;

	xp = hpub_checkfind(xpar,name);
	if (xp->count == 1 || xp->format != PFINT)
		hpub_perr("invalid parameter count or format");
	*count = xp->count;
	return(xp->val.v_pi);
}

float *hpub_getparamf2(xpar,name,count)

struct hpub_xparlist *xpar;
char *name;
int *count;

{
	struct hpub_extpar *xp;

	xp = hpub_checkfind(xpar,name);
	if (xp->count == 1 || xp->format != PFFLOAT)
		hpub_perr("invalid parameter count or format");
	*count = xp->count;
	return(xp->val.v_pf);
}

struct hpub_extpar *hpub_checkfind(xpar,name)

struct hpub_xparlist *xpar;
char *name;

{
	struct hpub_extpar *xp;

	if (hpub_checkname(name))
		hpub_perr("invalid parameter name");
	if ((xp = hpub_findparam(xpar,name)) == HP_NULLPAR)
		hpub_perr("parameter missing in header");
	return(xp);
}

int hpub_checkparam(xpar,name)

struct hpub_xparlist *xpar;
char *name;

{
	if (hpub_checkname(name))
		hpub_perr("invalid parameter name");
	return(hpub_findparam(xpar,name) != HP_NULLPAR ? 1 : 0);
}

void hpub_clearparam(xpar,name)

struct hpub_xparlist *xpar;
char *name;

{
	struct hpub_extpar *xp,*prev;

	if (hpub_checkname(name))
		hpub_perr("invalid parameter name");
	xp = xpar->params;
	prev = HP_NULLPAR;
	while (xp != HP_NULLPAR) {
		if (strcmp(name,xp->name) == 0)
			break;
		prev = xp;
		xp = xp->nextp;
	}
	if (xp == HP_NULLPAR)
		hpub_perr("missing parameter to clear");
	if (prev == HP_NULLPAR)
		xpar->params = xp->nextp;
	else
		prev->nextp = xp->nextp;
	xpar->numparam = xpar->numparam - 1;
}

struct hpub_extpar *hpub_findparam(xpar,name)

struct hpub_xparlist *xpar;
char *name;

{
	struct hpub_extpar *xp;

	xp = xpar->params;
	while (xp != HP_NULLPAR) {
		if (strcmp(name,xp->name) == 0)
			return(xp);
		xp = xp->nextp;
	}
	return(HP_NULLPAR);
}

int hpub_checkname(name)

char *name;

{
	char *p;

	p = name;
	while (*p != '\0') {
		if (*p == ' ' || *p == '\t')
			return(1);
		p++;
	}
	return(0);
}
