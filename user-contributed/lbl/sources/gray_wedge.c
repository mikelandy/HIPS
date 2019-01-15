/*

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 * gray_wedge.c - generate a gray wedge
 *
 * Usage: gray_wedge
 *          -r rows -w cols -m upper_value [-b, -s, -i, -f] -n num_frames
 *
 * Defaults:	rows: 512, cols: 512, upper value: max, byte output, frames: 1
 *
 * Load:	cc -o gray_wedge gray_wedge.c -lhipl
 *
 * Bill Johnston, LBL
 *
 */

#include <hipl_format.h>
#include <stdio.h>

int       fr, num_rows, num_cols, num_cols_per_gval, i_col, repl_cols, i_row,
          i_frm;
struct header hd;

int	num_rows = 512;
int	num_cols = 512;
int	num_frms = 1;

float     two_pow_8_m1 = 255.;
float     two_pow_16_m1 = 65535.;
float     two_pow_32_m1 = 4294967295.;
float     max_gray, gray_step, gray_val;
double    max_gray_fp;
int       max_gray_fx;

int       foflag = 0;		/* -f specified, float output */
int       ioflag = 0;		/* -i specified, integer output */
int       soflag = 0;		/* -s specified, short output */
int       boflag = 1;		/* -b specified, byte output (default) */
int       mxflag = 0;		/* -m alternate max gray value specified */
int       debug = 0;
void bout(),iout(),sout(),fout();

int main(argc, argv)
    int       argc;
    char    **argv;

{
    int       ac = 1;
    int       ga = 0;

/*    fprintf( stderr, "argc, %d; argv[0], %s; argv[1], %s; argv[2], %s; argv[3], %s; argv[4], %s\n", argc, argv[0], argv[1], argv[2], argv[3], argv[4]);
*/

    Progname = strsave(*argv);
    if (ac > 1) {
	while (argv[ac][0] == '-') {
	    /* -D   set debug) */
	    if (argv[ac][1] == 'D')
		debug++;
	    /* -b   byte (8 bit) format (the default) */
	    if (argv[ac][1] == 'b')
		boflag++;
	    /* -s   for short (16 bit) pixel output */
	    if (argv[ac][1] == 's') {
		soflag++;
		boflag = 0;
	    }
	    /* -i   for integer (32 bit) pixel output */
	    if (argv[ac][1] == 'i') {
		ioflag++;
		boflag = 0;
	    }
	    /* -f   for float (single precision) pixel output */
	    if (argv[ac][1] == 'f') {
		foflag++;
		boflag = 0;
	    }
	    /* -n   number of frames to output */
	    if (argv[ac][1] == 'n') {
		num_frms = atoi(argv[++ac]);
	    }
	    /* -r   number of rows */
	    if (argv[ac][1] == 'r') {
		num_rows = atoi(argv[++ac]);
	    }
	    /* -w   number of cols */
	    if (argv[ac][1] == 'w') {
		num_cols = atoi(argv[++ac]);
	    }
	    /* -m   maximum gray value */
	    if (argv[ac][1] == 'm') {
		mxflag++;
		/* convert number both ways until we know output format */
		max_gray_fx = atoi(argv[++ac]);
		max_gray_fp = atof(argv[ac]);
	    }
	    ac++;
	    if (ac > (argc - 1))
		break;
	}
    }
    if (debug > 0)
	fprintf(stderr, "boflag, %d; soflag, %d; ioflag, %d; foflag, %d num_rows,\
            %d; num_cols, %d; num_frms, %d;\n",
	      boflag, soflag, ioflag, foflag, num_rows, num_cols, num_frms);


    if (boflag > 0) {
	if (mxflag == 0)
	    max_gray = two_pow_8_m1;
	else
	    max_gray = max_gray_fx;

	bout(argc, argv);
    }
    if (ioflag > 0) {
	if (mxflag == 0)
	    max_gray = two_pow_32_m1;
	else
	    max_gray = max_gray_fx;

	iout(argc, argv);
    }
    if (soflag > 0) {
	if (mxflag == 0)
	    max_gray = two_pow_16_m1;
	else
	    max_gray = max_gray_fx;

	sout(argc, argv);
    }
    if (foflag > 0) {
	if (mxflag == 0)
	    max_gray = 1.0;
	else
	    max_gray = max_gray_fp;

	fout(argc, argv);
    }
    return(0);
}

void bout(argc, argv)
    int       argc;
    char    **argv;
{

    /* PFBYTE	 */
    unsigned char *ofr;

/* init_header
 (&image_header,orig_name,seq_name,num_frame, orig_date,rows,cols,
 bits _per_pixel,bit_packing,pixel_format, seq_desc); */

    /* PFBYTE	 */
    init_header(&hd, "", "", num_frms, "", num_rows, num_cols, PFBYTE, 1, "");

/* update_header(&image_header,argc,argv)
Updates the image header seq_history according to the current command string.*/

    update_header(&hd, argc, argv);

    write_header(&hd);

    /* PFBYTE	 */
    ofr = (unsigned char *) halloc(num_rows * num_cols, sizeof(char));

    gray_val = 0;
    gray_step = max_gray / ((float) (num_cols - 1));
    num_cols_per_gval = num_cols / max_gray;
    if (num_cols_per_gval > 1)
	num_cols_per_gval = 1;

    /* index distinct cols and grey value  */
    for (i_col = 0; i_col <= (num_cols - 1); i_col += num_cols_per_gval) {

	/* replicate the cols, if necessary */
	for (repl_cols = 0; repl_cols <= (num_cols_per_gval - 1); repl_cols++) {
	    /* rows  */
	    for (i_row = 0; i_row <= (num_rows - 1); i_row++)
		ofr[i_row * num_cols + i_col + repl_cols] = gray_val + 0.5;
	}

	gray_val = gray_val + gray_step;
    }

    /* generate frames  */
    for (i_frm = 0; i_frm <= (num_frms - 1); i_frm++) {
	/* PFBYTE	 */
	if (fwrite(ofr, num_rows * num_cols * sizeof(char), 1, stdout) != 1)
	    perr(HE_MSG, "write error");
    }
}

void sout(argc, argv)
    int       argc;
    char    **argv;
{

    /* PFSHORT	 */
    unsigned short *ofr;

    /* PFSHORT	 */
    init_header(&hd, "", "", num_frms, "", num_rows, num_cols, PFSHORT, 1, "");

/* update_header(&image_header,argc,argv)
Updates the image header seq_history according to the current command string.*/

    update_header(&hd, argc, argv);

    write_header(&hd);
    /* PFSHORT	 */
    ofr = (unsigned short *) halloc(num_rows * num_cols, sizeof(short));

    gray_val = 0;
    gray_step = max_gray / ((float) (num_cols - 1));
    num_cols_per_gval = num_cols / max_gray;
    if (num_cols_per_gval < 1)
	num_cols_per_gval = 1;

    /* index distinct cols and grey value  */
    for (i_col = 0; i_col <= (num_cols - 1); i_col += num_cols_per_gval) {

	/* replicate the cols, if necessary */
	for (repl_cols = 0; repl_cols <= (num_cols_per_gval - 1); repl_cols++) {
	    /* rows  */
	    for (i_row = 0; i_row <= (num_rows - 1); i_row++)
		ofr[i_row * num_cols + i_col + repl_cols] = gray_val + 0.5;
	}

	gray_val = gray_val + gray_step;
    }

    /* generate frames  */
    for (i_frm = 0; i_frm <= (num_frms - 1); i_frm++) {
	/* PFSHORT	 */
	if (fwrite(ofr, num_rows * num_cols * sizeof(short), 1, stdout) != 1)
	    perr(HE_MSG, "write error");
    }
}

void iout(argc, argv)
    int       argc;
    char    **argv;
{

    /* PFINT	 */
    unsigned int *ofr;

    /* PFINT	 */
    init_header(&hd, "", "", num_frms, "", num_rows, num_cols, PFINT, 1, "");

/* update_header(&image_header,argc,argv)
Updates the image header seq_history according to the current command string.*/

    update_header(&hd, argc, argv);

    write_header(&hd);
    /* PFINT	 */
    ofr = (unsigned int *) halloc(num_rows * num_cols, sizeof(int));

    gray_val = 0;
    gray_step = max_gray / ((float) (num_cols - 1));
    num_cols_per_gval = num_cols / max_gray;
    if (num_cols_per_gval < 1)
	num_cols_per_gval = 1;

    /* index distinct cols and grey value  */
    for (i_col = 0; i_col <= (num_cols - 1); i_col += num_cols_per_gval) {

	/* replicate the cols, if necessary */
	for (repl_cols = 0; repl_cols <= (num_cols_per_gval - 1); repl_cols++) {
	    /* rows  */
	    for (i_row = 0; i_row <= (num_rows - 1); i_row++)
		ofr[i_row * num_cols + i_col + repl_cols] = gray_val + 0.5;
	}

	gray_val = gray_val + gray_step;
    }

    /* generate frames  */
    for (i_frm = 0; i_frm <= (num_frms - 1); i_frm++) {
	/* PFINT	 */
	if (fwrite(ofr, num_rows * num_cols * sizeof(int), 1, stdout) != 1)
	    perr(HE_MSG, "write error");
    }
}

void fout(argc, argv)
    int       argc;
    char    **argv;
{

    /* PFFLOAT	 */
    float    *ofr;

    /* PFFLOAT	 */
    init_header(&hd, "", "", num_frms, "", num_rows, num_cols, PFFLOAT, 1, "");

/* update_header(&image_header,argc,argv)
Updates the image header seq_history according to the current command string.*/

    update_header(&hd, argc, argv);

    write_header(&hd);
    /* PFFLOAT	 */
    ofr = (float *) halloc(num_rows * num_cols, sizeof(float));

    gray_val = 0;
    gray_step = max_gray / ((float) (num_cols - 1));
    num_cols_per_gval = 1;

    /* index distinct cols and grey value  */
    for (i_col = 0; i_col <= (num_cols - 1); i_col += num_cols_per_gval) {

	/* replicate the cols, if necessary */
	for (repl_cols = 0; repl_cols <= (num_cols_per_gval - 1); repl_cols++) {
	    /* rows  */
	    for (i_row = 0; i_row <= (num_rows - 1); i_row++)
		ofr[i_row * num_cols + i_col + repl_cols] = gray_val;
	}

	gray_val = gray_val + gray_step;
    }

    /* generate frames  */
    for (i_frm = 0; i_frm <= (num_frms - 1); i_frm++) {
	/* PFFLOAT	 */
	if (fwrite(ofr, num_rows * num_cols * sizeof(float), 1, stdout) != 1)
	    perr(HE_MSG, "write error");
    }
}
