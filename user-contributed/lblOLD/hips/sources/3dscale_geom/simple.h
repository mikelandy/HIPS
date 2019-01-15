/* simple.h: definitions of some simple, common constants and macros */

#ifndef SIMPLE_HDR
#define SIMPLE_HDR

/* $Header: simple.h,v 1.6 89/04/26 11:32:51 ph Locked $ */

#include <stdio.h>

/* better than standard assert.h: doesn't gag on 'if (p) assert(q); else r;' */
#ifndef NDEBUG
#   define assert(p) if (!(p)) \
    { \
    fprintf(stderr, "Assertion failed: %s line %d: p\n", __FILE__, __LINE__); \
    exit(1); \
    } \
    else
# else
#   define assert(p)
#endif

#define str_eq(a, b)	(strcmp(a, b) == 0)

#ifndef MIN
#define MIN(a, b)	((a)<(b) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a, b)	((a)>(b) ? (a) : (b))
#endif
#ifndef ABS
#define ABS(a)		((a)>=0 ? (a) : -(a))
#endif

#define SWAP(a, b, t)	{t = a; a = b; b = t;}
#define LERP(t, a, b)	((a)+(t)*((b)-(a)))
#define ALLOC(ptr, type, n)  assert(ptr = (type *)malloc((n)*sizeof(type)))
#define ALLOC_ZERO(ptr, type, n)  assert(ptr = (type *)calloc(n, sizeof(type)))

#define PI 3.14159265358979323846264338
#define RAD_TO_DEG(x) ((x)*(180./PI))
#define DEG_TO_RAD(x) ((x)*(PI/180.))

/* note: the following are machine dependent! (ifdef them if possible) */
#define MINSHORT -32768
#define MINLONG -2147483648
#define MININT MINLONG
#ifndef MAXINT	/* sgi has these in values.h */
#   define MAXSHORT 32767
#   define MAXLONG 2147483647
#   define MAXINT MAXLONG
#endif


#ifdef hpux	/* hp's unix doesn't have bzero */
#   define bzero(a, n) memset(a, 0, n)
#endif

#endif
