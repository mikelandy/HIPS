/*	Copyright (c)	1991	Jin, Guojun
#
@	MUST include <math.h>
@	-t	trace long run for -r and -g.
@	-p	execute part of total or percentage of total loops.
@	-d	directly read and write.
@	-M	memory allocate and free times.
@ AUTHOR:	Jin Guojun - LBL	2/1/91
*/

#include <errno.h>
#include <time.h>
#include <math.h>
#include "header.def"
#include "imagedef.h"

#define	inbuf	uimg.src
#define	obuf	uimg.dest
#define	pxl_bytes	uimg.pxl_in
#define	frm	uimg.frames
#define	row	uimg.height
#define	cln	uimg.width

U_IMAGE	uimg;
bool	acc_dir, fast, memaf, trace, drw, total, part, pipe;
char	usage[]="options\n\
-s	sequential\n-m	r->c->f\n-f	f->c->r\n\
-g	f->r->c\n-c	c->f->r\n-r	r->f->c\n\
-R	register_mode(FAST)\n-S	F R C--set SIZE\n\
-t	tracing for -r or -g\n-p	partial execution for -r og -g\n\
-d[#]	directly read and write(file copy using entire memory)\n\
	The # is writing times (entire size factor)\n\
-M	memory allocating times\n";

time_t	tm0, tm1;


main(argc, argv)
int	argc;
char**	argv;
{
/* !!!	input number is start from 1 and convert to from 0	*/
MType	i, c, fsize;
float	percent;

format_init(&uimg, IMAGE_INIT_TYPE, HIPS, -1, *argv, "S20-1");
uimg.pxl_out = pxl_bytes = 1;

for(i=1; i<argc; i++)
    if (*argv[i] == '-')
	switch(argv[i][1])
	{
	case 'c':
	case 'r':
	case 'f':
	case 'g':
	case 'm':
	case 's':
		acc_dir = argv[i][1];	break;
	case 'd':
		drw = atoi(argv[i]+2);
		if (drw<=0)	drw = 1;
		break;
	case 'p':
		percent = atof(argv[i]+2);	break;
	case 'M':	memaf = atoi(argv[i]+2);	break;
	case 'P':	pipe++;	break;
	case 'R':	fast++;	break;
	case 'S':
		if (isdigit(*argv[i+1]))	frm=atoi(argv[++i]);
		if (isdigit(*argv[i+1]))	row=atoi(argv[++i]);
		if (isdigit(*argv[i+1]))	cln=atoi(argv[++i]);
		if (!frm)	frm=128;
		if (!row)	row=128;
		if (!cln)	cln=128;
		break;
	case 't':	trace++;	break;
	case 'o':
		if (freopen(argv[i]+2, "wb", stdout))	break;
		message("%s can't be opened for write", argv[i]+2);
	default:
		usage_n_options(usage, i, argv[i]);
	}
    else if ((in_fp=freopen(argv[i], "rb", stdin)) == NULL)
	syserr("input file %s not found", argv[i]);

io_test(fileno(in_fp), message("Using CTRL-D to continue"));

fseek(in_fp, 0L, 0);
pipe += errno;

if (!frm)	(*uimg.header_handle)(HEADER_READ, &uimg, 0, 0);

if (percent < 5)	percent = 5;
part = percent * cln / 100;

time(&tm0);
message("begin processing: %s", ctime(&tm0));

fsize = cln * row;
total = fsize * frm;

if (memaf){
    while(memaf--){
	inbuf = zalloc(pxl_bytes, total, "inbuf");
	time(&tm1);
	message("allocating %d bytes at %u: in second %s\n",
		total*pxl_bytes, inbuf, tm1-tm0);
	free(inbuf);
	time(&tm1);
	message("free %d bytes at %u: in second %s\n",
		total*pxl_bytes, inbuf, tm1-tm0);
    }
    exit(0);
}

inbuf = zalloc(pxl_bytes, total, "inbuf");
time(&tm1);
message("end of allocating: at %d seconds	", tm1-tm0);

if (pipe)	i = upread(inbuf, pxl_bytes, total, in_fp);
else	i = fread(inbuf, pxl_bytes, total, in_fp);

time(&tm1);
message("=>	end of reading: at %d seconds\n", tm1-tm0);
if (i != total)
    message("Reading image %ld cells, rest %u are 0\n", i, total-i);

if (drw){
	c = total % drw;
	total /= drw;
	while (drw--){
	    i = fwrite(inbuf, pxl_bytes, total-c*!drw, out_fp);
	    if (i != total)
		syserr("directly writing %d", i);
	}
}
else{
message("%d frames, %d rows, %d columns => memory %u\n\
(execute %d columns for -r and -g options)\n", frm, row, cln, total, part);
if (fast){
register byte	*bp, *tmpp, tmp;
register MType	f, r;
	message(" -- in register mode (FAST)\n");
switch(acc_dir)
{
default:
case 's':	/*	access buf sequentially	by column	*/
	message("sequential access\n");
	for (f=0, bp=inbuf; f<total; f++)
		tmp = bp[f];
	break;
case 'm':
	mesg("row->column->frame access\n");
	for (f=0, tmpp=inbuf; f<frm; f++, tmpp+=fsize)
	    for (c=0; c<cln; c++)
		for (r=0, bp=tmpp+c; r<row; r++, bp+=cln)
			tmp += *bp;
	break;
case 'f':
	message("frame->column->row access\n");
	for (r=0; r<row; r++)
	    for (c=0, tmpp=(byte*)inbuf+r*cln; c<cln; c++, tmpp++)
		for (f=0, bp=tmpp; f<frm; f++, bp+=fsize)
			tmp = *bp;
	break;
case 'c':
	mesg("column->frame->row access\n");
	for (r=0; r<row; r++)
	    for (f=0, tmpp=(byte*)inbuf+r*cln; f<frm; f++, tmpp+=fsize)
		for (c=0, bp=tmpp; c<cln; c++)
			tmp = *bp++;
	break;
case 'r':
	mesg("row->frame->column access\n");
	for (c=0; c<cln; c++){
	if (trace)	fprintf(stderr, "\r%d", c);
	if (c >= part)	break;
	    for (f=0, tmpp=(byte*)inbuf+c; f<frm; f++, tmpp+=fsize)
		for (r=0, bp=tmpp; r<row; r++, bp+=cln)
			tmp = *bp;
	}
	break;
case 'g':
	mesg("frame->row->column access\n");
	for (c=0; c<cln; c++){
	if (trace)	fprintf(stderr, "\r%d", c);
	if (c >= part)	break;
	    for (r=0, tmpp=(byte*)inbuf+c; r<row; r++, tmpp+=cln)
		for (f=0, bp=tmpp; f<frm; f++, bp+=fsize)
			tmp = *bp;
	}
}
}
else{
byte	*bp, *tmpp, tmp;
MType	f, r;
message(" -- in memory mode (Regular)\n");
switch(acc_dir)
{
default:
case 's':	/*	access buf sequentially	by column	*/
	message("sequential access\n");
	for (i=0, bp=inbuf; i<total; i++)
		tmp = bp[i];
	break;
case 'm':
	mesg("row->column->frame access\n");
	for (f=0, tmpp=inbuf; f<frm; f++, tmpp+=fsize)
	    for (c=0; c<cln; c++)
		for (r=0, bp=tmpp+c; r<row; r++, bp+=cln)
			tmp += *bp;
	break;
case 'f':
	message("frame->column->row access\n");
	for (r=0; r<row; r++)
	    for (c=0, tmpp=(byte*)inbuf+r*cln; c<cln; c++, tmpp++)
		for (f=0, bp=tmpp; f<frm; f++, bp+=fsize)
			tmp = *bp;
	break;
case 'c':
	mesg("column->frame->row access\n");
	for (r=0; r<row; r++)
	    for (f=0, tmpp=(byte*)inbuf+r*cln; f<frm; f++, tmpp+=fsize)
		for (c=0, bp=tmpp; c<cln; c++)
			tmp = *bp++;
	break;
case 'r':
	mesg("row->frame->column access\n");
	for (c=0; c<cln; c++){
	if (trace)	fprintf(stderr, "\r%d", c);
	if (c >= part)	break;
	    for (f=0, tmpp=(byte*)inbuf+c; f<frm; f++, tmpp+=fsize)
		for (r=0, bp=tmpp; r<row; r++, bp+=cln)
			tmp = *bp;
	}
	break;
case 'g':
	mesg("frame->row->column access\n");
	for (c=0; c<cln; c++){
	if (trace)	fprintf(stderr, "\r%d", c);
	if (c >= part)	break;
	    for (r=0, tmpp=(byte*)inbuf+c; r<row; r++, tmpp+=cln)
		for (f=0, bp=tmpp; f<frm; f++, bp+=fsize)
			tmp = *bp;
	}
}
}
}
time(&tm1);
message("finish: at %d seconds\n", tm1-tm0);
}
