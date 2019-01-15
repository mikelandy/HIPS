/*	Copyright (c)	1990	Jin, Guojun
%
% mixadd.c - add each frame in a sequence by the corresponding frame of another
%	sequence pixel by pixel. If the file which has higher hirechy consists
%	of less frames, then that last frame is added by each later frames in
%	the input sequence, otherwise corresponding frames are used.	*/

char	usage[]="{%s}\n\
mixadd filter_file [-base_value [-r]] [-a array] [-f #] [-n #] < image [> output]\n\
@ -#	add this value to each pixel in every frame which is taken from	\n\
@		standard input (ignore any input in command line)	\n\
@ -a array_file	will add some value from array file to corresponding frame.\n\
@		The array file format is:	\n\
@	\n\
@		start_frame(0 -- f-1)	number_frame_be_processed(nfbp)	\n\
@		v1 v2 ... v_nfbp	\n\
@\n\
@		More information, see -f & -n options.	\n\
@\n\
@ -f #	starting frame to process. If -a used after -f, then	\n\
@		start_frame value in array_file will be used. Otherwise,\n\
@		-f # will be used.	\n\
@ -n #	number of frames will be processed. If -a used and nfbp	\n\
@		in array_file is less then -n #, the v_nfbp will be used\n\
@		for later (n - nfbp) frames.	\n\
@	Also, if -a used without -n option, then number of frames	\n\
@		which equal to total frames substrct start_frame	\n\
@		will be processed.	\n\
@\n\
@	The following combinations of image formats are supported:	\n\
@\n\
@		File sequence		Standard input sequence	\n\
@\n\
@		any non pyramid format combinations		\n\
@		integer pyramid		integer pyramid		\n\
@		float pyramid		float pyramid		\n";
/*
% AUTHOR	Guojun Jin - LBL	12/30/90
% last change	3/15/91	-- add array operation.
*/

#include "header.def"
#include "imagedef.h"

U_IMAGE	*img, *img1;

#define	ibuf	img1->dest
#define	obuf	img->dest
#define	itemp1	img1->src
#define	in_fp0	img->IN_FP
#define	out_fp0	img->OUT_FP
#define	in_fp1	img1->IN_FP

#define	GValue()	arget(argc, argv, &i, &k)
#define	SValue()	avset(argc, argv, &i, &k, 1)

MType	fsize, diffsize, addbase=0;
int	ccf=0, cf=0, *array, ap, bgn_frm, nfrm, rotate;


main (argc,argv)
int	argc;
char**	argv;
{
int	toplev, i;

/*	img has higher hirechy than img1	*/
img = (U_IMAGE *) ZALLOC((MType)sizeof(*img), 1L, "img"),
img1 = (U_IMAGE *) ZALLOC((MType)sizeof(*img1), 1L, "img1");

format_init(img, IMAGE_INIT_TYPE, HIPS, -1, *argv, "S16-1");
format_init(img1, IMAGE_INIT_TYPE, HIPS, -1, *argv, "S16-1");

for (i=1; i<argc; i++)
    if (*argv[i] == '-'){
	int	k=1;
	switch (argv[i][k++]){
	case 'a':	if (!SValue())	goto	aferr;
	{
		register FILE*	fp_array = fopen(argv[i]+k, "r");
		if (!fp_array)
aferr:			syserr("can't open data array %s", argv[i]);
		fscanf(fp_array, "%d %d", &bgn_frm, &ap);
		array = ZALLOC((MType)ap, (MType)sizeof(*array), "array");
		for (k=0; k<ap; k++)
		    if (fscanf(fp_array, "%d", array+k) < 0)
			syserr("array members are less than %d", ap);
	}
	break;
#ifdef	IBMPC
	case 'i':if (!SValue())	break;
		if ((in_fp0=freopen(argv[i]+k, "rb", stdin)))	break;
		syserr("can't open %s as image input", argv[i]);
#endif
	case 'f':if (SValue())
			bgn_frm = atoi(argv[i]+k) - 1;
		break;
	case 'n':	nfrm = GValue();
		break;
	case 'r':	rotate++;	break;
	case 'o':if (SValue() &&
			(out_fp0=freopen(argv[i]+k, "wb", stdout)))	break;
		syserr("%s can't be opened", argv[i]);
	default:
		if (isfloat(argv[i][1]))
			addbase = atoi(argv[i]+1);
		else
errout:			usage_n_options(usage, i, argv[i]);
	}
    }
    else if ((in_fp1=fopen(argv[i], "rb")) == NULL)
	syserr("can't open frame file -- %s", argv[i]);

io_test(fileno(in_fp0), goto	errout);

(*img->header_handle)(HEADER_READ, img, 0, 0);
img->o_form = img->in_form;
img->pxl_out = img->pxl_in;

if (!addbase && !ap) {
	(*img1->header_handle)(HEADER_FREAD, img1, in_fp1, 0);
	img1->o_form = img1->in_form;
	img1->pxl_out = img1->pxl_in;
	if (img->in_form < img1->in_form){
	register U_IMAGE*	itemp = img;	img = img1;	img1 = itemp;
	}
	diffsize = img->pxl_in - img1->pxl_in;
	if (diffsize)
		if (img->in_form == IFMT_COMPLEX)
			diffsize -= 4;
		else if (img->in_form == IFMT_DBLCOM)
			diffsize -= 12;
	if ((img->in_form==IFMT_FLOAT || img->in_form==IFMT_COMPLEX) &&
	    img1->in_form == IFMT_LONG)
		diffsize = 12;

	if (img->height != img->height || img->width != img1->width)
		prgmerr(0x73, "frame file and input header mismatch");
}
else{	if (img->in_form > IFMT_LONG)
		prgmerr(0x62, "Base value not used for FLOAT (%d)", addbase);
	img1->frames = 0;
}

if (bgn_frm<0)
	bgn_frm = 0;
else if (bgn_frm>img->frames)
	bgn_frm = img->frames-1;
if (nfrm<1)	nfrm = img->frames;
else if (nfrm + bgn_frm > img->frames)
	nfrm = img->frames - bgn_frm;

(*img->header_handle)(HEADER_WRITE, img, argc, argv, True);

#ifndef	HIPS_PUB
if (img->in_form == IFMT_LONGPYR || img->in_form == IFMT_FLOATPYR)
   if	(img->in_form != img1->in_form)
	prgmerr(0x46, "format not matching");
   else{
	if (upread(&toplev,1,sizeof(toplev),in_fp0) != sizeof(toplev))
		syserr("error reading standard input number of levels");
	if (upread(&i, 1, sizeof(i), in_fp1) != sizeof(i))
		syserr("error reading file input number of levels");
	if (toplev != i)
		syserr("pyramid number of levels mismatch");
	fsize = pyrnumpix(toplev,img->height,img->width);
	if (fwrite(&toplev,1,sizeof(toplev),out_fp0) != sizeof(toplev))
		syserr("error writing number of pyramid levels");
   }
else
#endif
	fsize = img->height*img->width;

if (img->in_form == IFMT_COMPLEX || img->in_form == IFMT_DBLCOM){
	if (img->in_form == img1->in_form)	ccf++;
	cf++;
}

if (img1->pxl_in)
	ibuf = zalloc(fsize, img1->pxl_in + (diffsize&7));

img->src = obuf = ZALLOC(fsize, img->pxl_out, "src");
if (diffsize && diffsize < 8)
	itemp1 = ZALLOC(fsize, img1->pxl_in, "itemp1");
else	itemp1 = ibuf;
img->load_all = 0;
for (i=bgn_frm; i; i--)	{
	if ((*img->std_swif)(FI_LOAD_FILE, img, 0, No))
		syserr("error during read passing");
	if (fwrite(obuf, img->pxl_out, fsize, out_fp0) != fsize)
		syserr("error during write passing");
}

message("passing %d & adding %d frames\n", bgn_frm, nfrm);

switch(img->in_form)
	{
	case IFMT_BYTE:
		addb(nfrm,img1->frames);	break;
	case IFMT_SHORT:
		addbs(nfrm,img1->frames);	break;
	case IFMT_LONG:
	case IFMT_LONGPYR:
		addint(nfrm,img1->frames);	break;
	case IFMT_FLOAT:
	case IFMT_COMPLEX:
	case IFMT_FLOATPYR:
		addfloat(nfrm,img1->frames);	break;
	default:
		adddbl(nfrm,img1->frames);
	}
for (i=img->frames - bgn_frm - nfrm; i; i--){
	if ((*img->std_swif)(FI_LOAD_FILE, img, 0, No))
		syserr("error during read passing");
	if (fwrite(obuf, img->pxl_out, fsize, out_fp0) != fsize)
		syserr("error during write passing");
}
exit(0);
}

#define	add_base(bound)	{ register int	sum;	\
	for(i=fsize; i--;)	{	\
		sum = *obp + addbase;	\
		if (sum>bound)	*obp++ = bound;	\
		else if (sum<0)	*obp++ = 0;	\
		else	*obp++ += addbase;	\
	}	}

addb(num_frame,fnum_frame)
{
int	f;
register int	i;
register byte	*obp,*ibp;

	for (f=0;f<num_frame;f++) {
	    if (fnum_frame) {
		fnum_frame--;
		if ((*img1->std_swif)(FI_LOAD_FILE, img1, 0, No) != fsize)
			syserr("reading error during b1-read");
	    }
	    if ((*img->std_swif)(FI_LOAD_FILE, img, 0, No) != fsize)
		syserr("reading error during b0_read");

	obp = (byte*)obuf;	ibp = (byte*)ibuf;
	if (f < ap)	addbase = array[f];
	if (addbase)
		if (rotate)
		    for(i=fsize; i--;)
			*obp++ += addbase;
		else	add_base(255)
	else for(i=0; i<fsize; i++, ibp++)
		if (*obp + *ibp > 255)	*obp++ = 255;
		else	*obp++ += *ibp;
	if (fwrite(obuf, 1, fsize, out_fp0) != fsize)
		syserr("error during write byte");
	}
}

addbs(num_frame,fnum_frame)
{
int	f;
register int	i;
register unsigned short	*obp,*ibp;

	for (f=0;f<num_frame;f++) {
	    if (fnum_frame) {
		fnum_frame--;
		switch (diffsize){
		case 0:	if((*img1->std_swif)(FI_LOAD_FILE, img1, 0, No) != fsize)
				syserr("reading error during bs1");
			break;
		case 1:	if((*img1->std_swif)(FI_LOAD_FILE, img1, 0, No) != fsize)
				syserr("reading during bs transfer1");
			btos(itemp1, ibuf, fsize);
			break;
		default:syserr("BS1 to int error %d", diffsize);
		}
	    }
	    if ((*img->std_swif)(FI_LOAD_FILE, img, NULL, No) != fsize)
		syserr("reading during SRead");

	    obp = (unsigned short *)obuf;
	    ibp = (unsigned short *)ibuf;
	    if (f < ap)	addbase = array[f];
	    if (addbase)	add_base(65535)
	    else    for (i=fsize; i--;)
			*obp++ += *ibp++;
	    if (fwrite(obuf, img->pxl_out, fsize, out_fp0) != fsize)
			syserr("during write short");
	}
}

addint(num_frame,fnum_frame)
{
register int	i,*obp,*ibp;
int	f;

	for (f=0;f<num_frame;f++) {
	    if (fnum_frame) {
		fnum_frame--;	/* (sizeof(int)-diffsize%4)	*/
		if ((*img1->std_swif)(FI_LOAD_FILE, img1, 0, No) != fsize)
			syserr("reading error during btoi transfer");
		switch(diffsize){
		case 2:	stoi(itemp1, ibuf, fsize);	break;
		case 3: btoi(itemp1, ibuf, fsize);	break;
		default:	syserr("X to int error %d", diffsize);
		}
	    }
	    if ((*img->std_swif)(FI_LOAD_FILE, img, 0, No) != fsize)
		syserr("during last int sequence read");
	    obp = (int*)obuf;
	    ibp = (int*)ibuf;
	    if (f < ap)	addbase = array[f];
	    if (addbase)
		for(i=fsize; i--;)
			*obp++ += addbase;
	    else    for (i=fsize; i--;)
			*obp++ += *ibp++;
	    if (fwrite(obuf, img->pxl_out, fsize, out_fp0) != fsize)
			syserr("during write int");
	}
}

addfloat(num_frame,fnum_frame)
{
int	f;
register int	i;
register float	*obp, *ibp;
	for (f=0;f<num_frame;f++) {
	    if (fnum_frame) {
		fnum_frame--;	/* (sizeof(float)-diffsize%4) */
		if ((i=(*img1->std_swif)(FI_LOAD_FILE, img1, 0, No)) != fsize)
			syserr("reading during xtof transfer %d", i);
		else	message("read %d bytes\n", i);
		switch(diffsize){
		case 12:itof(itemp1, ibuf, fsize);	break;
		case 3:	btof(itemp1, ibuf, fsize);	break;
		case 2:	stof(itemp1, ibuf, fsize);	break;
		case 0:	break;
		default:syserr("X to float diff %d", diffsize);
		}
	    }
	    if ((i=(*img->std_swif)(FI_LOAD_FILE, img, 0, No)) != fsize)
			syserr("during last float sequence read %d", i);
	    obp = (float*)obuf;
	    ibp = (float*)ibuf;
	    for (i=fsize; i--;) {
		*obp++ += *ibp++;
		if (ccf)	*obp++ += *ibp++;
		else if (cf)	*obp++;
	    }
	    if (fwrite(obuf,img->pxl_out,fsize,out_fp0) != fsize)
			syserr("during write float");
	}
}

adddbl(num_frame,fnum_frame)
{
int	f,s2=1<<ccf, s1=1<<cf;
register int	i;
register double	*obp, *ibp;
	for (f=0;f<num_frame;f++) {
	    if (fnum_frame) {
		fnum_frame--;
		if (diffsize)
		{
		if((i=upread(itemp1, (sizeof(double)-(diffsize&7)), fsize, in_fp1))
			!= fsize)
			syserr("reading during transfer");
		else	message("read %d bytes\n", i);
		switch(diffsize)	{
			case 4:	ftod(itemp1, ibuf, fsize);	break;
			case 12:itod(itemp1, ibuf, fsize);	break;
			case 6:	stod(itemp1, ibuf, fsize);	break;
			case 7:	btod(itemp1, ibuf, fsize);	break;
			default:syserr("X to double error %d", diffsize);
		}
		} else if (upread(ibuf,s2 * sizeof(double),fsize,in_fp1) !=
			fsize)
				syserr("during read");
	    }
	    if (upread(obuf, s1 * sizeof(double), fsize, in_fp0) != fsize)
			syserr("during fixed sequence read");
	    obp = (double *)obuf;
	    ibp = (double *)ibuf;
	    for (i=0;i<fsize;i++) {
		*obp++ += *ibp++;
		if (ccf)	*obp++ += *ibp++;
		else if (cf)	*obp++;
	    }
	    if (fwrite(obuf, s1 * sizeof(double), fsize, out_fp0) != fsize)
			syserr("write double");
	}
}

btos(ibp, obp, n)
register byte*	ibp;
register short*	obp;
register int	n;
{
#ifdef	_DEBUG_
message("%d byte to short\n", n);
#endif
while (n--)	*obp++ = *ibp++;
}

btoi(ibp, obp, n)
register byte*	ibp;
register int*	obp, n;
{
#ifdef	_DEBUG_
message("%d byte to integer\n", n);
#endif
while (n--)	*obp++ = *ibp++;
}

btof(ibp, obp, n)
register byte*	ibp;
register float*	obp;
register int	n;
{
#ifdef	_DEBUG_
message("%d byte to float\n", n);
#endif
while (n--)	*obp++ = (float)*ibp++;
}

btod(ibp, obp, n)
register byte*	ibp;
register double	*obp;
register int	n;
{
while (n--)	*obp++ = *ibp++;
}

stoi(ibp, obp, n)
register unsigned short	*ibp;
register int	*obp, n;
{
#ifdef	_DEBUG_
message("%d short to integer\n", n);
#endif
while (n--)	*obp++ = *ibp++;
}

stof(ibp, obp, n)
register unsigned short	*ibp;
register float*	obp;
register int	n;
{
#ifdef	_DEBUG_
message("%d short to float\n", n);
#endif
while (n--)	*obp++ = (float)*ibp++;
}

stod(ibp, obp, n)
register unsigned short	*ibp;
register double	*obp;
register int	n;
{
#ifdef	_DEBUG_
message("%d short to double\n", n);
#endif
while (n--)	*obp++ = *ibp++;
}

itof(ibp, obp, n)
register int*	ibp, n;
register float*	obp;
{
#ifdef	_DEBUG_
message("%d itof\n", n);
#endif
while (n--)	*obp++ = *ibp++;
}

itod(ibp, obp, n)
register int*	ibp, n;
register double	*obp;
{
#ifdef	_DEBUG_
message("%d itod\n", n);
#endif
while (n--)	*obp++ = *ibp++;
}

ftod(ibp, obp, n)
register float*	ibp;
register double	*obp;
register int	n;
{
#ifdef	_DEBUG_
message("%d FtoD\n", n);
#endif
while (n--)	*obp++ = *ibp++;
}
