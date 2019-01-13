/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * update_hdr.c - HIPL Picture Format sequence history update
 *
 * Michael Landy - 2/2/82
 * modified for HIPS 2 - msl 1/3/91
 */

#include <stdio.h>
#include <hipl_format.h>

long time();
char *ctime();

int update_header(hd,argc,argv)

struct header *hd;
int argc;
char **argv;

{
	return(update_headerc(hd,argc,argv,TRUE));
}

int update_headern(hd,argc,argv)

struct header *hd;
int argc;
char **argv;

{
	return(update_headerc(hd,argc,argv,FALSE));
}

int update_headerc(hd,argc,argv,pflag)

struct header *hd;
int argc;
h_boolean pflag;
char **argv;

{
	int ac,len,i,len2;
	char *s,*s2;
	char **av;
	long tm;

	if (TestTime() != 0){
	  fprintf(stderr,"Permission denied\n");
	  exit(HIPS_ERROR);}
	av = argv;
	ac = argc;
	len = 38 + hd->sizehist;
	while (ac--)
		len += strlen(*(av++)) + 1;
	if ((s = (char *) hmalloc(len)) == (char *) HIPS_ERROR)
		return(HIPS_ERROR);
	strcpy(s,hd->seq_history);
	len = hd->sizehist - 1;
	if (s[len] != '\n') {
		s[0] = '\0';
		len = 0;
	}
	else if (pflag) {
		s[len++] = '|';
		s[len++] = '\\';
		s[len++] = '\n';
		s[len] = '\0';
	}
	else
		len++;
	s2 = s + len;
	ac = argc;
	av = argv;
	while (ac--) {
		strcat(s2,*(av++));
		strcat(s2," ");
	}
	strcat(s2,"\"-D ");
	tm = time(0);
	strcat(s2,ctime(&tm));
	len2 = strlen(s2);
	for (i=0;i<len2;i++)
		if (s2[i]=='\n')
			s2[i] = ' ';
	s2[len2-1] = '\"';
	s2[len2] = '\n';
	s2[len2+1] = '\0';
	hd->sizehist = len+len2+1;
	if (hd->histdealloc)
		free(hd->seq_history);
	hd->seq_history = s;
	hd->histdealloc = TRUE;
	return(HIPS_OK);
}
