static char *SccsId = "%W%      %G%";

/*---------------------------------------
 * read a rule from a specified file
 * Barry Shepherd, 1985.
 ------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include	<ctype.h>
#include	"vacls.def.h"
#include	"vacls.ext.h"

#define		MAXATTVALEN	10
#define		MAXATTNAMLEN	10
#define		MAXPATHS	100
#define	COMMITTEE	1
#define	STRINGIO	1 

#ifdef STRINGIO
/*-----------------------------------------------------------------------*/
/* storage for rules expressed as linear strings */

#define		LOWER		0
#define		UPPER		1

int	ruleline[MAXATTR+1][2] ;
#endif

/* ----------------------------------------------------------------------*/
/* tree stats storage */

int	depth,path[MAXPATHS],leafclass[MAXCLASS+1],usedatts[MAXATTR+1] ;
float	avuniqlen,avpathlen ;
/* ----------------------------------------------------------------------*/

typedef	struct	node	*nodeptr ;

int getanum(),classtoken(),savestr(),uniqlen(),readline();
int getrange(),ruleclass(),ruleleaf();
void buildline(),strgout(),copyline(),loadnode(),getsons(),setrule();

/* -----------------------------------------------------------------------*/
/* depth should be init by the calling fn */

struct node *parserule(sfp,nodecnt)
int *nodecnt ;
FILE *sfp ;
{	
	int i,attnum,searchcnt;
	static int curleafnum ;
	char avalnam[MAXATTVALEN],attrnam[MAXATTNAMLEN] ;
	char searchstr[6] ;
	struct node *thisnode,*lastchild,*childnode;

	if (!depth)
		curleafnum = 0 ;
	if (fscanf(sfp,"%s",attrnam)==1)	/*get attr or class name*/
	{	thisnode= (struct node *)malloc(nodesize) ;
		*nodecnt += 1 ;
		thisnode->brother = NULL ;
		thisnode->firstson= NULL ;
		if (attrnam[0] == '[')		/* [ => attr, else class*/
		{	i=1 ;
			while (attrnam[i++] != ']') ;
			attrnam[--i] = '\0' ;
			attnum = getanum(&attrnam[1]) ;
			thisnode->splitatt = attnum ;

			/* tree stats */
			depth++ ; 	/* ie path[0] never used! */
			path[depth] = attnum ;
			usedatts[attnum] += 1 ;

			for (i=0;i<att[attnum].nval;i++)
			{	if (fscanf(sfp,"%s %*s",avalnam)==1) /*get att value,ignore colon */
				{	if (avalnam[0]=='<')
						sscanf(&avalnam[1],"%d",&thisnode->splitval);
				}
				else
				{	printf("unexpected rule end!\n") ;
					return(NULL) ;
				}
				childnode = parserule(sfp,nodecnt) ;
				if (i)
					lastchild->brother=childnode;
				else
					thisnode->firstson = childnode ;
				lastchild = childnode ;
			}
			depth-- ;
		}
		else
		{	thisnode->splitatt = 0 ;
			if (! strcmp(attrnam,"search"))
			{	fscanf(sfp,"%s",searchstr) ;
				thisnode->splitval = -2 ;
			}
			else
			  if (classtoken(attrnam,&thisnode->splitval) == ERROR) {
				/* this class does not occur in the avf.
				   Because savestr is only used to store
				   the attribute & class names and since
				   the class names are last we can append
				   this new name to the end of this storage */
				savestr(attrnam);
				numclass++ ;	/* this is also the index */
				classtoken(attrnam,&thisnode->splitval) ;
			  }
			/*printf("%d ",depth) ;*/
			avpathlen += depth ;
			avuniqlen += uniqlen(path,depth) ;
			leafclass[thisnode->splitval] += 1 ;
			thisnode->leafnum = curleafnum++ ;
		}
		return(thisnode) ;
	}
	else
	{	if (*nodecnt)
			printf("unable to read attribute at node %d!\n",*nodecnt) ;
		return(NULL) ;
	}
}

#ifdef STRINGIO

int makestrings(root,stream)
struct node *root ;
FILE *stream ;
{
	int numstrgs,i ;

	if (root->splitatt == 0)
	{	fprintf(stream,"%s\n",classname(root->splitval)) ;
		return(1) ;
	}
	else
	{	numstrgs= 0;
		for (i=0;i<numattr+1;i++)
		{	ruleline[i][0] = 0 ;
			ruleline[i][1] = 0 ;
		}
		buildline(root->firstson,root,&numstrgs,stream) ;
		return(numstrgs) ;
	}
}


void buildline(thisnode,parent,strnum,stream)
int *strnum ;
FILE *stream ;
struct node *thisnode, *parent ;
{
	int templine[MAXATTR+1][2] ;
	int i ;

	copyline(ruleline,templine) ;

	if (parent)				/* ie if 1st son */
		ruleline[parent->splitatt-1][UPPER]= parent->splitval;

	if (thisnode->splitatt != 0)		/* ie if an attribute */
		buildline(thisnode->firstson,thisnode,strnum,stream) ;
	else					/* its a class */
		if (thisnode->splitval && (thisnode->splitval <= numclass))
		{	ruleline[numattr][0]= thisnode->splitval ;
			*strnum += 1 ;
			printf("s%d: ",*strnum) ;
			strgout(stream,ruleline) ;
		}

	if (thisnode->brother)			/* if it has a brother */
	{	copyline(templine,ruleline) ;
		ruleline[parent->splitatt-1][LOWER]= parent->splitval;
		buildline(thisnode->brother,0,strnum,stream) ;
	}
}

void copyline(sor,dest)
int sor[MAXATTR+1][2],dest[MAXATTR+1][2] ;
{
	int i ;
	for (i=0;i<numattr+1;i++)
	{	dest[i][0] = sor[i][0] ;
		dest[i][1] = sor[i][1] ;
	}
}

void strgout(stream,strg)
FILE *stream ;
int strg[MAXATTR+1][2] ;
{	int i ;
	for (i=0;i<numattr;i++)
	{	if (strg[i][LOWER])
			fprintf(stream," %d<=",strg[i][LOWER]) ;
		else
			fprintf(stream," ") ;
		if (strg[i][LOWER] || strg[i][UPPER])
			fprintf(stream,"%s",getaname(i+1)) ;
		if (strg[i][UPPER])
			fprintf(stream,"<%d  ",strg[i][UPPER]);
	}
	fprintf(stream,"	%s\n",classname(strg[numattr][LOWER])) ;
}

struct node *readstring(f,nodecnt)
FILE *f ;
int *nodecnt ;
{	int attnum,upper,lower,pos,i,temp;
	char line[80],attnam[10] ;
	struct node *root,*thisnode,*son1,*son2 ;

	if (readline(f,line)!=0)
	{
		thisnode= (struct node *)malloc(nodesize) ;
		thisnode->brother= NULL ;
		thisnode->firstson= NULL;
		root = thisnode ;
		*nodecnt = 1 ;
		pos = i = 0 ;

		while ((i = getrange(&line[pos +=i],&lower,attnam,&upper)))
		{
			attnum= getanum(attnam) ;
			if (lower)
			{	loadnode(thisnode,attnum,lower) ;
				getsons(thisnode,&son1,&son2) ;
				classtoken("other",&temp);
				loadnode(son1,0,temp) ;
				thisnode = son2 ;
				*nodecnt += 2;
			}
			if (upper)
			{	loadnode(thisnode,attnum,upper) ;
				getsons(thisnode,&son1,&son2) ;
				classtoken("other",&temp);
				loadnode(son2,0,temp) ;
				thisnode = son1 ;
				*nodecnt += 2 ;
			}
		}
		thisnode->splitatt = 0 ;
		classtoken(attnam,&thisnode->splitval) ;
		return(root) ;
	}
	return(NULL) ;
}

void getsons(parent,son1,son2)
nodeptr	parent, *son1, *son2 ;
{
	*son1 = (struct node *)malloc(nodesize) ;
	*son2 = (struct node *)malloc(nodesize) ;
	parent->firstson = *son1 ;
	(*son1)->brother = *son2 ;
	(*son1)->firstson = NULL ;
	(*son2)->brother = NULL ;
	(*son2)->firstson = NULL ;
}

void loadnode(nod,att,val)
struct node *nod ;
int att,val ;
{	nod->splitatt = att ;
	nod->splitval = val ;
}

int getrange(strg,lower,name,upper)
char *strg,*name ;
int *lower,*upper;
{
	int i ;
	*lower= *upper = i = 0 ;
	while ((strg[i]==' ')||(strg[i]=='	')) i++ ;
	while ((strg[i]!=' ')&&(strg[i]!='	')&&(strg[i]!='\0'))
	{
		if (strg[i] == '<')
		{	strg[i] = ' ' ;
			*upper = 1;
		}
		else
			if (strg[i] == '=')
			{	strg[i] = ' ' ;
				*lower = 2 ;
				*upper = 0 ;
			}
		i++ ;
	}
	switch (*lower + *upper)
	{
		case 3:
			sscanf(strg,"%d %s %d",lower,name,upper) ;
			break ;
		case 2:	
			sscanf(strg,"%d %s",lower,name) ;
			break ;
		case 1:
			sscanf(strg,"%s %d",name,upper) ;
			break ;
		case 0:
			sscanf(strg,"%s",name);
			return(0) ;
			break ;
		default:
			printf("getrange:unexpected case!\n") ;
			return(0) ;
	}
	return(i) ;
}
#endif

#ifdef COMMITTEE
void committee(start,fin,f,leaflabel)
int start,fin,leaflabel;
FILE *f ;
{
	int i,j,k,best;
	int classcnt[MAXCLASS] ;
	for (i=numprim+1;i<=numprim+numsec;i++)
	{
		for (j=0;j<=numclass;j++)
			classcnt[j] = 0 ;
		for (j=start;j<=fin;j++)
		{	setrule(j) ;
			classcnt[ruleclass(exlist[i],root)] += 1 ;
		}
		k=0;
		for (j=1;j<=numclass;j++)
			if (classcnt[j])
				if (strcmp("other",classname(j))) {
					k += 1;
					best= j;
				}
		fprintf(f,"%s	",classname(*exlist[i])) ;
		switch (k)
		{
			case 0:
				fprintf(f,"null\n") ;
				break ;
			case 1:
				fprintf(f,"%s",classname(best));
				if (leaflabel) {
					setrule(k);
					fprintf(f,"%d\n",ruleleaf(exlist[i],root));
				}
				else
					fprintf(f,"\n") ;
				break ;
			default:
				if ( classcnt[*exlist[i]] )
					fprintf(f,"gdsrch	%d",k);
				else
					fprintf(f,"bdsrch	%d",k);
				for (j=1;j<=numclass;j++)
				  if (classcnt[j])
			  	  	fprintf(f,"	%s",classname(j));
				fprintf(f,"\n") ;
				break ;	

		}
	}
}
#endif

int uniqlen(path,len)
int *path,len ;
{
	int i,ulen,attcnt[MAXATTR+1] ;

	ulen = 0 ;
	for (i=1;i<=numattr;i++)
		attcnt[i] = 0 ;
	for (i=1;i<=len;i++)
		attcnt[path[i]] += 1 ;
	for (i=1;i<=numattr;i++)
		if (attcnt[i])
			ulen++ ;
	return(ulen) ;
}
