/*	label.c		Roman Yangarber		86/02/23	*/

/*
 * label - accepts an arbitrary sequence as input, and creates
 *
 * usage:
 *
 * label [-T textfile | textstring] [-j{rcl}] [-o{udrl}] [-x n] [-y n] [-a d]
 *			[-v n] [-f n] [-b n] [-F fontname] < inseq > outseq
 * label -g [-T textfile | textstring] [-j{rcl}] [-o{udrl}]
 *			[-v n] [-f n] [-b n] [-F fontname] > outseq
 *
 * Label creates rasters using the Berkeley fonts.  It can either add labels
 * to images sequences, or, using the -g switch, create a frame with text in
 * it.  The text string can be given in the command line, or using -T, come 
 * from a file.  The other switches are:
 *	-j	adjustment - right, center, or left.
 *	-o	orientation - text reads upwards, downwards, rightwards, or
 *			leftwards (these are all rotations of normal text).
 *	-x	x position relative to the columns of the input sequence 
 *			(0=leftmost, M=middle, E=end/rightmost, nnn=column nnn).
 *	-y	y position relative to the rows of the input sequence 
 *			(0=leftmost, M=middle, E=end/bottom, nnn=row nnn).
 *	-a	alignment, as in Leroy:
 *			2xxxx5xxxx8
 *			xxxxxxxxxxx
 *			1xxxx4xxxx7
 *			xxxxxxxxxxx
 *			0xxxx3xxxx6
 *		the alignment key tells which pixel of the text block is
 *		positioned at pixel (x,y).
 *	-v	vertical space of nnn pixels between each new text line.
 *	-f	foreground color of text.
 *	-b	background color of text.  A background or foreground value
 *			of 0 leaves the image values alone, allowing a matte.
 *	-F	Font name from /usr/lib/vfont, default is times.r.6.
 *
 * to load:	cc -O -o label label.c -lhips
 *
 */

/* can't include vfont.h: its "header" conflicts with hipl_format.h
   #include	<vfont.h>
*/
/*	vfont.h	4.1	83/05/03	*/
/*
 * The structures header and dispatch define the format of a font file.
 *
 * See vfont(5) for more details.
 */
struct vfont_header {
	short magic;
	unsigned short size;
	short maxx;
	short maxy;
	short xtend;
}; 

struct dispatch {
	unsigned short addr;
	short nbytes;
	char up,down,left,right;
	short width;
};
#define	SPACE_WIDTH	5	/* default width for space character */
#define MAXLINES	100	/* max lines for label on screen */

#include	<stdio.h>
#include	<sys/file.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<hipl_format.h>

/* adjustment constants */
#define LEFT	(-1)
#define CENTER	0
#define RIGHT	1
#define UP	2
#define DOWN	3
		/* assume these numbers safely illegal screen coordinates */
#define MIDDLE	20000	/* out of range of screen to be substituted later */
#define END	30000	/* out of range of screen to be substituted later */

#define ADJUST(x)	( (justify==RIGHT) ? max_col - linecol[x] : \
	( (justify==CENTER) ? (max_col-linecol[x]) / 2 : 0 ) )
#define Modifier	(argv[argx][2]? (argv[argx]+2) : \
			(space_in_opt = 1, argv[argx+1]) )

#define	DISPATCH_OFFSET	10
#define	RASTER_OFFSET	2570
extern	errno;

char	usage[] =
"	usage: label [-g] text-string [-j{rcl}] [-o{udrl}] [-a n] [-x n] [-y n] \n\
	[-F fontname] [-T textfile] < inseq > outseq\n";

int	space_in_opt;

struct	dispatch	*d, *dispatches;
struct	stat	statbuf;

char	*rasterp, *rasters;
int	sized;
int	fd, od, ret, i, j, rsize;

struct	header	hd;

int	totbytes;
int	nr_label=0;
int	nc_label=0;
int	row, col, bits_across, max_up, max_down, max_col;
int	temp,
	x, y,
	align_up, align_down, align_right, align_left;

char	bgcolor = 0,fgcolor = 255;
char	*line, *Label, *bytep;
char	*labelend;
int	bit;

int	textlen, numlines, linecol[MAXLINES];
char	*letterp, *Text;
char	Fontfile[50] = "/usr/lib/vfont/times.r.6";
char	*Font;

char	*oframe, *iframe;
int	inumbytes, onumbytes,
	i_hpad = 0, l_hpad = 0, i_vpad = 0, l_vpad = 0, nr_image, nc_image,
	orientation,
	iincr, jincr,	/* increments for copying label to output */
	generate_only,	/* assume input image */
	justify,	/* left justify default */
	align = 0,
	linex,
	vspace = 0,		/* vertical spacing between rows */
	fr, numfr;
int	Ualign[] = {6,3,0,7,4,1,8,5,2};
int	Dalign[] = {2,5,8,1,4,7,0,3,6};

main(argc,argv)

int	argc;
char	**argv;
{
int		argx;
register	char	*op, *ip;

	Progname = strsave(*argv);
	sized = (sizeof *d);
	Font = Fontfile + 15;	/* point to fontname */

	argx = 1;
	if ( argc < 2)
		perr(HE_MSG,usage);

	orientation = RIGHT;	/* defaults */
	justify = LEFT;
	generate_only = 0;
	align = 5;
	x = MIDDLE;
	y = END;

	while (argx < argc)	{
		/* assume at first no space between option and modifier */
		space_in_opt = 0;
		if ( argv[argx][0] == '-' ) {
			switch ( argv[argx][1] ) {
			case 'g':
				generate_only = 1;
				break;
			case 'x':
				if (*Modifier == 'M')
					x = MIDDLE;
				else if (*Modifier == 'E')
					x = END;
				else
					x = atoi(Modifier);
				break;
			case 'y':
				if (*Modifier == 'M')
					y = MIDDLE;
				else if (*Modifier == 'E')
					y = END;
				else
					y = atoi(Modifier);
				break;
			case 'v':
				vspace = atoi(Modifier);
				break;
			/* grey level or color grounds options */
			case 'b':
				bgcolor = atoi(Modifier);
				break;
			case 'f':
				fgcolor = atoi(Modifier);
				break;
			/* text alignment option */
			case 'a':
				align = atoi(Modifier);
				break;
			/* text orientation options */
			case 'o':
				switch ( *Modifier ) {
				case 'l':
					orientation = LEFT;
					break;
				case 'u':
					orientation = UP;
					break;
				case 'd':
					orientation = DOWN;
					break;
				case 'r':
					orientation = RIGHT;	/* defaults */
					break;
				default:
					perr(HE_MSG,"invalid orientation option");
				}
				break;
			/* text justification options */
			case 'j':
				switch ( *Modifier ) {
				case 'c':
					justify = CENTER;
					break;
				case 'r':
					justify = RIGHT;
					break;
				case 'l':
					justify = LEFT;	/* defaults */
					break;
				default:
					perr(HE_MSG,"invalid adjustment option");
				}
				break;
			case 'T':	/* text inside file */
				if ((fd= open(Modifier,O_RDONLY,0)) < 0 )
					perr(HE_MSG,"can't open textfile");
				if (fstat(fd,&statbuf) == -1) {
					fprintf(stderr,"errno - %d",errno);
					perr(HE_MSG,"can't stat Textfile");
				}
				textlen = statbuf.st_size;
				if((Text=letterp=(char *)malloc(textlen))== 0)
					perr(HE_MSG,"can't alloc Text space");
				if(read(fd,Text,textlen) != textlen)
					perr(HE_MSG,"Text read error");
				break;
			case 'F':
				strcpy(Font,Modifier);
				break;
			default:
				fprintf(stderr,"illegal option-%s\n",argv[argx]);
				perr(HE_MSG,usage);
			}
		}
		else	{
			Text = letterp = argv[argx];
			textlen = strlen(Text);
		}
		if (space_in_opt) argx++;
		argx++;
	}


	/* read in label text dispatches */

	if ( (fd = open(Fontfile, O_RDONLY, 0)) <= 0)
		perr(HE_OPEN,Fontfile);

	if( (dispatches= d =((struct dispatch *)malloc(textlen * sized)) ) == 0)
		perr(HE_MSG,"can't alloc dispatches");

	max_col = nc_label = nr_label = totbytes = 0;
	numlines = 1;
		/* find dispatch for letter */
	for ( i = 0; i < textlen; letterp++,i++) {
	
		if (*letterp == '\n') {
			linecol[numlines-1]= nc_label;
			numlines ++;
			if (max_col < nc_label) max_col = nc_label;
			nc_label = 0;
		}
		else {
			if(lseek(fd,DISPATCH_OFFSET+(*letterp)*sized,L_SET)<=0)
				perr(HE_MSG,"seek set error ");
			if (read( fd,  d, sized ) != sized )
				perr(HE_MSG,"can't read dispatch");

			totbytes += d-> nbytes;
				/* compute dimensions of raster */
			if (max_down < d->down ) max_down = d->down;
			if (max_up < d->up ) max_up = d->up;
			/* take care of space char */
			if (*letterp == ' ') {
				d->width = SPACE_WIDTH;
			}
			if (d->width > d->right)
				nc_label += d->width - d->left;
			else
				nc_label += d->right - d->left;
			d++;
		}
	}
	/* take size of last line into account */
	if (nc_label) {		/* i.e. : if (*(letterp-1) != '\n' ) */
		linecol[numlines-1] = nc_label;
		if (max_col < nc_label)
			max_col = nc_label;
		}
	if (nc_label < max_col)	nc_label = max_col;
	nr_label = (max_up + max_down) + vspace;

	/* read in label rasters */

	if (!totbytes) perr(HE_MSG,"NULL Label !");
	if ( (rasters = rasterp = ((char *)malloc(totbytes)) ) == 0)
		perr(HE_MSG,"can't alloc rasters");

	letterp = Text;
	d = dispatches;		/* reset pointer */
	for ( i = 0; i < textlen; letterp++, i++) {
		if (*letterp != '\n') {
			if ( lseek(fd, RASTER_OFFSET + d->addr, L_SET)<= 0 )
				perr(HE_MSG,"seek set error ");
			if (read( fd, rasterp, d->nbytes)!= d->nbytes )
				perr(HE_MSG,"can't read raster");
			rasterp += d->nbytes;
			d++;
		}
	}

	/* unpack label and fill Label array */

	totbytes = numlines*nr_label*nc_label;
	if ( (Label = (char *) ( (orientation == RIGHT) ?
		malloc(totbytes) : malloc(2*totbytes) ) )== 0)
			perr(HE_MSG,"core allocation failed");
	labelend = Label + totbytes;

	/* zero Label frame */

	op = Label;
	for ( i = 0; i < totbytes; i++)
		*op++ = bgcolor;

	linex=0;
	col = ADJUST(linex);
	letterp = Text;
	line = Label;
	rasterp = rasters;
	d = dispatches;
	for ( i=0; i<textlen; i++, letterp++ ) {
		if (*letterp == '\n') {
			line += nr_label*nc_label;
			linex++;
			col = ADJUST(linex);
		}
		else {
			bits_across = d->left + d->right;
			bytep = rasterp;
			for (row = max_up- d->up; row < max_up + d->down; row++)
			{
				op = line + row*nc_label + col;
				/* unpack one row */
				bit = 0;
				for (j = 0; j< bits_across; j++) {
					*(op + j - d->left) =
						((*bytep &(0200>>bit)) )
						? fgcolor : bgcolor;
					if (++bit == 8) {
						bit = 0;
						bytep++;
					}
				}
				if (bit)
					bytep++;
			}
			col += d->width;
			rasterp += d->nbytes;
			d++;
		}
	}
	nr_label *= numlines;
	
	/* rotate label into proper orientation */

	ip = Label;

	if (orientation != RIGHT) {
		switch ( orientation ) {
		case UP:		/* bottom to top */
			op = labelend + (nc_label - 1) *nr_label;
			iincr = 1;
			jincr = - nr_label;
			align = Ualign[align];
			break;
		case DOWN:		/* top to bottom */
			op = labelend + nr_label - 1;
			iincr = - 1;
			jincr = nr_label;
			align = Dalign[align];
			break;
		case LEFT:		/* upside down right to left */
			op= labelend + nr_label*nc_label - 1;
			iincr = - nc_label;
			jincr = - 1;
			align = 8 - align; 	/* clever way to invert */
			break;
		}

		while ( ip < labelend ) {
			for (j = 0; j< nc_label; j ++ )
				*(op + j*jincr) = *ip++;
			op += iincr;
		}
		if (orientation == UP || orientation == DOWN ) {
			/* swap number of rows and cols */
			temp = nc_label;
			nc_label = nr_label;
			nr_label = temp;
		}
		Label = labelend;	/* use latter label array */
	}	/* end rotation */

	/* generate a single frame containing label and exit */

	numfr = 1;
	if ( generate_only ) {
		init_header(&hd,"","",numfr,"",nr_label,nc_label,PFBYTE,1,"");
		update_header(&hd,argc,argv);
		write_header(&hd);

#ifdef	ULORIG
		if (fwrite(Label,nr_label*nc_label,1,stdout) != 1)
			perr(HE_MSG,"Label write error");
#else
		op = Label + (nr_label - 1)*nc_label;
		for ( i = 0; i < nr_label; i++) {
			if (fwrite(op,nc_label,1,stdout) != 1)
				perr(HE_MSG,"Label write error");
			op -= nc_label;
		}
#endif
		exit(0);
	}

	/* determine alignment values */
	switch (align / 3 ) {
	case 0:
		align_left = 0;
		break;
	case 1:
		align_left = nc_label / 2;
		break;
	case 2:
		align_left = nc_label - 1;
		break;
	}
	switch (align % 3 ) {
	case 0:
		align_up = nr_label - 1;
		break;
	case 1:
		align_up = nr_label / 2;
		break;
	case 2:
		align_up = 0;
		break;
	}
	align_down = nr_label - align_up - 1;
	align_right = nc_label - align_left - 1;

/*
 * update input sequence
 */

	read_header(&hd);
	if (hd.pixel_format != PFBYTE)
		perr(HE_MSG,"label: pixel format must be unpacked bytes");
	inumbytes = hd.ocols*hd.orows*sizeof(unsigned char);
	nr_image = hd.orows; nc_image = hd.ocols;

	/* compute middle and end coordinates for rows and columns if needed */
	/* can't do this earlier since if generate only header is never read */
	if (x == MIDDLE)
		x = nc_image / 2;
	else if (x == END)
		x = nc_image;
	if (y == MIDDLE)
		y = nr_image / 2;
	else if (y == END)
		y = nr_image;

	if ( (temp = y - align_up) < 0) {
		hd.orows +=  -temp;
		i_vpad = -temp;
	}
	else
		l_vpad = temp;
	if ( (temp = y + align_down) > nr_image - 1) {
		hd.orows +=  temp - (nr_image - 1);
	}
	if ( (temp = x - align_left) < 0) {
		hd.ocols +=  -temp;
		i_hpad = -temp;
	}
	else
		l_hpad = temp;
	if ( (temp = x + align_right) > nc_image - 1) {
		hd.ocols +=  temp - (nc_image - 1);
	}

	setsize(&hd,hd.orows,hd.ocols);
	update_header(&hd,argc,argv);
	write_header(&hd);
	onumbytes = hd.ocols*hd.orows*sizeof(unsigned char);
	numfr = hd.num_frame;

	if ((iframe = (char *) malloc(inumbytes)) == NULL)
		perr(HE_MSG,"label: can't allocate core");
	if ((oframe = (char *) malloc(onumbytes)) == NULL)
		perr(HE_MSG,"label: can't allocate core");

	/* zero output frame */

	op = oframe;
	for ( i = 0; i < hd.orows; i++)
		for ( j = 0; j < hd.ocols; j++)
			*op++ = 0;

	for(fr = 0; fr< numfr; fr++) {

		/* tack on input image: */
		if (fread(iframe, inumbytes,1,stdin) != 1)
			perr(HE_MSG,"label: error during read");
		ip = iframe;
		for ( i = 0; i < nr_image; i++) {
			op = oframe + (i_vpad + i)*hd.ocols + i_hpad;
			for ( j = 0; j < nc_image; j++)
				*op++ = *ip++;
		}

		/* tack on label: */
		ip = Label;
		for ( i = 0; i < nr_label; i++) {
			op = oframe + (l_vpad + i)*hd.ocols + l_hpad;
			for ( j = 0; j < nc_label; j++) {
				if ( *ip )
					*op = *ip;
				ip++; op++;
			}
		}

#ifdef	ULORIG
		if (fwrite(oframe, onumbytes,1,stdout) != 1)
			perr(HE_MSG,"label: write error");
#else
		op = oframe + (hd.orows - 1)*hd.ocols;
		for ( i = 0; i < hd.orows; i++) {
			if (fwrite(op,hd.ocols,1,stdout) != 1)
				perr(HE_MSG,"Label write error");
			op -= hd.ocols;
		}
#endif
	}
}
