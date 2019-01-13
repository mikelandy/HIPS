/* rotate90.c - rotate frames by 90 degrees
*
* usage: rotate90 [-right] [<] input [> output]
*
*	The -r switch rotates to the right (clockwise). The frames are rotated
*	to the left (counterclockwise) in default.
*
* compile: cc -O4 -o rotate90 rotate90.c -lscs4 -lccs -lhips -lrle -ltiff
*
* AUTHOR:	Jin Guojun - LBL
*/

#include "header.def"
#include "imagedef.h"

U_IMAGE	uimg;

#define	ibuf	uimg.src
#define	obuf	uimg.dest
#define	tbuf	uimg.cnvt

char	usage[] = "option\n\
-r (==> turn to right. The default is to left--CounterClockWise)\n\
[<] in_file [> output]\n";


main(argc, argv)
int	argc;
char	*argv[];
{
bool	rflag=0, t2f;
int	i, f, newr, newc, fsize;

format_init(&uimg, IMAGE_INIT_TYPE, HIPS, uimg.color_dpy=-1, *argv, "S20-2");

for (i=1; i<argc; i++)
	if (*argv[i]=='-' && argv[i][1]=='r')	rflag++;
	else if (! (in_fp=freopen(argv[i], "rb", stdin)) )
errout:		usage_n_options(usage, i, argv[i]);

io_test(stdin_fd, goto	errout);

(*uimg.header_handle)(HEADER_READ, &uimg, 0, 0);
(*uimg.header_handle)(HEADER_TRANSF, &uimg, 0);
i = isColorImage(uimg.color_form);
if (i > 1)
	uimg.color_form = CFM_ILC;
t2f = uimg.color_form==CFM_ILC;

if (uimg.in_form > IFMT_FLOAT)
	prgmerr('f', "image must be in ints or float format");
newc = uimg.height;
newr = uimg.width;

uimg.sub_img_h = newr;
uimg.sub_img_w = newc;
fsize = newr * newc;
uimg.sub_img = True;

uimg.update_header = (*uimg.header_handle)(HEADER_TO, &uimg, 0, No);
(*uimg.header_handle)(HEADER_WRITE, &uimg, argc, argv, True);

obuf = nzalloc(fsize, uimg.pxl_out, "obuf");

i = uimg.pxl_in;
uimg.load_all = 0;

for (f=0; f<uimg.frames; f++) {
	(*uimg.std_swif)(FI_LOAD_FILE, &uimg, 0, No);
	if (t2f)	{
		tbuf = nzalloc(i=4, fsize, "tbuf");
		ilc_transfer(tbuf, ibuf, fsize, 3, 0, i);
		free(ibuf);	ibuf = tbuf;
	}
	rotate90(ibuf, obuf, newr, newc, i, rflag);
	if (t2f)	ilc_transfer(obuf, obuf, fsize, 4, 0, i=3);
	if (fwrite(obuf, i, fsize, out_fp) != fsize)
		syserr("write");
}
exit(0);
}
