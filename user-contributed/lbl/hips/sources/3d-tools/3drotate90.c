/*
@ 3DROTATE90 - rotate 3D image into another 3D image for displaying and looking
@	the different surface of the image.
@
@	-l (look from left side) slices vertically (y-z surface, parallel to Y
@		axle). It works exactly same as in 3drotate.
@	-L horizontally rotate90 to left. NO keep U-L corner.
@	-t(top) specify slicing horizontally (x-z surface, parallel to X axle).
@		this option is similar to in 3drotate but slower.
@ After using 3DROTATE90 twice in same direction will get same result as
@	using 3DROTATE90 once in the other direction. So 3drotate90 only can
@	let people to see the front, left and top. For other 3 directions. use
@	3DROTATE180 goto the back first and then use 3drotate90 to see the
@	right (left to back) and bottom (top to back).
@	-L will help to see different direction.
@
%	Copyright (c)	1991	Jin, Guojun
*/
char	usage[]="options\n\
-l	rotate to left with upper-left corner no changed\n\
-L	rotate to left horizontally\n\
-t	rotate to top with U-P-C no changed\n\
-c #	starting column for -l or -L\n\
-r #	starting row for -t\n\
-n #	number of frames will output\n\
[<] input [> -o output]\n";
/*
@ Improtant Note:
@	MUST include <math.h> in this file
@
@ AUTHOR:	Jin Guojun - LBL	2/1/91
*/

#include "header.def"
#include "imagedef.h"
#include <math.h>

U_IMAGE	uimg;
bool	slice_dir, t2f;

#define	ibuf	uimg.src
#define	obuf	uimg.dest
#define	tbuf	uimg.cnvt
#define	p_size	uimg.pxl_in
#define	o_size	uimg.pxl_out
#define	img_w	uimg.width
#define	img_h	uimg.height

#define	GValue()	arget(argc, argv, &i, &c);
#define	F2T	if (t2f) ilc_transfer(obuf, obuf, fsizeNew, 4, 0, o_size=3)


main(argc, argv)
int	argc;
char**	argv;
{
/*  !!!	input number is start from 1 and convert to from 0. See line 110 */
int	bgn_cln=1, bgn_row=uimg.color_dpy=1, frmsNew=0;
MType	i, f, r, c,
	fsizeNew, fsize;

format_init(&uimg, IMAGE_INIT_TYPE, HIPS, HIPS, *argv, "S20-2");

for (i=1; i<argc; i++)
    if (*argv[i] == '-') {
	c = 1;
	switch (argv[i][c++])	{
	case 'c':
		bgn_cln = GValue();	break;
	case 'r':
		bgn_row = GValue();	break;
	case 'L':
	case 'l':
	case 't':
		slice_dir = argv[i][1];	break;
	case 'n':
		frmsNew = GValue();	break;
	case 'o':
		if (avset(argc, argv, &i, &c, 1) &&
			(out_fp=freopen(argv[i]+c, "wb", stdout)))	break;
		msg("%s can't be opened for write", argv[i]+c);
	default:
errout:		usage_n_options(usage, i, argv[i]);
	}
}
else if ((in_fp=freopen(argv[i], "rb", stdin)) == NULL)
	syserr("input file %s not found", argv[i]);

io_test(fileno(in_fp), goto	errout);

(*uimg.header_handle)(HEADER_READ, &uimg, 0, 0);
(*uimg.header_handle)(HEADER_TRANSF, &uimg, 0);
if (uimg.color_form == CFM_SEPLANE)
	uimg.color_form = CFM_ILC;
t2f = uimg.color_form == CFM_ILC;
fsize = img_w * img_h;

if (bgn_row > img_h)	bgn_row = img_h;
if (bgn_cln > img_w)	bgn_cln = img_w;
if (bgn_cln)	bgn_cln--;
if (bgn_row)	bgn_row--;

ibuf = nzalloc(p_size, fsize*uimg.frames, "ibuf");
uimg.load_all = uimg.frames;
(*uimg.std_swif)(FI_LOAD_FILE, &uimg, 0, No);
if (t2f)	{
	tbuf = nzalloc(p_size=4, fsize*uimg.frames, "tbuf");
	ilc_transfer(tbuf, ibuf, fsize*uimg.frames, 3, 0, 4);
	free(ibuf);	ibuf = tbuf;
}

switch(slice_dir)
{
case 't':	/*	look from top	*/
default:
	if (!frmsNew || frmsNew > img_h-bgn_row)
		frmsNew = img_h - bgn_row;
	i = img_h = img_w;	img_w = uimg.frames;
	uimg.frames = frmsNew;	frmsNew += bgn_row;
	fsizeNew = img_w * img_h;
	obuf = nzalloc(p_size, fsizeNew, "tbuf");
	{
	char	mesgbuf[1024];
	sprintf(mesgbuf,
	"horizontal slicing at row%d (%d frames). Rotate forward & turn right\n",
		bgn_row+1, uimg.frames);
	msg("%s", mesgbuf);
	(*uimg.header_handle)(ADD_DESC, &uimg, mesgbuf);
	}
	(*uimg.header_handle)(HEADER_WRITE, &uimg, argc, argv, True);

	switch(p_size)	{
	case 1:	{
	register byte	*inbp, *obp, *itmp, *otmp;
	    for (r=bgn_row; r<frmsNew; r++){
		inbp = itmp = (byte*)ibuf + r*i;
		obp = otmp = obuf;
		for (f=0; f < img_w; f++, inbp=itmp+=fsize, obp=otmp+f)
		    for (c=0; c < img_h; c++, obp+=img_w)
			*obp = *inbp++;
		if (fwrite(obuf, o_size, fsizeNew, out_fp) != fsizeNew)
			syserr("error during h write.");
	    }
	}break;
	case 2:	{
	register short	*inbp, *obp, *itmp, *otmp;
	    for (r=bgn_row; r<frmsNew; r++){
		inbp = itmp = (short*)ibuf + r*i;
		obp = otmp = obuf;
		for (f=0; f < img_w; f++, inbp=itmp+=fsize, obp=otmp+f)
		    for (c=0; c < img_h; c++, obp += img_w)
			*obp = *inbp++;
		if (fwrite(obuf, o_size, fsizeNew, out_fp) != fsizeNew)
			syserr("error during h write.");
	    }
	}break;
	case 4:	{
	register long	*inbp, *obp, *itmp, *otmp;
	    for (r=bgn_row; r<frmsNew; r++){
		inbp = itmp = (long*)ibuf + r*i;
		otmp = obuf;
		for (f=0; f < img_w; f++, inbp=itmp+=fsize)
		    for (c=0, obp=otmp+f; c<img_h; c++, obp+=img_w)
			*obp = *inbp++;
		F2T;
		if (fwrite(obuf, o_size, fsizeNew, out_fp) !=
		   fsizeNew)	syserr("error during h write.");
	    }
	}
	}	/* end of switch(p_size)	*/
	break;
case 'l':	/*	look from left side.	*/
	if (!frmsNew || frmsNew > img_w-bgn_cln)
		frmsNew = img_w - bgn_cln;
	i = img_w;	img_w = img_h;
	img_h = uimg.frames;	uimg.frames = frmsNew;
	fsizeNew = img_w * img_h;
	obuf = nzalloc(p_size, fsizeNew, "lbuf");
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
	switch(p_size){
	case 1:	{
	register byte	*inbp, *tmpp=ibuf, *obp=obuf;
		for (f=0, tmpp+=c; f < img_h; f++, tmpp+=fsize)
		    for (r=0, inbp=tmpp; r<img_w; r++, inbp+=i)/* old_cln */
			*obp++ = *inbp;
		}break;
	case 2:	{
	register short	*inbp, *tmpp=ibuf, *obp=obuf;
		for (f=0, tmpp+=c; f < img_h; f++, tmpp+=fsize)
		    for (r=0, inbp=tmpp; r<img_w; r++, inbp+=i)/* old_cln */
			*obp++ = *inbp;
	}break;
	case 4:	{
	register long	*inbp, *tmpp=ibuf, *obp=obuf;
		for (f=0, tmpp+=c; f < img_h; f++, tmpp+=fsize)
		    for (r=0, inbp=tmpp; r<img_w; r++, inbp+=i)/* old_cln */
			*obp++ = *inbp;
		}
		F2T;
	}	/* end switch	*/
	if (fwrite(obuf, o_size, fsizeNew, out_fp) != fsizeNew)
		syserr("error during v write.");
    }	/* end of for	*/
	break;
case 'L':	/*	look from left side.	Y-Z => xyz = 'z'	*/
    if (!frmsNew || frmsNew > img_w-bgn_cln)
	frmsNew = img_w - bgn_cln;
    i = img_w;	img_w = uimg.frames;	uimg.frames = frmsNew;
    fsizeNew = img_w * img_h;
    obuf = zalloc(p_size, fsizeNew, "vbuf");

    {
    char	mesgbuf[1024];
	sprintf(mesgbuf,
	"vertical slicing at column%d (%d frames). Rotate to left horizontally\n",
		bgn_cln+1, frmsNew);
	msg("%s", mesgbuf);
	(*uimg.header_handle)(ADD_DESC, &uimg, mesgbuf);
    }
    (*uimg.header_handle)(HEADER_WRITE, &uimg, argc, argv, True);

    for (c=bgn_cln; c<frmsNew; c++){
	switch(p_size) {
	case 1:	{
	register byte	*inbp, *itmp, *obp, *otmp;
		itmp = (byte*)ibuf + c + fsize*(img_w-1);
		obp = otmp = obuf;
		for (f=0; f < img_w; f++, itmp-=fsize, obp=otmp+f)
		    for (inbp=itmp, r=img_h; r--; inbp+=i, obp+=img_w)
			*obp = *inbp;
	}break;
	case 2:	{
	register short	*inbp, *itmp, *obp, *otmp;
		itmp = (short*)ibuf + c + fsize*(img_w-1);
		obp = otmp = obuf;
		for (f=0; f < img_w; f++, itmp-=fsize, obp=otmp+f)
		    for (r=img_h, inbp=itmp; r--; inbp+=i, obp+=img_w)
			*obp = *inbp;
	}break;
	case 4:	{
	register long	*inbp, *itmp, *obp, *otmp;
		itmp = (long*)ibuf + c + fsize*(img_w-1);
		obp = otmp = obuf;
		for (f=0; f < img_w; f++, itmp-=fsize, obp=otmp+f)
		    for (r=img_h, inbp=itmp; r--; inbp+=i, obp+=img_w)
			*obp = *inbp;
	}
		F2T;
	}	/* end of switch (p_size)	*/
	if (fwrite(obuf, o_size, fsizeNew, out_fp) != fsizeNew)
		syserr("error during L write.");
    }	/* end of for img_w	*/
}	/* end of switch(dir)	*/
return	0;
}
