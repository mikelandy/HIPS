#include "header.def"
#include "imagedef.h"
#include <string.h>
#include <math.h>

U_IMAGE	uimg;

#define	ibuf	uimg.src
#define	frm	uimg.frames
#define	cln	uimg.width
#define	row	uimg.height

#ifndef	LDLIB
#define	LDLIB	"/usr/local/magicprt.pro"
#endif
#ifndef	MFont
#define	MFont	"NewCenturySchlbk-Bold"
#endif
#define	FBWR	4.0
#define	FWR	3.29
#ifndef	IGP
#define	IGP	0
#endif

/* MAGICPRINT.C - display images as halftones on a Postscript device
%
%	Copyright (c)	1990	Jin, Guojun
*/
char	usage[]="options\n\
-h & -w specify the output halftone size in inches. They default to an aspect\n\
	ratio equal to that of the images, and if none is specified,\n\
	the maximum dimension is 6 inches.  The screen can be either\n\
	a circle screen (the default) or -l a line screen.\n\
The screen frequency (in screen cells per inch) is set by -f; the screen angle\n\
	in degrees is set by -a. The defaults are 60 cells per inch and\n\
	45 degrees, they provide 25 gray levels. Gray levels are mapped\n\
	linearly to percent of dots painted. This filter works for byte and\n\
	bit-packed (IFMT_MSBF) images.\n\
-F font_name	\n\
-S #	set FONT and its SIZE for printing title string.\n\
-P #	is to get height where to place the title string.\n\
-B	place string at Bottom of a page.\n\
-H	print the all header information below the image\n\
-X[#]	put header in a box. Usually, the box size is same as image's.\n\
	An immediately following real number will change the box width\n\
	(range 3.8 - 8.4 inch).\n\
-d	print history and description only\n\
-g #	insert gap between image and box(in dots).\n\
-T	command line is the title string.\n\
-t | -v string	\n\
	print the string under or at right side of the image\n\
-b #	begin at frame #.\n\
-n #	print the number of frames which only you want.\n\
-c	center the image. Default image Position is on the top of a page.\n\
-p [#]	image top Position.\n\
	If not further # given, top of an image is 1.5 inch from TOP.\n\
-m	make upside down image like in the water.\n\
-s	page height (default = 11)	\n\
-o #	change or adjust the number of frame printed on footnote, used for\n\
	the input frames have already been cut off some.\n",
buffer[128], proline[128], *ldlib=LDLIB, *FONT=MFont;

/*
% Compile:
% cc -O -o magicprint $(@DEF) magicprt.c -lscs6 -lccs -lhips -lrle -ltiff -lm \
%	-ljpeg -lpixrect
%
% AUTHOR:	Jin Guojun - LBL	12/10/90
*/

#define	FontW(fontsz)	fontsz/fwr

/*	This offset is for top and bottom margin adjust	*/
#ifndef	MGPOffset
#define	MGPOffset	.05	/* 0.04-0.4 inch Printer depends */
#endif
#ifndef	VT
#define	VT	10
#endif

#define	DtoA	M_PI/180
#define	Mmirror(x, ref)	{ register int tmp=(int)(beta/ref);	\
	x=(tmp&1) ? (tmp+1)*ref-x : x-tmp*ref;	}
#define	GValue()	arget(argc, argv, &i, &c)
#define	SValue()	avset(argc, argv, &i, &c, 1)

bool	lflag, setflag, mirror, rota, SeqOnly;
float	fwr=FBWR, ih, iw, MaxImgH = -MGPOffset * 2,
	sf=60, sa=45;


main(argc, argv)
int	argc;
char*	*argv;
{
FILE	*profile;
bool	bottom=0,	/*	set title at bottom of the page	*/
	center=0,	/*	plcae image in middle of a page	*/
	printhd=0,	/*	print all header under image	*/
	title=0, Vt=0;	/*	title vertically at right side	*/
int	i, fr,py=0,	/*	footnote position from bottom	*/
	offf=1,	/*	offset of frame number for printing	*/
	bgn_p=1,	/*	begining frame number	*/
	num_p=1,	/*	number of page will be printed	*/
	linesp,	xg=0, FSZ=10;
float	sy=0, boxw=0,	/*	size & bool	*/
	pph=10.6, ppw=8.4,
	leftbd, margin=.1,
	alph, beta, radius;
char	*ip, *Title=0, *vstr=0;
MType	fsize;

#define	c	fr

format_init(&uimg, IMAGE_INIT_TYPE, HIPS, -1, *argv, "D5-1");
uimg.o_form = IFMT_BYTE;

for (i=1; i<argc; i++)
    if (*argv[i] == '-')	{
	c = 1;
	switch(argv[i][c++])	{
	case 'a':sa = GValue();	goto	sfa;
	case 'f':sf = GValue();
sfa:		setflag++;	break;
	case 'g':	xg = GValue();	break;
	case 'h':	ih = GValue();	break;
	case 'w':	iw = GValue();	break;
	case 'l':	lflag++;	setflag++;	break;
	case 'm':	message("Image is mirrored into water\n");
		mirror++;		break;
	case 'd':	SeqOnly++;	goto	prthd;
	case 'X':if (!argv[i][c])	boxw=1;
		else	boxw = atof(argv[i]+c);
prthd:	case 'H':	printhd++;	break;
	case 'b':	bgn_p = GValue();	break;
	case 'B':	bottom++;	break;
	case 'c':	center++;	break;
	case 'n':	num_p = GValue();	break;
	case 'o':	offf = GValue();	break;
	case 'P':	py = GValue() * 72;	break;
	case 'r':	rota = GValue();	break;
	case 'p':	sy = GValue();
		if (!sy)	sy = 9.6;	break;
	case 's':	pph = GValue();	break;
	case 'F':
		if (SValue())	FONT = argv[i]+c;
		if (FONT[strlen(FONT)-1] != 'd')	fwr = FWR;
		break;
	case 'S':	FSZ = GValue();	break;
	case 'v':if (SValue())	{	vstr=argv[i]+c;	break;	}
	case 'V':Vt++;
	case 't':if (SValue())	Title = argv[i]+c;
	case 'T':title++;	break;
	default:
info:		usage_n_options(usage, i, argv[i]);
	}
    }
    else if ((in_fp=freopen(uimg.name=argv[i], "rb", stdin)) == NULL)
		syserr("can not open %s for input", argv[i]);
io_test(fileno(in_fp),	i--;	goto	info);

if ((*uimg.header_handle)(HEADER_READ, &uimg, 0, 0))
	prgmerr('t', "unknown image type");
if (printhd && uimg.in_type != uimg.o_type)
	(*uimg.std_swif)(FI_INIT_NAME, &uimg, uimg.name, 0),
	(*uimg.header_handle)(HEADER_TO, &uimg, 0, 0);

MaxImgH += pph;
if (pph > 14)	ppw = 10.8;

if (!ih)
   if (!iw)
	if (row > cln)
	{	ih = 6.;	iw = ih * cln / row;	}
	else {	iw = 6.;	ih = iw * row / cln;	}
   else	ih = iw * row / cln;
else	if (!iw)
scale:	iw = ih * cln / row;

if (title)	{
   if (center && ih > MaxImgH || ih > MaxImgH + MGPOffset * 2)
	{	ih = MaxImgH;	goto	scale;	}
   if (!Title)	{
	Title = zalloc((MType)argc, 16L);
	strcpy(Title, *argv);
	for (i=1; i<argc; i++)
	{	strcat(Title, " ");
		strcat(Title, argv[i]);
	}
   }
}
if (!sy) {
	sy = pph - ih - MGPOffset;
	if (center)
		sy /= 2.;
}
else	sy -= ih;
ih /= 2.;	iw /= 2.;

linesp = (FSZ>>2) + VT;
if (!py)
   if (!bottom && ih < pph/2.)
	py = (sy - .2) * 72 - (FSZ >> 4);/* 72 points = 1 inch */
   else	py = linesp;

if (uimg.o_form!=IFMT_BYTE && uimg.in_type!=FITS &&
    uimg.in_form!=IFMT_BYTE && uimg.in_form!=IFMT_MSBF)
	prgmerr('f', "image must be in byte format");
fsize = (uimg.in_form==IFMT_MSBF)? (((cln + 7)/8) * row) : (row * cln);

radius = sqrt(ih*ih + iw*iw);
alph = atan2(ih, iw);
beta = abs(rota)*DtoA;
Mmirror(beta, M_PI_2);

{
register float	fh=2*radius*sin(alph+rota*DtoA), fw=2*radius*cos(alph-rota*DtoA);
message("Image size is %d rows by %d columns & printed as => %.3f x %.3f\n",
	row, cln, ih*2, iw*2);
message("%s screen:  %.3f cells per inch, angle %.3f degrees, diagonal=%.3f\n",
	lflag ? "Line" : "Circle", sf, sa, alph);
message("Rotate %d => %.3f height x %.3f width\n", rota, fh, fw);
if (fh>pph || fw>ppw)
	mesg("printing size too large");
}
/*	if begin page not set properly, set it to page 1 */
if (bgn_p > frm || bgn_p < 1)	bgn_p = 1;

/*	if # of pages for printing out of range, reset it	*/
num_p += --bgn_p;
if (num_p<1 || num_p>frm)	num_p = frm;

message("%s: Total %d frame(s).	Start at frame %d and print %d frame(s)\n",
	*argv, frm, bgn_p+offf, num_p-bgn_p);

ibuf = zalloc(fsize, (MType)sizeof(char));

if ((profile=fopen(ldlib, "r")) == NULL)	/* copy prologue */
	syserr("can't open profile file %s", ldlib);
while(fgets(proline, 80, profile))	fputs(proline, stdout);
printf("%.2f %.2f %.2f %.2f setpos\n", ppw / 2., sy, iw, ih);

/*	gausy for printing notation and header	*/
if (!boxw || 3.8 > boxw || boxw > ppw){
	leftbd = ppw/2. - iw + margin;
	if (leftbd > 2.2)	leftbd=2.2;
}
else leftbd = (ppw - boxw)/2 + margin;

py -= (radius*sin(alph+beta) - ih)*72;
printf("%d %.2f %.2f %d %d %.2f setform\n",
	py-linesp*title+xg, -margin, leftbd, rota && IGP, -linesp, FontW(FSZ));

if (bgn_p)	fseek(in_fp, bgn_p * fsize, 1);

for(fr=bgn_p; fr<num_p; fr++){
	(*uimg.std_swif)(FI_LOAD_FILE, &uimg, uimg.load_all=0, No);
	message("%u bytes read in frame[%d]\n", fsize, fr+offf);

	printf("gsave\n");	/*	save graphic state.	*/

	if(setflag)	/*	set screen if necessary	*/
		if(lflag)	printf("%f %f setline\n", sf, sa);
		else		printf("%f %f setcirc\n", sf, sa);

/* set up string for temporary postscript storage of one row of image */
	printf("%d setimstr\n", (uimg.in_form==IFMT_MSBF)? ((cln + 7) / 8):cln);

/* set up parameters for magicprint */
	printf("%d %d setrc\n", cln, row);
	printf("%d setnb\n", (uimg.in_form==IFMT_MSBF) ? 1 : 8);

/* set up position, scale and output single page data */
	if (rota)
		printf("%.2f %.2f %d Rotate\n",	radius,90-alph*180/M_PI-rota,rota);
	else	printf("placeim\n");
	printf("scaleim\n");
	if (mirror)	printf("imusd\n");	/*	up side down	*/
	else		printf("imgo\n");

	for(i=0, ip=ibuf; i < fsize;){
		printf("%02x", (byte) *ip++);
		if (!(++i % 35))	printf("\n");
	}
/* restore graphics state & start text output	*/
	printf("\ngrestore\n/%s findfont %d scalefont setfont\n", FONT, FSZ);
if (printhd){
char*	phd = (char*)(*uimg.std_swif)(FI_HIPS_HEADER_FORMAT, &uimg, NULL);
	if (SeqOnly)
		phd = strchr(phd+64, 'S');
	printf("LeftBd UC moveto newline\n");
	do {
		for (i=0; *phd != '\n'; i++)
			if (*phd == '|' && *(phd+1) == '\\')
			{	phd+=2;	i--;	}
			else	buffer[i] = *phd++;
		buffer[i]=0;
		if (!strncmp(buffer, "Number of fr", 12))
		{	sprintf(proline, "       :: [frame %d]", fr+offf);
			strcat(buffer, proline);
		}
		i ^= i;
		while(buffer[i] == ' ')	i++;
		if (strlen(buffer+i))
			printf("(%s) show\nnewline\n", buffer+i);
	} while(*++phd);
	if (boxw)	printf("Box\n");
	}
if (title | Vt){
	if (num_p > 1 && !printhd)
		sprintf(buffer, "%s : frame %d", Title, fr+offf);
	else	strcpy(buffer, Title);
	printf("gsave\n");
	if(Vt>1)	printf("(%s) vshow\n", buffer);
	else if(Vt)
		printf("RS %d %.1f Vput\n(%s) show\n", strlen(buffer), pph/2., buffer);
	else	printf("%d %d Hput\n(%s) show\n",py, strlen(buffer), buffer);
	printf("grestore\n");
	}
if(vstr) printf("%.2f %d %.2f Vput\n(%s) show\n",
		radius*cos(alph-beta), strlen(vstr), sy+ih, vstr);
printf("showpage\n");
}
printf("restorestate\n");
}
