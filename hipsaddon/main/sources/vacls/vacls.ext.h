/* SccsId = "%W%        %G%" */

/* debug flag and output stream */
#include "vacls.def.h"

extern int debug[] ;
extern char *bugtext;
extern FILE *outstream;

/* attribute and class storage */

	extern int numattr;	/* number attributes to be read in	 */

	struct attrib {
		int name;	/* offset index for name string		 */
		int type;	/* attrib type ('logical' or 'integer')  */
		int nval; 	/* number of attribute values		 */
		int firstval;	/* offset index for first value string   */
		int dontuse ;	/* true/false flag: used in att_avail()  */
		int order ;	/* ranking to be used when equal costs	 */
		}  ;
	extern struct attrib att[];

	extern int nextindex ;	/* next offset index to be used		 */

	extern unsigned blocklen;  /* length of char storage allocated	 */
	extern char *blockstart;  /* pointer to start of calloc storage	 */
	extern int offset[];	/* array of offsets for calloc strings   */

	extern int numclass;	/* number of classes to be read in	 */
	extern int firstclass;  /* index to offset for classes		 */
	extern int classtart;	/* start ndex of real classes */

/* malloc storage for examples  */
	
	extern int numprim ;	/* number examp in primary store	 */
	extern int numsec ;	/* number examples in secondary store    */

	extern int *exlist[];	/* pointers to  examples          	 */

/* node storage								 */

	struct node {

		int splitatt;	/* attribute on which node is split	 */
		int splitval;	/* split value (INTEGER type only)	 */
		int count[MAXCLASS] ;
		int numegs ;
		float avent ;
		float pent ;
		float chi ;
		struct node *firstson;	/*ptr to first son               */
		struct node *brother;	/* ptr to next brother of node   */ 
		int nexamp;	/* number examples assigned to this node */
		int leafnum;	/* so classifications can say whick leaf */
				/* thay came from. (BAS 17/6/86)         */
		};
	extern int numnodes ;	/* current number of nodes in tree	*/
	extern struct node *root; /* pointer to the root node		*/
	extern int nodesize ;	/* node size in bytes	*/

	extern int curlev ;	/* current level in tree traversal	*/
	extern int usedatt[];	/* list of used logical attributes */

	extern struct node *nodptr[];	/* ptrs to all nodes for printing*/
					/* not needed after debug phase */
