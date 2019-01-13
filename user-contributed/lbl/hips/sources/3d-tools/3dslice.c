/*
% 3DSLICE.C - slicing 3D image to 2D image for displaying and looking the
%	inside of the image. Default is to slice horizontally (x-z surface,
%	parallel X axle). -dv specify slicing vertically (y-z surface, Z axle).
%	-dd doing slicing diagnally and -p XYZ gives choice for parallel to
%	which axle, and -a # gives angle which range is -90 -- +90 degree.
%	But you can specify any angle, the program can convert it into this
%	range.
%
%	Copyright (c)	1990	Jin, Guojun
*/
char	usage[]="options	\n\
-a#		slice angle	\n\
-d[h] [v] [d]	slice direction	\n\
-p[x] [y] [z]	parallel to axle\n\
-c#		begin column	\n\
-r#		begin row	\n\
-f#		begin frame	\n\
-n#		number of frames will be processed\n\
input [> output]\n\
% Note:	\n\
%	The result of slicing is always from bottom to top.	\n\
%	To expain this, we use following example:		\n\
%	Parallel Y axle slicing. Coordinate is (x, y, z);	\n\
%	(0, 0, 0 - 0, 1, 0) corner by using +angle -f options.	\n\
%	(1, 0, 0 - 1, 1, 0) corner by using -angle -c options.	\n\
%	(1, 0, 1 - 1, 1, 1) using +angle & -c options.		\n\
%	(0, 0, 1 - 0, 1, 1) using -angle & -f options.		\n\
%	For 1 & 3, the view-line is (1, 0, 1 - 1, 1, 1).	\n\
%	For 2 & 4, the view_line is (1, 0, 0 - 1, 1, 0).	\n\
%	The other surfaces will be similar.	\n";
/*
* AUTHOR:	Guojun Jin - LBL	11/15/90
*/

#include "header.def"
#include "imagedef.h"
#include <math.h>

U_IMAGE	uimg;

#define	inbuf	uimg.src
#define	obuf	uimg.dest
#define	cols	uimg.width
#define	rows	uimg.height
#undef	frms

double	ssin, sinl, cosl, tanl;

#ifndef	GValue
#define	GValue()	arget(argc, argv, &i, &c)
#endif


main(argc, argv)
int	argc;
char**	argv;
{
byte	*inbp, *obp, *tmpp;
bool	bc=0, bf=0;	/* default = row	*/
int	*ibuf, *ibp,	/* for diagnal interpolation	*/
	slice_dir, l_r=1,/* default for parallel to Z axle	*/
	xyz='z', width,
	bgn_row=1,	/* all working number is start from 1	*/
	bgn_cln=0,	/* and keeps in all diagnal routines	*/
	bgn_frm=0,	/* and changed to from 0 at right angle routine	*/
	f, r, c, df,
	rowsNew, clnsNew, fsizeNew;
MType	i, fsize, frames=1, frms;
float	angle=45, rfmod, cfmod;

format_init(&uimg, IMAGE_INIT_TYPE, HIPS, HIPS, *argv, "S20-1");

for (i=1; i<argc; i++)
    if (*argv[i] == '-' || *argv[i] == '+') {
	c = 1;
	switch (argv[i][c++]){
	case 'a':
		if (avset(argc,argv,&i,&c,0))
			angle = atof(argv[i]+c);	break;
#ifdef	_DEBUG_
	case 'D':	debug++;	break;
#endif
	case 'c':
		bc = bgn_cln = GValue();
		bgn_frm = 0;	break;
	case 'f':
		bf = bgn_frm = GValue();
		bc = 0;	break;
	case 'd':
		slice_dir = argv[i][c];	break;
	case 'n':
		frames = GValue();	break;
	case 'p':
		slice_dir = xyz = argv[i][c];	break;
	case 'r':
		bgn_row = GValue();
		bf=0;	break;
	case 'o':
		if (out_fp=freopen(argv[i]+2, "wb", stdout))	break;
	default:
errout:		usage_n_options(usage, i, argv[i]);
	}
    }
    else if ((in_fp = freopen(argv[i], "rb", stdin)) == NULL)
	    syserr("input file %s not found", argv[i]);

io_test(fileno(in_fp), goto	errout);

angle -= (int)(angle/180) * 180;
if (fabs(angle) > 90)
	angle -= 180;
if (!angle)     slice_dir = 'h';

(*uimg.header_handle)(HEADER_READ, &uimg, 0, 0);
if (uimg.in_form != IFMT_BYTE)
	syserr("Non BYTE format isn't available Now! Use 3drotate please");

uimg.pxl_out = uimg.pxl_in;
uimg.o_form = uimg.in_form;
fsize = cols * rows;	frms = uimg.frames;

inbuf = NZALLOC((MType)frms, fsize, "inbuf");
/* for right angle only, diagnal routines will alloc obuf by themself	*/
if (slice_dir=='h' || slice_dir=='v' || !slice_dir)
	obuf = NZALLOC(frames, fsize, "obuf");

/* parallel to Z axle only needs 1 integer bufffer,
	and parallel to X or Y needs 2 buffers	*/
ibuf = (int*)zalloc(fsize, (MType)(sizeof(*ibuf) << (xyz=='x' || xyz=='y')), "ibuf");

(*uimg.std_swif)(FI_LOAD_FILE, &uimg, uimg.load_all=frms, No);

switch(slice_dir)
{
case 'v':
	if (--bgn_cln < 0)	bgn_cln = cols/3;
	if (frames > cols-bgn_cln)	frames = cols - bgn_cln;
	i = cols;	cols = frms;
	frms=frames;	fsizeNew = rows*cols;
	(*uimg.header_handle)(HEADER_WRITE, &uimg, argc, argv, True);

	message("vertical at %d column\n", bgn_cln);
	for (c=0; c<frames; c++)
	{
		tmpp = (byte*)inbuf + c + bgn_cln;
		obp = (byte*)obuf;
		for (r=0; r < rows; r++, tmpp+=i)
		    for (f=0, inbp=tmpp; f<cols; f++, inbp += fsize)
			*obp++ = *inbp;
		if(fwrite(obuf, uimg.pxl_out, fsizeNew, out_fp) != fsizeNew)
			syserr("error during write.");
	}
	break;
case 'h':
case 0:
	if (--bgn_row < 0)	bgn_row = rows/3;
	if (frames > rows-bgn_row)	frames = rows - bgn_row;
	rows = frms;
	frms=frames;	fsizeNew = rows*cols;
	(*uimg.header_handle)(HEADER_WRITE, &uimg, argc, argv, True);

	message("horizontal at %d rows\n", bgn_row);
	for (r=0; r<frames; r++)
	{	inbp = (byte*)inbuf + (r+bgn_row)*cols + fsize*(rows-1);/* rows = old_img.frames */
		obp = (byte*)obuf;
		for (f=0; f < rows; f++, inbp -= fsize + cols)
		    for (c=0; c < cols; c++)
			*obp++ = *inbp++;
		if(fwrite(obuf, uimg.pxl_out, fsizeNew, out_fp) != fsizeNew)
			syserr("error during write h");
	}
	break;
default:
case 'd':{
int	big_row, big_cln;
byte	*tbuf;
	message("diagnal: angle = %.2f parallel %c axle\n", angle, xyz);
	angle *= M_PI / 180.;
	cosl = cos(angle);
	ssin = sin(angle);
	sinl = fabs(ssin);
	tanl = sinl / cosl;
	if (bgn_frm > frms)	bgn_frm = frms;
	if (bgn_row > rows)	bgn_row = rows;
	if (bgn_cln > cols)	bgn_cln = cols;
	switch(xyz){
	case 'x':	/* max fsize = sqrt(r^2 + f^2) * c	*/
		big_row = sqrt((double)(rows*rows + frms*frms));
		big_cln = cols;
		break;
	case 'y':
		big_row = rows;
		big_cln = sqrt((double)(frms*frms + cols*cols));
		break;
	case 'z':
	default:
		big_row = frms;
		big_cln = sqrt((double)(rows*rows + cols*cols));
	}
	obuf = nzalloc(big_row, big_cln, "d_obuf");
	if (frames>1){
	    tbuf = (byte*)zalloc(big_row, big_cln, "tbuf");
	    message("FramingR %d, FramingC %d\n", big_row, big_cln);
	}
	for (f=0; f<frames; f++){
	    switch(xyz) {
	    case 'x':
		/*###############################################
		#	column is same as old one		#
		&	Z axle angle = 0. Parallel to X axle	#
		#	Z parameter (frame #) is main parameter	#
		###############################################*/
		width = cols;	l_r = fsize;
		if (bf){
		    if (angle > 0){
			rowsNew = (frms - bgn_frm + 1) / cosl;
			bgn_row = 1;
		    }
		    else{
			rowsNew = bgn_frm / cosl;
			bgn_row = bgn_frm * tanl;
			if (bgn_row > rows)
			{
				bgn_frm = (bgn_row - rows + 1)/tanl;
				bgn_row = rows;
			}
			else	bgn_frm = 1;
		    }
		    if (rowsNew * sinl > rows)	rowsNew = rows / sinl;
		}
		else{	/* bgn_row mode	*/
			rowsNew = (rows - bgn_row + 1) / sinl;
			if (rowsNew*cosl > frms)
				rowsNew = frms/cosl;
			if (angle>0)	bgn_frm = 1;
			else{
				bgn_frm = frms - (int)(rowsNew * cosl);
				bgn_row += rowsNew * sinl;
			}
		}
		message("rn=%d, br=%d, bf=%d\n", rowsNew, bgn_row, bgn_frm);
		clnsNew = cols;
		obp = (byte*)obuf + clnsNew;
		inbp = (byte*)inbuf+(bgn_frm-1)*fsize;
		ibp = ibuf+fsize;
		/* 1st coln is integer. There is no need of interpolating	*/
		memcpy(obuf, inbp+(bgn_row-1)*cols, clnsNew);
		for (i=df=0; i<fsize; i++)	*ibp++ = *inbp++;
		for(r = 1; r < rowsNew; r++)	/* rowsNew = frames	*/
		{
		    cfmod = r * cosl - df;
		    if (cfmod>=0)
		    {	ibp = ibuf+fsize;	df++;
			memcpy(ibuf, ibp, fsize*sizeof(*ibp));
			for(i = 0; i < fsize; i++)
				*ibp++ = (int)(*inbp++ & 0xFF);
		    }
		    else	cfmod += 1;
		    i = rfmod = r * ssin;
		    ibp = ibuf + (bgn_row-1 + i) * cols;
		    rfmod -= i;
		    for(c = 0; c < clnsNew; c++)
			*obp++ = interpolation(ibp++, width, l_r, fabs(rfmod), cfmod);
		}
	break;
		/*###############################################
		#	new rows is old row on X-Y surface	#
		&	Z axle angle = 0. For consistency.	#
		#	new column is old row. Parallel Y axle	#
		###############################################*/
	    case 'y':
		width = 1;	l_r = fsize;
		if (bf){
		    if (angle > 0){
			rowsNew = (frms - bgn_frm + 1) / cosl;
			bgn_cln = 1;
		    }
		    else{
			rowsNew = bgn_frm / cosl;
			bgn_cln = bgn_frm * tanl;
			if (bgn_cln > cols)
			   {	bgn_frm = (bgn_cln - cols + 1)/tanl;
				bgn_cln = cols;
			   }
			else	bgn_frm = 1;
		    }
		    if (rowsNew*sinl>cols)	rowsNew = cols/sinl;
		}
		else{	/* bgn_cln mode	*/
		    if (!bgn_cln)	bgn_cln++;
		    rowsNew = (cols - bgn_cln + 1) / sinl;
		    if (rowsNew*cosl > frms)	rowsNew = frms/cosl;
		    if (angle>0)	bgn_frm = 1;
		    else{
			bgn_frm = frms - (int)(rowsNew * cosl);
			bgn_cln += rowsNew * sinl;
		    }
		}
		message("nr=%d, bc=%d, bf=%d\n", rowsNew, bgn_cln, bgn_frm);
		clnsNew = rows;
		obp = (byte*)obuf;
		inbp = (byte *)inbuf+(bgn_frm-1)*fsize;
		ibp = ibuf+fsize;
		for (i=0; i<fsize; i++)	*ibp++ = *inbp++;

		for(r=df=0; r < rowsNew; r++)
		{
		    cfmod = r * cosl - df;
		    if (cfmod>=0){
			ibp = ibuf+fsize;	df++;
			memcpy(ibuf, ibp, fsize*sizeof(*ibp));
			for(i = 0; i < fsize; i++)
				*ibp++ = (int)(*inbp++ & 0xFF);
		    }
		    else	cfmod += 1;
		    i = rfmod = r * ssin;
		    ibp = ibuf + bgn_cln-1 + i;
		    rfmod -= i;
		    for(c=0; c < clnsNew; c++, ibp+=cols)
			*obp++ = interpolation(ibp, width, l_r, fabs(rfmod), cfmod);
		}
	break;
		/*###############################################
		#	new column is on the X-Y surface	#
		&	X axle angle degree = 0.		#
		#	new row is old frame. Parallel Z axle	#
		###############################################*/
	    default:
	    case 'z':	width = cols;
		if (bc){
		    if (angle > 0){
			clnsNew = (cols - bgn_cln + 1) / cosl;
			bgn_row = 1;
		    }
		    else{
			clnsNew = bgn_cln / cosl;
			bgn_row = clnsNew * sinl;
			if (bgn_row > rows)
			{	bgn_cln = (bgn_row-rows+1) / tanl;
				bgn_row = rows;
			}
			else	bgn_cln = 1;
		    }
		    if (clnsNew * sinl > rows)	clnsNew = rows / sinl;
		}
		else{
		    clnsNew = (rows - bgn_row + 1) / sinl;
		    if (clnsNew * cosl > cols)	clnsNew = cols / cosl;
		    if (angle>0)	bgn_cln = 1;
		    else{
			bgn_cln = cols - clnsNew * cosl;
			bgn_row += clnsNew *sinl;
		    }
		}
		message("nc=%d, bc=%d, br=%d\n", clnsNew, bgn_cln, bgn_row);
		rowsNew = frms;
		obp = (byte*)obuf;
		inbp = (byte*)inbuf;	/* convert to integer	*/
		for(r=0; r < rowsNew; r++)	/* rowsNew = frames	*/
		{
		    ibp = ibuf;
		    for(i=0; i < fsize; i++)
			*ibp++ = (int)(*inbp++ & 0xFF);
		    ibp = ibuf + (bgn_row-1)*cols + (bgn_cln-1);
		    *obp++ = *ibp;
		    for(c=1; c < clnsNew; c++){	/* colsNew = ?/cosl */
		    register int	j = rfmod = c * ssin;
			i = cfmod = c * cosl;
			*obp++ = interpolation(ibp+j*cols+i, width, l_r,
				rfmod-j, cfmod-i);
		    }
		}
	}/* end of switch(xyz)	*/

	if (!f){	/* first frame */
	register int	r=rows, c=cols, f=frms;
		if (frames > 1){
			cols=big_cln;	rows=big_row;
		}
		else {	cols=clnsNew;	rows=rowsNew;	}
		frms=frames;
		fsizeNew = rows*cols;
		(*uimg.header_handle)(HEADER_WRITE, &uimg, argc, argv, True);
		frms=f;	rows=r;	cols=c;
	}
	if (bf)	bgn_frm += Sign(angle);
	else if(bc)	bgn_cln += Sign(angle);
	else	bgn_row += Sign(angle);
	{
	register byte*	opp=(byte*)obuf;
	if (frames>1){
		opp = tbuf;
		framing(obuf, opp, rowsNew, clnsNew, big_row, big_cln);
		if (bc && (!bgn_cln || bgn_cln > cols) ||
			!bgn_row || bgn_row > rows ||
			bf && (!bgn_frm || bgn_frm > frms))
		    f = frames;	/*	STOP -- out of boundary	*/
	}
	i = fwrite(opp, uimg.pxl_out, fsizeNew, out_fp);
	if(i!=fsizeNew)	syserr("error during write (%c)%d", xyz, i);
	}
    }
}
}
exit(0);
}


/*=======================================================================
*	cur_p is relative to current position.		|		*
*	width:						| 10 (-a) 11	*
*		for X-Y (Z axle) is column - offset	|		*
*		for Y-Z (X axle) is row - offset	| 00	01	*
*		for Z-X (Y axle) is frames - ofset	|		*
*	riF, ciF is exactly current position.		| 10	11	*
*======================================================================*/
interpolation(cur_p, span, l_r, uh, rw)
register int	*cur_p;
float	uh, rw;	/* the fraction percentage	*/
{
register float	fh = 1.0 - uh,	/* the % of regular position	*/
		fw = 1.0 - rw,
	p00, p01, p10, p11;

p00 = *cur_p * fh;
p10 = *(cur_p + span) * uh;
p01 = *(cur_p + l_r) * fh;
p11 = *(cur_p + span + l_r) * uh;
return	(fw*(p00 + p10) + rw*(p01 + p11));
}

framing(ibp, obp, ir, ic, or, oc)
byte	*ibp, *obp;
{
int	mtop = (or - ir) >> 1,
	mleft= (oc - ic) >> 1;
register int	r, c;

for (r=0; r<mtop; r++)
    for (c=0; c<oc; c++)
	*obp++ = 0;
for (r=0; r<ir; r++){
	for (c=0; c<mleft; c++)
		*obp++ = 0;
	for (c=0; c<ic; c++)
		*obp++ = *ibp++;
	for (c=0; c<oc-ic-mleft; c++)
		*obp++ = 0;
}
for (r=0; r<or-ir-mtop; r++)
    for (c=0; c<oc; c++)
	*obp++ = 0;
}
