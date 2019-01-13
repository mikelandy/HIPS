
#define pix(a,x,y,xdim)	(*((unsigned char *) (a + (x+(y)*(xdim)))))
#define	NOT_DONE	99999
#define PI		3.141593

/* most satts routines can optionally give debug text send debug
 * info to the frame store
 */
#define DEBUG	1
#define NODEBUG 0

#define MAXBORDLEN	2000
#define MAXARCS		200
#define MAXCORNERS	200

struct blobstruct {

	/* where the pic data is to be found! */
	unsigned char *pic ;
	unsigned char *bo_pic ;
	unsigned char *holepic;
	unsigned char *workpic;
	int xd,yd ;
	int xst,yst;
	float xc,yc ;

	/* attribute values */
	int area;
	float rawarea ;
	double majdir,majvar,minvar;
	int maxrad,minrad,shrwid ;
	int maxdir, mindir ;
	int symaxlen, syminlen ;
	float pa,avdist,avper,avrad,bparea;
	float maxper,minper;
	float palen,pawid;
	float maxlen,perplen;
	char class[20] ;
	int holecnt ;

	/* blobs are maintained in a linked list */
	struct blobstruct *next ;

	/* the holes within this blob are also kept in a linked list
	 * holelist is the pointer to the list, individual holes use
	 * the 'next' field (see above) to link themselves.
	 * A hole would only use 'holelist' if it also had holes
	 * (ie blobs within it)
	 */
	struct blobstruct *holelist ;
};

/* this holds all info computed by the border segmentation routine */
struct arcdata {
	char type[20] ;		/* "line" or "curve" at present */
	int len ;		/* number of pixels in this arc */
	float curvat ;		/* measure of its curature ( 0 for a line) */
	int x1,y1,x2,y2 ;	/* end points of arc (pixel co-ords)*/
	float ang ;		/* the K value (in degrees) for 2nd endpnt */
} ;

/* this contains everything that can possibly tweaked in the
 * segmentation routine */
struct seg_params {
	int K ;			/* the reqd K curvature */
	int smooth ;		/* a flag (1/0) to pass th kcurvats */
	int minseglen ;		/* minimum length of a curve */
	float bendK ;		/* minimum K for a break in a curve */
	float cornerK ;		/* minimum K before a corner is detected */
	float linefit ;		/* percentage of shortest length (pythag)
				 * before a curve is considered */
	float meansq ;		/* percentage of average sqd dist 
				 * to proposed line
				 * before a curve is considered */
} ;

