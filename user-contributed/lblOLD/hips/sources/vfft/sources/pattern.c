/*
%	PATTERN . C
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

This software is copyright (C) by the Lawrence Berkeley Laboratory.
Permission is granted to reproduce this software for non-commercial
purposes provided that this notice is left intact.

It is acknowledged that the U.S. Government has rights to this software
under Contract DE-AC03-765F00098 between the U.S.  Department of Energy
and the University of California.

This software is provided as a professional and academic contribution
for joint exchange. Thus, it is experimental, and is provided ``as is'',
with no warranties of any kind whatsoever, no support, no promise of
updates, or printed documentation. By using this software, you
acknowledge that the Lawrence Berkeley Laboratory and Regents of the
University of California shall have no liability with respect to the
infringement of other copyrights by any part of this software.

For further information about this notice, contact William Johnston,
Bld. 50B, Rm. 2239, Lawrence Berkeley Laboratory, Berkeley, CA, 94720.
(wejohnston@lbl.gov)

For further information about this software, contact:
	Jin Guojun
	Bld. 50B, Rm. 2275, Lawrence Berkeley Laboratory, Berkeley, CA, 94720.
	g_jin@lbl.gov

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%	pattern generates the variant square pattern.
*/
char	usage[]="options\n\
[-c #]		columns [256]\n\
[-r #]		rows [256]\n\
[-fr #]		frames for repeating output [1]\n\
[-h #]		line height [8]\n\
[-fx #]		function number on X axis\n\
[-fy #]		function number on Y axis\n\
[-p #]		circles of sine function\n\
[-s #]		step of equal difference [2]\n\
[-w #]		line width [8]\n\
[<] input [> output]\n\n\
functions:\n\
0	square - n, n, n\n\
1	step - 1,2,3, ...\n\
2	equal difference - 1, 4, 7, 10, ... {ed=3 => -s 3}\n\
3	pascal tri-angle\n\
4	power of 2\n\
5	sine\n";
/*
% compile:	cc -O -o DEST/pattern pattern.c -lscs1 -lccs -lhips -lm
%
% AUTHOR:	Guojun Jin - LBL	5/1/91
*/

#include <math.h>
#include "header.def"
#include "imagedef.h"

U_IMAGE	uimg;

#define	inbuf	uimg.src
#define	obuf	uimg.dest
#define	row	uimg.height
#define	cln	uimg.width
#define	frm	uimg.frames

#ifdef	_DEBUG_
extern	int	debug;
#endif
#define	SValue(type)	avset(argc, argv, &l, &f, type)

int	line_height=8, line_width=8, illuminance[2]={0, 255}, step_len=2;
bool	Msg;


main(argc, argv)
int	argc;
char*	argv[];
{
int	l, f, fsize, total, point, bgrd, fx=0, fy=0, circle=1;
byte	*buf, *linebuf, *line0, *line1;

format_init(&uimg, IMAGE_INIT_TYPE, HIPS, -1, *argv, "S16-1");

uimg.width = uimg.height = 256;
uimg.frames = uimg.pxl_out = 1;
uimg.o_form = IFMT_BYTE;
(*uimg.std_swif)(FI_INIT_NAME, &uimg, *argv, 0);

for (l=1; l<argc; l++)
    if (*argv[l] == '-'){
	f=1;
	switch (argv[l][f++]){
	case 'c':
		if (SValue(0))
			cln = atoi(argv[l]+f);
		break;
	case 'f':
	{
	register int	who=argv[l][f++];
		if (SValue(0))
			f = atoi(argv[l]+f);
		switch(who){
		case 'r':
			frm = f;
			break;
		case 'x':
			fx = f;
			break;
		case 'y':
			fy = f;
		}
	}	break;
	case 'h':
		if (SValue(0))
			line_height = atoi(argv[l]+f);
		break;
	case 'p':
		if (SValue(0))
			circle = atoi(argv[l]+f);
		break;
	case 'r':
		if (SValue(0))
			row = atoi(argv[l]+f);
		break;
	case 's':
		if (SValue(0))
			step_len = atoi(argv[l]+f);
		break;
	case 'w':
		if (SValue(0))
			line_width = atoi(argv[l]+f);
		break;
	case 'v':	Msg++;
		break;
	case 'o':if (SValue(1) && freopen(argv[l]+f, "wb", out_fp))	break;
		message("%s can't be opened", argv[l]);
	default:
info:		usage_n_options(usage, l, argv[l]);
	}
    }
    else if (freopen(argv[l], "r", stdin)==NULL)
	syserr("can't open frame file - %s",argv[l]);

io_test(stdout_fd, goto info);

fsize = row * cln;
buf = nzalloc(row, cln, "buf");
linebuf = nzalloc(2, cln, "linebuf");
line0 = linebuf;
line1 = linebuf + cln;

for (total=f=0; total<cln; f++){
register int	samples=function(f, fx, line_width, circle);

	total += samples;
	if (total > cln)	samples -= total - cln;
	point = illuminance[f & 1];
	bgrd = illuminance[!(f & 1)];
	do {
		*line0++ = point;
		*line1++ = bgrd;
	} while(--samples);
}
line0 = linebuf;
line1 = linebuf + cln;
{
register byte	*bp = buf, *lp;
    for (total=f=0; total<row; f++){
    register int	lines = function(f, fy, line_height, circle);

	total += lines;
	if (total > row)	lines -= total - row;
	lp = (f&1) ? line0 : line1;
	for (l=0; l<lines; l++, bp+=cln)
		memcpy(bp, lp, cln);
    }
}

(*uimg.header_handle)(HEADER_WRITE, &uimg, argc, argv, True);

for (f=frm; f--;){
	l = fwrite(buf, cln, row, out_fp);
	if (l != row)
		syserr("[%d] write %d", row, l);
}
exit(0);
}

function(n, fn, width, period)
register int	n, fn, width;
{
static int	base=1;
register int	v;

if (!width)	return	base=1;

switch (fn)	{
default:
case 0:	v = 1;	break;
case 1:	v = n*step_len+1;	break;
case 2:	v = n*width + 1;	break;
case 3:	v = base += n;	break;
case 4:	v = 1 << n;	break;
case 5:	v = sin(n*period*M_PI / width) * width;
	if (! v)	v++;
	else if (v<0)	v = -v;
}
return	v * width;
}
