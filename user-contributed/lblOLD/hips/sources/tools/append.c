/*	Copyright (c)	1990	Jin, Guojun

%	APPEND .C
%	AUTHOR	Jin Guojun	Oct. 30, 1990
%
% important note:
%	APPEND can pipe in but can not pipe out. i.e., output
%	must spscity a file_name. It can not be a ">" or "|" symbol.
*/

#include <stdio.h>
#ifndef	BufSize
#define	BufSize	40960
#endif
#define	not_string(n, s)	strcmp(argv[n], s)
#define	is_string(n, s)		!not_string(n, s)

main(argc, argv)
int	argc;
char*	argv[];
{

FILE	*in_fp=stdin, *out_fp;
char	buf[BufSize], *ifname=0;
register unsigned	i=2, j;

if (argc < i || is_string(1, "-h"))
usg:
   {	fprintf(stderr, "Usage: %s outfile -r repeat -s string\n", *argv);
	fprintf(stderr, "	%s outfile [< | -f source_file] [-t title]\n", *argv);
	fprintf(stderr, "	%s -h\n", *argv);
	exit(1);
   }

if ((out_fp = fopen(argv[1], "ab")) == NULL)
   {	perror(argv[1]);	exit(2);	}

if (*argv[i] != '-')
	ifname = argv[i];
if (ifname){
	if ((in_fp=freopen(ifname, "rb", stdin)) == 0){
		perror(ifname);
		exit(3);
	}
	goto	apnd;	/*	case 3	*/
}

if (not_string(i, "-r"))	{	/* not case 1	*/
   if (not_string(i, "-f"))	goto	if_ttl;
   {
	in_fp = fopen(argv[++i], "rb");	/*	case 2	*/
	if (!in_fp)
		{
		fprintf(stderr, "can open file %s for input\n", argv[3]);
		exit(-1);
		}
	i++;
if_ttl:	if (is_string(i, "-t") && argc > ++i)	{
		strcpy(buf, "\nTitle: ");
		strcat(buf, argv[i]);
		fwrite(buf, strlen(buf), 1, out_fp);
	}
	fprintf(out_fp, "\n");
apnd:	do{
		i = fread(buf, 1, BufSize, in_fp);
		j = fwrite(buf, 1, i, out_fp);
		if (j != i){
			perror("wrtie");
			exit(5);
		}
	}while (!feof(in_fp) && i == BufSize);
   }
} else	if (not_string(4, "-s"))	goto	usg;
	else {
		j = strlen(argv[5]);		/*	case 2	*/
		for (i = 0; i < j; i++)
			*(buf + i) = argv[5][i] - '0';
		*(buf + i) = 0;
	/*	fprintf(stderr, "tst: %s %s\n", argv[5], buf);	*/

		for (i = atoi(argv[3]); i--;)
			fwrite(buf, j, 1, out_fp);
	}
fclose(out_fp);
exit(0);
}
