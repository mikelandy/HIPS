/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * vectgen.c - track a thinned and categorized image and simulate vector coding
 *
 * usage:	vectgen [-p] [-d] [-x] [-c short [long [break]]] <iseq >oseq
 *
 * -p causes the graph to be printed for each frame.  -d is a debugging switch
 * which prints the graph on errors. -x yields a byte pixel formatted image as
 * output, so that cut points can be observed, otherwise a spanning tree is 
 * output. -c allows the user to change the minmax distance criterion. The short
 * threshold is used below the break point, and the long criterion is used above
 * it.  The defaults are short = 1.5, long = 1.5, break = 12.
 *
 * to load:	cc -o vectgen vectgen.c -lhips -lm
 *
 * Mike Landy - 11/22/82
 * HIPS 2 - msl - 7/24/91
 */

#include <stdio.h>
#include <hipl_format.h>

#define	NULLNODE	((struct node *) 0)
#define	NULLFOLL	((struct follower *) 0)

#define	CUT	04
#define U	010
#define E	020
#define I	040
#define MM	0100
#define M	0200

#define	PCUT	0140

#define	picture(x,y)	(pict[((y)*c)+(x)])
#define	inversedir(d)	((((d)+3)%8)+1)

int	fr,r,c;
int	numM,numE,numMM,numU,numC,numnode,numfoll;
int	countM,countE,countMM,countU,countC;
byte	*pict;

struct	node {
	short	x,y,type,mark;
	struct follower *flist;
	struct node	*nextnode;
}	*ofreenode,*freenode,**Mnode,**Enode,**Unode,**MMnode,
	*getnode(),*findnode(),**ndq;

struct	follower {
	short dir,mark;
	struct node *goal,*nextnode;
	struct follower *nextfoll;
}	*ofreefoll,*freefoll,*getfoll();

int	xdir[10] = {0,1,1,0,-1,-1,-1,0,1,1};
int	ydir[10] = {0,0,1,1,1,0,-1,-1,-1,0};
h_boolean inprintg=FALSE,dflag,pflag,xflag;
double	shortc,longc;
int	breakc;

static Flag_Format flagfmt[] = {
	{"c",{LASTFLAG},1,{{PTDOUBLE,"1.5","short"},{PTDOUBLE,"1.5","long"},
		{PTINT,"12","break"},LASTPARAMETER}},
	{"p",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"d",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"x",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	LASTFLAG};
int types[] = {PFBYTE,LASTTYPE};
void perr2(),clearmarks(),resetcore(),printgraph(),printnode(),printnodel();
void fillgoal(),outspan(),outtree();
int neighbor(),divseg();
float distsq();

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd,hdp,hdo;
	Filename filename;
	FILE *fp;
	int method,row,col,i,j,k,f,d,ii,l,first,m,firstd,lastd,cntUs;
	byte *p;
	struct node **np,*node,*node2,*currpt,*nb;
	struct follower *foll,*foll2,*fl1,*fl2;
	h_boolean Uflg;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&shortc,&longc,&breakc,&pflag,&dflag,
		&xflag,FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	method = fset_conversion(&hd,&hdp,types,filename);
	shortc *= shortc;
	longc *= longc;
	dup_headern(&hdp,&hdo);
	if (!xflag)
		setformat(&hdo,PFSPAN);
	r = hd.orows; c = hd.ocols;
	fr = hdp.num_frame;
	if (!pflag)
		write_headeru(&hdo,argc,argv);
	pict = hdp.image;
	for (f=0;f<fr;f++) {
		fprintf(stderr,"%s: starting frame %d\n",Progname,f);
		fread_imagec(fp,&hd,&hdp,method,f,filename);

		/* compute percent of white pixels, allocate nodes and followers
			and link up free lists */

		if (f==0) {
			p = pict;
			for (i=0,j=0;i<r*c;i++)
				if (*p++)
					j++;
			numU = 11*j/10+5;
			numE = 4*j/10+5;
			numM = j/5+5;
			numMM = j/4+5;
			numC = (numU + numMM)/4;
			numnode = numU + numE + numM + numMM;
			numfoll = 2*numnode;
			ofreenode = freenode = (struct node *) 
				halloc(numnode,sizeof(struct node));
			ofreefoll = freefoll = (struct follower *) 
				halloc(numfoll,sizeof(struct follower));
			Enode = (struct node **) 
				halloc(numE,sizeof(struct node **));
			Unode = (struct node **) 
				halloc(numU,sizeof(struct node **));
			Mnode = (struct node **) 
				halloc(numM,sizeof(struct node **));
			MMnode = (struct node **) 
				halloc(numMM,sizeof(struct node **));
			ndq = (struct node **) 
				halloc(numE+numM+numC,sizeof(struct node **));
			for (i=0,node2=node=freenode;i<numnode-1;i++)
				(node++)->nextnode = ++node2;
			for (i=0,foll2=foll=freefoll;i<numfoll-1;i++)
				(foll++)->nextfoll = ++foll2;
		}

		/* set up node category arrays */

		for (countM=0,countMM=0,countE=0,countU=0,row=0,p=pict;row<r;
		    row++) {
			for (col=0;col<c;col++) {
				switch(*p++) {

				case 0:
					continue;
				case E:
				case I:
					if (++countE>numE)
						perr2("E overflow");
					node = getnode();
					Enode[countE-1] = node;
					node->x = col;
					node->y = row;
					node->type = E;
					continue;
				case M:
					if (++countM>numM)
						perr2("M overflow");
					node = getnode();
					Mnode[countM-1] = node;
					node->x = col;
					node->y = row;
					node->type = M;
					continue;
				case MM:
					if (++countMM>numMM)
						perr2("MM overflow");
					node = getnode();
					MMnode[countMM-1] = node;
					node->x = col;
					node->y = row;
					node->type = MM;
					continue;
				case U:
					if (++countU>numU)
						perr2("U overflow");
					node = getnode();
					Unode[countU-1] = node;
					node->x = col;
					node->y = row;
					node->type = U;
					continue;
				default:
					perr2("unknown pixel category");
				}
			}
		}

		/* track from each Endpoint nodes, linking nodes with
			followers, marking nodes as they are passed, and
			stopping when an Endpoint or branch point (M or MM)
			is reached, with a preference for M points as the
			next follower */

		for (ii=0,np=Enode;ii<countE;ii++,np++) {
			if ((*np)->mark)
				continue;
			(*np)->mark++;
			if (picture((*np)->x,(*np)->y) == I)
				continue;
			currpt = *np;
			while(1) {
			  for (k=0;k<2;k++) {	/* M points in first pass */
			    for (d=1;d<=8;d++) {
				if ((l = neighbor(currpt->x,currpt->y,d))
				    && (k > 0 || l == M)) {
					nb = findnode(currpt->x,currpt->y,d);
					i = nb->type;
					if (nb->mark && (i==U || i==E))
						continue;
					nb->mark++;
					fl1 = getfoll();
					fl2 = getfoll();
					fl1->nextfoll = currpt->flist;
					currpt->flist = fl1;
					fl1->dir = d;
					fl1->nextnode = nb;
					fl2->nextfoll = nb->flist;
					nb->flist = fl2;
					fl2->dir = inversedir(d);
					fl2->nextnode = currpt;
					if (nb->type != U)
						goto Unextnode;
					currpt = nb;
					goto Unextf;
				}
			    }
			  }
			  perr2("dangling U");
			  Unextf:;
			}
			Unextnode:;
		}

		/* track in two directions from each as yet unmarked U node,
			linking with followers and stopping as branch points
			(M or MM) are reached, with a preference for M points
			as the next follower. If return to where we started,
			make this U an M, since have a simple closed contour. */
					
		for (ii=0,np=Unode;ii<countU;ii++,np++) {
			if ((*np)->mark)
				continue;
			(*np)->mark++;
			cntUs = 1;
			for (i=1;i<=2;i++) {
			    currpt = *np;
			    first = 1;
			    while(1) {
			      for (k=0;k<2;k++) {	/* M points first */
				for (d=1;d<=8;d++) {
				    if ((l = neighbor(currpt->x,currpt->y,d))
					&& (k > 0 || l == M)) {
					if (i==2 && first &&
						((m=abs(d-firstd))<2 || m==7))
						continue;
					nb = findnode(currpt->x,currpt->y,d);
					Uflg = FALSE;
					if (nb->mark && nb->type==U) {
						if (i==1 && cntUs>2 && nb==*np)
							Uflg = TRUE;
						else
							continue;
					}
					if (!Uflg)
						nb->mark++;
					cntUs++;
					fl1 = getfoll();
					fl2 = getfoll();
					fl1->nextfoll = currpt->flist;
					currpt->flist = fl1;
					fl1->dir = d;
					fl1->nextnode = nb;
					fl2->nextfoll = nb->flist;
					nb->flist = fl2;
					fl2->dir = inversedir(d);
					fl2->nextnode = currpt;
					if (Uflg) {
						pict[((*np)->y)*c+(*np)->x] = M;
						if (++countM>numM)
							perr2("M overflow");
						Mnode[countM-1] = *np;
						(*np)->type = M;
						Unode[ii] = NULLNODE;
						goto uUnextii;
					}
					if (i==1 && first)
						firstd = d;
					first = 0;
					if (nb->type != U)
						goto uUnexti;
					currpt = nb;
					goto uUnextfoll;
				    }
				}
			      }
			      perr2("dangling uU");
			      uUnextfoll:;
			    }
			    uUnexti:;
			}
			uUnextii:;
		}

		/* track from each marked MM to an M, with preference for M's
			over unmarked MM's */

		for (ii=0,np=MMnode;ii<countMM;ii++,np++) {
			if (((*np)->mark)!=1)
				continue;
			/* if ((*np)->mark > 1)
				perr2("MM mark > 1????"); */
			currpt = *np;
			while (1) {
			  for (k=0;k<2;k++) {	/* M points first */
			    for (d=1;d<=8;d++) {
				if ((l = neighbor(currpt->x,currpt->y,d))
				    && ((k > 0 && l == MM) || l == M)) {
					nb = findnode(currpt->x,currpt->y,d);
					if (l==MM && nb->mark)
						continue;
					nb->mark += 2; /* so this point, if it
						is an MM, won't get picked up
						in a later iteration of this
						loop as a starting point */
					fl1 = getfoll();
					fl2 = getfoll();
					fl1->nextfoll = currpt->flist;
					currpt->flist = fl1;
					fl1->dir = d;
					fl1->nextnode = nb;
					fl2->nextfoll = nb->flist;
					nb->flist = fl2;
					fl2->dir = inversedir(d);
					fl2->nextnode = currpt;
					currpt = nb;
					if (l==M)
						goto MMnextp;
					else
						goto nextMM;
				}
			    }
			  }
			  perr2("dangling MM");
			  nextMM:;
			}
			MMnextp:;
		}

		/* track in two directions from each as yet unmarked MM node,
			linking with followers and stopping as M points are
			reached, with a preference for M points as the next
			follower */
					
		for (ii=0,np=MMnode;ii<countMM;ii++,np++) {
			if ((*np)->mark)
				continue;
			lastd = j = 0; k = -1;
			for (d=1;d<=9;d++) {
				m = 0;
				if ((l = neighbor((*np)->x,(*np)->y,d))) {
					nb = findnode((*np)->x,(*np)->y,d);
					if (l==M || (l==MM && nb->mark==0))
						m++;
				}
				if (k == -1) {
					k = m;
					if (m)
						lastd = d;
				}
				else if (k != m) {
					k = m;
					if (!m || !lastd || (d-lastd!=2))
						j++;
					if (m)
						lastd = d;
				}
			}
			if (j != 4) {
				if (dflag)
					fprintf(stderr,
						"%s: wierd leftover MM%d\n",
						Progname,ii);
				continue;
			}
			(*np)->mark++;
			for (i=1;i<=2;i++) {
			    currpt = *np;
			    first = 1;
			    while(1) {
			      for (k=0;k<2;k++) {	/* M points first */
				for (d=1;d<=8;d++) {
				    if ((l = neighbor(currpt->x,currpt->y,d))
					&& ((k > 0 && l == MM) || l == M)) {
					if (i==2 && first &&
						((m=abs(d-firstd))<2 || m==7))
						continue;
					nb = findnode(currpt->x,currpt->y,d);
					if (nb->mark && nb->type==MM)
						continue;
					nb->mark++;
					fl1 = getfoll();
					fl2 = getfoll();
					fl1->nextfoll = currpt->flist;
					currpt->flist = fl1;
					fl1->dir = d;
					fl1->nextnode = nb;
					fl2->nextfoll = nb->flist;
					nb->flist = fl2;
					fl2->dir = inversedir(d);
					fl2->nextnode = currpt;
					if (i==1 && first)
						firstd = d;
					first = 0;
					if (nb->type == M)
						goto uMMnexti;
					currpt = nb;
					goto uMMnextf;
				    }
				}
			      }
			      perr2("dangling uMM");
			      uMMnextf:;
			    }
			    uMMnexti:;
			}
		}

		/* Link up any neighboring M points */

		for (ii=0,np=Mnode;ii<countM;ii++,np++) {
			currpt = *np;
			for (d=1;d<=8;d++) {
				if (neighbor(currpt->x,currpt->y,d) == M) {
					nb = findnode(currpt->x,currpt->y,d);
					for (foll=currpt->flist;foll!=NULLFOLL;
					    foll=foll->nextfoll)
						if (foll->nextnode==nb)
							goto Mnextdir;
					fl1 = getfoll();
					fl2 = getfoll();
					fl1->nextfoll = currpt->flist;
					currpt->flist = fl1;
					fl1->dir = d;
					fl1->nextnode = nb;
					fl2->nextfoll = nb->flist;
					nb->flist = fl2;
					fl2->dir = inversedir(d);
					fl2->nextnode = currpt;
				}
				Mnextdir:;
			}
		}

		/* now fill in goal pointers */

		for (i=0,np=Enode;i<countE;i++,np++)
			for (foll=(*np)->flist;foll!=NULLFOLL;
			    foll=foll->nextfoll)
				fillgoal(*np,foll);
		for (i=0,np=Mnode;i<countM;i++,np++)
			for (foll=(*np)->flist;foll!=NULLFOLL;
			    foll=foll->nextfoll)
				fillgoal(*np,foll);

		/* next, decide how to divide each segment, if needed */

		countC = 0;
		for (i=0,np=Enode;i<countE;i++,np++)
			for (foll=(*np)->flist;foll!=NULLFOLL;
			    foll=foll->nextfoll)
				if (foll->mark==0)
					countC += divseg(*np,foll);
		for (i=0,np=Mnode;i<countM;i++,np++)
			for (foll=(*np)->flist;foll!=NULLFOLL;
			    foll=foll->nextfoll)
				if (foll->mark==0)
					countC += divseg(*np,foll);

		if (dflag)
			fprintf(stderr,"%s: frame %d, %d cutpoints added\n",
				Progname,f,countC);

		/* next print graph or output spanning tree */

		if (pflag)
			printgraph();
		else if (xflag)
			write_image(&hdo,f);
		else {
			clearmarks();
			outspan();
		}
		resetcore();
	}
	return(0);
}

void perr2(s)

char *s;

{
	fprintf(stderr,"%s: %s\n",Progname,s);
	if (dflag && !inprintg)
		printgraph();
	perr(HE_MSG,"aborting");
}

struct node *getnode()

{
	struct node *n;

	if ((n = freenode) == 0)
		perr2("node freelist empty");
	freenode = n->nextnode;
	n->nextnode = 0;
	return(n);
}

struct follower *getfoll()

{
	struct follower *n;

	if ((n = freefoll) == 0)
		perr2("follower freelist empty");
	freefoll = n->nextfoll;
	n->nextfoll = 0;
	return(n);
}

struct node *findnode(x,y,d)

short x,y;
int d;

{
	struct node **np;
	int xx,yy,i;

	xx = x + xdir[d];
	yy = y + ydir[d];
	for (i=0,np=Unode;i<countU;i++)
		if ((*np)->x == xx && (*np)->y == yy)
			return(*np);
		else
			np++;
	for (i=0,np=Enode;i<countE;i++)
		if ((*np)->x == xx && (*np)->y == yy)
			return(*np);
		else
			np++;
	for (i=0,np=Mnode;i<countM;i++)
		if ((*np)->x == xx && (*np)->y == yy)
			return(*np);
		else
			np++;
	for (i=0,np=MMnode;i<countMM;i++)
		if ((*np)->x == xx && (*np)->y == yy)
			return(*np);
		else
			np++;
	perr2("findnode failure");
	return((struct node *) 0); /* for lint */
}

void clearmarks()

{
	struct follower *foll;
	struct node *node;
	int i;

	for (i=0,node=ofreenode;i<numnode;i++)
		(node++)->mark = 0;

	for (i=0,foll=ofreefoll;i<numfoll;i++)
		(foll++)->mark = 0;
}

void resetcore()

{
	struct follower *foll,*foll2;
	struct node *node,*node2;
	int i;

	freenode = ofreenode;
	for (i=0,node2=node=freenode;i<numnode;i++) {
		node->mark = 0;
		node -> flist = NULLFOLL;
		if (i==numnode-1)
			node -> nextnode = NULLNODE;
		else
			(node++) -> nextnode = ++node2;
	}

	freefoll = ofreefoll;
	for (i=0,foll2=foll=freefoll;i<numfoll;i++) {
		foll->mark = 0;
		foll->goal = NULLNODE;
		foll->nextnode = NULLNODE;
		if (i==numfoll-1)
			foll -> nextfoll = NULLFOLL;
		else
			(foll++) -> nextfoll = ++foll2;
	}
}

void printgraph()

{
	struct node **np;
	int i;

	inprintg++;	/* prevent infinite loops if get an error */
	for (i=0,np=Enode;i<countE;i++)
		printnode("E",i,*np++);
	for (i=0,np=Mnode;i<countM;i++)
		printnode("M",i,*np++);
	for (i=0,np=Unode;i<countU;i++)
		printnode("U",i,*np++);
	for (i=0,np=MMnode;i<countMM;i++)
		printnode("MM",i,*np++);
	inprintg = 0;
}

void printnode(label1,label2,node)

char *label1;
int label2;
struct node *node;

{
	struct follower *foll;

	if ((node->type) & CUT)
		fprintf(stderr,"C");
	if (node == NULLNODE) {
		fprintf(stderr,"%s%d is NULL\n",label1,label2);
		return;
	}
	fprintf(stderr,"%s%d (%d,%d) type %o mark %d nextnode ",label1,label2,
		node->x,node->y,node->type,node->mark);
	printnodel(node->nextnode);
	if (node->flist == NULLFOLL) {
		fprintf(stderr," has no followers\n");
		return;
	}
	fprintf(stderr," followers:\n");
	for (foll=node->flist;foll!=NULLFOLL;foll=foll->nextfoll) {
		fprintf(stderr,"	%ld dir %d mark %d goal ",
			(long) (foll-ofreefoll),foll->dir,
			foll->mark);
		printnodel(foll->goal);
		fprintf(stderr," nextnode ");
		printnodel(foll->nextnode);
		fprintf(stderr,"\n");
	}
}

void printnodel(node)

struct node *node;

{
	int i;
	struct node **np;

	if (node==NULLNODE) {
		fprintf(stderr,"NULL");
		return;
	}
	i = node->type;
	if (i & CUT)
		fprintf(stderr,"C");
	i &= ~CUT;
	switch(i) {
	case U:
		for (i=0,np=Unode;i<countU;i++,np++)
			if (*np==node) {
				fprintf(stderr,"U%d",i);
				return;
			}
		break;
	case E:
		for (i=0,np=Enode;i<countE;i++,np++)
			if (*np==node) {
				fprintf(stderr,"E%d",i);
				return;
			}
		break;
	case M:
		for (i=0,np=Mnode;i<countM;i++,np++)
			if (*np==node) {
				fprintf(stderr,"M%d",i);
				return;
			}
		break;
	case MM:
		for (i=0,np=MMnode;i<countMM;i++,np++)
			if (*np==node) {
				fprintf(stderr,"MM%d",i);
				return;
			}
		break;
	default:
		perr2("unknown type in printnodel");
	}
	perr2("can't find node in printnodel");
}

void fillgoal(goalnode,foll)

struct node *goalnode;
struct follower *foll;

{
	struct node *node,*prev;
	struct follower *f,*fb,*ff;

	prev = goalnode;
	node = foll->nextnode;
	while (1) {
		for (f=node->flist;f!=NULLFOLL;f=f->nextfoll) {
			if (f->nextnode == prev)
				fb = f;
			else
				ff = f;
		}
		fb -> goal = goalnode;
		if (node->type == M || node->type == E)
			break;
		prev = node;
		node = ff -> nextnode;
	}
}

int neighbor(x,y,d)

int x,y,d;

{
	int i,j;

	i = y + ydir[d];
	j = x + xdir[d];
	if (i<0 || i>=r || j<0 || j>=c)
		return(0);
	return((int) pict[i*c+j]);
}

#define	NUMPT	100
#define	NUMSEG	20

int divseg(nd,fl)

struct node *nd;
struct follower *fl;

{
	int px[NUMPT],py[NUMPT],ope[NUMSEG],cls[NUMSEG];
	int i,npt,nope,ncls,nmaxd,nextc,crit,ncut;
	struct node *pnd[NUMPT],*gl,*prev,*nxt;
	struct follower *fb,*ff,*f;
	float maxd,distsq(),d;

	/* track the segment, filling the point array */

	gl = fl->goal;
	fl->mark++;
	px[0] = nd->x;
	py[0] = nd->y;
	pnd[0] = nd;
	prev = nd;
	npt = 1;
	nxt = fl->nextnode;

	while(1) {
		for (f=nxt->flist;f!=NULLFOLL;f=f->nextfoll) {
			if (f->nextnode == prev)
				fb = f;
			else
				ff = f;
		}
		fb->mark++;
		if (npt>=NUMPT)
			perr2("divseg point array overflow");
		px[npt] = nxt->x;
		py[npt] = nxt->y;
		pnd[npt++] = nxt;
		if (nxt == gl)
			break;
		prev = nxt;
		nxt = ff->nextnode;
		ff->mark++;
	}

	if (npt <= 4)
		return(0);

	/* set criterion based on segment length */

	crit = npt <= breakc ? shortc : longc;

	/* apply Ramer-like method to the segment, adding cutpoints at
	   the points at maximum distance from the segment until the entire
	   polygon meets the criterion */

	ncut = nope = ncls = 0;
	ope[nope++] = -1;
	ope[nope] = npt-1;
	cls[ncls] = 0;
	
	while (ope[nope] != -1) {
		maxd = -1;
		nmaxd = -1;
		for (i=cls[ncls]+1;i<ope[nope];i++) {
			d = distsq(px[cls[ncls]],py[cls[ncls]],
				   px[ope[nope]],py[ope[nope]],
				   px[i],py[i]);
			if (d > maxd) {
				maxd = d;
				nmaxd = i;
			}
		}
		if (maxd > crit) {
			if (++nope >= NUMSEG)
				perr2("divseg open stack overflow");
			ope[nope] = nmaxd;
		}
		else {
			if (++ncls >= NUMSEG)
				perr2("divseg closed stack overflow");
			cls[ncls] = ope[nope--];
		}
	}

	/* now fill in cut point labels, and redo goal pointers */

	if (ncls <= 1)
		return(0);
	nextc = 1;
	fl -> goal = pnd[cls[nextc]];
	prev = nd;
	nxt = fl->nextnode;
	while (1) {
		for (f=nxt->flist;f!=NULLFOLL;f=f->nextfoll) {
			if (f->nextnode == prev)
				fb = f;
			else
				ff = f;
		}
		if (nxt == gl) {
			fb->goal = pnd[cls[nextc-1]];
			break;
		}
		else if (nxt == pnd[cls[nextc]]) {
			fb->goal = pnd[cls[nextc-1]];
			ff->goal = pnd[cls[nextc+1]];
			pnd[cls[nextc]]->type |= CUT;
			pict[((pnd[cls[nextc]])->y)*c+((pnd[cls[nextc]])->x)] =
				PCUT;
			nextc++;
			ncut++;
		}
		else {
			fb->goal = pnd[cls[nextc-1]];
			ff->goal = pnd[cls[nextc]];
		}
		prev = nxt;
		nxt = ff->nextnode;
	}
	return(ncut);
}

#define dsq(a,c,b,d)	((float) ((a-b)*(a-b)+(c-d)*(c-d)))
#define odsq(a,b)	((float) ((a-b)*(a-b)))

float distsq(x0,y0,x1,y1,x,y)

int x0,y0,x1,y1,x,y;

{
	float A,B,xtw,ytw;

	if (x0 == x1) {
		if (y0 == y1)
			return(dsq(x0,y0,x,y));
		else {
			if (y0 < y1) {
				if (y < y0)
					return(dsq(x0,y0,x,y));
				else if (y > y1)
					return(dsq(x1,y1,x,y));
				else
					return(odsq(x0,x));
			}
			else {
				if (y < y1)
					return(dsq(x1,y1,x,y));
				else if (y > y0)
					return(dsq(x0,y0,x,y));
				else
					return(odsq(x0,x));
			}
		}
	}
	else if (y0 == y1) {
		if (x0 < x1) {
			if (x < x0)
				return(dsq(x0,y0,x,y));
			else if (x > x1)
				return(dsq(x1,y1,x,y));
			else
				return(odsq(y0,y));
		}
		else {
			if (x < x1)
				return(dsq(x1,y1,x,y));
			else if (x > x0)
				return(dsq(x0,y0,x,y));
			else
				return(odsq(y0,y));
		}
	}
	else {
		A = ((float) (x1-x0))/(y1-y0);
		B = 1/A;
		xtw = (y + A*x + (B*x0 - y0))/(A + B);
		if (x0 < x1) {
			if (xtw < x0)
				return(dsq(x0,y0,x,y));
			else if (xtw > x1)
				return(dsq(x1,y1,x,y));
			else {
				ytw = y - A*(xtw - x);
				return(dsq(x,y,xtw,ytw));
			}
		}
		else {
			if (xtw < x1)
				return(dsq(x1,y1,x,y));
			else if (xtw > x0)
				return(dsq(x0,y0,x,y));
			else {
				ytw = y - A*(xtw - x);
				return(dsq(x,y,xtw,ytw));
			}
		}
	}
}

void outspan()

{
	int i,mind,ox,oy,x,y,d,dx,dy;
	struct node **np;

	ox = 0;
	oy = 0;
	while(1) {
		mind = 1000;
		for (i=0,np=Enode;i<countE;i++,np++)
			if (!((*np)->mark)) {
				x = (*np)->x;
				y = (*np)->y;
				dx = x - ox;
				dy = y - oy;
				if (dx>=-7 && dx<=8 && dy>=-7 && dy<=8)
					d = abs(dx) + abs(dy);
				else
					d = 500;
				if (d < mind)
					mind = d;
			}
		if (mind > 500)
			goto eElp;
		for (i=0,np=Enode;i<countE;i++,np++)
			if (!((*np)->mark)) {
				x = (*np)->x;
				y = (*np)->y;
				dx = x - ox;
				dy = y - oy;
				if (dx>=-7 && dx<=8 && dy>=-7 && dy<=8)
					d = abs(dx) + abs(dy);
				else
					d = 500;
				if (d == mind) {
					outtree(*np,&ox,&oy);
					goto cElp;
				}
			}
		cElp:;
	}
eElp:
	ox = 0;
	oy = 0;
	while(1) {
		mind = 1000;
		for (i=0,np=Mnode;i<countM;i++,np++)
			if (!((*np)->mark)) {
				x = (*np)->x;
				y = (*np)->y;
				dx = x - ox;
				dy = y - oy;
				if (dx>=-7 && dx<=8 && dy>=-7 && dy<=8)
					d = abs(dx) + abs(dy);
				else
					d = 500;
				if (d < mind)
					mind = d;
			}
		if (mind > 500)
			goto eMlp;
		for (i=0,np=Mnode;i<countM;i++,np++)
			if (!((*np)->mark)) {
				x = (*np)->x;
				y = (*np)->y;
				dx = x - ox;
				dy = y - oy;
				if (dx>=-7 && dx<=8 && dy>=-7 && dy<=8)
					d = abs(dx) + abs(dy);
				else
					d = 500;
				if (d == mind) {
					outtree(*np,&ox,&oy);
					goto cMlp;
				}
			}
		cMlp:;
	}
eMlp:
	putchar('E');
}

void outtree(ind,ox,oy)

struct node *ind;
int *ox,*oy;

{
	struct node *currpt,*nextpt;
	struct follower *f;
	short cq,eq;
	h_boolean Bflag;

	cq = eq = 0;
	ndq[0] = ind;
	Bflag = FALSE;
	ind->mark++;
	putchar('I');
	fwrite(&(ind->x),sizeof(short),1,stdout);
	fwrite(&(ind->y),sizeof(short),1,stdout);
	*ox = ind->x;
	*oy = ind->y;

	while (cq <= eq) {
		currpt = ndq[cq];
		while(1) {
			nextpt = NULLNODE;
			for (f=currpt->flist;f!=NULLFOLL;f=f->nextfoll) {
				if (!(f->mark)) {
					if (nextpt==NULLNODE) {
						f->mark++;
						nextpt = f->goal;
					}
					else if (f->goal==nextpt)
						f->mark++;
				}
			}
			if (nextpt == NULLNODE)
				break;
			for (f=nextpt->flist;f!=NULLFOLL;f=f->nextfoll)
				if (f->goal==currpt)
					f->mark++;
			if (Bflag) {
				putchar('B');
				fwrite(&cq,sizeof(short),1,stdout);
				Bflag = FALSE;
			}
			putchar('C');
			fwrite(&(nextpt->x),sizeof(short),1,stdout);
			fwrite(&(nextpt->y),sizeof(short),1,stdout);
			*ox = nextpt->x;
			*oy = nextpt->y;
			currpt = nextpt;
			if (!(currpt->mark)) {
				currpt->mark++;
				eq++;
				if (eq > numE+numM+numC)
					perr2("outtree stack overflow");
				ndq[eq] = currpt;
			}
		}
		cq++;Bflag = TRUE;
	}
}
