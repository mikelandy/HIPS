static char *SccsId = "%W%      %G%";

/*-------------------------------------------
 * Induction algorothm and tree printing routines.
 * Barry Shepherd, 1985.
 * ---------------------------------------------  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "vacls.def.h"
#include "vacls.ext.h"

#define	GUESSCLASH	1
#define TRUE	1
#define	FALSE	0

int cutoff=0,auto_term=FALSE ;
float chilimit=0,inflimit=0 ;
int guessclash=0 ;	
int oldwrong,newwrong;/* new bits for test set termination */
void err(),freenodes(),split(),countex();
int passex(),classes(),sameatts(),bestclass(),bestattr(),checksec();
int att_avail(),getanum();
char *valstr();

/* ------------------------------------------------------------------------ */

struct node *addnode(attnum, attval, thresh, exinput, exptr)
int attnum;
int *exinput;		/*  pointer for input example array */
int **exptr;		/*  used to pass output exmp array ptr  */
int attval;
int thresh;		/* threshold (for INTEGER attribute only) */
{
	int *exused;		/* pointer to array of examples for this node*/
	struct node *newnode;
	int i, max;
	int ex;
	int oldval;
	int extemp[MAXEX];	/* temporary storage for example list */

/*  get space from system for the new node */
	if ( (newnode = (struct node *)malloc(nodesize)) == NULL)
		err("addnode: NULL return from malloc");

/*  initialize the new node variables             */
	numnodes++;
	nodptr[numnodes] = newnode;
	newnode->splitatt = 0;
	newnode->splitval  = 0;
	newnode->firstson  = NULL;
	newnode->brother   = NULL;

/* initialize the temporary node-building data structures */
	curlev++;

	if ( curlev == 1)   {		/* root node */
	if ( (exused=(int *)malloc( (numprim+1)*sizeof(numattr) )) == NULL )
			 	err("addnode:  NULL return from malloc");
		for (i=1; i<=numprim; i++)    /* assign all primary examples */
				    *(exused+i) = i;
		*exused = numprim;
	        newnode->nexamp = numprim;
		}

	else    {			/* intermediate or leaf node */
		max = 0;		/* get list of ex's to pass to son */
		for (i=1; i<= *exinput  ; i++)
		    {
		    ex = *(exinput + i);
		    oldval = getattr(ex,attnum);
		    if ( passex(oldval, attnum, attval, thresh) == YES)  {
			max++;
			extemp[max] = ex;
			}
		    }
		if ( (exused=(int *)malloc( (max+1)*sizeof(numattr)) ) == NULL)
				err("addnode: NULL return for leaf malloc");
		*exused = max;
		for (i=1;i<=max;i++)	*(exused+i) = extemp[i];
		newnode->nexamp = max;
		}

	*exptr = exused;		/* pointer value we can send back */
	return(newnode);
}

/* -------------------------------------------------------------------  */
extern int numleaves ;
	 
void induce()	/* induce a rule from primary store examples	*/
{
	int *exused;		/* pointer to list of examp #'s used by root */
	int *exdummy = (int *) 0;/* dummy because root has no parent examples */

	if (numnodes > 0) 	freenodes();

	root = addnode(0,0,0, exdummy, &exused);
	oldwrong = 10000 ;
	numleaves = 0 ;
	split(root, exused);	
	free (exused);

	printf("\n%d node rule generated\n",numnodes);

	return;
	}
/* ------------------------------------------------------------------------ */
/*  release a node's storage (and that of all nodes below it)        */

void	release (topnode)
	struct node *topnode;
	{
	int i;

	if (topnode->firstson != NULL)		/* there is a subtree */
			release(topnode->firstson);

	if (topnode->brother != NULL)		/* there are brother(s) */
			release (topnode->brother);

	free (topnode);
	return;
	}

/* ----------------------------------------------------------------------  */
/* free the nodes of current tree and reset numnodes to 0		*/

void	freenodes()
	{
	int i;

	numnodes = 0;
	curlev = 0;
	release (root);
	printf("tree reinitialized\n");
	return;
	}

/* ---------------------------------------------------------------------- */
/* calc entropy before split */

float pentr(count,numegs)
int *count,*numegs ;
{
	int i ;
	float pent,probcl ;
	extern double log();

	pent = 0 ;
	for (i=1;i<=numclass;i++)
	{	probcl = ((float)count[i])/(*numegs) ;
		pent -= probcl * log(probcl) ; 	
	}
	return(pent) ;
}

/* ------------------------------------------------------------------------ */

void split(parent, parused)
struct node *parent;
int *parused;			/* examples used by parent */
{
	int attr;		/* index for attribute to split on */
	int threshold;		/* threshold for splitting INTEGER type */
	struct node *prevson;	/* pointer to previous son		*/
	struct node *sonstore[3] ;
	int *sonused[3]	;	/* new bit - considering int atts only */
	struct node *son;	/* pointer to son being created		*/
	int i,temp;
	int parcl;		/* number of parent classes */
	int count[MAXCLASS];
	float ch,ent ;

/* first check to see if this is a leaf node.
 *  NOTE that if it is, the class of the leaf is stored in splitval position */

	parcl = classes(parused,parent->count);

	if ( parcl == 1) {			/* regular leaf node */
		parent->splitval = getclass(  *(parused+1)    );
		parent->leafnum = numleaves++ ;
	}
	else if ( parcl == 0) {		/* null leaf (no examples) */
		parent->splitval = 0;
		parent->leafnum = numleaves++ ;
	}
	else if (sameatts(parused)) {		/* they are ALL clashes */
		if (guessclash)
			parent->splitval = bestclass(1,parent->count) ;
		else
			parent->splitval = -parcl;
		parent->leafnum = numleaves++ ;
	}
	else  if ((*parused < cutoff) && (cutoff)) { /*no.egs is too small*/
		parent->splitval = bestclass(1,parent->count) ;
		parent->leafnum = numleaves++ ;
	}
	/* it isn't a leaf; we have to split */
	else
	{	/* get attr to split on */
		attr = bestattr(parused,&threshold,&ent,&ch);

		/* if class/att combs seem random */
	    	if ((ch < chilimit) && ( *parused >= 0))
			parent->splitval = bestclass(1,parent->count) ;
		else 
		{
			/* --------------------------------------------------
			   these few lines work if there are no
			   don't cares in the example set. However
			   if there are don't cares then the routine
			   pentr() does not correctly return 
			   the parent entropy (its much too small)
			   and the tree is always terminated here!! 

			parent->pent = pentr(parent->count,parused)  ;
			parent->avent = ent/(*parused) ;
			if ( (*parused)*(parent->pent-parent->avent)/numprim < inflimit)
				parent->splitval = bestclass(1,parent->count) ;
			else
			------------------------------------------*/
			{

	    		usedatt[curlev] = attr;
	    		parent->splitatt = attr;
	    		parent->splitval = threshold;
			parent->numegs = *parused ;
			parent->chi = ch ;
	    		prevson = NULL;

/* this is the ordinary way..... */

			if (!auto_term) {
	    			for (i=1; i<=att[attr].nval; i++) {
					son=addnode(attr,i,threshold,parused,&sonused[0]);
					if (i==1)	
		    				parent->firstson = son;
					else
		    				prevson->brother = son;
					split(son, sonused[0]);
					prevson = son;
					curlev--;	
					free (sonused[0]);
				}
			}

/* and this is the clever way..... */

	    		else {
				for (i=1;i<=att[attr].nval;i++) {
			     		son=addnode(attr,i,threshold,parused,&sonused[i]);
					numnodes-- ; /*the ++ is in addnode */
					sonstore[i] = son ;
					if (i==1)	
		    				parent->firstson = son;
					else
		    				prevson->brother = son;
					classes(sonused[i],son->count) ;
					son->splitval = bestclass(1,son->count) ;
					prevson = son;
					curlev--; /* curlev++ is in addnode */	
				}
				printf("checking the sec.store for node %d\n"
						,numnodes) ;
				newwrong = checksec(&temp) ;
				fprintf(stderr,"%d wrong, level= %d\n",newwrong,curlev) ;
				if ((newwrong < oldwrong) || (!oldwrong)) {
					curlev++ ;
					oldwrong = newwrong ;
	    				for (i=1; i<=att[attr].nval; i++) {
						numnodes++ ;
						split(sonstore[i],sonused[i]);
					}
					curlev--;	
				}
				else {
					parent->splitatt = 0 ;	
					parent->splitval = bestclass(1,parent->count) ;
				}
	    			for (i=1; i<=att[attr].nval; i++)
					free (sonused[i]);
			}
			}
	    	}
	}
	usedatt[curlev] = 0;
	return;
}

/*--------------------------------------------------------------------- */
/* determine best class if cut-off is in use*/

int bestclass(criterion,count)
int criterion,*count ;
{	int i,best ;
	best = 1;
	switch(criterion)
	{	case 1:
			for (i=2;i<=numclass+2;i++)
				if (count[i] > count[best])
					best = i ;
			break ;
	}
	return(best) ;
}

/* ---------------------------------------------------------------------*/

	int classes(exused,count) /* get number of classes for the examples of
					the list exused  */
	int *exused,*count ;		/* pointer to example list */
				/* *exused is # ex's, *(exused+i) is i'th ex */
				/* this points to the list 'exlist' */
	{
	int i, num,tot ;

	for (i=1;i<=numclass+2; i++)	count[i] = 0;

	for (i=1; i<= *exused ; i++) 
		count[ getclass( *(exused + i) )   ]++;

	count[0] = *exused ;
	num = 0;
	for (i=1; i<=numclass+2; i++)
		{
		if (count[i]>0) num++;
		}
	return(num);
	}

/* ---------------------------------------------------------------------- */
	int sameatts(exused)	/* returns 1 if ALL exused are clashes,else 0 */

	int *exused;
	{
		int i,j ;
		for (i=2;i<= *exused;i++)
			for (j=1;j<= numattr;j++)
				if (att_avail(j)==YES)	/*just to save time*/
					if (getattr(*(exused+1),j)!=getattr(*(exused+i),j))
						return(0);
		return(1);
	}

/* -------------------------------------------------------------------- */
/* decide whether a particular example should be passed to son */
	int passex(exvalue, attnum, attval, threshold)
	int exvalue;
	int attnum;
	int attval;
	int threshold;
	{
	if ( exvalue == DONTCARE)				return(YES);

	if ( att[attnum].type == INTEGER)  {
		if (( attval == 1) && (exvalue  < threshold))	return(YES); 

		if ((attval == 2) && (exvalue >= threshold))	return(YES); 
		}
	else if ( exvalue == attval) 				return(YES);

	return(NO);
	}
/* ------------------------------------------------------------------------ */
/* find the best attribute number (and threshold if INT) 
	for the current node in the traversal			
	Examples are in exused[][], node->nexamp		*/
	 
	int bestattr(exused,threshold,avent,bestchi)
	int *threshold;
	int *exused;		/* pointer to list of examples to use */
	float *bestchi,*avent ;
	{
	int i;
	float smin, entrdiff;
	float chsq ;
	int best;
	int thresh,minorder ;
	char attname[MAXLIN];
	extern float slogical();
	extern float sinteger();

	*threshold = 0;
	thresh = 0;
	smin = 1.00e20;
	minorder = MAXATTR + 1 ;

	if (debug[4])
	{	fprintf(outstream,"%d examples\n",*exused);
		for (i=1;i<=*exused;i++)  fprintf(outstream,"%d ",*(exused+i) );
		fprintf(outstream,"\n");
	}
	if (debug[3])  fprintf(outstream,"\n");

	for (i=1; i<=numattr; i++)
	{	if (att_avail(i) == YES)
		{	if (att[i].type == INTEGER) 
				entrdiff = sinteger(exused, i, &thresh, &chsq);	
	 		else
				entrdiff = slogical(exused, i);
			if ( entrdiff <= smin)
				if ((entrdiff<smin)||(att[i].order<minorder))
				{	smin = entrdiff;
					best = i;
					*threshold = thresh;
					*bestchi = chsq ;
					minorder = att[i].order ;
				}
	        	if (debug[2]) 
	 	  	fprintf(outstream,"%5d  %15s %12.5f%9d\n",
					i,getaname(i),entrdiff,thresh);
		}
	}

	if (debug[3])
	{	do
		{	fprintf(outstream,"attr & thresh to split on:");
			scanf("%s%d",attname,&thresh);
		}
		while ( (best = getanum(attname) )  == 0);
		*threshold = thresh;
	}

	if (smin >= 1.0e19)
		err("bestattr: entropy error");
	*avent = smin ;
	return(best);
}

/* ---------------------------------------------------------------------- */
/* compute entropy diffenence for an integer attribute  */

	float sinteger(exused, attnum, thresh, chsq)
	int *exused;
	int attnum;
	int *thresh;
	float *chsq ;
	{
	int i;
	int sortlist[MAXEX][2];
	int ex;
	float smin, entr;
	int oldval, newval, thr;
	int p[MAXCLASS][MAXVALS];
	extern float entropy(),chisq() ;
	extern int compar();

	if (att[attnum].type != INTEGER) err("sinteger: wrong type");

	for (i=1; i<=*exused; i++)  {
		ex = *(exused+i);
		sortlist[i-1][0] = ex;    /* example numbers */
		sortlist[i-1][1] = getattr(ex,attnum);  /* attr value */
		}

	qsort(sortlist, *exused, sizeof(i)*2, compar);

	smin = 1.0e20;
	oldval =  sortlist[0][1] ;
	for (i=1; i<*exused; i++)
	{   if ((newval =  sortlist[i][1])   != oldval)
	    {	thr = (newval + oldval+1)/2;
	        oldval = newval;
		countex(exused, attnum, thr, usedatt, curlev-1, p);
		entr = entropy(numclass, 2, p);
		if (entr < smin)
		{	smin = entr;
			*thresh = thr;
			*chsq = chisq(numclass,2,p) ;
		}
	    }
	}
	return(smin);
	}

/* ----------------------------------------------------------------------- */
void	printsort(str,array,len)
	char *str;
	int array[MAXEX][2];
	int len;
	{
	int i;
	printf("%s\n",str);
	for (i=0; i<len; i++)  
		printf("%5d.  %8d   %8d\n",i,array[i][0],array[i][1]);
	}
/* ------------------------------------------------------------------------ */
/* compute entropy for a logical attribute                             */

	float slogical(exused, attnum)
	int *exused;		/* list of examples to use */
	int attnum;		    /* number of attribute to split */
	{
	int thresh=0;		/* dummy here, countex is used for INT also */
	float entrdiff;
	int p[MAXCLASS][MAXVALS]; /* place to store probabilities  */
	extern float entropy();

	countex(exused, attnum,thresh, usedatt, curlev-1, p);   /* load pop array */

	entrdiff = entropy(numclass+2, att[attnum].nval, p);  /* get entropy */

	return (entrdiff);
	}
/* -------------------------------------------------------------------------- */
/*  conpute entropy difference from population array  */

	float entropy( nclass, nval, p)
	int nclass;			/* number classes (first p dimension) */
	int nval;		/* number attribute values (sec  p dimension) */
	int p[MAXCLASS][MAXVALS];  	/* class/attr value  population array */
					/*  see countex and countint routines */
	{
	int i,j,k;
	float initial, final, sum, pi;
	extern double log();

	final = 0.0;			/* compute final entropy */
	for (j=1; j<=nval; j++) {

	    if ( p[0][j] > 0) {   /* if examples with this att value: */
		for (i=1; i<=nclass; i++)  {
			if ( (pi=p[i][j]) > EPSILON)   
				final -=  pi*log(pi/(float)p[0][j]  );
			}
	    }
	}

	return(final);
	}
/* -------------------------------------------------------------------------- */
/* computeo chi-sq for attr (if log) or att/thresh comb (if int) */

float chisq(nclass,nval,p)
int p[MAXCLASS][MAXVALS] ;
int nclass,nval ;
{
	int i,j ;
	float chsq,expnum ;

	chsq = 0 ;
	for (i=1;i<=nclass;i++)
	{	for (j=1;j<=nval;j++)
		{	expnum = ((float)(p[i][0] * p[0][j]))/p[0][0] ;
			if (expnum)
			   chsq += ((p[i][j]-expnum)*( p[i][j]-expnum))/expnum ;
		}
	}
	return(chsq) ;
}

/* ------------------------------------------------------------------------- */
/* count class/attribute value occurrences
   inputs are:
	exused		pointer to list of examples
	attnum		attribute number to split on
	usedlist	list of splitting attributes in current tree path
	nused		length of usedlist
   returns:
	p[0][0]		total number of examples (expanded, with previous
					splits accounted for )
	p[0][j]		# examples of all classes for j'th value of attnum
	p[i][0]		# examples of i'th class, for all attribute values
	p[i][j]		# examples of i'th class, for j'th value of attnum
*/
void	countex(exused, attnum, thresh,  usedlist, nused, p)
	int *exused;
	int attnum;
	int thresh;
	int usedlist[];
	int nused;
	int p[MAXCLASS][MAXVALS];
	{
	int numval;
	int i,j;
	int ex,cl,aval,num_ex;
	int atused, n;
					/* zero the counter array */	
	numval = att[attnum].nval;
	for (i=0; i<=numclass+2; i++)  {
		for (j=0; j<=numval; j++)  p[i][j] = 0;  }
		
	for (i=1; i<=*exused; i++)  {
		ex = *(exused + i);		/* get example number */
		aval = getattr(ex,attnum);	/* get attr value index */
		cl = getclass(ex);		/* get class index */
		if (att[attnum].type == INTEGER) {
			if (aval != DONTCARE)  {
				if (aval < thresh) aval = 1;
				else		   aval = 2;
				}
			}
		num_ex = getattr(ex,numattr+1); /* expanded # examples */

		for (j=1; j<=nused; j++) {	/* correct for split DONTCARES*/
		    atused = usedlist[j];
		    if ((atused == attnum) && (att[attnum].type != INTEGER))
					err("countex: attnum already used");
		    if (getattr(ex,atused) == DONTCARE)
					num_ex /= att[atused].nval;
		    }
		p[0][0] += num_ex;
		p[cl][0] += num_ex;

		if (aval == DONTCARE)   {
		    n = num_ex/numval;
		    for (j=1; j<=numval; j++)  {
			p[0][j] += n;
			p[cl][j] += n;
			}
		    }
		else   {
		    p[0][aval] += num_ex;
		    p[cl][aval] += num_ex;
		    }
	}

	if (debug[5]) {
		fprintf(outstream,"population array -- %s\n",getaname(attnum));
		if (att[attnum].type == INTEGER) 
					fprintf(outstream,"  %d",thresh);
		fprintf(outstream,"\n");
		fprintf(outstream,"class\n");
		for (i=0; i<=numclass+2; i++)    {
		    fprintf(outstream,"%4d ",i);
		    for (j=0; j<=att[attnum].nval; j++)  
					fprintf(outstream,"%8d",p[i][j]);
			fprintf(outstream,"\n");
			}
		}

	return;
	}

/* ---------------------------------------------------------------------- */
/* EVERYTHING FROM HERE IS DUPLICATED IN PRINTTREE.C */

int klev,nodesofar;
float treeinf ;

void	printnodes(strm)		/* print all the nodes */
	FILE *strm;
	{
	int i;
	int a;

	fprintf(strm," %d nodes in tree\n\n", numnodes);

	for (i=1; i<=numnodes; i++)   {
		fprintf(strm," %9p%4d. ",  nodptr[i], i);/* node address, num */
		a = nodptr[i]->splitatt;
		if ( a > 0 )
		    fprintf(strm,"  %-12s  ",getaname(a) ); /* split attrib */
 	        else
		    fprintf(strm,"  %12s  ", "LEAF");

		if ( a == 0 )			/* leaf node */
			fprintf(strm,"%-10s", classname(nodptr[i]->splitval ));
		else				/* test node */
			fprintf(strm,"%10d", nodptr[i]->splitval);

		fprintf(strm,"%8p%8p%7d\n",nodptr[i]->firstson,nodptr[i]->brother,
							 nodptr[i]->nexamp);
		}
		
	return;
	}

/* --------------------------------------------------------------------- */

/* print the tree below node */

void printtree(topnode, parnode, parattr, aval, level,strm,labelleaves)
struct node *topnode;	/* points to top of tree to print */
struct node *parnode;   /* points to parent node */
int parattr;		/* attribute which topnode's parent split */
int aval;		/* attribute value of parattr for topnode */
int level;		/* nesting level of topnode (root=1)      */
FILE *strm;
int labelleaves ;	/* when set all leaves will have a unique
			  label:
			  if labelleaves == 1, then the  label is the leafnumber
			  if labelleaves == 2 , the label is a leaf/class number
			  eg: "apple" will become "apple1","apple2",...*/
{
	struct node *temptr ;
	char str1[20];
	char str2[20];
	int i;
	static int leaf[MAXCLASS];	/* used when labelleaves is true */
	float infgain ;

	if (topnode == NULL)
	{	fprintf(strm," no rule exists \n");
		return;
	}
	else if (!level) {	/* ie root node */

		/* is there is a subtree */
		if (topnode->firstson == NULL) {
			fprintf(strm,"%s\n",classname(topnode->splitval)) ;
			return;
		}
		if (labelleaves==2)
			for (i=0;i<=numclass;i++)
				leaf[i] = 0 ;

		fprintf(strm,"[%s]",getaname(topnode->splitatt));
		level = 1;
		klev = 0;
		nodesofar=1 ;
		infgain = topnode->pent - topnode->avent;
		treeinf = infgain ;

/*		fprintf(strm,"\n%80s"," ");
		fprintf(strm,"e%d c%0.1f p%0.2f c%0.2f d%0.2f",
			topnode->numegs,topnode->chi,topnode->pent,
			topnode->avent,infgain) ;
		fprintf(strm," it%0.2f %0.2f",treeinf,treeinf/nodesofar) ; 
*/ 
	}

	else {
		for (i=2; i<level; i++)  fprintf(strm,"         ");

		if (att[parattr].type == INTEGER) {
		    if ( aval == 1)  {
			strcpy(str1, "<");
			sprintf(str2,"%d", parnode->splitval);
			fprintf(strm,"%9s",strcat(str1,str2) );
		    }
		    else   {
			strcpy(str1, ">=");
			sprintf(str2,"%d", parnode->splitval);
			fprintf(strm,"%9s",strcat(str1,str2) );
		    }
		}
		else
		    fprintf(strm,"%9s", valstr(parattr, aval, str1) );

		fprintf(strm," : ");
		if ( (i=topnode->splitatt) != 0) { 	/* if test node */
			nodesofar++ ;
			fprintf(strm,"[%s]",getaname(i));
			infgain = topnode->pent - topnode->avent;
			if (numprim)
				treeinf += (topnode->numegs*infgain)/numprim ;

		/*	fprintf(strm,"\n%80s"," ") ;
			for (i=0;i<level;i++)
				fprintf(strm," ") ;
			fprintf(strm,"e%d c%0.1f p%0.2f c%0.2f d%0.2f",
				topnode->numegs,topnode->chi,topnode->pent,
				topnode->avent,infgain) ;
			fprintf(strm," it%0.2f %0.2f",treeinf,treeinf/nodesofar);*/ 

		}
		else					/* leaf node    */
			if ((i= -(topnode->splitval)) > 0)
			{	fprintf(strm,"search (%d)",i);
				if ((temptr=topnode->firstson) != NULL)
					while (i>0)
					{	fprintf(strm," %s",classname(temptr->splitatt)) ;
						if (--i)
						{	fprintf(strm," %s",classname(temptr->splitval)) ;
							if (--i)
							{	fprintf(strm," %s",classname(temptr->nexamp)) ;
								temptr= temptr->firstson ;
								i-- ;
							}
					
						}
					}
			}
			else
			{	fprintf(strm,"%s",classname(topnode->splitval));
				/* next bit may help(when the rule is tested) to
				   spot uni/multi modal leaf effects */
				if (labelleaves==2)
				   fprintf(strm,"%d",leaf[topnode->splitval]++);
				else if (labelleaves==1)
					fprintf(strm,"%d",topnode->leafnum) ;
			}

		/*	{	fprintf(strm,"%s %d=%d%% ",
					classname(topnode->splitval),
					topnode->count[topnode->splitval],
					(100*topnode->count[topnode->splitval])/topnode->count[0]);
				for (i=1;i<=numclass;i++)
					if ((topnode->count[i])&&(i!=topnode->splitval))
					  fprintf(strm,"%s %d=%d%% ",
						classname(i),topnode->count[i],
						(100*topnode->count[i])/topnode->count[0]);
			}
		*/

	     }
	 fprintf(strm,"\n");
	
	if ((topnode->firstson != NULL)&&(topnode->splitatt != 0)) 
	  printtree(topnode->firstson,topnode,topnode->splitatt,1,level+1,
						strm,labelleaves);
	
	if (topnode->brother != NULL)		/* there are brother(s) */
           printtree(topnode->brother,parnode, parattr, aval+1, level,
			strm,labelleaves);

	return;
}
/* ---------------------------------------------------------------------- */

/* print the tree below node - as 'C' code.  */

void cprinttree(topnode,parnode,parattr,aval,level,strm)
struct node *topnode;		/* ponts to top of tree to print */
struct node *parnode;		/* points to parent node         */
int parattr;			/* attribute which topnode's parent split */
int aval;			/* attribute value of parattr for topnode */
int level;			/* nesting level of topnode (root=1)      */
FILE *strm;
{
#ifdef CTREE
	int i;
	if (numnodes<1)
		fprintf(strm," no rule exists\n");
	else
	{
		if (att[parattr].type == INTEGER)
		{	if (aval>1)
			{	for (i=0;i<level-1;i++)
					fprintf(strm,"	");
				fprintf(strm,"else\n");
			}
		}	
		if (att[parattr].type == LOGICAL)
		{	for (i=0;i<level-1;i++)
				fprintf(strm,"	");
			fprintf(strm,"case %d:\n",aval);
		}
		for (i=0;i<level;i++)
			fprintf(strm,"	");

		if (topnode->splitatt != 0)	/* if not a leaf */
		{
			if (att[topnode->splitatt].type==INTEGER)
				fprintf(strm,"if (%s<%d)\n",getaname(topnode->splitatt),topnode->splitval);
			else
				fprintf(strm,"switch(%s) {\n",getaname(topnode->splitatt));
			if (topnode->firstson != NULL)	/* any sons? */
				cprinttree(topnode->firstson,topnode,topnode->splitatt,1,level+1,strm);
		}
		else					/* it's a leaf */
		{	if (topnode->splitval < 0)	/*search leaf?*/
				fprintf(strm,"class = search;\n");
			else				/*class or null */
				fprintf(strm,"class = %s;\n",classname(topnode->splitval));
		}
		if (att[parattr].type == LOGICAL)
		{	for (i=0;i<level-1;i++)
				fprintf(strm,"	");
			fprintf(strm,"break;\n");
		}
		if (topnode->brother != NULL) 	/* has it got brothers? */
			cprinttree(topnode->brother,parnode,parattr,aval+1,level,strm);
		else
			if (att[parattr].type == LOGICAL)
			{	for (i=0;i<level-1;i++)
					fprintf(strm,"	");
				fprintf(strm,"}\n");
			}
	}
#endif
}
