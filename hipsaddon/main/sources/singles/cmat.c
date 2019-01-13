static char *SccsId = "%W%      %G%";

/* confusion matrix for acls misclassifications */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAXNOCLASSES 120
#define NAMSIZE	20
#define TRUE	1
#define FALSE	0

char classnam[MAXNOCLASSES][NAMSIZE] ; 
int numatts,numclasses ;

/* the next line was a local variable to main!
 * cc complained of too many local vars so I made it a global
 * (not in main's stack frame anymore) and it seems to compile ok.
 * ! -- js 14/10/87
 */
int matrix[MAXNOCLASSES][MAXNOCLASSES] ;
int getline2();
void readattfile();

int main(argc,argv)
int argc ;
char *argv[] ;

{
	FILE *sfp ;
	int corrgtot,incorrgtot,nullgtot,gsrchgtot,bsrchgtot ;
	int i,j,sind,dind,null,summary,resonly,errorsonly ;
	int badeg,desnull,badsrch,gudsrch ;
	int confcnt[MAXNOCLASSES],miscnt ;
	char sorcla[9],descla[9],*s ;
	char line[80] ;
	int corrtot[MAXNOCLASSES],incorrtot[MAXNOCLASSES],nulltot[MAXNOCLASSES],bsrchtot[MAXNOCLASSES],gsrchtot[MAXNOCLASSES];
	float sucrate,failrate,nullrate,bsrchrate,gsrchrate,tempfl ;

	while (--argc > 0 && (*++argv)[0] == '-')
		for (s=argv[0]+1;*s!='\0';s++)
			switch (*s) {
				case 'n':
					null=1 ;
					break ;
				case 's':
					summary=1;
					break ;
				case 'r':
					resonly=1 ;
					break ;
				case 'e':
					errorsonly=1;
					break ;
				}

	if (argc < 1)
	{	printf("no enough args\n") ;
		exit(-1) ;
	}
	if ((sfp=fopen((argv++)[0],"r"))==NULL)
	{	printf("can't open %s\n",argv[0]) ;
		exit(-1) ;
	}
	readattfile(sfp) ;
	if (argc == 2)
	{	if ((sfp=fopen(argv[0],"r")) == NULL)
		{	fprintf(stderr,"can't open %s\n",argv[0]) ;
			exit(-1) ;
		}
	}
	else
		sfp= stdin ;
	
	for (i=0;i<MAXNOCLASSES;i++)
	{	for (j=0;j<MAXNOCLASSES;j++)
			matrix[i][j] = 0 ;
		corrtot[i] = 0 ;
		incorrtot[i] = 0 ;
	}

	while (getline2(sfp,line)!=0) 
	{	sscanf(line,"%s	%s",sorcla,descla); 
		for (i=0;i<numclasses;i++)
			if (!strcmp(sorcla,classnam[i]))
			{	sind = i ;
				i = numclasses + 2 ;
			}
		if (i == numclasses)
		{	strcpy(classnam[i],sorcla);
			numclasses++ ;
			sind = i ;
		}

/* old way.................................................
		{	printf(stderr,"bad source class found in input file: %s\n",sorcla) ;
		}
...........................................................*/

		for (j=0;j<numclasses;j++)
			if (!strcmp(descla,classnam[j]))
			{	dind = j ;
				j = numclasses + 2 ;
			}
		desnull = FALSE ;
		badsrch = FALSE ;
		gudsrch = FALSE ;
		badeg   = FALSE ;

		if (j == numclasses)
		{	if (!strcmp(descla,"null"))
				desnull = TRUE ;
			else
			{	if (!strcmp(descla,"bdsrch"))
					badsrch = TRUE ;
				else
				{	if (!strcmp(descla,"gdsrch"))
						gudsrch = TRUE ;
					else
					{	strcpy(classnam[j],descla);
						numclasses++ ;
						dind = j ;
						matrix[sind][dind] += 1 ;
					}
/*
					{	fprintf(stderr,"bad result class found in input file: %s\n",descla) ;
						badeg = TRUE ;
					}
*/
				}
			}

		}
		else
			matrix[sind][dind] += 1 ;

		if (!badeg)
		{	if (desnull)
			{	nulltot[sind] += 1;
				nullgtot += 1;
			}
			else
			{	if (badsrch)
				{	bsrchtot[sind] += 1 ;
					bsrchgtot += 1;
				}
				else
				{	if (gudsrch)
				 	{	gsrchtot[sind] += 1;
						gsrchgtot += 1;
				 	}
					else
				 	{	if (sind == dind)
						{	corrtot[sind] += 1 ;
							corrgtot += 1 ;
						}
						else
						{	incorrtot[sind] += 1 ;
							incorrgtot += 1;
						}
					}
				 }
			}
		}
	}
  


	printf("CONFUSION MATRIX....\n\n") ;
	if (!resonly)
	{
	for (j=0;j<numclasses;j++)
		confcnt[j] = 0 ;

	for (j=0;j<numclasses;j++)
		for (i=0;i<numclasses;i++)
			if (matrix[i][j] && ( i!=j) )
				confcnt[j] += 1 ;
	if (!summary)
		for (i=0;i<numclasses;i++) {
			if (confcnt[i]) {
				printf("	%s",classnam[i]) ;
			}
			else if (!errorsonly) {
				printf("	%s",classnam[i]) ;
			}
		}

	printf("	%%suc   COR INC") ;
	if (null)
		printf(" NUL BSH GSH") ;
	for (i=0;i<numclasses;i++)
	{	if (!summary)
		{	miscnt = 0 ;
			for (j=0;j<numclasses;j++)
				if (matrix[i][j] && (j!=i))
					miscnt += 1 ;
			if (nulltot[i]||bsrchtot[i]||gsrchtot[i])
				miscnt += 1 ;
			if ((miscnt)||(!errorsonly))
			{	printf("\n%s	",classnam[i]) ;
				for (j=0;j<numclasses;j++)
				{  if ((matrix[i][j] && (j!=i))||(!errorsonly))
						printf("%d	",matrix[i][j]);
				   else
						if (confcnt[j])
							printf(".	");
				}
			}
		}
		else
			printf("\n%s	",classnam[i]) ;
		if ( (summary) || (miscnt) || (!errorsonly))
		{	if ((tempfl= corrtot[i]))
				printf("%5.1f",100*tempfl/(corrtot[i]+incorrtot[i]+nulltot[i]+bsrchtot[i]+gsrchtot[i])) ;
			else
				printf("%5.1f",tempfl) ;
			printf("   %2d   %2d",corrtot[i],incorrtot[i]) ;
			if (null)
				printf(" %2d   %2d   %2d",nulltot[i],bsrchtot[i],gsrchtot[i]) ;
		}
	}
	}
	printf("\n\ntotals:		correct   %d	incorr. %d",corrgtot,incorrgtot) ;
	if (null)
		printf("	null	%d	bsearch	%d	gsearch	%d",nullgtot,bsrchgtot,gsrchgtot) ;

	tempfl = corrgtot + incorrgtot + nullgtot + bsrchgtot + gsrchgtot ;
	sucrate = (corrgtot*100)/tempfl ;
	failrate = (incorrgtot*100)/tempfl ;
	nullrate = (nullgtot*100)/tempfl ;
	bsrchrate = (bsrchgtot*100)/tempfl ;
	gsrchrate = (gsrchgtot*100)/tempfl ;

	printf("\n%% of total:	success %3.2f %%	failure	%3.2f %%",sucrate,failrate) ;
	if (null)
	{	printf("	null	%3.2f %%",nullrate) ;
		printf("	bsearch	%3.2f %%",bsrchrate) ;
		printf("	gsearch	%3.2f %%\n",gsrchrate) ;
	}
}

/* gets a line (up to \n) from stream, leaves it in line[] */

int getline2(stream, line)
	FILE *stream;
	char *line; {
	register char c, *lptr;

	lptr=line ; 
	do {
	c = *lptr = getc(stream) ; 
	lptr++;
	} while ( (c != '\n') && (c != EOF)) ;
        *(lptr-1) = (char)0;

	return(strlen(line));
}

void readattfile(fp)
FILE *fp ;
{
	int i,j,k ;
	int endofline,instring ;
	char temp[20] ;
	fscanf(fp,"%d",&numatts) ;
	for (i=0;i<numatts;i++)
	{	fscanf(fp,"%*s %s",temp) ;
		if (strcmp(temp,"integer"))
		{	endofline = FALSE ;
			instring = FALSE ;
			j=k=0 ;
			do
			{	fscanf(fp,"%c",temp);
				if (temp[0] == '\n')
					endofline = TRUE ;
				else
					if ((temp[0]==' ')||(temp[0]=='\t'))
					{	if (instring)
						{	instring = FALSE ;
							k = 0 ;
							j++  ;
						}
					}
					else
						instring = TRUE ;
			}
			while (!endofline) ;
		}
	}
	fscanf(fp,"%d",&numclasses) ;
	for (i=0;i<numclasses;i++)
	{	fscanf(fp,"%s",classnam[i]) ;
		printf("%s ",classnam[i]) ;
	}
	fclose(fp) ;
}
