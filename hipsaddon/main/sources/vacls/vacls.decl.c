static char *SccsId = "%W%      %G%";

#include <stdio.h>
#include "vacls.def.h"

/* debug flag and output stream */
int debug[10];
char bugtext[6];
FILE *outstream;

/* attribute and class storage */

	int numattr;		/* number attributes to be read in	 */

	struct attrib {
		int name;	/* offset index for name string		 */
		int type;	/* attrib type ('logical' or 'integer')  */
		int nval; 	/* number of attribute values		 */
		int firstval;	/* offset index for first value string   */
		int dontuse ;	/* true/false flag: used in att_avail()  */
				/* stops atts occuring in trees	(B.A.S)	 */
		int order ;	/* allows a ranking to be placed on atts */
				/* to be used in the event of equal costs*/
				/* initial ordering is that of the attfil*/
				/* as is the case with normal ACLS (B.A.S)*/
		} att[MAXATTR] ;

	int nextindex = 0 ;	/* next offset index to be used		 */

	unsigned blocklen = 0;  /* length of char storage allocated	 */
	char *blockstart;	/* pointer to start of calloc storage	 */
	int offset[MAXNAMES];	/* array of offsets for calloc strings   */

	int numclass;		/* number of classes to be read in	 */
	int firstclass;  	/* index to offset for classes		 */
	int classtart ;		/* index to offset for real classes      */		


/* malloc storage for examples  */
	
	int numprim = 0;	/* number examp in primary store	 */
	int numsec = 0;		/* number examples in secondary store    */

	int *exlist[MAXEX];	/* pointers to  examples          	 */

/* node storage								 */

	struct node {

		int splitatt;	/* attribute on which node is split	 */
		int splitval;	/* split value (INTEGER type only)	 */
		int count[MAXCLASS+1] ; /* to store the freq of classes  */
					/* arriving at this node (B.A.S) */
		int numegs ;	/* B.A.S */
		float	avent ;	/* av ent of split subsets */
		float	pent ;	/* entropy in unsplit egset */
		float	chi ;	/* chi-sq val for unsplit egset 	*/
		struct node *firstson;	/*ptr to first son               */
		struct node *brother;	/* ptr to next brother of node   */ 
		int nexamp;	/* number examples assigned to this node */
		int leafnum;	/* so that classifications can say which */
				/* leaf they came from (B.A.S., 17/6/86) */
		};
	int numnodes = 0;	/* current number of nodes in tree	*/
	struct node *root;	/* pointer to the root node		*/
	int nodesize = sizeof(struct node);	/* node size in bytes	*/

	int curlev = 0;	/* current level in tree traversal	*/
	int usedatt[MAXLEVELS];		/* list of used logical attributes */

	struct node *nodptr[MAXEX];	/* ptrs to all nodes for printing*/
					/* not needed after debug phase */
