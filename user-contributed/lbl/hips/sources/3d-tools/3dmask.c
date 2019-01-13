/* 3dmask.c --	filter an image by applying one or more masks and then
%		applying another function to the various mask outputs.
%		The input is in Byte, Short, Integer and Floating Point format,
%		the output is floating point.
#
%	Copyright (c)	1990 - 1995	Jin, Guojun -	All rights reserved
%
% usage:
%	3dmask [-m filter_number] [+r] [-M] [<] input [> +o output]
%			or
%	3dmask [-f filter_descriptor_file] [+r] [<] input [> +o output]
%
%	+o output is for binary & ascii difference on PC.
%	The default filter is 1.  The definition for each of these filters
%	is to be found in /???/3dmasks/mask#?.@, where #? is the combinations
%	of the filter_size+number_of_sets+function_number and filter type;
%	@ is extension used to describe function, rotation direction, angle,
%	or complicated filter name etc..
%	The -f switch allows a new filter to be supplied by the user.
%	The format of the filter definition file is as follows:
%
%	"filter name and description"
%	masksize number_of-Set_of_Masks Function_name
%	Number_of_masks_in_set1
%	frame_position	mask--1
%	  .
%	  .
%	  .
%	frame_position	mask--(masksize)
%
%	number_of_masks-(number-of-set)
%	frame_position	mask--1
%	  .
%	  .
%	  .
%	frame_position	mask--(masksize)
%
% where the masksize is the length of a side of all masks (which must be
% square), masks are given as a sequence of integers in column-fastest order;
% frame-position is for 3rd direction (frames), its range is from
% -1/2 masksize to 1/2 masksize; the number-of-masks is different in each set,
% since the elements of some filter frames are total zero, for fast processing,
% we omit those frames, also the frame-position will missing in that(those)
% set(s); the externsion is used for exactly same filter model with different
% mask operations, such as Sobel, Prewitt etc.;
% and the function applied to the output of the masks is chosen from:
%
%	1	MAXABS	- the maximum absolute value of all mask outputs
%	2	MEANSQ  - the square root of the sum of the squares of all masks
%	3	SUMABS  - the sum of the absolute value of all mask outputs
%	4	MAX	- the maximum mask output
%	5	MAXFLR	- the maximum mask output, floored at zero.
%	6	MAXSFLR	- the larger of |mask-1| and |mask-2|, minus |mask-3|,
%			  floored at zero.
%	7	MUL	- the product of the mask outputs, each floored at zero.
%	8	NORM	- the first mask output normalized by the sum of the
%			  mask entries.
%	9	DIFF	- the value of the pixel minus the normalized mask
%			  output. Byte format only.
%	10	ORIENT	- compute orientation: 360*atan(mask2/mask1)/2*PI
%
% note:	important MESSAGE -- Be sure to use -O (Optimize) option to compile.
%	Otherwise, you get 76% slower !!
% cc -O -DMY_LIB=$(LIB_PATH) -o DESTDIR/3dmask 3dmask.c -lccs -lhips -lm
%
% AUTHOR:	Jin Guojun - LBL	12/6/90
% 2/1/91:	adding short, int & float model
% 3/16/95:	macaroni (Macro) for threading
*/

#include "header.def"
#include "imagedef.h"
#include <math.h>

U_IMAGE	uimg;

#define	frm		uimg.frames
#define	row		uimg.height
#define	col		uimg.width

#ifndef	MY_LIB
#define	MY_LIB		"/home/itg8/users/jin/3dmasks/mask"
#endif

#define	t_frm(f)	(f<0 ? 0 : (f>=frm ? frm-1 : f))
#define	t_col(c)	(c<0 ? 0 : (c>=col ? col-1 : c))
#define	t_row(r)	(r<0 ? 0 : (r>=row ? row-1 : r))
#ifdef	BAD_CHAR_C
#define	BAD_CHAR_C	& 0xFF
#else
#define	BAD_CHAR_C
#endif
#define	masking_win(it_t, B_OPS) {	\
	register it_t	*bufp;		\
	if (flag)	{		\
	it_t	*tmpp = buf[t_frm(f+framen) % masksz];		\
		for (dr = minusd; dr <= plusd; dr++)	{	\
			bufp = tmpp + t_row(r+dr) * col;	\
			for (dc=minusd; dc <= plusd; dc++)	\
				k += *mskp++ * (*(bufp + t_col(c+dc)) B_OPS); \
		}	\
	} else	{	\
	bufp = (it_t *)buf[(f+framen) % masksz] + (r+minusd)*col + c + minusd;\
		for (dr=minusd; dr <= plusd; dr++)		\
		{	for (dc=minusd; dc <= plusd; dc++)	\
			    k += *mskp++ * (*bufp++ B_OPS);	\
			bufp += bp_inc;	\
		}	\
	}	\
}

arg_fmt_list_string	arg_fmt[] =	{
	{"-M", "%b", 1, 1, 1, "display Messages"},
	{"-f", "%1 %s+", 0, 2, 0, "non-standard mask file name"},
	{"-m", "%1 %s++", 0, 2, 0, "standard mask number [1]"},
	{"+r", "%b", -1, 1, 0, "reverse values in all masks"},
	{"-o", "%s", 0, 1, 1, "output_file	for non-binary file system"},
	{" [<] in_file [> out_file]", 0,0,0, NULL},	NULL};
char	*s, maskfile[128] = MY_LIB;

#define	name	maskfile


main(argc, argv)
int	argc;
char**	argv;
{
char	**fl, *of_name;
MType	fsize, i, j, k;
int	mfunc,
	sm, smask,	/*	# of mask sets		*/
	nm, nmask,	/*	# of masks in each set	*/
	masksz,
	**mlist,	/*	mask buffers		*/
	mskrev=1,	/*	mask reversor		*/
	bp_inc, normd;	/*	normalization divisor	*/
int	flag,	/* out of bound flag for any one side boundary	*/
	boundl, boundr,
	boundt, boundb,
	boundf, bound,
	minusd, plusd,
	f=0, r, c, df, dr, dc,
	framen=0,
	*mval;		/*	masks working place	*/
float	val, *obuf, *obp;
void	*buf[MaxMaskSZ];	/* maximum frames can be loaded into memory */
bool	Msg=0;
double	d2r = 180 / M_PI;
FILE	*mfp;
register int	*mskp;

format_init(&uimg, IMAGE_INIT_TYPE, HIPS, -1, *argv, "Mar15-5");

	if ((i=parse_argus(&fl, argc, argv, arg_fmt,
		&Msg, &f,&maskfile, &f,&maskfile, &mskrev, &of_name)) < 0)
		exit(i);
	if (of_name && !(out_fp=freopen(of_name, "wb", stdout)))	{
		message("output file %s opend error", of_name);
errout:		parse_usage(arg_fmt),	exit('i');
	}
	if (i && !(in_fp=freopen(fl[0], "rb", in_fp)))
		syserr("input file %s not found", fl[0]);

io_test(fileno(in_fp),	goto	errout);

if (!f)	strcat(maskfile, "1");

(*uimg.header_handle)(HEADER_READ, &uimg, 0, 0);

if (uimg.in_form > IFMT_FLOAT)	syserr("format level should be lower than FP");
uimg.o_form = IFMT_FLOAT;
uimg.pxl_out = sizeof(float);

(*uimg.header_handle)(HEADER_WRITE, &uimg, argc, argv, True);

if ((mfp=fopen(maskfile,"r")) == NULL)
	syserr("can't open mask file %s", maskfile);
s = name;
while((*s++ = getc(mfp)) != '\n');	*s = NULL;
fscanf(mfp, "%d %d %d", &masksz, &smask, &mfunc);

if (masksz<1 || masksz>MaxMaskSZ || smask<1 || smask>MAXMASKSet ||
	mfunc<1 || mfunc>MAXMASKFUNCS)
	message("Bad filter descriptor value:	mask_size=%d, ",masksz),
	syserr("sets of masks=%d, function_number=%d\n", smask, mfunc);

fsize = row * col;
mlist	= ZALLOC((MType)smask, SIZEOF(*mlist), No);
mval	= ZALLOC((MType)smask, SIZEOF(*mval), No);
obuf	= NZALLOC(fsize, SIZEOF(*obuf), "obuf");
for (i=0; i < masksz; i++)	/* locate working frame-bufffers	*/
	buf[i] = ZALLOC(fsize, SIZEOF(buf[0]), "bufi");

for (sm=normd=0; sm < smask; sm++)	{	/* Reading Mask Sets	*/
	fscanf(mfp, "%d", &nm);/* get the number of masks in this set	*/
	mskp = nzalloc((MType)(masksz*masksz+1)*nm+1, SIZEOF(*mskp));
	mlist[sm] = mskp;
	*mskp++ = nm;		/* first value is mask numbers	*/
	if (nm>masksz)	syserr("%dmasks in set%d is > masksize %d",nm,sm,masksz);

	for (; nm > 0; nm--)	/* fetch mask elements	*/
	    for (k=masksz*masksz; k>=0; k--){	/* incl frame offset	*/
		if (fscanf(mfp, "%d", &j) == EOF)
		   syserr("%s: unexpected end of the filter descriptor file\n",
			argv[0]);
		normd += j;	/* sum all mask elements in every sets	*/
		*mskp++ = j * mskrev;
	    }
}

message("%s[%s]: using filter:	%s\n", *argv, Mversion, name);

plusd	= masksz / 2;		/* set central position	*/
minusd	= plusd - masksz + 1;
boundl=boundt=boundf = -minusd;	/* compute boundaries of where the	*/
boundr	= col - plusd - 1;	/* mask overlaps the image completely	*/
boundb	= row - plusd - 1;	/* for more efficient convolution &	*/
bound	= frm - plusd - 1;	/* calculate bufp move difference for	*/
bp_inc	= col - masksz;		/* down one pos where not at boundary.	*/

/*************************************************
********	start	masking		*********/

for (df=0; df<plusd; df++)	/* get first half set of frames	*/
    if ((f=upread(buf[df], uimg.pxl_in, fsize, in_fp)) != fsize)
	syserr("unexpected end of image_file");
for (f=0; f<frm; f++)
{
	obp = obuf;
	if (f+df < frm)	/* get next frame	*/
	   if (upread(buf[(f+df) % masksz], uimg.pxl_in, fsize, in_fp) != fsize)
		syserr("unexpected end_of_file frame{%d}", f);
#ifdef	_DEBUG_
	else if(Msg)	message("masking frame[%d], ef[%d], lc[%d]\n",
			f, f+df, (f+df) % masksz);
	message("now: buf=%d, t_frm=%d\n",
		buf[t_frm(f+framen)%masksz], t_frm(f+framen));
#endif
	for (r=0;r < row; r++)
	    for (c=0; c < col; c++)	{
		flag = (r < boundt || r > boundb ||
			c < boundl || c > boundr ||
			f < boundf || f > bound);

		for (sm=0; sm < smask; sm++)	{
		    k = 0;
		    mskp = mlist[sm];
		    nmask = *mskp++;
		    for (nm=0; nm < nmask; nm++)	{
			framen = *mskp++;
			switch (uimg.in_form)	{
			case IFMT_BYTE:	masking_win(byte, BAD_CHAR_C);	break;
			case IFMT_SHORT:masking_win(short, );	break;
			case IFMT_LONG:	masking_win(long_32,);	break;
			case IFMT_FLOAT:masking_win(float, );
			}	/* end switch (uimg.in_form)	*/
		    }	/* end for nm	*/
			mval[sm] = k; /* / (masksz * masksz);*/
		/* need not divid by size since must use scale_gray anyway */
		}	/* end for mask	set	*/

		switch(mfunc) {
		case MASKFUNC_MAXABS:
			k = abs(mval[0]);
			for (i=1; i < smask; i++)
				if(abs(mval[i]) > k)	k = abs(mval[i]);
			val = k;
			break;
		case MASKFUNC_MEANSQ:
			for (i=k=0; i < smask; i++)
				k += mval[i]*mval[i];
			val = sqrt((double)k);
			break;
		case MASKFUNC_SUMABS:
			for (i=k=0; i < smask; i++)
				k += abs(mval[i]);
			val = k;
			break;
		case MASKFUNC_MAXX:
		case MASKFUNC_MAXFLR:
			k = mval[0];
			for (i=1; i < smask; i++)
				if (mval[i] > k)	k = mval[i];
			val = (k>0 || mfunc == MASKFUNC_MAXX) ? k : 0;
			break;
		case MASKFUNC_MAXSFLR:
			k = abs(mval[0]) > abs(mval[1])
				? abs(mval[0]) : abs(mval[1]);
			k -= abs(mval[2]);
			val = k>0 ? k : 0;
			break;
		case MASKFUNC_MUL:
			for (k=1, i=0; i < smask; i++)
				k *= mval[i]>0 ? mval[i] : 0;
			val = k;
			break;
		case MASKFUNC_NORM:
			val = ((float)mval[0])/normd;
			break;
		case MASKFUNC_DIFF:
			val = *((byte*)buf[f % masksz]+uimg.pxl_in*(r*col+c)) -
				(float)mval[0] / normd;
			break;
		case MASKFUNC_ORIENT:
			if (mval[0] || mval[1])
				val = d2r * atan2( (double)mval[1],
					(double)mval[2] );
			else	val = 0;
			break;
		default:
			syserr("bad function type?%d", mfunc);
		}
		*obp++ = val;
	    }	/* end for coln	& end for row	*/

	if (fwrite(obuf, sizeof(*obuf), fsize, out_fp) != fsize)
		syserr("<#%d>: write error", f);
#ifdef	_DEBUG_
	else if(Msg)	message("frame %d	done\n", f);
#endif
}	/*	end for frame	*/
}
