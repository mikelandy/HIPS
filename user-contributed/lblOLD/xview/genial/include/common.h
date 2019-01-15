/*
 * common.h -- for use in files that use arch/base/common.c
 *
 */

#include <string.h>
#include <math.h>
#include <malloc.h>

#define BSIZE 1024 /* number of points to realloc() at a time */
#define paddr(i,j,img) img->data+j*img->width+i

extern double distance(), idist();

extern unsigned long dval(), getpix();

extern char *fstring();

extern unsigned char readhex();

extern char *sys_errlist[]; /* system error list */
extern int errno;
