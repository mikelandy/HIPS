/* SccsId = "%W%        %G%" */

#define hsh(A,B)	(  A  + 256 *  B  )

#define SHELL		hsh('!','\0')
#define	ATT_ORDER_SWOP	hsh('a','o')
#define	ATT_PRINT	hsh('a','p')
#define ATT_SUPRESS	hsh('a','s')
#define ATT_RELEASE	hsh('a','u')
#define	BOOLEAN_EGS	hsh('b','o')
#define BUILD_RULE	hsh('b','u')
#define	SET_CUTOFF	hsh('c','u')
#define CHECK_EGS	hsh('c','h')
#define	CRSPICS_CLASSIF	hsh('c','l')
#define CONCEPT		hsh('c','o')
#define DELETE_PRIM	hsh('d','p')
#define DELETE_SEC	hsh('d','s')
#define	DELETE_EGS	hsh('d','e')
#define DELETE_CLASS	hsh('d','c')
#define	EXPAND_EGS	hsh('e','x')
#define	GENERATE_CODE	hsh('g','e')
#define HELP		hsh('h','\0')
#define INDUCE		hsh('i','n')
#define INDUCE_AUTOTERM hsh('i','a')
#define RANGE_INDUCE	hsh('i','r')
#define	LIVECAMERA	hsh('l','\0')
#define MOVE_EGS	hsh('m','o')
#define PRINT_EGS	hsh('p','r')
#define PRINT_NODES	hsh('p','n')
#define	PRINT_COVERS	hsh('p','s')
#define	QUIT		hsh('q','\0')
#define READ_COVERS	hsh('r','c')
#define	READ_USING_RULE	hsh('r','e')
#define	RELABEL_RULE	hsh('r','l')
#define READ_PRIM	hsh('r','p')
#define READ_RULE	hsh('r','r')
#define READ_SEC	hsh('r','s')
#define	READ_TEL_WHERE	hsh('r','t')
#define	PRINT_RULES	hsh('r','u')
#define	SET_DEBUGFLAGS	hsh('s','d')
#define SUMMARISE_SEC	hsh('s','s')
#define SWITCH_OUTPUT	hsh('s','w')
#define USER_EG_INPUT	hsh('u','\0')
#define CLASSIFY_SEC	hsh('x','p')
#define CONF_MATRIX	hsh('x','s')
#define LEAFIFY_SEC	hsh('X','p')
#define LEAFCONF_MATRIX	hsh('X','s')

/* BINEYE commands only */

#define TRAIN		hsh('t','r')
#define EXPORT		hsh('e','p')

