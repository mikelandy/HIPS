/*	HexDUMP . C
%
%	Copyright (c)	Jin Guojun
%
%AUTHOR:	Jin Guojun - LBL	Oct. 30, 1990
*/
#include <ctype.h>
#include "stdef.h"

arg_fmt_list_string	arg_fmt[] =	{
	{"-b[x]", "%i %x", 0, 1, 1, "begin at given position. x = HEX"},
	{"-d[x]", "%i %x", 0, 1, 1,
		"displament relocates the starting position"},
	{"-l", "%-", 0, 1, 0, "tell file size only"},
	{" in -- name or pipe.	out -- standard out.", No, 0, 0, 0,
		"note:	This dump will skip repeating lines."}, NULL	};
char*	Progname;

#define	LineSize	16

#if	defined sparc | defined	NEWisprint
#undef	isprint
#define	isprint(c)	((c) > 0x1F && (c) < 0x7F)
#endif


main(argc, argv)
int	argc;
char **	argv;
{
int	l=0, lns=0, r=0, s=0, offset=0;
char	buf[LineSize], bbuf[LineSize], **fl;
FILE*	in_fp = stdin;
register char*	p = buf, *bp = bbuf;

Progname = *argv;
if (parse_argus(&fl, argc, argv, arg_fmt, &l, &l, &offset, &offset, &l) < 0)
	exit(-1);

if (fl && (in_fp = freopen(fl[0], "rb", stdin)) != stdin)	{
	fprintf(stderr, "can't open %s to read\n", fl[0]);	exit(1);
}

if (l < 0)	{
	fseek(in_fp, 0, 2);
	l = ftell(in_fp);
	fprintf(stderr, "file size is %d[%XH]\n", l, l);
	exit(0);
}
if(l)	offset = l;
if (offset)	fseek(in_fp, offset, 0);
do {
register int	i;
loop:	l += s;	lns++;
	s = fread(p, sizeof(char), LineSize, in_fp);
	if (!s)	goto	prt;
	for (i=0; i < s && p[i] == bp[i]; i++);
	if (i == s)
	{	r++;	goto	loop;	}
	else if (r) {
prt:		printf("** %02X ... skipped %d[%XH] lines\n", bp[0]&0xFF,r,r);
		r = 0;
		}

	for (i=0; i < s && i < 8; i++)	printf("%02X ", p[i] & 0xFF);
	printf("- ");
	while (i < s)	printf("%02X ", p[i++] & 0xFF);
	printf("%2c", ' ');

	for (i=0; i < s; i++)
		if (isprint(p[i]))
			printf("%c", p[i]);
		else	printf(".");
	printf("  %08X\n", l);
	memcpy(bp, p, s);
    }	while(s == 16 && !feof(in_fp));
fclose(in_fp);
printf("Total %d lines<%d bytes>\n", lns, --lns*16+s);

exit(0);
}
