/*				See below for general comments ? */
/* standard C libraries */
#include <stdio.h>
#include <hipl_format.h>

/* Program modified by Debby Trytten on June 10, 1989

   Removed reading in and manipulating of hips header, the input of 
interactive options, and the output of results.
   Reading in and manipulating the header and writing out results are 
done from canny, which is the routine that calls this.
   The options that are used are -t -d in this version.  If verbose output 
is desired the variable flagv can be set to 1.  This is handy for debugging.

						Joe Miller May 23, 1989

	P.S.  Note:  This routine is just called as
			thin(*edgemap,rows,cols)
		from the canny program (my stuff).  

 * old program usage (before Deb changed this to  use -t and -d as
 * described in her comment above).
 * usage:	thin [-t] [-d] [-c] [-m] [-a] [-s] [-v] <in >out
 *
 * This program thins white-on-black images in two ways, and then categorizes
 * the points in the image.  The algorithms are derived from those of
 * Sakai, et. al. (Comp. Graph. and Image Proc. 1, 1972, 81-96).  The program
 * operates in several passes, any combination of which can be chosen with
 * switches:
 *
 *	Switch		Pass
 *	  -t		1) Thin the image by deleting points with 3 to 5
 *			   8-neighbors and 2 transitions.  This pass is repeated
 *			   until no further deletions occur unless the -s 
 *			   (single-pass) switch is given.
 *	  -d		2) Thin the image further, so that diagonal lines are
 *			   at most 1 pixel wide, but 8 connectivity is
 *			   preserved.  Delete pixels which have 2-6 8-neighbors
 *			   and exactly one 8-connected gap in the ring its
 *			   8-neighbors.
 *	  -c		3) Categorize pixels as Endpoints, Multiple branch
 *			   points, Isolated points, or Uninteresting points.
 *			   Multiple branch points are categorized as M's if
 *			   6 or more transitions are found, otherwise as MM.
 *	  -m		4) Multiple 8-neighbor MM point groups have an M point
 *			   replace the MM closest to the center of the group.
 *
 * The -a switch implies all four passes. The -v switch (verbose) prints the
 * number of deletions in pass 1, etc.  The -s switch keeps the first two passes
 * from being repeated if changes were made.  If no switches are given, the
 * default is -t -d.
 *
*/

#define U	010
#define E	020
#define I	040
#define MM	0100
#define M	0200
#define	NMM	200
#define absval(f) (((f)<0)?(-(f)):(f))

int flagpass[4] = {0,0,0,0};
int flags = 0;
int flagv = 0;
int rinc[10] = {0,0,-1,-1,-1,0,1,1,1,0};
int cinc[10] = {0,1,1,0,-1,-1,-1,0,1,1};
int mrow[NMM],mcol[NMM],mlabel[NMM],maxmult,maxlabel;
int row,col;
char *pic;
int neighbor();
void addmult();

void thin(edges,height,width)
int height,width;
unsigned char *edges;

{int frameno,r,c,change,trans,neigh,pass,first,gap,gapopen;
int i,j;
int mini,sumr,sumc,label,npt,Mfound;
float avgr,avgc,mindist,dist;
char *ppic;
char *temp;
unsigned char *temp2;

flagpass[0]=1;
flagpass[1]=1;
row = height;
col = width;
pic = (char * ) halloc(row*col,sizeof (char));

flagv=0;   /* can be set to 1 for debugging */
frameno=0;

temp2 = edges;
temp=pic;
for (i=0; i<height * width; i++)
  {*temp = (char)*temp2;
   temp++;
   temp2++;
 };

    for (pass=0;pass<4;pass++)
	if (flagpass[pass]) {
	    
	    if (flagv)
		fprintf(stderr,"thin: frame %d pass %d\n",frameno,pass);
	    do {
		maxlabel = maxmult = change = 0;

		ppic = pic;

		/*  neighborhood:  432
				   501
			   678
	    	and 9 = 1 for modular arithmetic */

	    for (r=0;r<row;r++) {
			for (c=0;c<col;c++) {
			if ((*ppic&0377)!=0) {
			    switch(pass) {

			    case 0:
			    case 2:
				neigh = trans = 0;
				for (i=1;i<=8;i++) {
				  if (neighbor(r,c,i)!=0)
						neigh++;
					if (((neighbor(r,c,i)==0) ^
					    (neighbor(r,c,i+1)==0))!=0)
						trans++;
				}
				if (pass==0) {
				    if (trans==2 && neigh>2 && neigh<6) {
					*ppic++ = 0;
					change++;
				    }
				    else
					ppic++;
				}
				else {   /* pass 2 */
				    if (trans == 2 && neigh < 3)
					*ppic++ = E;
				    else if (neigh == 0)
					*ppic++ = I;
				    else if (trans >= 6)
					*ppic++ = M;
				    else if (neigh > 2)
					*ppic++ = MM;
				
				    else
					*ppic++ = U;
				  }
				continue;

			    case 1:
				neigh = trans = 0;
				for (i=1;i<=8;i++) {
					if (neighbor(r,c,i)) {
						neigh++;
						first = i;
					}
					if (((neighbor(r,c,i)==0) ^
					    (neighbor(r,c,i+1)==0))!=0)
						trans++;
				}
				if (neigh<2 || neigh>6 || 
				    (neigh==2 && trans<4) ||
				    (neigh==3 && trans>4)) {
					ppic++;
					continue;
				}
				gap = gapopen = 0;
				for (i=first+1;i<first+8;i++) {
				    j = ((i - 1) % 8) + 1;
				    if (gapopen) {
					if (neighbor(r,c,j))
						gapopen = 0;
				    }
				    else {
					if (j%2==1 && neighbor(r,c,j)==0) {
						gapopen++;
						gap++;
					}
				    }
				}
				if (gap == 1) {
					*ppic++ = 0;
					change++;
				}
				else
				  ppic++;
				continue;

			    case 3:
				if ((*ppic&0377)==MM || (*ppic&0377)==M) {
				    ppic++;
				    first = 0;
				    for (i=1;i<=8;i++) {
					if (neighbor(r,c,i)==MM || neighbor(r,c,i)==M) {
					    if (first++ == 0)
						addmult(r,c,++maxlabel);
					    addmult(r+rinc[i],
						c+cinc[i],maxlabel);
					}
				    }
				}
				else
					ppic++;
				continue;

			    default:
				perr(HE_MSG,"unknown pass!");
			    }
			}
			else
				ppic++;
	        }
	    }

	    if (pass == 3) {
		while (1) {
		    label = 0;
		    for (i=0;i<maxmult;i++)
			if ((label = mlabel[i]) != 0)
			    break;
		    if (label == 0)
			break;
		    Mfound = sumr = sumc = npt = 0;
		    for (i=0;i<maxmult;i++)
			if (mlabel[i] == label) {
			    sumr += mrow[i];
			    sumc += mcol[i];
			    if ((*(pic + mrow[i]*col+mcol[i])&0377) == M)
				Mfound++;
			    npt++;
			}
		    if (Mfound) {
			for (i=0;i<maxmult;i++) {
				if (mlabel[i] == label)
					mlabel[i] = 0;
			}
			continue;
		    }
		    if (npt < 2)
			perr(HE_MSG,"only one M point!??");
		    avgr = ((float) sumr)/npt;
		    avgc = ((float) sumc)/npt;
		    mindist = 10000000.;
		    for (i=0;i<maxmult;i++) {
			if (mlabel[i] == label) {
			    dist = absval(avgr-mrow[i]) +
				absval(avgc-mcol[i]);
				    if (dist<mindist) {
				mindist = dist;
				mini = i;
			    }
			}
		    }
		    for (i=0;i<maxmult;i++) {
			if (mlabel[i]==label) {
			    if (i == mini) {
				*(pic + mrow[i]*col + mcol[i]) = M;
				change++;
			    }
			    mlabel[i] = 0;
			}
		    }
		}
	    }
		if (flagv) fprintf(stderr,"thin: frame %d reversals = %d\n",frameno,change);
	    } while (change!=0 && flags==0 && (pass==0 || pass==1));
	}
temp2 = edges;
for (i=0; i<height * width; i++)
  {*temp2 = (unsigned) *pic;
   pic++;
   temp2++;
 };
}
/********************************************************************************************/
void addmult(row,col,label)

int row,col,label;

{
	int i,oldlabel;

	oldlabel = 0;
	for (i=0;i<maxmult;i++)
		if (mrow[i]==row && mcol[i]==col)
			oldlabel = mlabel[i];
	if (oldlabel) {
		for (i=0;i<maxmult;i++)
			if (mlabel[i] == oldlabel)
				mlabel[i] = label;
	}
	else {
		if (maxmult >= NMM)
			perr(HE_MSG,"M point overflow");
		mrow[maxmult] = row;
		mcol[maxmult] = col;
		mlabel[maxmult++] = label;
	}
}

/**************************************************************************************/

int neighbor(r,c,d)

int r,c,d;

{
	int i,j;

	i = r + rinc[d];
	j = c + cinc[d];
	if (i<0 || i>=row || j<0 || j>=col)
		return(0);
	else
		return(pic[i*col+j]&0377);
}

