static char *SccsId = "%W%      %G%";

/* -------------------------------------------------
 * I/O of example data, checking of examples against rules.
 * Barry Shepherd, 1985.
 * -------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "vacls.def.h"
#include "vacls.ext.h"

extern	int numrules ;
int newcnt,dupcnt,clashcnt ;
int loadexamp(),nextword(),attrtoken(),classtoken(),savestr(),checkex();
int checkrule();
void addex(),err(),induce(),moveconts(),storerule(),move(),addex(),exprompt();
FILE *filopn();

/* read examples from a file */

int readexamp(destn)

int destn;

{
	char line[MAXLIN];
	FILE *strm;
	char filename[MAXLIN];
	int errcnt,totcnt,status ;

	strm = filopn("Enter example file name: ",filename,"r");
	if (strm==NULL)
		return(ERROR);

	totcnt=newcnt=dupcnt=clashcnt=errcnt=0 ;
	while (fgets(line,MAXLIN,strm) != NULL) {
		/* first see if all attributes occur */
		if (loadexamp(line,destn,ALLATTS) == OKAY) {
			totcnt++ ;
			continue ;
		}
		/* see if it contains only cur. avail atts*/
		if (loadexamp(line,destn,AVAILATTS) == OKAY) {
			totcnt++ ;
			continue ;
		}
		printf("ignoring:- %s\n",line);
		errcnt++ ;
	}
	printf("total read=%d\n",totcnt) ;
	if ( (destn!=PRIM) && (destn!=SEC) )
		printf("new=%d,dups=%d,clashes=%d\n",newcnt,dupcnt,clashcnt);
	return(OKAY);
	}

/* ---------------------------------------------------------------- */

/* load example from line; store in system allocated space
   line format is list of attribute values, class value, newline	*/

int loadexamp(line,destn,whichatts)

char line[MAXLIN];
int destn,whichatts;

{
	char word[MAXLIN];
	int nchar, nc, i;
	int agreedest,contrdest ;
	int newexamp[MAXEX];
	int type;
	int number;

	nchar = 0;

/*  first load the new example into the array newexamp[]     */

	for (i=1;i<=numattr;i++)
	{
		if ((whichatts == AVAILATTS)&&(att[i].dontuse))
			word[0] = '-' ;	/* dont care*/
		else {
	     		nc = nextword(line + nchar, word);
			if (nc == EOF)
				return(ERROR); 
			nchar += nc;
		}
		if (attrtoken(i,word,&newexamp[i]) !=  OKAY)
			return(ERROR);
	}
	nc = nextword(line + nchar, word);
	if (nc == EOF)
		return(ERROR);

	/* old style... reject if class not in att defn file  
	if (classtoken(word,newexamp) != OKAY) return (ERROR);
	*/
	if (classtoken(word,newexamp) == ERROR) {
		savestr(word); /* new class added */
		numclass++;
		classtoken(word,newexamp) ;
	}
	
	newexamp[numattr+1] = 1;		/* compute # of expanded ex's */
	for (i=1; i<=numattr; i++)  {
		if (newexamp[i] == DONTCARE) {
			if (att[i].type == LOGICAL)
				newexamp[numattr+1] *= att[i].nval;
		    	else if (att[i].type == INTEGER)
				newexamp[numattr+1] *= 2;
		    	else
				printf("loadexamp:  bad attribute type");
		}
	}

/*  decide which store it belongs in, and put it there     */

	switch (destn)
	{
		case SAME:
		case DSEC:
		case CPRIM:
		case DCCHNG:
			contrdest=PRIM ;
			agreedest=SEC ;
			break ;
		case PRIM:
			addex(newexamp,PRIM) ;
			return(OKAY);
			break ;
		case SEC:
			addex(newexamp,SEC) ;	
			return(OKAY) ;
			break ;
	}

	type = checkex(newexamp, &number);	/* is it NEW, SUBSET, ...? */

	switch (type)  {

	case NEW:
		newcnt += 1;
		if ( checkrule(newexamp) == AGREE)
		{	if (destn==SAME)
				printf("agrees with rule; added to secondary store\n");
			addex(newexamp, agreedest);
		}
		else
		{	if (destn==SAME)
				printf("contradicts rule; added to primary store\n");
			addex(newexamp, contrdest);
		}
		break;

	case  SUBSET:
		printf("subset of #%d; ignored\n",number);
		break;

	case  SAME:
		printf("same as #%d; ", number);
		dupcnt += 1 ;
		switch (destn)
		{
			case SAME:
			case CPRIM:
				printf("ignored\n");
				break;
			case DSEC:
			case DCCHNG:
				printf("added to secondary\n");
				addex(newexamp,SEC) ;
				break;
		}
		break;

	case  SUPERSET:
		printf("superset of #%d; n",number);
		for (i=0; i<=numattr+1; i++)
			*(exlist[number]+i) = newexamp[i];
		break;

	case  CLASH:
		printf("clashes with #%d; ",number);
		clashcnt += 1;
		switch(destn)
		{
			case SAME:
			case DSEC:
				printf("added to secondary\n");
				addex(newexamp,SEC);
				break;
			case CPRIM:
			case DCCHNG:
				printf("added to primary\n") ;
				addex(newexamp,PRIM);
				break;
		}
		break;
	default:
		err("loadexamp:  switch fell through to default");
		break;
	  }    /* end switch */

	return(OKAY);
	}

/* ----------------------------------------------------------------------- */

/* find the class of example ex according to rule headed by head node   */

int ruleclass(ex, head)
int ex[];
struct node *head;
{
	struct node *nextnode;
	struct node *temp;
	int i,son,attr;

	attr = head->splitatt ;
	if (att[attr].type == INTEGER)
	{	if (ex[attr]<head->splitval)
			son=1 ;
		else	son=2 ;
	}
	else
		son=ex[head->splitatt] ;

	if (head->splitatt == 0) 		/* leaf node */
	{	if (head->splitval == 0)	/* null leaf */
			return(0);		/* "null" stored in class[0]*/
		if (head->splitval < 0)		/* search leaf */
			/* note -N => search, with N being no.classes */
			return(1);		/* search stored in class[1] */
		else
			return(head->splitval);		/*class stored there */
	}

	else {					/* go to next level of tree*/
		nextnode = head->firstson;
		for (i=2; i<=son; i++)
	   	  {	temp = nextnode->brother;
			nextnode = temp;		}
		return(ruleclass(ex,nextnode) );
		}
}

/* return the leaf number when the eg is classified */
int ruleleaf(ex, head)
int ex[];
struct node *head;
{
	struct node *nextnode;
	struct node *temp;
	int i,son,attr;

	attr = head->splitatt ;
	if (att[attr].type == INTEGER)
	{	if (ex[attr]<head->splitval)
			son=1 ;
		else	son=2 ;
	}
	else
		son=ex[head->splitatt] ;

	if (head->splitatt == 0) 		/* leaf node */
		return(head->leafnum);		/*class stored there */

	else {					/* go to next level of tree*/
		nextnode = head->firstson;
		for (i=2; i<=son; i++) {
	   	  	temp = nextnode->brother;
			nextnode = temp;		
		}
		return(ruleleaf(ex,nextnode) );
	}
}

/* -------------------------------------------------------------------------- */
/* check an example against current rule, returning AGREE or CONTRADICT    */

	int checkrule(ex)
	int ex[];
	{
	int class;

	if (numnodes == 0) return(CONTRADICT);

	class = ruleclass(ex,root);
	if (class == ex[0]) 
		return(AGREE);
	else
		return(CONTRADICT);
	}
/* -------------------------------------------------------------------------- */
/* checks all secondary in current rule & outputs pairs (class,classified as)*/

void printconts(f,leaflabel)

FILE *f ;
int leaflabel;

{
	int i ;
	for (i=numprim+1;i<=numprim+numsec;i++) {
	 	fprintf(f,"%s	%s",classname(*exlist[i]),
			    	    classname(ruleclass(exlist[i],root)));
		if (leaflabel)
			fprintf(f,"%d\n",ruleleaf(exlist[i],root));
		else
			fprintf(f,"\n");
	}
}

/* ------------------------------------------------------------------------- */
/* checks secondary & returns no.of contradictions		   	*/

int checksec(numsrch)
int *numsrch ;
{
	int i,j,guess,srchcnt ;
	j=0;
	srchcnt = 0 ;
	for (i=numprim+1;i<=numprim+numsec;i++)
	{	guess = ruleclass(exlist[i],root) ;
		if (*exlist[i] != guess )
			j++;
		if ( guess == numclass+1)
			srchcnt++ ;
	}
	*numsrch = srchcnt ;
	return(j) ;
}

/*--------------------------------------------------------------------- */
/* incrementaly build a rule 						*/

int buildrule()
{
	int i,numsearch ;
	i=0 ;
	induce();
	while (checksec(&numsearch))
	{	moveconts() ;
		induce() ;
		i++ ;
	}
	storerule(numrules++) ;
	return(i) ;
}

/*----------------------------------------------------------------------------*/
/* moves all contradictions from sec to prim */
 
void moveconts()
{
	int i ;
	for (i=numprim+1;i<=numprim+numsec;i++)
		if ( *exlist[i] != ruleclass(exlist[i],root) )
			move(i) ;
}


/* -------------------------------------------------------------------------- */
/* either stores true class name (if not mainclass) & replaces with "other"   */
/* or replaces true class name with stored class */

void changegs(mainclass)
int mainclass;
{
	int i;

	if (mainclass)
	{	for (i=1;i<=numprim;i++)
		{	*(exlist[i]+numattr+2) = *(exlist[i]) ;
			if (*(exlist[i]) != mainclass)
				classtoken("other",exlist[i]); /* 13/6/86 */
				/* *(exlist[i]) = otherclass ; old way */
		}
	}
	else
	{	for (i=1;i<=numprim;i++)
			*(exlist[i]) = *(exlist[i]+numattr+2) ;
	}
}

/* ------------------------------------------------------------------------- */
/* check new example against existing store (primary and secondary)
    to determine if it is NEW CLASH SAME SUBSET SUPERSET            */

    int checkex(newex, exnumber)
    int newex[];
    int *exnumber;		/* variable to return ex number (if needed) */
    {
    int same, newless, newgtr, diff;	/* counters for examples */
    int i,j;
    int type;
    int new, old;
    int posclash;

    if ( (numprim+numsec) < 1)  return(NEW);	/* first example is new */

    posclash=0;
    for (i=1; i<=numprim+numsec; i++)  {
	same = newless = newgtr = diff = 0;
	for (j=1; j<=numattr; j++)  {
		new = newex[j];
		old = *(exlist[i]+j);

		if 	 (new == old) 		same++;
		else if  (new == DONTCARE) 	newgtr++;
		else if  (old == DONTCARE)	newless++;
		else      			diff++;
		}
	
	if 	(same == numattr)		type = SAME;
	else if ((same+newless) == numattr)	type = SUBSET;
	else if ((same+newgtr ) == numattr)	type = SUPERSET;
	else					type = NEW;

	if  (type != NEW)
	{	if (( newex[0] != *(exlist[i]))&&(type == SAME))
			posclash = 1;
		else
		{	if (newex[0] != *(exlist[i]))
				type = CLASH ;
			*exnumber = i;
			break;
		}
	}
	
    }
    if (posclash)
	if (type!=SAME)
		type = CLASH ;
    return(type);
    }

/* -------------------------------------------------------------------- */

/* add an example to primary or secondary store   */

	void addex(newex,store)
	int newex[];	/* 0=class, 1-numattr=attributes,numattr+1 not used */
	int store;	/* PRIM or SEC */
	{
	int size;
	int *p;
	void *malloc();
	int i;
	int numex;

	numex = numprim + numsec;
	size = (numattr+2+1)*sizeof(numattr);

	if ( (p=(int *)malloc(size)) == NULL)		/* get space from system */
		err("addex:  NULL return from malloc");

	for (i=0; i<=numattr+1; i++)	/* move example to new space */
		*(p+i) = newex[i];

/* now set the example pointers */
	
	if (store == SEC)  {		/* example goes to secondary store */
		exlist[numex+1] = p;
		numsec++;
		}
	else {				/* example goes to primary store */
		for (i=numex; i>numprim; i--)	exlist[i+1] = exlist[i];

		exlist[numprim+1] = p;
		numprim++;
		}

	return;
	}			
/* -------------------------------------------------------------------- */

void del_examp(code)
int code;
{
	int i,j,k;
	int numex;
	int num;
	char line[MAXLIN];

	numex = numprim + numsec;
	if (code==0)
	{	printf("\nrange (\"0 0\" for  none): ");
		num = scanf("%d%d",&i,&j);
		if (num == 0)
			return;
		if (num == 1)
		{   if (i<1 || i>numex ) 
		    	printf("<%d> out of example range (1,%d)\n",i,numex);
		    else
		    {	for (k=i;k<numex;k++)
		         	exlist[k] = exlist[k+1];
		     	printf(" Example #%d deleted\n",i);
		     	if (i <= numprim) 
				numprim--;
		     	else
				numsec--;
		    }
		    return;
		}
	}
	else
	{	if (code==PRIM)
		{	i=1;
			j=numprim;
		}
		else
		{	i=numprim+1;
			j=numprim+numsec;
		}
	}

	if (i<1 || j>numex || j<i) return;

	for (k=i;k<=j; k++)
		free(exlist[k]) ;
	for (k=i;k< numex+i-j;k++)
		exlist[k] = exlist[k+j-i+1];
	if (i<=numprim) 
	{	k = min(j-i+1,numprim-i+1);
		numprim -= k;
		numsec  -= j-i+1-k;
	}		
	else
		numsec -= j-i+1;			
	printf("Examples %d to %d deleted\n",i,j);

}

/* ----------------------------------------------------------------- */
/* delete a class from prim */
void del_class(clasnum)
int clasnum ;
{
	int i,k ;

	for (i=1;i<=numprim;i++)
		if ( *exlist[i] == clasnum )
		{	for (k=i;k<numprim+numsec;k++)
				exlist[k] = exlist[k+1] ;
			numprim-- ;
		}
}

/* ----------------------------------------------------------------------*/
/* expand an example to one example for each value of dontcare attribute */

/* NOTE   THIS ROUTINE SHOULD BE CHANGED TO USE ADDEX() */

void	eg_expand()
	{
	int examp;
	int numargs;
	int i,j,k;
	int numex;
	int attexp;
 	int size;
	int nmove;
	void *malloc();

	size = (numattr + 2)*sizeof(numattr);
	numex = numprim + numsec;
	for (;;)	{	/* get a valid example to expand  */
	    printf("\nexample to expand: ");
	    numargs = scanf("%d",&examp);
	    if (examp<1 || examp>numex) continue;
	    
	    j = 0;			/* write list of dont care attributes */
	    for (i=1; i<=numattr; i++)
	    {
		if( (getattr(examp,i) == DONTCARE) && (att[i].type == LOGICAL) )
		{    j++;
		     printf("%6d  %10.10s\n",i,getaname(i) );  		}
	    }
	    if (j == 0)
	    {printf("No logical \"don't care\" attributes for this example\n");
		continue;						}
	 
	    else	{		/* get attribute number to expand */
	        printf("\nattribute to expand: ");
		numargs = scanf("%d",&attexp);
		if (getattr(examp,attexp) != DONTCARE) continue;
		if (att[attexp].type != LOGICAL)       continue;
		}
	break;
	}

	nmove = att[attexp].nval - 1;
	getattr(examp,numattr+1) /= att[attexp].nval;  /* correct # ex's */

	for (i=numex; i>examp; i--) 	/*  spread examples to make room */
		exlist[i+nmove] = exlist[i];

	getattr(examp, attexp) = 1;	/* first new example is at examp */

	for (i=1; i<=nmove; i++)	/* make the other new examples	 */
	{
		if ( (exlist[examp+i] = malloc(size)) == NULL)  /* get space */
		       err("eg_expand:  NULL return from malloc");

		for (j=0;j<=numattr+1; j++)
		    getattr(examp + i, j) = getattr(examp, j);
		getattr(examp + i, attexp) = i+1;	
	}

	if (examp <= numprim)		/* reset store counters	*/
		numprim += att[attexp].nval - 1;
	else
		numsec  += att[attexp].nval - 1;

	return;
	}
/* ----------------------------------------------------------------------- */

/* move a range of examples from one range to the other			 */

void	move(egno)
	int egno;
	{
	int i1, i2;
	int numex;
	int numargs;
	char line[MAXLIN];
	int nmove;
	int k;

	numex= numprim+numsec ;
	if (egno)
		i1=i2=egno ;
	else
	{
		for (;;)  	    		/* get a legal range to move */
	   	{ printf("\nrange to move: ");
	   	 numargs = scanf("%d%d",&i1,&i2);
	   	 if ( numargs != 2)  continue;
	   	 if (i2<i1 || i1<1 || i2>numex) continue;
	   	 if (i1<=numprim && i2>numprim)  continue;
	   	 break;
		}
	}
	nmove = i2 - i1 + 1;
	if (i1 <= numprim)	/* move from primary to secondary */
	{
	    for (k=0;k<nmove;k++)	/* move examples to end */
	        exlist[numex+1+k] = exlist[i1+k];
	    
	    for (k=i1; k<numex+nmove; k++)   /* compress examples back */
		exlist[k] = exlist[k+nmove];

	    numprim -= nmove;		   /* reset example type counters */
	    numsec  += nmove;
	 printf("\nExamples %d to %d moved from primary to secondary store\n",  
			 i1,i2);
 	}
	else
	{			/* move from secondary to primary */
	    for (k=numex; k>numprim; k--)  /* move sec examples up */
		 exlist[k+nmove] = exlist[k];
	   
	    for (k=i1;k<=i2;k++)	/* move selected examples down */
		exlist[k-i1+numprim+1] = exlist[k+nmove];

	    for (k=i2+1+nmove; k<=numex+nmove; k++)  /* move top sec ex's down */
		exlist[k-nmove] = exlist[k];

	    numprim += nmove;
	    numsec  -= nmove;
	  printf("\nExamples %d to %d moved from secondary to primary store\n",
			i1,i2);
								}
	return;
	}
/* ------------------------------------------------------------------ */

void printexamp(str,titles)

FILE *str;
int titles;

{
	int i,j,top,bot,numargs,topprim;
	char vstring[20],*valstr();

	printf("\nprimary store = 1 to %d",numprim);
	if (numsec)
		printf(".... secondary = %d to %d",numprim+1,numprim+numsec) ;
	printf("\nrange to print: ");
	numargs = scanf("%d %d",&bot,&top) ;
	if (numargs == 1) top=bot;
	if (numargs > 2) return;
	if ((top>numprim+numsec)||(bot>top))
	{	printf("out of range 1 to %d\n",numprim+numsec);
		return ;
	}
	if (top>numprim)
		topprim=numprim ;
	else
		topprim=top;
	if (titles) {	
		fprintf(str,"\n Examples \n\n");
		exprompt(str);
	}
	if ((numprim > 0) && (bot <= numprim))
	{	if (titles)
			fprintf(str," Primary Store\n");
		for (i=bot; i<=topprim;i++) 
	  	{
			if (titles)
	  			fprintf(str,"%4d. ",i);
			for (j=1; j<= numattr; j++)
				if (!att[j].dontuse)
					fprintf(str," %2s",valstr(j,getattr(i,j),vstring)  );
			fprintf(str,"	%s",classname( getclass(i) ) ) ;
			/* until don't cares are done properly ignore next line
			fprintf(str,"	%d",getattr(i,numattr+1)) ;
			*/
			fprintf(str,"\n");
	  	}
	}

	if ((numsec > 0) && (top > numprim))
	{
		if (titles)
			fprintf(str,"\n Secondary Store\n");
	  	for (i=numprim+1; i<=top; i++) 
	  	{	if (titles)
	    			fprintf(str,"%4d. ",i);
			for (j=1;j<= numattr; j++)
				if (!att[j].dontuse)
					fprintf(str," %2s",valstr(j,getattr(i,j),vstring )  );
			fprintf(str,"  %s",classname( getclass(i)) ) ;
			fprintf(str,"\n");
	  	}
	}
	return;
}

/* ---------------------------------------------------------------- */
/* print the list of example numbers  ( for debug purposes ) */
void	printlist(exused)
	int *exused;
	{
	int i;

	printf("%d examples\n", *exused);
	for (i=1; i<=*exused; i++)
		printf("%6d ", *(exused+i) );
	printf("\n");
	return;
	}
/* ----------------------------------------------------------------------- */
