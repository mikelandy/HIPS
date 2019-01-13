static char *SccsId = "%W%      %G%";


/* -------------------------------------------------------------------  */
#include <stdio.h>
#include <ctype.h>
#include "aclsdef.h"
#include "aclsext.h"

int	nodesofar ;
float	treeinf ;

/* ---------------------------------------------------------------------- */
	printnodes(strm)		/* print all the nodes */
	FILE *strm;
	{
	int i;
	int a;

	fprintf(strm," %d nodes in tree\n\n", numnodes);

	for (i=1; i<=numnodes; i++)   {
		fprintf(strm," %9u%4d. ",  nodptr[i], i);/* node address, num */
		a = nodptr[i]->splitatt;
		if ( a > 0 )
		    fprintf(strm,"  %-12s  ",getaname(a) ); /* split attrib */
 	        else
		    fprintf(strm,"  %12s  ", "LEAF");

		if ( a == 0 )			/* leaf node */
			fprintf(strm,"%-10s", classname(nodptr[i]->splitval ));
		else				/* test node */
			fprintf(strm,"%10d", nodptr[i]->splitval);

		fprintf(strm,"%8u%8u%7d\n",nodptr[i]->firstson,nodptr[i]->brother,
							 nodptr[i]->nexamp);
		}
		
	return;
	}

/* --------------------------------------------------------------------- */
/* print the tree below node */

void printtree(topnode, parnode, parattr, aval, level,strm,uniqueclass)
struct node *topnode;	/* points to top of tree to print */
struct node *parnode;   /* points to parent node */
int parattr;		/* attribute which topnode's parent split */
int aval;		/* attribute value of parattr for topnode */
int level;		/* nesting level of topnode (root=1)      */
int uniqueclass;	/* if set all leaves will have a unique label..
			   if (for eg) a class label is "apple" then the apple
			   leaves will be "apple1","apple2","apple3".... */
FILE *strm;
{
	struct node *temptr ;
	char str1[20];
	char str2[20];
	int i;
	int leaf[MAXCLASS]; /* for use if uniqueclass flag set */	
	float infgain ;

	if ( numnodes < 1)
	{	fprintf(strm," no rule exists \n");
		return(0) ;
	}
	else if (numnodes == 1)
		fprintf(strm,"%s\n",classname(topnode->splitval)) ;

	else if (topnode == root)	/*  root, just write out attribute */
		{
		fprintf(strm,"[%s]",getaname(topnode->splitatt));
		level = 1;
		nodesofar=1 ;
/*		infgain = topnode->pent - topnode->avent;
		treeinf = infgain ;
  		fprintf(strm,"\n%80s"," ");
		fprintf(strm,"e%d c%0.1f p%0.2f c%0.2f d%0.2f",topnode->numegs,topnode->chi,topnode->pent,topnode->avent,infgain) ;
		fprintf(strm," it%0.2f %0.2f",treeinf,treeinf/nodesofar) ; */ 
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
		if ( (i=topnode->splitatt) != NULL) 	/* if test node */
		{	nodesofar++ ;
			fprintf(strm,"[%s]",getaname(i));
		/*	infgain = topnode->pent - topnode->avent;
			treeinf += (topnode->numegs*infgain)/numprim ;
		  	fprintf(strm,"\n%80s"," ") ;
			for (i=0;i<level;i++)
				fprintf(strm," ") ;
			fprintf(strm,"e%d c%0.1f p%0.2f c%0.2f d%0.2f ",
					topnode->numegs,topnode->chi,
					topnode->pent,topnode->avent,infgain) ;
			fprintf(strm,"it%0.2f %0.2f",treeinf,treeinf/nodesofar);
		*/ 
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
				/* next bit added 16/6/86 to enable all leaves 
				   to be given a different label, this may 
				   enable within-class patterns to be detected 
				   during testing */
				if (uniqueleaves)
				   fprintf(strm,"%d",leaf[topnode->splitval]++);
			}

		/*	{	fprintf(strm,"%s %d=%d%% ",
					classname(topnode->splitval),
					topnode->count[topnode->splitval],
					(100*topnode->count[topnode->splitval])/topnode->count[0]);
				for (i=1;i<=numclass;i++)
					if ((topnode->count[i])&&(i!=topnode->splitval))
					  fprintf(strm,"%s %d=%d%% ",
						classname(i),
						topnode->count[i],
						(100*topnode->count[i])/topnode->count[0]);
			}
		 */
	     }
	 fprintf(strm,"\n");
	
	if ((topnode->firstson != NULL)&&(topnode->splitatt != NULL))		/* there is a subtree */
	printtree(topnode->firstson,topnode,topnode->splitatt,1,level+1,strm);
	
	if (topnode->brother != NULL)		/* there are brother(s) */
           printtree(topnode->brother,parnode, parattr, aval+1, level,strm);

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

		if (topnode->splitatt != NULL)	/* if not a leaf */
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
