/*
*NAME
*     bin_mask.c - modify the bits of each pixel in an image by bitwise
*                 ANDing them with a given mask
*SYNOPSIS
*     bin_mask [-m]
*DESCRIPTION
*     Bin_mask sets the bits of each pixel in the input image
*     by ANDing them with the input mask.  The input sequence must
*     be byte or short format.  The output sequence is the same as
*     the input.
*
*     The input mask is specified with the option m.  The -m option
*     calls for a string of 1's and 0's.  If -m is not set then the
*     default is '100,' setting the lower two bits of each pixel in
*     the image to zero.  Masks are left filled with 1's.
*
*     -m %s string of 1's and 0's defining the mask (default = '100')
*
*     -adapted from scale_gray by Lara Lewis, LBL, - 22 June 1990
*/

#include <hipl_format.h>
#include <stdio.h>

int       sflag = FALSE;	/* short format */
int       bflag = TRUE;		/* byte format */

int       debug = TRUE;

 /* input image declerations */
unsigned char *start_in_image_buf_byte, *in_image_byte;
unsigned short *start_in_image_buf_short, *in_image_short;

 /* output image declerations */
unsigned char *start_out_image_buf_byte, *out_image_byte;
unsigned short *start_out_image_buf_short, *out_image_short;

int       out_image_size_bytes;
int       in_image_size_bytes;

unsigned char byte_bit_mask = 255;
unsigned short int short_bit_mask = 65535;

unsigned long int bit_mask = 0xFFFFFFFF;
int       masklen;

int       i, frame_num;

unsigned char byte_Pix_in;
unsigned short int short_Pix_in;
unsigned char byte_Pix_out;
unsigned short int short_Pix_out;

int       num_rows, num_cols, num_frames, image_size;

int main(argc, argv)
    int       argc;
    char    **argv;

{
    int       ac = 1;
    int       ga = 0;
    struct header hd;
    char      tmp[100];

    Progname = strsave(*argv);
    read_header(&hd);
    update_header(&hd, argc, argv);

/*	identify input data type and set default lower and upper limits */

    if (hd.pixel_format != PFBYTE && hd.pixel_format != PFSHORT)
	perr(HE_MSG, "Input sequence must be short or byte.");

    if (hd.pixel_format == PFBYTE) {
	bflag = TRUE;
    }
    if (hd.pixel_format == PFSHORT) {
	sflag = TRUE;
    }
/* -------------------------------------------------------*/

    ga = ac;
    if (argc > 1)
	while (argv[ac][0] == '-') {	/* arg processing  */
	    if (argv[ac][1] == 'D')
		debug = TRUE;
/*     -m   defines the mask to be used */
	    if (argv[ac][1] == 'm') {
		/* check if mask is too long */
		masklen = strlen(argv[++ac]);
		if (bflag) {
		    if (masklen > 8)
			perr(HE_MSG, "Mask too long");
		    else if (masklen > 16)
			perr(HE_MSG, "Mask too long");
		}

		/*
		 * left shift the original mask of all 1's by the length of
		 * the given mask so that the created mask will have 1's to
		 * the left of it
		 */

/*       bit_mask = bit_mask << masklen;*/

		/* read mask into an integer */
/*       bit_mask = bit_mask | strtol(argv[ac], NULL, 2); */
		bit_mask = strtol(argv[ac], NULL, 2);

		/* size the mask according to the image format */
		if (bflag)
		    byte_bit_mask = (unsigned char) bit_mask;
		if (sflag)
		    short_bit_mask = (unsigned short) bit_mask;
	    }
	    ac++;
	    if (ac > (argc - 1))
		break;


	    if (ga == ac)	/* no arg consumed */
		perr(HE_MSG, "Incomprehensible arguments.");
	}			/* end of arg processing */

    if (debug) {
	fprintf(stderr, "Args: bflag, %d; sflag, %d;\n", bflag, sflag);
	fprintf(stderr, "Args: masklen, %d;\n", masklen);
	fprintf(stderr, "Args: bit_mask (hexdecimal) %lx;\n", bit_mask);
    }
/* -------------------------------------------------------*/

    write_header(&hd);
    num_rows = hd.orows;
    num_cols = hd.ocols;
    image_size = num_rows * num_cols;
    num_frames = hd.num_frame;


/* -----------------------   set up data type specific image storage ---------*/

    if (bflag) {
	start_in_image_buf_byte = (unsigned char *) halloc(image_size, sizeof(char));
	in_image_byte = start_in_image_buf_byte;
	in_image_size_bytes = image_size * (sizeof(char));
	start_out_image_buf_byte = (unsigned char *) halloc(image_size, sizeof(char));
	out_image_byte = start_out_image_buf_byte;
	out_image_size_bytes = image_size * (sizeof(char));
    }
    if (sflag) {
	start_in_image_buf_short = (unsigned short *) halloc(image_size, sizeof(short));
	in_image_short = start_in_image_buf_short;
	in_image_size_bytes = image_size * (sizeof(short));
	start_out_image_buf_short = (unsigned short *) halloc(image_size, sizeof(short));
	out_image_short = start_out_image_buf_short;
	out_image_size_bytes = image_size * (sizeof(short));
    }
/*---------------------  start by frame processing  -----------------*/

    fprintf(stderr,
	    "bin_mask: There are %d frames in this file.\n", num_frames);

    for (frame_num = 0; frame_num < num_frames; frame_num++) {	/* do sequence */
	fprintf(stderr,
		"bin_mask: Working on frame number %d \n", frame_num + 1);

	if (bflag) {
	    if (fread(start_in_image_buf_byte, in_image_size_bytes, 1, stdin) != 1) {
		sprintf(tmp, "Unexpected end-of-file in frame number %d. %d bytes \
          			    read for this frame.", frame_num, i);
		perr(HE_MSG, tmp);
	    }
	    in_image_byte = start_in_image_buf_byte;	/* set pointer to start
							 * of image */
	}
	if (sflag) {
	    if (fread(start_in_image_buf_short, in_image_size_bytes, 1, stdin) != 1) {
		sprintf(tmp, "Unexpected end-of-file in frame number %d. %d bytes \
          			    read for this frame.", frame_num, i);
		perr(HE_MSG, tmp);
	    }
	    in_image_short = start_in_image_buf_short;
	}
	for (i = 0; i < image_size; i++) {	/* process the frame */
	    if (bflag)
		byte_Pix_in = *in_image_byte++;
	    if (sflag)
		short_Pix_in = *in_image_short++;

	    /* set specified lower bits to zero */
	    if (bflag)
		byte_Pix_out = byte_Pix_in & byte_bit_mask;

	    if (sflag)
		short_Pix_out = short_Pix_in & short_bit_mask;

	    /* finish up	 */

	    if (bflag)
		*out_image_byte++ = byte_Pix_out;
	    if (sflag)
		*out_image_short++ = short_Pix_out;

	}			/* frame complete */


	if (bflag)
	    if (fwrite(start_out_image_buf_byte, out_image_size_bytes, 1, stdout)
		!= 1) {
		sprintf(tmp, "Error during write of frame number %d.\n", frame_num);
		perr(HE_MSG, tmp);
	    }
	/* reset image buffer pointer for each frame */
	out_image_byte = start_out_image_buf_byte;

	if (sflag)
	    if (fwrite(start_out_image_buf_short, out_image_size_bytes, 1, stdout)
		!= 1) {
		sprintf(tmp, "Error during write of frame number %d.\n", frame_num);
		perr(HE_MSG, tmp);
	    }
	out_image_short = start_out_image_buf_short;


    }				/* seq complete  */
    return (0);
}
