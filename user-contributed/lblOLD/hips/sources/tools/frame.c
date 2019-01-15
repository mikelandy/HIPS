/*
#	FRAMING . C
%
%	Copyright (c)	1990	Jin Guojun
%
%	framing -- put image into a frame
%
% AUTHOR:	Jin Guojun - LBL	11/15/90
*/

#include <math.h>
#include "header.def"
#include "imagedef.h"

arg_fmt_list_string	arg_fmt[] =	{
	{"-D", "%d", True, 1, 0, "\t\tDebug level"},
	{"-f", "%d %d", 0, 2, 1, "\trows [ columns ]	frame size\n\
%	if no frame size given, framing will pick larger size of input	\n\
%	rows and columns as frame size. If input rows equal to columns,	\n\
%	framing will enlarge old frame size by 5%."},
	{"-o", "%s", 0, 1, 1, "output file name"},
	{"-v", "%d", 255, 1, 0,
	"frame value [%.0f if not # specified. default = 0]"},	NULL};
int	fvalue;
U_IMAGE	uimg;

#ifndef	FMODULE
#define	FMODULE	16
#endif
#define	inbuf	uimg.src
#define	obuf	uimg.dest
#define	frm	uimg.frames
#define	row	uimg.height
#define	cln	uimg.width
#define	pxl_bytes	uimg.pxl_out

#define	framing(f_type, in_buf, out_buf, i_r, i_c, o_r, o_c)	{	\
register f_type	*ibuf=(f_type*)in_buf, *o_buf=(f_type*)out_buf;	\
	int	ir = i_r, ic = i_c, or = o_r, oc = o_c;	\
	int	mtop = (or - ir) >> 1,	\
		mleft= (oc - ic) >> 1;	\
	register int	r, c;		\
		\
	for (r=0; r < mtop; r++)	\
	    for (c=0; c < oc; c++)	\
		*o_buf++ = fvalue;	\
	for (r=0; r<ir; r++)	{	\
		for (c=0; c < mleft; c++)	\
			*o_buf++ = fvalue;	\
		for (c=0; c < ic; c++)		\
			*o_buf++ = *ibuf++;	\
		for (c=0; c < oc-ic-mleft; c++)	\
			*o_buf++ = fvalue;	\
	}	\
	for (r=0; r<or-ir-mtop; r++)	\
	    for (c=0; c<oc; c++)	\
		*o_buf++ = fvalue;	\
}


main(argc, argv)
int	argc;
char**	argv;
{
char	**fl, *of_name;
int	i_row, i_cln, o_row, o_cln=0, f, t2f;
MType	fsize, ofsize;

format_init(&uimg, IMAGE_INIT_TYPE, HIPS, uimg.color_dpy=-1, *argv, "F10-3");

	if ((f=parse_argus(&fl, argc, argv, arg_fmt,
		&debug, &o_row, &o_cln, &of_name, &fvalue)) < 0)
		exit(f);
	if (of_name && !(out_fp=freopen(of_name, "wb", stdout)))
errout:		parse_usage(arg_fmt),	exit(0);
	if (f && !(in_fp=freopen(fl[0], "rb", stdin)))
	    syserr("input file %s not found", fl[0]);

io_test(fileno(in_fp),	goto	errout);

(*uimg.header_handle)(HEADER_READ, &uimg, 0, 0);
uimg.o_form = uimg.in_form;
uimg.pxl_out = uimg.pxl_in;
if (isColorImage(uimg.color_form) > 1)
	uimg.color_form = CFM_ILC;
t2f = uimg.color_form==CFM_ILC;

i_row = row;	i_cln = cln;
fsize = i_row * i_cln;
if (!o_row) {
   if (row != cln)
	o_row = o_cln = MAX(row, cln);
   else	o_row = o_cln = row * 1.05;
}
else	{
	if (o_row==row && o_cln==cln)
		syserr("no thing done");
	if (o_row < row)	o_row = row * 1.05;
	if (!o_cln)	o_cln = o_row;
	if (o_cln < cln)	o_cln = cln * 1.05;
}
cln = o_cln % FMODULE;
if (cln)	o_cln += FMODULE - cln;	/* width must be word aligned */
row = o_row;	cln = o_cln;

(*uimg.header_handle)(HEADER_WRITE, &uimg, argc, argv, True);

cln = i_cln,	row = i_row;	/* restore for reading data */
obuf = zalloc(t2f ? 4 : uimg.pxl_in, ofsize = o_row * o_cln, "obuf");

message("%s %d x %d x %d (x %d bytes)\n", *argv, o_row, o_cln, frm,
			pxl_bytes = t2f ? 3 : uimg.pxl_in);

for (f=0; f<frm; f++)	{
	(*uimg.std_swif)(FI_LOAD_FILE, &uimg, uimg.load_all=0, No);

	switch(pxl_bytes) {
	case 1:	framing(byte, inbuf, obuf, i_row, i_cln, o_row, o_cln);
		break;
	case 2:	framing(short, inbuf, obuf, i_row, i_cln, o_row, o_cln);
		break;
	case 3:	of_name = nzalloc(4, fsize, "tbuf");
		ilc_transfer(of_name, inbuf, fsize, 3, 0, 4);
		free(inbuf);	inbuf = of_name;
	case 4:	framing(long, inbuf, obuf, i_row, i_cln, o_row, o_cln);
		if (t2f)        ilc_transfer(obuf, obuf, ofsize, 4, 0, 3);
		break;
	default:	syserr("wrong format");
	}
	if (fwrite(obuf, pxl_bytes, ofsize, stdout) != ofsize)
		syserr("write new frame %d", f);
}
}
