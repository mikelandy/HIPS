/*
% 3DROTATE180 - rotate 3D image into another 3D image for displaying
%		and looking from the back toward front.
%
%	Copyright (c)	1991	Jin, Guojun
*/
char	usage[]="options\n\
%	Regularly, rotate horizontally to back (along Y axle).	\n\
%	-f #	frame # which begin to process.			\n\
%	-n #	number of frames will be rotated and output.	\n\
%	-r	rolls image along X axle to back.		\n\
%	[<] infile [> [-o] outfile]\n";
/*
@ Improtant Note:
@	MUST include <math.h> in this file
@
@ AUTHOR:	Jin Guojun - LBL	2/14/91
*/

#include "header.def"
#include "imagedef.h"
#include <math.h>

U_IMAGE	uimg;

#define	inbuf	uimg.src
#define	obuf	uimg.dest


main(argc, argv)
int	argc;
char**	argv;
{
/*  !!!	input number is start from 1 and convert to from 0. See line 75	*/
MType	bgn_frm=0, frmsNew=0,
	i, f, r, c, fsize;
bool	rolling=0;

format_init(&uimg, IMAGE_INIT_TYPE, HIPS, -1, *argv, "S20-1");

for (i=1; i<argc; i++)
    if (*argv[i] == '-'){
	c=1;
	switch(argv[i][c++]){
	case 'f':
		bgn_frm = arget(argc,argv,&i,&c);	break;
	case 'n':
		frmsNew = arget(argc,argv,&i,&c);	break;
	case 'r':
		rolling++;	break;
	case 'o':
		if (avset(argc, argv, &i, &c, 1) &&
			(out_fp=freopen(argv[i]+c, "wb", stdout)))	break;
		message("%s can't be opened for write", argv[i]);
	default:
errout:		usage_n_options(usage, i, argv[i]);
	}
    }
    else if ((in_fp=freopen(argv[i], "rb", stdin)) == NULL)
	    syserr("input file %s not found", argv[i]);

io_test(fileno(in_fp), goto	errout);

(*uimg.header_handle)(HEADER_READ, &uimg, 0, 0);
uimg.pxl_out = uimg.pxl_in;
uimg.o_form = uimg.in_form;
fsize = uimg.width * uimg.height;

bgn_frm--;
if (bgn_frm<1 || bgn_frm>uimg.frames)	bgn_frm = uimg.frames-1;

inbuf = nzalloc(uimg.pxl_in, fsize*uimg.frames, "inbuf");
obuf = nzalloc(uimg.pxl_in, fsize, "obuf");

uimg.load_all = uimg.frames;
i = (*uimg.std_swif)(FI_LOAD_FILE, &uimg, 0, No);

if (!frmsNew || frmsNew > bgn_frm)	frmsNew = bgn_frm+1;

{
char	mesgbuf[1024];
   sprintf(mesgbuf, "%s: start at frame %d (%d frames).	",
	*argv, bgn_frm+1, frmsNew);
   if (rolling)
	strcat(mesgbuf, "Rolling forward to back\n");
   else	strcat(mesgbuf, "Rotate horizontally to back\n");
   msg("%s\n", mesgbuf);
   (*uimg.header_handle)(ADD_DESC, &uimg, mesgbuf);
}
(*uimg.header_handle)(HEADER_WRITE, &uimg, argc, argv, True);

if (rolling)
switch (uimg.pxl_in){
case 1:	{
	register byte	*inbp, *tmpp, *obp;
	for (f=0; f<frmsNew; f++){
		tmpp = (byte*)inbuf;
		tmpp += (bgn_frm-f+1)*fsize - uimg.width;
		obp = (byte*)obuf;
		for (r=0; r < uimg.height; r++, tmpp -= uimg.width)
		    for (c=0, inbp=tmpp; c<uimg.width; c++)
			*obp++ = *inbp++;
	if (fwrite(obuf, uimg.pxl_in, fsize, stdout) != fsize)
		syserr("error during B write.");
	}
}break;
case 2:	{
	register short	*inbp, *tmpp, *obp;
	for (f=0; f<frmsNew; f++){
		tmpp = (short*)inbuf;
		tmpp += (bgn_frm-f+1)*fsize - uimg.width;
		obp = (short*)obuf;
		for (r=0; r < uimg.height; r++, tmpp -= uimg.width)
		    for (c=0, inbp=tmpp; c<uimg.width; c++)
			*obp++ = *inbp++;
	if (fwrite(obuf, uimg.pxl_in, fsize, stdout) != fsize)
		syserr("error during S write.");
	}
}break;
case 4:	{
	register int	*inbp, *tmpp, *obp;
	for (f=0; f<frmsNew; f++){
		tmpp = (int*)inbuf;
		tmpp += (bgn_frm-f+1)*fsize - uimg.width;
		obp = (int*)obuf;
		for (r=0; r < uimg.height; r++, tmpp -= uimg.width)
		    for (c=0, inbp=tmpp; c<uimg.width; c++)
			*obp++ = *inbp++;
	if (fwrite(obuf, uimg.pxl_in, fsize, stdout) != fsize)
		syserr("error during L write.");
	}
}
}
else for (f=0; f<frmsNew; f++){
	switch(uimg.pxl_in){
	case 1:	{
	register byte	*inbp, *obp, *tmpp = (byte*)inbuf;
		tmpp += (bgn_frm-f)*fsize + uimg.width - 1;
		obp = (byte*)obuf;
		for (r=0; r < uimg.height; r++, tmpp += uimg.width)
		    for (c=0, inbp=tmpp; c<uimg.width; c++)
			*obp++ = *inbp--;
	}break;
	case 2:	{
	register short	*inbp, *obp, *tmpp = (short*)inbuf;
		tmpp += (bgn_frm-f)*fsize + uimg.width - 1;
		obp = (short*)obuf;
		for (r=0; r < uimg.height; r++, tmpp += uimg.width)
		    for (c=0, inbp=tmpp; c<uimg.width; c++)
			*obp++ = *inbp--;
	}break;
	case 4:	{
	register int	*inbp, *obp, *tmpp = (int*)inbuf;
		tmpp += (bgn_frm-f)*fsize + uimg.width - 1;
		obp = (int*)obuf;
		for (r=0; r < uimg.height; r++, tmpp += uimg.width)
		    for (c=0, inbp=tmpp; c<uimg.width; c++)
			*obp++ = *inbp--;
	}
	}/* end of switch (uimg.pxl_in)	*/
	if (fwrite(obuf, uimg.pxl_in, fsize, stdout) != fsize)
		syserr("error during write.");
}
return(0);
}
