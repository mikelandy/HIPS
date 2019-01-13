/*
	MDISPHIST - display histgram files as a bar graph in multiple ways.
%
% usage:	mdisphist [-m maxcnt] [-F] [-M] [-R] [-g [#]] [-n] [-r] [-w #]
%			[<] mhistogram [> [-o] graphimage]
%
%	Copyright (c)	1991	Jin, Guojun
*/
char	usage[]="options	\n\
-m #	specifies an initial maximum bincount for use in scaling the	\n\
	displays.  Otherwise, the maximum in the first histogram is used,\n\
	and changed when selfs is set by -R (Retrieve maxcnt).		\n\
-F [#]	add frame around each histogram. It is good for seeing the histogram,\n\
	but need more calculation for powertool to spread multiple frames.\n\
-M	display more message.	\n\
-R	flag is servered for mhisto using -r option.\n\
-g [#]	grids for both row and column.	\n\
-n	output negative histo-graph.	\n\
-r	will recalculate the maximum count for each frame. If mhisto has\n\
	already used -r option, use -R option in mdisphist is good for saving\n\
	time.	\n\
-w #	specify the output window size. Default is 256 x 256.	\n\
	[<] mhistogram [> graphimage]\n";
/*
@ compile:	cc -O -o mdisphist mdisphst.c -lccs -lhipsh -lhips -lm
@
@ AUTHOR:	Guojun Jin - Lawrence Berkeley Laboratory	1/20/91
@ changes:
@	2/7/91	hist buffer <= MType
@	6/17/91	add thicker frame
*/

#include "header.def"
#include "imagedef.h"
#include <math.h>

U_IMAGE	uimg;
#define	buf	uimg.src

#define	border	127
#define	bground	0
#define	bar	255

#ifndef	Visual
#define	Visual	48
#endif

#define	grid	border + Visual
#define	GValue()	arget(argc, argv, &r, &c)

#ifdef	_DEBUG_
extern	int	debug;
#endif

bool	grids, frac_grid, neg,
	selfs,
	ofsz,
	recalc,
	BFRAME,
	Msg;


main(argc, argv)
int	argc;
char**	argv;
{
MType	k, r, c, f, fr,
	numbin, binwidth,
	maxcnt=0, goff=0,	/* grid offset */
	*hist, image_size=256, image_frame;
byte	Bar;
float	ratio,	/* 	window compress ratio	*/
	factor;	/*	window enlarge factor	*/

format_init(&uimg, IMAGE_INIT_TYPE, HIPS, -1, *argv, "S16-1");

for (r=1; r<argc; r++)
    if (*argv[r] == '-')	{
	c = 1;
	switch(argv[r][c++]) {
#ifdef	_DEBUG_
	case 'D':debug++;	break;
#endif
	case 'F':BFRAME = GValue();
		if (BFRAME<1 || BFRAME>8)	BFRAME=1;
		break;
	case 'M':	Msg++;	break;
	case 'R':	selfs++;	break;
	case 'm':
		maxcnt = GValue();	break;
	case 'g':
		grids = GValue();
		if (!grids)	grids = 8;
		break;
	case 'n':
		neg++;	break;
	case 'r':
		recalc++;	break;
	case 'w':
		ofsz = GValue();
		if (!ofsz)	ofsz--;
		break;
	default:
errout:		usage_n_options(usage, r, argv[r]);
	}
    }
    else if (!freopen(argv[r], "rb", stdin))
		syserr("%s -- not found", argv[r]);

io_test(stdin_fd, goto	errout);

(*uimg.header_handle)(HEADER_READ, &uimg, 0, 0);
r = uimg.height;
c = uimg.width;
f = uimg.frames;

if (uimg.in_form != IFMT_HIST)
	syserr("image must be in histogram format");
uimg.o_form = IFMT_BYTE;
uimg.pxl_out = 1;

if (ofsz)
   if (ofsz<0)
	image_size <<= 1;
   else	image_size = ofsz;
image_frame = image_size + (BFRAME<<1);
uimg.width = uimg.height = image_frame;
if (grids){
	grids = image_size / grids;
	frac_grid = image_size % grids;
	goff = BFRAME;
}
else	grids = frac_grid = image_frame;

if (upread(&binwidth, 1, sizeof(binwidth), stdin) != sizeof(binwidth))
	syserr("error during read bin width");
if (upread(&numbin, 1, sizeof(numbin), stdin) != sizeof(numbin))
	syserr("error during read bin number");

(*uimg.header_handle)(HEADER_WRITE, &uimg, argc, argv, True);

hist = zalloc(numbin, sizeof(*hist), "hist");
buf = nzalloc(image_frame, image_frame, "buf");

if (Msg){
	message("%s: image size was %d x %d\n", *argv, r, c);
	message("%s: there are %d (width=%d)bins ## grid : size=%d, frac=%d\n",
		*argv, numbin, binwidth, grids, frac_grid);
}

for(fr=0; fr<f; fr++) {
register int	i, j;
register byte*	bp = buf;
	for (i=0; i<image_frame; i++)
	    for (j=0; j<image_frame; j++)
		if (!j || j==image_frame-1 || !i || i==image_frame-1)
			*bp++ = border;
		else if (((i-frac_grid-goff) % grids) && ((j-goff) % grids))
			*bp++ = bground;
		else
			*bp++ = grid;

	if (upread(&k, 1, sizeof(k), stdin) != sizeof(k))
		syserr("can not read max count");
	if (selfs || !maxcnt)	maxcnt = k;
	if (upread(hist, sizeof(*hist), numbin, stdin) != numbin)
		syserr("error during read histgram");
	if (recalc || !maxcnt){
		register MType	max=0, *hp=hist;
		for (i=0; i<numbin; i++, hp++)
			if (*hp > max)	max = *hp;
		maxcnt = max;
		if (Msg)
			message("maximum count in this frame is %d\n", max);
	}
	if (numbin < image_size){
		ratio = 1;
		factor = (float)image_size/numbin;	/* need repeating */
	}
	else{	ratio = (float)numbin / image_size;
		factor = 1;	/* need compress	*/
	}
	Bar = bar / ratio;
	if (Bar < Visual)
		Bar = Visual;
	if (Msg)
	message("%s: frame %d maxcnt increased to %d :: diagram compress %.2f\n",
		*argv, fr, maxcnt, ratio);

	for (i=0; i<image_size; i++)	/* horizontal process */
	    for (r=0; r<ratio; r++){	/* Accumulate bar for compress window */
		bp = buf;
#ifdef ULORIG
		bp += i + (image_frame-1)*BFRAME;
#else
		bp += i + (image_frame-1)*(image_size+BFRAME);
#endif
		j = image_size * hist[(int)(i*ratio/factor + r)] / maxcnt;
		if (j > image_size)	j = image_size;	/* bra height	*/

		for (k=0; k<j; k++) {	/* vertical process	*/
			if (Bar + *bp > 255)	*bp = 255;
			else	*bp += Bar;
#ifdef ULORIG
			bp += image_frame;
#else
			bp -= image_frame;
#endif
		}
	    }
	j = image_frame * image_frame;
	if (neg) for (i=j, bp=buf; i--; bp++)	*bp = -1 - *bp;
	i = fwrite(buf, 1, j, stdout);
	if (i != j)	syserr("error during write [%d] %d", j, i);
}
exit(0);
}
