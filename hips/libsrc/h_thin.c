/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_thin.c - thin a white-on-black image
 *
 * This program thins white-on-black images in two ways, and then categorizes
 * the points in the image.  The algorithms are derived from those of
 * Sakai, et. al. (Comp. Graph. and Image Proc. 1, 1972, 81-96).  The program
 * operates in several passes, any combination of which can be chosen with
 * the passflag argument:
 *
 *	Pass
 *	1) Thin the image by deleting points with 3 to 5
 *	   8-neighbors and 2 transitions.  This pass is repeated
 *	   until no further deletions occur unless the -s 
 *	   (single-pass) switch is given.
 *	2) Thin the image further, so that diagonal lines are
 *	   at most 1 pixel wide, but 8 connectivity is
 *	   preserved.  Delete pixels which have 2-6 8-neighbors
 *	   and exactly one 8-connected gap in the ring its
 *	   8-neighbors.
 *	3) Categorize pixels as Endpoints, Multiple branch
 *	   points, Isolated points, or Uninteresting points.
 *	   Multiple branch points are categorized as M's if
 *	   6 or more transitions are found, otherwise as MM.
 *	4) Multiple 8-neighbor MM point groups have an M point
 *	   replace the MM closest to the center of the group.
 *
 * vflag (verbose) prints the number of deletions in pass 1, etc.  sflag
 * keeps the first two passes from being repeated if changes were made.
 *
 * Unlike most subroutines, h_thin operates on the input image itself.
 *
 * pixel formats: BYTE
 *
 * Michael Landy - 10/22/82
 * HIPS 2 - msl - 8/4/91
 */

#include <hipl_format.h>

#define U	010
#define E	020
#define I	040
#define MM	0100
#define M	0200
#define	NMM	1000
#define absval(f) (((f)<0)?(-(f)):(f))

static int rinc[10] = {0,0,-1,-1,-1,0,1,1,1,0};
static int cinc[10] = {0,1,1,0,-1,-1,-1,0,1,1};
static int mrow[NMM],mcol[NMM],mlabel[NMM],maxmult,maxlabel;
static int row,col,ocol;
static byte *pic;
int addmult(),neighbor();

int h_thin(hd,passflag,sflag,vflag,f)

struct header *hd;
h_boolean passflag[4],sflag,vflag;
int f;

{
	switch(hd->pixel_format) {
	case PFBYTE:	return(h_thin_b(hd,passflag,sflag,vflag,f));
	default:	return(perr(HE_FMTSUBR,"h_thin",
				hformatname(hd->pixel_format)));
	}
}

int h_thin_b(hd,passflag,sflag,vflag,f)

struct header *hd;
h_boolean passflag[4],sflag,vflag;
int f;

{
	return(h_thin_B(hd->firstpix,hd->rows,hd->cols,hd->ocols,passflag,
		sflag,vflag,f));
}

int h_thin_B(image,nr,nc,nlp,passflag,sflag,vflag,f)

byte *image;
int nr,nc,nlp,f;
h_boolean passflag[4],sflag,vflag;

{
	int nex;
	int change,trans,neigh,pass,first,gap,gapopen;
	int i,j,r,c;
	int mini,sumr,sumc,label,npt,Mfound;
	float avgr,avgc,mindist,dist;
	byte *ppic;
	char msg[100];

	nex = nlp-nc;
	row = nr;
	col = nc;
	ocol = nlp;
	pic = image;
    for (pass=0;pass<4;pass++)
	if (passflag[pass]) {

	    if (vflag) {
		sprintf(msg,"h_thin: frame %d pass %d",f,pass);
		perr(HE_IMSG,msg);
	    }
	    do {
		maxlabel = maxmult = change = 0;

		ppic = pic;

		/*  neighborhood:  432
				   501
				   678
		and 9 = 1 for modular arithmetic */

	    for (r=0;r<row;r++) {
		for (c=0;c<col;c++) {
			if (*ppic) {
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
				if (*ppic==MM || *ppic==M) {
				    ppic++;
				    first = 0;
				    for (i=1;i<=8;i++) {
					if (neighbor(r,c,i)==MM ||
					  neighbor(r,c,i)==M) {
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
		ppic += nex;
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
			    if ((*(pic + mrow[i]*nlp+mcol[i])) == M)
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
				*(pic + mrow[i]*nlp + mcol[i]) = M;
				change++;
			    }
			    mlabel[i] = 0;
			}
		    }
		}
	    }

	    if (vflag && pass<2) {
		sprintf(msg,"h_thin: frame %d reversals=%d",f,change);
		perr(HE_IMSG,msg);
	    }
	    if (vflag && pass==3) {
		sprintf(msg,"h_thin: frame %d changes=%d",f,change);
		perr(HE_IMSG,msg);
	    }

	    } while (change!=0 && !sflag && (pass==0 || pass==1));
	}
	return(HIPS_OK);
}

int addmult(row,col,label)

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
	return(HIPS_OK);
}

int neighbor(r,c,d)

int r,c,d;

{
	int i,j;

	i = r + rinc[d];
	j = c + cinc[d];
	if (i<0 || i>=row || j<0 || j>=col)
		return(HIPS_OK);
	else
		return((int) pic[i*ocol+j]);
}
