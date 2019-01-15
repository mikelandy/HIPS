/* Copyright (C) 1989, 1991 Aladdin Enterprises.  All rights reserved.
   Distributed by Free Software Foundation, Inc.

This file is part of Ghostscript.

Ghostscript is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY.  No author or distributor accepts responsibility
to anyone for the consequences of using it or for whether it serves any
particular purpose or works at all, unless he says so in writing.  Refer
to the Ghostscript General Public License for full details.

Everyone is granted permission to copy, modify and redistribute
Ghostscript, but only under the conditions described in the Ghostscript
General Public License.  A copy of this license is supposed to have been
given to you along with Ghostscript so you can know your rights and
responsibilities.  It should be in a file named COPYING.  Among other
things, the copyright notice and this notice must be preserved on all
copies.

	ansi2knr.c

	Convert ANSI function declarations & protocols to K&R syntax

  Modified:	Jin Guojun	(K&R and ANSI mixture)
  Date :	Tue, June 1, 1993

  Modified:	Jin Guojun	(Non restricted function style & protocols)
  Date :	Wed, Jan 5, 1994
  Copyright (c)	1994 Lawrence Berkelry Laboratory
*/

#include <stdio.h>
#include <ctype.h>
#include <malloc.h>

#ifdef BSD
#include <strings.h>
#define strchr index
#else
#ifdef VMS
	extern char *strcat(), *strchr(), *strcpy(), *strupr();
	extern int strcmp(), strlen(), strncmp();
#else
#include <string.h>
#endif
#endif

/* Usage:
	ansi2knr input_file output_file
 * If no output_file is supplied, output goes to stdout.
 * There are no error messages.
 *
 * ansi2knr recognizes functions by seeing a non-keyword identifier
 * at the left margin, followed by a left parenthesis,
 * with a right parenthesis as the last character on the line.
 * It will recognize a multi-line header if the last character
 * on each line but the last is a left parenthesis or comma.
 * These algorithms ignore whitespace and comments, except that
 * the function name must be the first thing on the line.
 * The following constructs will confuse it:
	- Any other construct that starts at the left margin and
	    follows the above syntax (such as a macro or function call).
	- Macros that tinker with the syntax of the function header.
 */

/* Scanning macros */
#define isidchar(ch) (isalnum(ch) || (ch) == '_')
#define isidfirstchar(ch) (isalpha(ch) || (ch) == '_')

main(argc, argv)
    int argc;
    char *argv[];
{	FILE *in, *out;
#define bufsize 5000			/* arbitrary size */
	char *buf;
	char *line;
	switch ( argc )
	   {
	default:
		printf("Usage: ansi2knr input_file [output_file]\n");
		exit(0);
	case 2:
		out = stdout; break;
	case 3:
		out = fopen(argv[2], "w");
		if ( out == NULL )
		   {	fprintf(stderr, "Cannot open %s\n", argv[2]);
			exit(1);
		   }
	   }
	in = fopen(argv[1], "r");
	if ( in == NULL )
	   {	fprintf(stderr, "Cannot open %s\n", argv[1]);
		exit(1);
	   }
	fprintf(out, "#line 1 \"%s\"\n", argv[1]);
	buf = malloc(bufsize);
	line = buf;
	while ( fgets(line, (unsigned)(buf + bufsize - line), in) != NULL ) {
retry:		switch ( argc=test1(buf) )
		   {
		case 1:			/* a function */
			convert1(buf, out);
			break;
		case -2:	/* protocols	*/
			for (argc=0; buf[argc-1] != '('; argc++)
				putc(buf[argc], out);
			line = strchr(buf+argc, ')');
			fputs(line, out);
			break;
		case -1:		/* maybe the start of a function */
			line = buf + strlen(buf);
			if ( line != buf + (bufsize - 1) ) /* overflow check */
				continue;
			/* falls through */
		default:		/* not an ANSI function */
			if (argc > 0)	{	/* flush key words	*/
			int	c = buf[argc];
				buf[argc] = 0;
				fprintf(out, "%s", buf);
				buf[argc] = c;
				strcpy(buf, buf+argc);
				goto	retry;
			} else	fputs(buf, out);
			break;
		   }
		line = buf;
	}
	if ( line != buf ) fputs(buf, out);
	free(buf);
	fclose(out);
	fclose(in);
	return 0;
}

/* Skip over space and comments, in either direction. */
char *
skipspace(p, dir)
    register char *p;
    register int dir;			/* 1 for forward, -1 for backward */
{	for ( ; ; )
	   {	while ( isspace(*p) ) p += dir;
		if ( !(*p == '/' && p[dir] == '*') ) break;
		p += dir;  p += dir;
		while ( !(*p == '*' && p[dir] == '/') )
		   {	if ( *p == 0 ) return p;	/* multi-line comment?? */
			p += dir;
		   }
		p += dir;  p += dir;
	   }
	return p;
}

/*
 * Write blanks over part of a string.
 */
int
writeblanks(start, end)
    char *start;
    char *end;
{	char *p;
	for ( p = start; p < end; p++ ) *p = ' ';
	return 0;
}

/*
 * Test whether the string in buf is a function definition.
 * The string may contain and/or end with a newline.
 * Return as follows:
 *	0 - definitely not a function definition;
 *	1 - definitely a function definition;
 *	-1 - may be the beginning of a function definition,
 *		append another line and look again.
 */
int
test1(buf)
    char *buf;
{	register char *p = buf;
	char *bend;
	char *endfn;
	int contin, firstword_iskey;
	static	int	flushed_key;

	if ( !isidfirstchar(*p) )
		return 0;		/* no name at left margin */
	bend = skipspace(buf + strlen(buf) - 1, -1);
	switch (*bend) {
	case ')': contin = 1;	break;
	case '(':
	case ',': contin = -1;	break;
	case ';': contin = -2;	break;	/* for protocols	*/
	default: return 0;		/* not a function	*/
	}
	while ( isidchar(*p) ) p++;
	endfn = p;
	firstword_iskey = is_keyword(buf, endfn - buf);
	if (contin == -2 && !firstword_iskey && !flushed_key)
		return	0;
	if (contin + 1)	flushed_key = 0;	/* no continue on flush	*/
	p = skipspace(p, 1);
	if ( *p != '(' )	{	/* key word + functoin name */
		if (firstword_iskey && strncmp(buf, "return", 6))	{
		    if (*p == '*')	{
			while (*p == '*')	p++;
			endfn = skipspace(p, 1);
			flushed_key = 1;
			return	endfn - buf;	/* flush it anyway	*/
		    }
		    for (endfn=bend=p; *bend; bend++)	{
			if ((flushed_key = *bend == '('))
				return	 endfn - buf;
			if (isspace(*bend) && is_keyword(p, bend - p))
				endfn = ++bend,	p = bend;
		    }
		}
		return	0;	/* not a function	*/
	}
	p = skipspace(p+1, 1);
	if ( *p == ')' )	return 0;	/* no parameter */
	if (*p)	{
		while (!isspace(*p))	p++;
		bend = skipspace(p, 1);
		if (*bend == ')')	/* spaces on both sides	*/
			p = bend + 1;
		if (!*p-- || *p == ',' && !next_is_key(bend - 1, 0))
			return 0;	/* not an ANSI C notation	*/
	/*	if (*p == ')' && (*p == ';' || *p == '\n'))	{	*/
		if (*p == ')' && strncmp(buf, "return", 6))	{
			return	next_is_key(endfn, p);
		}	/* single K&R argu. or others?	*/
	}
	/* Check that the apparent function name isn't a keyword. */
	/* We only need to check for keywords that could be followed */
	/* by a left parenthesis (which, unfortunately, is most of them). */
	if (firstword_iskey)	return	0;
return	contin;
}

is_keyword(buf, len)
{
static char *words[] =	{
	"asm", "auto", "case", "char", "const", "double",
	"extern", "float", "for", "if", "int", "long",
	"register", "return", "short", "signed", "sizeof",
	"static", "switch", "time_t", "typedef", "unsigned",
	"void", "volatile", "while", "FILE", "LOCAL", 0
	};
char **key = words;
char *kp;
	while ( (kp = *key) )	{
		if (strlen(kp) == len && !strncmp(kp, buf, len))
			return	1;	/* name is a keyword */
		key++;
	}
return	0;
}

next_is_key(endfn, p)
char	*endfn, *p;
{
	endfn = skipspace(endfn, 1) + (p != 0);
	if (!p)	{
		p = endfn;
		while (isidfirstchar(*p)) p++;
	}
return	is_keyword(endfn, p - endfn);
}

int
convert1(buf, out)
    char *buf;
    FILE *out;
{	char *endfn = strchr(buf, '(') + 1;
	register char *p;
	char **breaks;
	unsigned num_breaks = 2;	/* for testing */
	char **btop;
	char **bp;
	char **ap;
top:	p = endfn;
	breaks = (char **)malloc(sizeof(char *) * num_breaks * 2);
	if ( breaks == 0 )
	   {	/* Couldn't allocate break table, give up */
		fprintf(stderr, "Unable to allocate break table!\n");
		fputs(buf, out);
		return -1;
	   }
	btop = breaks + num_breaks * 2 - 2;
	bp = breaks;
	/* Parse the argument list */
	do
	   {	int level = 0;
		char *end = NULL;
		if ( bp >= btop )
		   {	/* Filled up break table. */
			/* Allocate a bigger one and start over. */
			free((char *)breaks);
			num_breaks <<= 1;
			goto top;
		   }
		*bp++ = p;
		/* Find the end of the argument */
		for ( ; end == NULL; p++ )
		   {	switch(*p)
			   {
			case ',': if ( !level ) end = p; break;
			case '(': level++; break;
			case ')': if ( --level < 0 ) end = p; break;
			case '/': p = skipspace(p, 1) - 1; break;
			default: ;
			   }
		   }
		p--;			/* back up over terminator */
		/* Find the name being declared. */
		/* This is complicated because of procedure and */
		/* array modifiers. */
		for ( ; ; )
		   {	p = skipspace(p - 1, -1);
			switch ( *p )
			   {
			case ']':	/* skip array dimension(s) */
			case ')':	/* skip procedure args OR name */
			   {	int level = 1;
				while ( level )
				 switch ( *--p )
				   {
				case ']': case ')': level++; break;
				case '[': case '(': level--; break;
				case '/': p = skipspace(p, -1) + 1; break;
				default: ;
				   }
			   }
				if ( *p == '(' && *skipspace(p + 1, 1) == '*' )
				   {	/* We found the name being declared */
					while ( !isidfirstchar(*p) )
						p = skipspace(p, 1) + 1;
					goto found;
				   }
				break;
			default: goto found;
			   }
		   }
found:		if ( *p == '.' && p[-1] == '.' && p[-2] == '.' )
		   {	p++;
			if ( bp == breaks + 1 )	/* sole argument */
				writeblanks(breaks[0], p);
			else
				writeblanks(bp[-1] - 1, p);
			bp--;
		   }
		else
		   {	while ( isidchar(*p) ) p--;
			*bp++ = p+1;
		   }
		p = end;
	   }
	while ( *p++ == ',' );
	*bp = p;
	/* Make a special check for 'void' arglist */
	if ( bp == breaks+2 )
	   {	p = skipspace(breaks[0], 1);
		if ( !strncmp(p, "void", 4) )
		   {	p = skipspace(p+4, 1);
			if ( p == breaks[2] - 1 )
			   {	bp = breaks;	/* yup, pretend arglist is empty */
				writeblanks(breaks[0], p + 1);
			   }
		   }
	   }
	/* Put out the function name */
	p = buf;
	while ( p != endfn ) putc(*p, out), p++;
	/* Put out the declaration */
	for ( ap = breaks+1; ap < bp; ap += 2 )
	   {	p = *ap;
		while ( isidchar(*p) ) putc(*p, out), p++;
		if ( ap < bp - 1 ) fputs(", ", out);
	   }
	fputs(")  ", out);
	/* Put out the argument declarations */
	for ( ap = breaks+2; ap <= bp; ap += 2 ) (*ap)[-1] = ';';
	fputs(breaks[0], out);
	free((char *)breaks);
	return 0;
}
