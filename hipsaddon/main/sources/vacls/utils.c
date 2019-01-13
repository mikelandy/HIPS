static char *SccsId = "%W%      %G%";

/* -----------------------------------------------------------
 * Odds and ends.
 * Barry Shepherd, 1985.
 * -------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "vacls.def.h"
#include "vacls.ext.h"

void err();

/*  open file for reading, using <prompt> message; return filename and stream */
FILE *filopn(prompt, filename, type)
char *type;		/* type of open ("r", "w", or "a"); */
char *prompt;		/* prompt message			*/
char *filename  ;	/* file name, returned to calling funct */
{
	FILE *stream;

	if (type[0] != 'r' && type[0] != 'w' && type[0] != 'a')
		err("filopn: bad file type");

	printf("\n%s ",prompt);
	scanf("%s", filename);
	if (strcmp(filename,"stdout") == 0)
		stream = stdout;
	else 
		stream = fopen (filename, type);

	if (stream == NULL)
		printf("can't open %s\n",filename);

        return (stream);  
}

/* --------------------------------------------------------------------- */

/*  save <string> in system allocated storage
    (blockstart + offset[savestr] )   points to the saved string
    error halt if unsuccesful storage allocation			*/

int savestr(string) 
char string[];
{
	int slen; 

	slen = strlen(string) + 1;
	if (blocklen == 0) {
		blocklen += slen;
		blockstart = (char *)malloc(blocklen);
		if (blockstart == NULL) err("savestr: malloc returned NULL");
		strcpy( blockstart+blocklen-slen, string );
	}
	else {
		blocklen += slen;
		blockstart = (char *)realloc(blockstart,blocklen);
		if (blockstart == NULL) err ("savestr: realloc returned NULL");
		strcpy( blockstart + blocklen - slen , string );
	}
	offset[nextindex] = blocklen - slen;
	return (nextindex++);
}

/* ----------------------------------------------------------------- */

/*  extract the first <word> set off by white space in <line>
    return number of characters of line used, 
	or EOF if '\n' encountered before non-white characters	*/

	int nextword(line, word)

	char *line;
	char *word;
	{
	char c;
	int i,j;
	/*    skip over leading white space */
	for (i=0;isspace(c=line[i]); i++ )
		if (c == '\n')  return (EOF)	;

	/*   now pick off the word          */
	j = 0;
	do {	
		word[j++] = c;
		c = line[++i] ;
	}
	while (!isspace(c) && (c != '\0'));

	word[j] = '\0';

	return(i);
	}

/* -------------------------------------------------------- */

/*  halt processing after printing <message>				*/

void	err (message)

	char *message;
	{
	printf("\n%s\n",message);
	printf("Processing halted\n");
	exit(1);
	}
/* ---------------------------------------------------------- */
 /*  compare for call from qsort */
	int compar(a,b)
	int a[2],b[2];
	{
	if      (a[1] <  b[1] )  return(-1);
	else if (a[1] == b[1] )  return( 0);
	else			 return (1);
	}
/* --------------------------------------------------------------------- */
	/* gets a line (up to \n) from stream, leaves it in line[] */
int readline(stream, line)
	FILE *stream;
	char *line; {
	register char c, *lptr;

	lptr=line ; 
	do {
	c = *lptr = getc(stream) ; 
	lptr++;
	} while ( (c != '\n') && (c != EOF)) ;
        *(lptr-1) = (char) 0;

	return(strlen(line));
}
