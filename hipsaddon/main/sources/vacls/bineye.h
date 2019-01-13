/* SccsId = "%W%        %G%" */

extern	unsigned char *pic ;

struct wind {
	struct	wind	*next ;
	char	name[20] ;
	int	camnum ;
	int	cxd,cyd,cxo,cyo;
	int	pxd,pyd ;
	int	thresh ;
	int	numexp,numshk,edgerm ;
	float	pixlen ;
	int	reduction ;
	char	rulename[20];
	struct	node	*rule ;
	struct  blobstruct *bloblist;
	unsigned char *pic ;
};

struct ruleptr {
	struct	ruleptr	*next ;
	char	name[20] ;
	int	num_nodes ;
	struct	node	*rule ;
} ;

