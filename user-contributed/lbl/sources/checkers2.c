/*

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
*NAME
* checkers2.c - generate a checker board pattern
*
* version of HIPS checkers.c with many more options
*  Bill Johnston, LBL
*
*SYNOPSIS
* checkers2 -hi_gray - lo_gray -np_col -np_row -r rows -c cols [-b, -s, -i, -f] -n num_frames
*
* Defaults:    hi_gray: 128; lo_gray: 0; np_col:10; np_row:10; rows: 512; cols: 512; byte output; frames: 1
*
* Load:	cc -o checkers2 checkers2.c -lhipl
*
*DESCRIPTION
*
*     Generate a checker board.
*     -hi_gray %f bright checker value
*     -lo_gray %f dim checker value
*
*     -np_col  %d number of patches in the column direction (across)
*     -np_row  %d number of patches in the row direction (down)
*
*     -r   number rows in the image
*     -c   number cols in the image
*     -n   number of frames to generate
*
*     -b   byte (8 bit) format (the default)
*     -s   for short (16 bit) pixel output
*     -i   for integer (32 bit) pixel output
*     -f   for float (single precision) pixel output
*
*/

#include <hipl_format.h>
#include <stdio.h>

int       num_rows, num_cols, repl_cols, i_frm, loc;
int       num_cols_per_patch, num_patchs;
int       cur_col, cur_row, i_patch_col, i_patch_row, repl_rows, num_rows_per_patch;

int	num_rows = 512;
int	num_cols = 512;
int	num_patchs_col_dir = 10;
int	num_patchs_row_dir = 10;
int	num_frms = 1;

float     two_pow_8_m1 = 255.;
float     two_pow_16_m1 = 65535.;
float     two_pow_32_m1 = 4294967295.;

float     patch_gray;
float     hi_gray, lo_gray;
float     hi_gray_arg, lo_gray_arg;
float     max_gray;
int       hi_g_flag, lo_g_flag;

int       foflag = 0;		/* -f specified, float output */
int       ioflag = 0;		/* -i specified, integer output */
int       soflag = 0;		/* -s specified, short output */
int       boflag = 1;		/* -b specified, byte output (default) */
int       debug = 0;
int       debug2 = 0;

 /* output image declerations */
unsigned char *start_out_image_buf_byte, *out_image_byte;
unsigned short *start_out_image_buf_short, *out_image_short;
unsigned int *start_out_image_buf_int, *out_image_int;
float    *start_out_image_buf_float, *out_image_float;

int       out_image_size_bytes;
int       image_size;

struct header hd;

int main(argc, argv)
    int       argc;
    char    **argv;

{
    int       ac = 1;
    int       ga = 0;
    char      tmp[100];

/*    fprintf( stderr, "argc, %d; argv[0], %s; argv[1], %s; argv[2], %s; argv[3], %s; argv[4], %s\n", argc, argv[0], argv[1], argv[2], argv[3], argv[4]);
*/
/* -------------------------------------------------------*/

    Progname = strsave(*argv);
    ga = ac;
    if (argc > 1)
	while (argv[ac][0] == '-') {	/* arg processing  */
	    if (argv[ac][1] == 'D')
		debug = TRUE;
	    if (argv[ac][1] == 'D' && argv[ac][2] == '2') {
		debug2 = TRUE;
		debug = TRUE;
	    }
/*-b   byte (8 bit) format (the default) */
	    if (argv[ac][1] == 'b' && argv[ac][2] == '\0')
		boflag = TRUE;
/*-s   for short (16 bit) pixel output */
	    if (argv[ac][1] == 's' && argv[ac][2] == '\0') {
		soflag = TRUE;
		boflag = 0;
	    }
/*-i   for integer (32 bit) pixel output */
	    if (argv[ac][1] == 'i' && argv[ac][2] == '\0') {
		ioflag = TRUE;
		boflag = 0;
	    }
/*-f   for float (single precision) pixel output */
	    if (argv[ac][1] == 'f' && argv[ac][2] == '\0') {
		foflag = TRUE;
		boflag = 0;
	    }
/*-r   number of rows in output image */
	    if (argv[ac][1] == 'r' && argv[ac][2] == '\0') {
		num_rows = atoi(argv[++ac]);
	    }
/*-c   number of rows in output image */
	    if (argv[ac][1] == 'c' && argv[ac][2] == '\0') {
		num_cols = atoi(argv[++ac]);
	    }
	    if ((foflag + ioflag + soflag) > 1)
		perr(HE_MSG, "Only one of -s, -i or -f may be specified.");

/*-np_row %i  number of patchs per row*/
	    if (argv[ac][1] == 'n' && argv[ac][2] == 'p'
		&& argv[ac][3] == '_' && argv[ac][4] == 'r') {
		num_patchs_row_dir = atoi(argv[++ac]);
	    }
/*-np_col %i  number of patchs per col*/
	    if (argv[ac][1] == 'n' && argv[ac][2] == 'p'
		&& argv[ac][3] == '_' && argv[ac][4] == 'c') {
		num_patchs_col_dir = atoi(argv[++ac]);
	    }
/*-hi_gray %f  gray value of brighter patch   */
	    if (argv[ac][1] == 'h' && argv[ac][2] == 'i'
		&& argv[ac][3] == '_' && argv[ac][4] == 'g') {
		hi_gray_arg = atof(argv[++ac]);
		hi_g_flag = 1;
	    }
/*-lo_gray %f  gray value of dimmer patch   */
	    if (argv[ac][1] == 'l' && argv[ac][2] == 'o'
		&& argv[ac][3] == '_' && argv[ac][4] == 'g') {
		lo_gray_arg = atof(argv[++ac]);
		lo_g_flag = 1;
	    }
	    ac++;
	    if (ac > (argc - 1))
		break;


	    if (ga == ac)	/* no arg consumed */
		perr(HE_MSG, "Incomprehensible arguments.");
	}			/* end of arg processing */


    if (debug > 0) {
	fprintf(stderr, "boflag:%d; soflag:%d; ioflag:%d; foflag:%d; num_rows:%d; num_cols:%d; num_frms:%d;\n",
	      boflag, soflag, ioflag, foflag, num_rows, num_cols, num_frms);
	fprintf(stderr, "num_patchs_col_dir:%d; num_patchs_row_dir:%d;\n",
		num_patchs_col_dir, num_patchs_row_dir);
    }
    image_size = num_rows * num_cols;

    /* PFBYTE  */
    if (boflag > 0) {
	start_out_image_buf_byte = (unsigned char *) halloc(image_size, sizeof(char));
	out_image_byte = start_out_image_buf_byte;
	out_image_size_bytes = image_size * (sizeof(char));

	max_gray = two_pow_8_m1;

	/* allocate output image, write header   */
	init_header(&hd, "", "", num_frms, "", num_rows, num_cols, PFBYTE, 1, "");

	/*
	 * Update the image header seq_history according to the current
	 * command string. update_header(&image_header,argc,argv)
	 */

	update_header(&hd, argc, argv);

	write_header(&hd);

    }
    if (ioflag > 0) {
	start_out_image_buf_int = (unsigned int *) halloc(image_size, sizeof(int));
	out_image_int = start_out_image_buf_int;
	out_image_size_bytes = image_size * (sizeof(int));

	max_gray = two_pow_32_m1;

	/* allocate output image, write header   */
	init_header(&hd, "", "", num_frms, "", num_rows, num_cols, PFINT, 1, "");

	update_header(&hd, argc, argv);

	write_header(&hd);

    }
    if (soflag > 0) {
	start_out_image_buf_short = (unsigned short *) halloc(image_size, sizeof(short));
	out_image_short = start_out_image_buf_short;
	out_image_size_bytes = image_size * (sizeof(short));

	max_gray = two_pow_16_m1;

	/* allocate output image, write header   */
	init_header(&hd, "", "", num_frms, "", num_rows, num_cols, PFSHORT, 1, "");

	update_header(&hd, argc, argv);

	write_header(&hd);

    }
    if (foflag > 0) {
	start_out_image_buf_float = (float *) halloc(image_size, sizeof(float));
	out_image_float = start_out_image_buf_float;
	out_image_size_bytes = image_size * (sizeof(float));

	max_gray = 1.0;

	/* allocate output image, write header   */
	init_header(&hd, "", "", num_frms, "", num_rows, num_cols, PFFLOAT, 1, "");

	update_header(&hd, argc, argv);

	write_header(&hd);

    }
/*  get gray levels for patchs  */
    if (hi_g_flag)
	hi_gray = hi_gray_arg;
    else
	hi_gray = max_gray / 2.;

    if (lo_g_flag)
	lo_gray = lo_gray_arg;
    else
	lo_gray = 0.;


    num_cols_per_patch = num_cols / num_patchs_col_dir;
    if (num_cols_per_patch < 1)
	num_cols_per_patch = 1;
    num_rows_per_patch = num_rows / num_patchs_row_dir;
    if (num_rows_per_patch < 1)
	num_rows_per_patch = 1;

    if (debug)
	fprintf(stderr, "lo_gray:%f; hi_gray:%f; num_cols_per_patch:%d; num_rows_per_patch:%d;\n", lo_gray, hi_gray, num_cols_per_patch, num_rows_per_patch);

    /* generate the cols of patchs (index patchs in col direction) */
    cur_col = 0;
    cur_row = 0;
    for (i_patch_col = 0; i_patch_col <= num_patchs_col_dir; i_patch_col++) {
	/* in each patch, replicate the cols  */
	for (repl_cols = 0; repl_cols <= (num_cols_per_patch - 1); repl_cols++) {
	    /* generate the rows (index patchs in rows direction)  */
	    cur_row = 0;
	    for (i_patch_row = 0; i_patch_row <= num_patchs_row_dir; i_patch_row++) {
		if ((i_patch_row + i_patch_col) % 2 == 0)
		    patch_gray = lo_gray;
		else
		    patch_gray = hi_gray;

		/* replicate the rows in the patch */
		for (repl_rows = 0; repl_rows <= (num_rows_per_patch - 1); repl_rows++) {
		    /*
		     * Fortran: ofr = row + col * row_max; C: ofr = col +
		     * row*col_max
		     */
		    loc = cur_row * num_cols + cur_col;

		    if (debug2)
			fprintf(stderr, "cur_col:%d; cur_row:%d; loc:%d; patch_gray:%f; \n",
				cur_col, cur_row, loc, patch_gray);

		    if (boflag)
			out_image_byte[loc] = patch_gray;
		    if (soflag)
			out_image_short[loc] = patch_gray;
		    if (ioflag)
			out_image_int[loc] = patch_gray;
		    if (foflag)
			out_image_float[loc] = patch_gray;

		    cur_row++;
		    if (cur_row >= (num_rows - 1))
			break;
		}
	    }
	    cur_col++;
	    if (cur_col >= (num_cols - 1))
		break;
	}
    }

    /* generate frames  */
    for (i_frm = 0; i_frm <= (num_frms - 1); i_frm++) {
	if (boflag)
	    if (fwrite(start_out_image_buf_byte, out_image_size_bytes, 1, stdout)
		!= 1) {
		sprintf(tmp, "Error during write of frame number %d.\n", i_frm);
		perr(HE_MSG, tmp);
	    }
	if (soflag)
	    if (fwrite(start_out_image_buf_short, out_image_size_bytes, 1, stdout)
		!= 1) {
		sprintf(tmp, "Error during write of frame number %d.\n", i_frm);
		perr(HE_MSG, tmp);
	    }
	if (ioflag)
	    if (fwrite(start_out_image_buf_int, out_image_size_bytes, 1, stdout)
		!= 1) {
		sprintf(tmp, "Error during write of frame number %d.\n", i_frm);
		perr(HE_MSG, tmp);
	    }
	if (foflag)
	    if (fwrite(start_out_image_buf_float, out_image_size_bytes, 1, stdout)
		!= 1) {
		sprintf(tmp, "Error during write of frame number %d.\n", i_frm);
		perr(HE_MSG, tmp);
	    }
    }
    return (0);
}
