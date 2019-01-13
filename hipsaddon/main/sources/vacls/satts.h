/* SccsId = "%W%        %G%" */


#define pix(a,x,y,xdim)	(*((unsigned char *) (a + (x+(y)*(xdim)))))
#define	NOT_DONE	99999

struct blobstruct {

	/* where the pic data is to be found! */
	unsigned char *pic ;
	unsigned char *bo_pic ;
	int xd,yd ;
	int xst,yst;
	float xc,yc ;

	/* attribute values */
	int area;
	float rawarea ;
	double majdir,majvar,minvar;
	int maxrad,minrad,shrwid ;
	float pa,avdist,avper,avrad,bparea;
	float maxper,minper;
	float palen,pawid;
	float maxlen,perplen;
	char class[20] ;

	/* blobs are maintained in a linked list */
	struct blobstruct *next ;
	};

