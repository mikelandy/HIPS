/*	Z_REOPEN . c
#
%	Copyright (c)	Jin Guojun
%
%	open file with .Z (LZW compress) handling.
%	if filename with suffix .Z, then zcat will be invoked, and either pipe
%	or a temp file will be used;  otherwise, stdin will be reopened.
*/

#include <stdio.h>
static char	tmp_file[] = "/tmp/ztmp",
	*decomp_cmd[] = {"zcat %s", "gzip -d < %s"};

FILE	*
zreopen(char *fn, int *zret, char *tmp)
{
FILE*	fp;
char	sbuf[128];
int	state = !strcmp(fn + strlen(fn) - 2, ".Z");
	if (!state && !strcmp(fn + strlen(fn) - 3, ".gz"))
		state=2;
	if (zret)	*zret = state;
	if (state--)	{
#	ifdef	NO_POPEN
		if (!tmp)	tmp = tmp_file;
		sprintf(sbuf, "zcat %s > %s", fn, tmp);
		if (system(sbuf) < 0)	return	0;
		fn = tmp;
	}
#	else
		sprintf(sbuf, decomp_cmd[state], fn);
		fp = popen(sbuf, "r");
	}
	else
#	endif
		fp = freopen(fn, "rb", stdin);
return	fp;
}

#ifdef	NO_POPEN
zclean(char* tmp)
{
if (!tmp)	tmp = tmp_file;
	chmod(tmp, S_IWRITE);
return	unlink(tmp);
}
#endif
