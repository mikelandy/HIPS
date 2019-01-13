/*
% 3DROTATE - rotate 3D image into another 3D image for displaying and looking
%	the different surface of the image.
%
%	Copyright (c)	1990	Jin, Guojun
*/
char	usage[]="options\n\
-l (look left side) slice vertically (y-z plane, parallel to Y axle).\n\
	If continuously doing, it goes to left->bottom->front. It is exactly\n\
	same as 3drotate90.\n\
-t specify slicing horizontally (x-z surface, parallel to X axle),\n\
	top goes to front. It is default. If continuously rotating,\n\
	then -> back -> bottom, and finally back to front again.\n\
	this option is similar to in 3drotate90, but it put upper left corner\n\
	to lower left corner. So rotation goes to top, rear(back),\n\
	bottom and back to front again. It can continiously work for\n\
	all surface and a faster than 3drotate90.\n\
-p @ doing slicing diagnally and @ gives choice for parallel to\n\
	which axle, and -a # gives angle which range is -90 -- +90 degree.\n\
	But you can specify any angle, the program can convert it into this\n\
	range.\n\
% Note:	\n\
%	If upper left corner keeping required, use 3drotate90 and 3drotate180\n\
%	to get every certain rotations.\n";

/*
# Very Improtant Note:
@
@	MUST include <math.h> in this file
@
@ AUTHOR:	Guojun Jin - LBL	11/15/90
*/

#include "header.def"
#include "imagedef.h"
#include <math.h>

int	one_more;
U_IMAGE	uimg;

#define	inbuf	uimg.src
#define	obuf	uimg.dest
#define	rows	uimg.height
#define	cols	uimg.width

#define	GValue()	arget(argc, argv, &i, &c)


main(argc, argv)
int	argc;
char**	argv;
{
int	slice_dir, xyz;
/* !!!	input number is start from 1 and convert to from 0. See line 110 */
int	bgn_cln=1, bgn_row=1, frmsNew=0;
MType	i, f, r, c,
	rowsNew, clnsNew, fsizeNew, fsize, width;
float	angle=45, riF, cjF,
	interpolation();
double	sinl, cosl;

format_init(&uimg, IMAGE_INIT_TYPE, HIPS, -1, *argv, "S20-1");

for (i=1; i<argc; i++)
    if (*argv[i] == '-' || *argv[i] == '+') {
	c=1;
	switch (argv[i][c++]) {
	case 'a':
		angle = GValue();	break;
	case 'c':
		bgn_cln = GValue();	break;
	case 'r':
		bgn_row = GValue();	break;
	case 'l':
	case 't':
		slice_dir = argv[i][1];	break;
	case 'p':
		slice_dir = xyz = argv[i][c];	break;
	case 'n':
		frmsNew = GValue();	break;
	case 'o':
		if (avset(argc, argv, &i, &c, 1) &&
			(out_fp=freopen(argv[i]+c, "wb", stdout)))	break;
	default:
errout:		usage_n_options(usage, i, argv[i]);
	}
    }
    else if ((in_fp=freopen(argv[i], "rb", stdin)) == NULL)
	    syserr("input file %s not found", argv[i]);

io_test(fileno(in_fp), goto	errout);

angle -= (int)(angle/180) * 180;
if (fabs(angle) > 90)
	angle -= 180;
if (slice_dir < 'x' && !angle && fabs(angle) == 90)
	syserr("use %s -l or -t options", *argv);

(*uimg.header_handle)(HEADER_READ, &uimg, 0, 0);
uimg.pxl_out = uimg.pxl_in;
uimg.o_form = uimg.in_form;
fsize = cols * rows;

if (bgn_row > rows)	bgn_row = rows;
if (bgn_cln > cols)	bgn_cln = cols;
if (bgn_cln)	bgn_cln--;
if (bgn_row)	bgn_row--;

inbuf = nzalloc(uimg.pxl_in, fsize*uimg.frames, "inbuf");

uimg.load_all = uimg.frames;
(*uimg.std_swif)(FI_LOAD_FILE, &uimg, 0, No);

switch(slice_dir)
{
case 't':	/*	Z-X => xyz = 'x'	look from top	*/
default:
	if (!frmsNew || frmsNew > rows-bgn_row)
		frmsNew = rows - bgn_row;
	i = rows;	rows = uimg.frames;
	uimg.frames = frmsNew;	frmsNew += bgn_row;
	fsizeNew = cols * rows;
	obuf = nzalloc(uimg.pxl_in, fsizeNew, "hbuf");

	{
	char	mesgbuf[1024];
	sprintf(mesgbuf,
	"horizontal slicing at row%d (%d frames). Rotate top to front\n",
		bgn_row+1, uimg.frames);
	msg("%s", mesgbuf);
	(*uimg.header_handle)(ADD_DESC, &uimg, mesgbuf);
	}
	(*uimg.header_handle)(HEADER_WRITE, &uimg, argc, argv, True);

	switch (uimg.pxl_in){
	case 1:	{
	register byte	*inbp, *obp, *tmpp;
	    for (r=bgn_row; r<frmsNew; r++){
		inbp = tmpp = (byte*)inbuf + r*cols;
		obp = (byte*)obuf + (rows-1)*cols;
		for (f=rows; f--; inbp=tmpp+=fsize, obp-=cols<<1)
		    for (c=cols; c--;)
			*obp++ = *inbp++;
		if (fwrite(obuf, uimg.pxl_in, fsizeNew, stdout) != fsizeNew)
			syserr("error during h write.");
	    }
	}break;
	case 2:	{
	register short	*inbp, *obp, *tmpp;
	    for (r=bgn_row; r<frmsNew; r++){
		inbp = tmpp = (short*)inbuf + r*cols;
		obp = (short*)obuf + (rows-1)*cols;
		for (f=rows; f--; inbp=tmpp+=fsize, obp-=cols<<1)
		    for (c=cols; c--;)
			*obp++ = *inbp++;
		if (fwrite(obuf, uimg.pxl_in, fsizeNew, stdout) != fsizeNew)
			syserr("error during h write.");
	    }
	}break;
	case 4:	{
	register int	*inbp, *obp, *tmpp;
	    for (r=bgn_row; r<frmsNew; r++){
		inbp = tmpp = (int*)inbuf + r*cols;
		obp = (int*)obuf + (rows-1)*cols;
		for (f=rows; f--; inbp=tmpp+=fsize, obp-=cols<<1)
		    for (c=cols; c--;)
			*obp++ = *inbp++;
		if (fwrite(obuf, uimg.pxl_in, fsizeNew, stdout) != fsizeNew)
			syserr("error during h write.");
	    }
	}
	}	/* end of switch(uimg.pxl_in)	*/
	break;
case 'l':	/*	look from left side.	Y-Z => xyz = 'z'	*/
	if (!frmsNew || frmsNew > cols-bgn_cln)
		frmsNew = cols - bgn_cln;
	i = cols;	cols = rows;
	rows = uimg.frames;	uimg.frames = frmsNew;
	fsizeNew = cols * rows;
	obuf = zalloc(uimg.pxl_in, fsizeNew, "vbuf");

	{
	char	mesgbuf[1024];
	sprintf(mesgbuf,
	"vertical slicing at column%d (%d frames). Rotate to left & turn left\n",
		bgn_cln+1, frmsNew);
	msg("%s", mesgbuf);
	(*uimg.header_handle)(ADD_DESC, &uimg, mesgbuf);
	}
	(*uimg.header_handle)(HEADER_WRITE, &uimg, argc, argv, True);

	for (c=bgn_cln; c<frmsNew; c++){
	    switch(uimg.pxl_in){
	    case 1:	{
	    register byte	*inbp, *tmpp, *obp;
		tmpp = (byte*)inbuf;
		obp = (byte*)obuf;
		for (f=0, tmpp+=c; f < rows; f++, tmpp+=fsize)
		    for (r=0, inbp=tmpp; r<cols; r++, inbp+=i)/* old_cln */
			*obp++ = *inbp;
		}break;
	    case 2:	{
	    register short	*inbp, *tmpp, *obp;
		tmpp = (short*)inbuf;
		obp = (short*)obuf;
		for (f=0, tmpp+=c; f < rows; f++, tmpp+=fsize)
		    for (r=0, inbp=tmpp; r<cols; r++, inbp+=i)
			*obp++ = *inbp;
		}break;
	    case 4:	{
	    register int	*inbp, *tmpp, *obp;
		tmpp = (int*)inbuf;
		obp = (int*)obuf;
		for (f=0, tmpp+=c; f < rows; f++, tmpp+=fsize)
		    for (r=0, inbp=tmpp; r<cols; r++, inbp+=i)/* old_cln */
			*obp++ = *inbp;
	    }
	    }	/* end of switch (uimg.pxl_in)	*/
	    if (fwrite(obuf, uimg.pxl_in, fsizeNew, stdout) != fsizeNew)
		syserr("error during v write.");
	}
	break;
case 'x':
case 'y':
case 'z':{
register char	*inbp, *obp;
int	*bp, *buf = (int*)zalloc(uimg.pxl_in, fsize, "buf");
	message("diagnal slicing\n");
	angle *= M_PI / 180.0;
	cosl = cos(angle);
	sinl = sin(angle);
	rowsNew = rows / cosl;
	clnsNew = cols / sinl;
	fsizeNew= rowsNew * clnsNew;
	if (angle > 0 && rowsNew * sinl > cols - bgn_cln)
		rowsNew = (cols - bgn_cln) / sinl;
	else if (angle < 0 && rowsNew * sinl < -bgn_cln)
		rowsNew = -bgn_cln / sinl;

	width = (xyz == 'x') ? cols : (xyz == 'y') ?
		(uimg.frames-1) * rows * cols :
		(angle < 0)? fsize - 1 : fsize + 1;

	bp = buf;	inbp = inbuf;	/* convert to integer	*/
	for(i = 0; i < fsize; i++)
		*bp++ = (int)(*inbp++ & 0xFF);
	obp = obuf = zalloc(uimg.pxl_in, fsizeNew, "dbuf");
	for(r = bgn_row; r < rowsNew; r++)	/* rowsNew = rows/cosl	*/
	    for(c = 0; c < clnsNew; c++){	/* colsNew = frames	*/
		riF = cosl * r + bgn_row;
		cjF = sinl * r + bgn_cln;
		if (r && c)
		*obp++ = (char)interpolation(bp + (int)floor(c * cosl), width, riF, cjF);
		else	*obp++ = bp[r * cols + c];
		}
	cols = clnsNew;	rows = rowsNew;
	if (fwrite(obuf, uimg.pxl_in, fsizeNew, stdout) != fsizeNew)
		syserr("error during write d.");
}
}	/* end of switch(dir)	*/
return(0);
}


float   interpolation(cur_pos, width, riF, cjF)
register int	*cur_pos, width;
float		riF, cjF;
{
float	lamio, lami1, lamjo, lamj1;
float	p00, p01, p10, p11;
register int	ri, cj;

ri = riF;	cj = cjF;
lami1 = riF - ri;	/* the % of fraction position	*/
lamio = 1.0 - lami1;	/* the % of regular position	*/
lamj1 = cjF - cj;
lamjo = 1.0 - lamj1;

p00 = *(cur_pos - width - one_more) * lamio;
p01 = *(cur_pos - width) * lamio;
p10 = *(cur_pos + width - one_more) * lami1;
p11 = *cur_pos * lami1;
return	(lamjo*(p00 + p10) + lamj1*(p01 + p11));
}
