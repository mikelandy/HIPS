static char *SccsId = "%W%      %G%";

/* -----------------------------------------------------
 * read attribute definition file,
 * check if attributes are OK to use during induction,
 * change the order in which attributes are considered during induction,
 * plus other sundries.
 * Barry Shepherd, 1985.
 * ------------------------------------------------------------------------ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "vacls.def.h"
#include "vacls.ext.h"

extern int nextword();
extern int savestr();
int att_avail();

/* read attributes and classes from stream */
int readattr(stream) 
FILE *stream;
{
	int i, nchar,nc;
	char line[MAXLIN];
	char vstr[MAXLIN];

	if (fscanf(stream,"%d",&numattr) <= 0)
		printf(" Error reading numattr");
	for (i=1;i<=numattr;i++)
	{	fscanf(stream, "%s",line);
		att[i].name = savestr(line);
		att[i].order = i ;
		fscanf(stream, "%s",line);
		if (strcmp(line,"integer") == 0)
		{
			att[i].nval = 2; 
			att[i].firstval = 0;
			att[i].type = INTEGER ;
		}
		else if (strcmp(line,"logical") == 0)
		{
			att[i].type = LOGICAL;
 			fgets(line,MAXLIN,stream);
			nchar = nextword(line,vstr);
			att[i].firstval = savestr(vstr);
			att[i].nval = 1;
			while ((nc=nextword(line+nchar,vstr)) != EOF)
			{
				(att[i].nval)++;
				nchar += nc;
				savestr(vstr) ;
			}
		}
		else 
		{	printf(" Error in type string");
			return(-1);
		}
	}
	return(numattr) ;
}

/* ------------------------------------------------------------------ */
	
/*  read the classes from strm and store them in system allocated space */

void readclass(strm)
FILE *strm;
{
	char line[MAXLIN];
	int numb,i;

	firstclass = nextindex;
	savestr("null");		/* classname(0) will return "NULL" */
	savestr("search") ;
	savestr("other");
	numclass = 2 ;
	classtart = 3 ;

	/*read class names if they have been specified, dont worry if not spec*/
	if ( fscanf(strm,"%d",&numb) == 1) {
		for (i=1;i<=numb;i++) {
			fscanf(strm,"%s",line) ;
			savestr(line);
		}
		numclass += numb ;
	}
}
/* ------------------------------------------------------------------ */
/*  echo print the attributes and classes				*/

void printatt(str,whichones)
FILE *str;
char whichones ;
{
	int first;
	int i,j;

	fprintf(str,"attr:\nnumber name order status type\n");
	for (i=1;i <=numattr;i++)
	{	if ( (whichones == 'a') || (!att[i].dontuse) )
		{	fprintf(str,"%d	%s	%d",i,getstrg(att[i].name),att[i].order);
			if (att[i].dontuse)
				fprintf(str," inactive") ;
			else
				fprintf(str,"   active  ") ;
			if (att[i].type == INTEGER)
				fprintf(str," integer\n");
			else
			{	first = att[i].firstval;
				fprintf(str," logical");
				for (j=first;j<first+att[i].nval;j++)
					fprintf(str,"%10s",getstrg(j) );
				fprintf(str,"\n");
			}
		}
	}

	fprintf(str,"Classes:\n");
	for (i=1;i<=numclass;i++)
		fprintf(str,"%s ",classname(i) );
}

void changeorder(anum,neword)

int anum,neword;

{
	int oldord,i ;

	oldord = att[anum].order ;
	if (neword < oldord)
	{	for (i=1;i<=numattr;i++)
			if ((att[i].order >= neword)&&(att[i].order < oldord))
				att[i].order += 1 ;
	}
	else if (neword > oldord)
	{	for (i=1;i<=numattr;i++)
			if ((att[i].order <= neword)&&(att[i].order > oldord))
				att[i].order -= 1 ;
	}
	att[anum].order = neword ;
}

/* ------------------------------------------------------------------ */
/* prompt user with attribute names					 */

void exprompt(str) 
FILE *str;
{
	int i;
	fprintf(str,"         ");
	for (i=1; i<=numattr; i++)
		if (att_avail(i) && (!att[i].dontuse))
			fprintf(str," %2s",getaname(i) );
	fprintf(str,"    class\n");
	return;
}
/* ----------------------------------------------------------------------- */

/* get integer token value for the class <word>
   return OKAY or ERROR					*/

	int classtoken(word,p)
	char *word;
	int *p;

	{
	int i;

	*p = 0;
	for (i=0; i<=numclass; i++)
		if (strcmp(word,classname(i)) == 0) *p = i;

	if (*p == 0)
		return(ERROR);
	return( OKAY);
	}


/* ------------------------------------------------------------------- */
 
/*  get attribute value string for i'th attribute		       */

char *valstr(i,value,vstring)
char *vstring;
int i,value;
{
	if (value == DONTCARE) 
	    return("-");
	else if ( att[i].type == LOGICAL )   {
	    strcpy(vstring, blockstart + offset[att[i].firstval+value-1]  ) ;
	    return (vstring);
	    }
	else {					/* attribute is INTEGER */
	    sprintf(vstring,"%d",value);
	    return(vstring);
	    }
}

/* ------------------------------------------------------------------- */

/*  get integer token value for index'th attribute, text <word>
    return OKAY or ERROR							*/

	int attrtoken(index, word, ptr)
	char *word;
	int index;
	int *ptr;
	{
	int intval;
	int i;
	char vstring[20];

	*ptr = 0;

	if (att[index].type == LOGICAL)  
	{	for (i=1; i<= att[index].nval; i++)  {
			valstr(index,i,vstring);
			if (strcmp(word, vstring)  == 0) *ptr = i;
			}
		if (word[0] == '-') *ptr = DONTCARE;
		if (*ptr == 0)	return(ERROR);
		else			return( OKAY);
								   }
	else				/* type is INTEGER */
		if (word[0] == '-')   {
			*ptr = DONTCARE;
			return(OKAY);
			}
		else {			/* get integer value string */
		sscanf(word,"%d",&intval);
		*ptr = intval;
		return(OKAY);		}

	}

/* ------------------------------------------------------------------- */
/*  get the attribute number for a name string (NULL if no match   */
	int getanum(attname)
	char *attname;
	{
	int i;
	int anum;
	anum = 0;
	for (i=1;i<=numattr; i++)   {
		if (strcmp(getaname(i),attname) == 0)  {
			anum = i;
			break;  }
	}
	return(anum);
	}
/* --------------------------------------------------------------------- */
  /* determine if attribute is available for splitting for use */

int	att_avail(attr)
	int attr;
	{
	int level;
	int avail;
	avail = YES;
	if (att[attr].dontuse) 		/* makes BS's expts easier */
		return(NO) ;
	if (att[attr].type == INTEGER) return(YES); /* multiple splits for int*/

	for (level=1; level<curlev; level++) {
		if ( usedatt[level] == attr )  avail = NO;
		}
	return(avail);
	}
