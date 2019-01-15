/*
% histo_simulate.c - build image frames according to histogram which is
%		produced by mhisto.
%
*********************************************************************
*********               COPYRIGHT NOTICE                *************
*********************************************************************

        This program is copyright (C) 1990, 1991, Regents  of  the
University  of  California.   Anyone may reproduce this software,
in whole or in part, provided that:
(1)  Any copy  or  redistribution  must  show  the
     Regents  of  the  University of California, through its
     Lawrence Berkeley Laboratory, as the source,  and  must
     include this notice;
(2)  Any use of this software must reference this  distribu-
     tion,  state that the software copyright is held by the
     Regents of the University of California, and  that  the
     software is used by their permission.

     It is acknowledged that the U.S. Government has  rights
to this software under  Contract DE-AC03-765F00098 between the U.S.
Department of Energy and the University of California.

     This software is provided as a professional  academic
contribution for  joint exchange.  Thus it is experimental, is
provided ``as is'', with no warranties of any kind  whatsoever,
no  support,  promise  of updates, or printed documentation.
Bug reports or fixes may be sent to the author, who may or may
not act on them as he desires.

*********************************************************************
%
% usage:
%	histo_simulate [-r#] [-c#] [-f#] [-S] [-E]
%			[<] histogram [> [-o] graphimage]
*/
char	usage[]="options\n\
% -r -c -f	specify the simulated image window and frames.\n\
%		default is 128 x 128 x input_frames.	\n\
%	-S	output SHORT format.	Default is BYTE.\n\
%	-E	spread fraction evenly. Otherwise rest add the padding value.\n\
% [<] histogram [> graph_image]\n";
/*
% compile:	cc -o histo_simulate histo_simulate.c -lccs -lhipsh -lhips -lm
%
% AUTHOR:	Jin Guojun - LBL	2/7/91
*/

#include <math.h>
#include "header.def"
#include "imagedef.h"

U_IMAGE	uimg;

#define	buf	uimg.dest
#define	pxl_bytes	uimg.pxl_out

#ifndef	HIST_TITLE_STR
#define	HIST_TITLE_STR	"HIST01"
#endif

#ifndef	Pad
#define	Pad	0
#endif
#ifdef	_DEBUG_
extern	int	debug;
#endif

bool	Msg, Ef, Ascii;

#define	GValue()	arget(argc, argv, &i, &j)


main(argc, argv)
int	argc;
char**	argv;
{
char	*hist_str;
int	r=128, c=128, f=0, frp;
MType	i, j, fsize0, fsizeNew,
	numbin, binwidth, *hist;
float	scale, offset=0;

format_init(&uimg, IMAGE_INIT_TYPE, HIPS, -1, *argv, "S20-1");
uimg.o_form = IFMT_BYTE;
uimg.pxl_out = 1;

for (i=1; i<argc; i++)
    if (*argv[i] == '-')	{
	j = 1;
	switch(argv[i][j++])	{
	case 'E':	Ef++;	break;
	case 'S':	uimg.o_form = IFMT_SHORT;
		uimg.pxl_out = sizeof(short);	break;
#ifdef	_DEBUG_
	case 'D':debug++;	break;
#endif
	case 'M':	Msg++;	break;
	case 'O':	offset = GValue();	break;
	case 'c':	c = GValue();	break;
	case 'f':	f = GValue();	break;
	case 'r':	r = GValue();	break;
	case 'o':if (avset(argc, argv, &i, &j, 1) &&
			freopen(argv[i]+j, "wb", stdout))	break;
		message("output file - %s", argv[i]);
	default:
info:		usage_n_options(usage, i, argv[i]);
	}
    }
    else if ((in_fp=freopen(argv[i], "rb", stdin)) == NULL)
		syserr("%s - not found", argv[i]);
io_test(stdin_fd, goto	info);

if ((*uimg.header_handle)(HEADER_READ, &uimg, 0, 0) || uimg.in_form != IFMT_HIST){
	fseek(in_fp, 0, 0);
	i = strlen(HIST_TITLE_STR);
	hist_str = zalloc(1, i+1, "hstr");
	fread(hist_str, 1, i, in_fp);
	if (strcmp(hist_str, HIST_TITLE_STR))
		syserr("input must be in histogram format");
	Ascii++;
	fscanf(in_fp, "%d %d %d %d %d\n", &binwidth, &numbin,
		&uimg.height, &uimg.width, &uimg.frames);
	uimg.o_form = IFMT_BYTE;
	uimg.pxl_out = 1;
	uimg.in_type = 0;
}
else	{
	if (upread(&binwidth, 1, sizeof(binwidth), stdin) != sizeof(binwidth))
		syserr("error during read bin width");
	if (upread(&numbin, 1, sizeof(numbin), stdin) != sizeof(numbin))
		syserr("error during read bin number");
}
message("%s:	image size was %d x %d\n", *argv, uimg.height, uimg.width);
fsize0 = (MType)uimg.height * uimg.width;
if (r<8)	r=uimg.height;
if (c<8)	c=uimg.width;
if (f<1)	f=uimg.frames;
fsizeNew = r*c;
scale = (float)fsizeNew/fsize0 + offset;
uimg.height = r;	uimg.width = c;	uimg.frames = f;
message("	New	image size is %d x %d x %ld bytes (scale=%.4f)\n",
	r, c, pxl_bytes, scale);

(*uimg.header_handle)(HEADER_WRITE, &uimg, argc, argv, True);

hist = (int*)nzalloc(numbin, (MType)sizeof(numbin));
buf = nzalloc(fsizeNew, pxl_bytes);

for (frp=0; frp<f; frp++){
	if (Ascii) {
		fscanf(in_fp, "%s %d", hist_str, &i);
		for (j=0; j<numbin; j++){
			if (j%4 == 0)	/* take away the line number */
				fscanf(in_fp, "%x ", &hist[j]);
			fscanf(in_fp, "%d ", &hist[j]);
		}
	}
	else	{
		if (upread(&i, 1, sizeof(i), stdin) != sizeof(i))
			syserr("can not read max count");
		j = upread(hist, sizeof(*hist), numbin, stdin);
		if (j != numbin)
			syserr("error during read histogram %ld", j);
	}
	msg("maximum count in this frame is %d\n", i);
	if (pxl_bytes != sizeof(short)){
	register byte*	bp = (byte*)buf;
		for (i=binwidth=0; i<numbin; i++){
		register unsigned num = hist[i] * scale;
			if (!num)	continue;
			for (j=0; j<num; j++)	*bp++ = i;
			binwidth += num;
		if(Msg)	message("add %d pixel at gray_level %ld\n", num, i);
		}
		for (i=0, j=fsizeNew-binwidth; i<j; i++)
			if (Ef)	*bp++=i;
			else	*bp++=Pad;
	}
	else	{
	register short*	bp = (short*)buf;
		for (i=binwidth=0; i<numbin; i++){
		register unsigned num = hist[i] * scale;
			if (!num)	continue;
			for (j=0; j<num; j++)	*bp++ = i;
			binwidth += num;
		if(Msg)	message("add %d pixel at gray_level %ld\n", num, i);
		}
		for (i=0, j=fsizeNew-binwidth; i<j; i++)
			if (Ef)	*bp++=i;
			else	*bp++=Pad;
	}
	message("%ld regular points & %ld pixel padded\n", binwidth, i);
	if (fwrite(buf, pxl_bytes, fsizeNew, stdout) != fsizeNew)
		syserr("w_error");
}
exit(0);
}
