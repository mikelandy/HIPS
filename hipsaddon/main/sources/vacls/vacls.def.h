/* SccsId = "%W%        %G%" */

/*  header for acls      */

#define getclass(I)   *(exlist[I])		/* get class for I'th examp */
#define getattr(I,J)  *(exlist[I] + J)		/* attr token, I'th ex, att J*/

#define getstrg(A)    blockstart + offset[A]
#define getaname(I)   blockstart + offset[att[I].name]
#define classname(I)  blockstart + offset[firstclass + I ]
#define min(A,B)      ((A) < (B) ? (A) : (B))
#define absval(A)     ((A) > (0) ? (A) : -(A) )
#define log2(A)	      (1.442695*log(A))

#define MAXNAMES     500    	/* max number names storable with calloc */
#define MAXCLASS      40+2	/* max number of classes		 */
#define MAXATTR      120	/* max number of attributes 		 */
#define MAXVALS       50
#define MAXEX        20000	/* max number of prim and sec  examples  */
#define MAXLEVELS     20	/* max number of levels in tree		 */
#define MAXLIN       500	/* characters in input line		 */
#define EPSILON      0.0001

#define INTEGER       -5
#define LOGICAL       -4
#define ERROR	      -3
#define OKAY          -2
#define YES	       1
#define NO 	       0
#define DONTCARE  -32767	/* wild value for use with INTEGER type */
#define	TOFEWATTS	-120
#define	AVAILATTS	-121
#define ALLATTS		-122

#define NEW	     -10
#define SUBSET       -11
#define SUPERSET     -12
#define SAME         -13
#define CLASH	     -14

#define PRIM         -15
#define SEC          -16

#define AGREE        -17
#define CONTRADICT   -18

#define DSEC	-111
#define CPRIM   -112
#define	DCCHNG	-113
