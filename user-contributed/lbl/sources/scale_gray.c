/*
*NAME
*     scale_gray - scale the pixel values of a sequence of images
*SYNOPSIS
*     scale_gray [-imin -imax -omin -omax][-e] [-f, -s or -i] [-A -B -C]
*DESCRIPTION
*     Scale_gray scales the  pixel  values  of  a  sequence  either
*     linearly  or  using a quadratic.  The input sequence must be
*     in byte,  short,  integer,  or  float  format.   The  output
*     sequence is:
*     -b   byte (8 bit) format (the default)
*     -s   for short (16 bit) pixel output
*     -i   for integer (32 bit) pixel output
*     -f   for float (single precision) pixel output
*
*     If no scale factors are given, the scaling  is  such  as  to
*     stretch the populated part of the histogram of the
*     first frame to use the entire range of the output pixel
*     format (8, 16, or 32 bits, that is 0-255, 0-65535, or  0-4294967295
*     for  fixed point formats, 0.0 to 1.0 for floating point pixels).
*     The entire sequence is scaled based on the first frame range
*     unless -e is given.
*
*     If an output range is given, then the input range is  mapped lineraly
*     to the output range. If an input range is given then that range
*     is mapped to the output range (instead of the entire input range).
*
*     -imin %f  lower end of the input range (default = 0)
*     -imax %f  upper end of the input range
*                            (default = max pixel value for the image)
*     -omin %f  lower end of the output range (default = 0)
*     -omax %f  upper end of the outptu range
*                           (default = max for the output data type))
*     -e   Forces re-normalization for every frame.  That  is  the
*          mapping  (imin,imax)  to  (omin,omax) is recom-
*          puted for every frame.
*     -c   Forces values that are outside the input range to be set to
*          zero in the output image. Default is to set them to imin or imax
*          before scaling.
*
*     Provided that an output range is NOT given, the mapping of the input
*     range to an output range can be given by a quadratic:
*     Pixels "x" are mapped to Ax*x+Bx+C.
*     -A %f    The quadratic coefficient (default is A = 0)
*     -B %f    the linear coefficient (default is B = 1.0)
*     -C %f    the constant term (default is C = 0)
*
*     Except for float  pixel  output  the  quadratic  values  are
*     clipped  to  fit  into the output data type (byte, short,
*     or integer pixels)
*     For
*     byte,  short  and  integer  output,  the  output  values are
*     rounded (by adding .5 before truncation).
*
*	-Bill Johnston, LBL - 26 Aug. 1989
*/
/*   This program is copyright (C) 1990, Regents  of  the
University  of  California.   Anyone may reproduce this software,
in whole or in part, provided that:
(1)  Any copy  or  redistribution  must  show  the
     Regents  of  the  University of California, through its
     Lawrence Berkeley Laboratory, as the source,  and  must
     include this notice;
(2)  Any use of this software must reference this  distribu-
     tion,  state that the software copyright is held by the
     Regents of the University of California, and  that  the
     software is used by their permission.

     It is acknowledged that the U.S. Government has  rights
to this software under  Contract DE-AC03-765F00098 between the U.S.
Department of Energy and the University of California.

     This software is provided as a professional  academic  contribu-
tion  for  joint exchange.  Thus it is experimental, is pro-
vided ``as is'', with no warranties of any kind  whatsoever,
no  support,  promise  of updates, or printed documentation.
Bug reports or fixes may be sent to the author, who may or may
not act on them as he desires.
*/

#include <stdio.h>

#define TRUE 1;
#define FALSE 0;

#include <hipl_format.h>

float two_pow_8_m1 = 255.;
float two_pow_16_m1 = 65535.;
float two_pow_32_m1 = 4294967295.;
float Pixel_max_value;

int foflag = FALSE;	/* -f specified, float output */
int ioflag = FALSE;	/* -i specified, integer output */
int soflag = FALSE;	/* -s specified, short output */
int boflag = TRUE;	/* -b specified, byte output */

int debug = FALSE;
int debug_first = FALSE;
int first = TRUE;	/* set FALSE after first frame, for setting max/min once only */
int lcount = 0;	/* underflow count */
int hcount = 0;	/* overflow count */
int eflag = FALSE;	/* -e specified, histogram every frame */
int clip_flag = FALSE;	/* -c specified, input range is cliped */

int fiflag = FALSE;  /* float input image */
int siflag = FALSE;  /* short integer input image */
int iiflag = FALSE;  /* integer input image */
int biflag = FALSE;  /* byte input image */

	/* input image declerations */
unsigned char *start_in_image_buf_byte, in_pixel_max_byte, in_pixel_min_byte, *in_image_byte;
unsigned short *start_in_image_buf_short, in_pixel_max_short, in_pixel_min_short, *in_image_short;
unsigned int *start_in_image_buf_int, in_pixel_max_int, in_pixel_min_int, *in_image_int;
float *start_in_image_buf_float, in_pixel_max_float, in_pixel_min_float, *in_image_float;

	/* output image declerations */
unsigned char *start_out_image_buf_byte, *out_image_byte;
unsigned short *start_out_image_buf_short, *out_image_short;
unsigned int *start_out_image_buf_int, *out_image_int;
float *start_out_image_buf_float, *out_image_float;

int out_image_size_bytes;
int in_image_size_bytes ;


int i, frame_num;

int min_in_flag = FALSE;  /* user input flag for low end of input range  */
float min_in;
int max_in_flag = FALSE;  /* user input flag for hi end of input range  */
float max_in;
int min_out_flag = FALSE;  /* user input flag for low end of output range  */
float min_out;
int max_out_flag = FALSE;  /* user input flag for hi end of output range  */
float max_out;

float in_pixel_min;
float in_pixel_max;

int int_temp1;
int int_temp2;

float Scal_i_o;
float Pix_in;
float Pix_out;

float A = 0.0;   /* default quadratic coeff */
float B = 1.0;   /* default linear coeff */
float C = 0.0;   /* default constant term */
int quad_flag = FALSE;	/* -A, B or C given. Invoke quadratic */

int num_rows, num_cols, num_frames, image_size;

int main(argc,argv)

int argc;
char **argv;

{
int ac = 1;
int ga = 0;
struct	header hd;

Progname = strsave(*argv);
if (argc > 1 &&  argv[ac][1] ==  'h') 
    {
    fprintf(stderr, "scale_gray, map image gray values by a window transform\n\
scale_gray [-imin -imax -omin -omax][-e] [-b, -f, -s or -i] [-A -B -C]\n\
-e   Forces re-normalization for every frame\n\
-c   Forces values that are outside the input range to be set to zero\n\
Provided that an output range is NOT given, the mapping of the input\n\
range to an output range can be given by a quadratic (Ax*x+Bx+C)\n\
-A  The quadratic coefficient (default is A = 0)\n\
-B  the linear coefficient (default is B = 1.0)\n\
-C  the constant term (default is C = 0)\n");
    exit (0);
    }

read_header(&hd);
update_header(&hd,argc,argv);

/*	identify input data type and set default lower and upper limits */

if (hd.pixel_format != PFFLOAT && hd.pixel_format != PFINT
     && hd.pixel_format != PFBYTE && hd.pixel_format != PFSHORT)
    perr(HE_MSG,"Input sequence must be float, integer, short or byte.");

if (hd.pixel_format == PFBYTE)
    {
    biflag=TRUE;
    /* max_in is either user supplied, or set to max (image) */
    }

if (hd.pixel_format == PFINT)
    {
    iiflag=TRUE;
    }

if (hd.pixel_format == PFSHORT)
    {
    siflag=TRUE;
    }

if (hd.pixel_format == PFFLOAT)
    {
    fiflag=TRUE;
    }
/* -------------------------------------------------------*/

ga = ac;
if (argc > 1)
{
while (argv[ac][0]=='-')
   {		/* arg processing  */
    if (argv[ac][1] ==  'D') debug=TRUE;

/*     -b   byte (8 bit) format (the default) */
    if (argv[ac][1] ==  'b' && argv[ac][2]=='\0') boflag=TRUE;
/*     -s   for short (16 bit) pixel output */
    if (argv[ac][1] ==  's' && argv[ac][2]=='\0') { soflag=TRUE; boflag = FALSE;}
/*     -i   for integer (32 bit) pixel output */
    if (argv[ac][1] ==  'i' && argv[ac][2]=='\0') { ioflag=TRUE; boflag = FALSE;}
/*     -f   for float (single precision) pixel output */
    if (argv[ac][1] ==  'f' && argv[ac][2]=='\0') { foflag=TRUE; boflag = FALSE;}
/*     -e   Forces re-normalization for every frame. */
    if (argv[ac][1] ==  'e' && argv[ac][2]=='\0') { eflag=TRUE;}
/*     -c   Forces values outside the input range to be set to zero. Default
*             is to set them to imin or imax. */
    if (argv[ac][1] ==  'c' && argv[ac][2]=='\0') clip_flag=TRUE;

/*     -A %f    The quadratic coefficient (default is A = 0) */
    if (argv[ac][1] ==  'A' && argv[ac][2]=='\0')
    	{ quad_flag=TRUE; A = atof(argv[++ac]);}
/*     -B %f    the linear coefficient (default is B = 1.0) */
    if (argv[ac][1] ==  'B' && argv[ac][2]=='\0')
    	{ quad_flag=TRUE; B = atof(argv[++ac]);}
/*     -C %f    the constant term (default is C = 0 */
    if (argv[ac][1] ==  'C' && argv[ac][2]=='\0')
    	{ quad_flag=TRUE; C = atof(argv[++ac]);}

    if ((foflag +  ioflag + soflag) > 1)
    	perr(HE_MSG,"Only one of -s, -i or -f may be specified.");

/*     -imin %d  lower end of the input range (default = 0) */
    if (argv[ac][1]=='i' && argv[ac][2]=='m' 
                          && argv[ac][3]=='i' && argv[ac][4]=='n')
       {
    	min_in = atof(argv[++ac]);
    	min_in_flag=TRUE;
       }

/*     -imax %d  upper end of the input range
*                            (default = max for input data type) */
    if (argv[ac][1]=='i' && argv[ac][2]=='m' 
                          && argv[ac][3]=='a' && argv[ac][4]=='x')
       {
    	max_in = atof(argv[++ac]);
    	max_in_flag=TRUE;
       }

/*     -omin %d  lower end of the output range (default = 0) */
    if (argv[ac][1]=='o' && argv[ac][2]=='m' 
                          && argv[ac][3]=='i' && argv[ac][4]=='n')
       {
    	min_out = atof(argv[++ac]);
    	min_out_flag=TRUE;
       }

/*     -omax %d  upper end of the output range (default = data type max) */
    if (argv[ac][1]=='o' && argv[ac][2]=='m' 
                          && argv[ac][3]=='a' && argv[ac][4]=='x')
       {
    	max_out = atof(argv[++ac]);
    	max_out_flag=TRUE;
       }

    ac++;
    if (ac > (argc -1)) break;


    if (ga == ac)	/* no arg consumed */
    	perr(HE_MSG,"Incomprehensible arguments.");
   }  /* end of arg processing */
}

if (debug)
    {
    fprintf(stderr, "scale_gray(D): Args: biflag, %d; siflag, %d; iiflag, %d; fiflag %d\n",
		biflag, siflag, iiflag, fiflag);
    fprintf(stderr,"scale_gray(D): Args: boflag, %d; soflag, %d; ioflag, %d; foflag, %d; eflag, %d; clip_flag %d;\n",
            boflag, soflag, ioflag, foflag, eflag, clip_flag);
    if (quad_flag)
    	fprintf(stderr, "scale_gray(D): Args: A, %f; B, %f; C, %f \n", A, B, C);
    fprintf(stderr, "scale_gray(D): Args: min_in %f; max_in %f; min_out %f; max_out %f\n",
		min_in, max_in, min_out, max_out);
    }


/*	identify output data type and set lower and upper limits */


if (foflag)
    {
    hd.pixel_format = PFFLOAT;

    if (!quad_flag)
	{
	if (!max_out_flag) /* then use default */
    	    max_out = 1.0;

	if (!min_out_flag) /* then use default */
    	    min_out = 0.;
	}
    if (debug)
	fprintf (stderr, "scale_gray(D): Float out: (no Pixel_max_value)\n");
    }

if (ioflag) /*  integer output  */
    {
    hd.pixel_format = PFINT;
    Pixel_max_value = two_pow_32_m1;

    if (!max_out_flag) /* then use default */
            max_out = two_pow_32_m1;

    if (!min_out_flag) /* then use default */
    	    min_out = 0.;
    if (debug)
	fprintf (stderr, "scale_gray(D): Int out: Pixel_max_value = %f \n", Pixel_max_value);
    }

if (soflag)
    {
    hd.pixel_format = PFSHORT;
    Pixel_max_value = two_pow_16_m1;

    if (!max_out_flag) /* then use default */
            max_out = two_pow_16_m1;

    if (!min_out_flag) /* then use default */
    	    min_out = 0.;
    if (debug)
	fprintf (stderr, "scale_gray(D): Short out: Pixel_max_value = %f \n", Pixel_max_value);

    }

if (boflag)
    {
    hd.pixel_format = PFBYTE;
    Pixel_max_value = two_pow_8_m1;

    if (!max_out_flag) /* then use default */
            max_out = two_pow_8_m1;

    if (!min_out_flag) /* then use default */
    	    min_out = 0.;
    if (debug)
	fprintf (stderr, "scale_gray(D): Byte out: Pixel_max_value = %f \n", Pixel_max_value);
    }

if (boflag + soflag + ioflag + foflag != 1)
    perr (HE_MSG," Pixel output type not set.");

/* -------------------------------------------------------*/

write_header(&hd);
num_rows = hd.orows;
num_cols = hd.ocols;
image_size = num_rows * num_cols;
num_frames = hd.num_frame;


/* -----------------------   set up data type specific image storage ---------*/

if (biflag)
    {
    start_in_image_buf_byte = (unsigned char *) halloc (image_size, sizeof (char));
    in_image_byte = start_in_image_buf_byte;
    in_image_size_bytes = image_size * (sizeof(char));
    }
if (siflag)
    {
    start_in_image_buf_short = (unsigned short *) halloc (image_size, sizeof (short));
    in_image_short = start_in_image_buf_short;
    in_image_size_bytes = image_size * (sizeof(short));
    }
if (iiflag)
    {
    start_in_image_buf_int = (unsigned int *) halloc (image_size, sizeof (int));
    in_image_int = start_in_image_buf_int;
    in_image_size_bytes = image_size * (sizeof(int));
    }
if (fiflag)
    {
    start_in_image_buf_float = (float *) halloc (image_size, sizeof (float));
    in_image_float = start_in_image_buf_float;
    in_image_size_bytes = image_size * (sizeof(float));
    }

if (boflag)
    {
    start_out_image_buf_byte = (unsigned char *) halloc (image_size, sizeof (char));
    out_image_byte = start_out_image_buf_byte;
    out_image_size_bytes = image_size * (sizeof (char));
    }
if (soflag)
    {
    start_out_image_buf_short = (unsigned short *) halloc (image_size, sizeof (short));
    out_image_short = start_out_image_buf_short;
    out_image_size_bytes = image_size * (sizeof (short));
    }
if (ioflag)
    {
    start_out_image_buf_int = (unsigned int *) halloc (image_size, sizeof (int));
    out_image_int = start_out_image_buf_int;
    out_image_size_bytes = image_size * (sizeof (int));
    }
if (foflag)
    {
    start_out_image_buf_float = (float *) halloc (image_size, sizeof (float));
    out_image_float = start_out_image_buf_float;
    out_image_size_bytes = image_size * (sizeof (float));
    }

/*---------------------  start by frame processing  -----------------*/

fprintf(stderr,
  "scale_gray: There are %d frames in this file.\n", num_frames);

for (frame_num=0; frame_num < num_frames; frame_num++)
    {	/* do sequence */
	fprintf(stderr,
  	"scale_gray: Working on frame number %d \n", frame_num+1);

    if(biflag)
	{
        if (fread(start_in_image_buf_byte, in_image_size_bytes,1,stdin) != 1)
	    fprintf(stderr,"scale_gray: Unexpected end-of-file in frame number %d. %d bytes \
          			    read for this frame.", frame_num, i);
    	in_image_byte = start_in_image_buf_byte; /* set pointer to start of image */
	}
    if(siflag)
	{
        if (fread(start_in_image_buf_short, in_image_size_bytes,1,stdin) != 1)
	    fprintf(stderr,"scale_gray: Unexpected end-of-file in frame number %d. %d bytes \
          			    read for this frame.", frame_num, i);
    	in_image_short = start_in_image_buf_short;
	}
    if(iiflag)
	{
        if (fread(start_in_image_buf_int, in_image_size_bytes,1,stdin) != 1)
	    fprintf(stderr,"scale_gray: Unexpected end-of-file in frame number %d. %d bytes \
          			    read for this frame.", frame_num, i);
    	in_image_int = start_in_image_buf_int;
	}
    if(fiflag)
	{
        if (fread(start_in_image_buf_float, in_image_size_bytes,1,stdin) != 1)
	    fprintf(stderr,"scale_gray: Unexpected end-of-file in frame number %d. %d bytes \
          			    read for this frame.", frame_num, i);
    	in_image_float = start_in_image_buf_float;
	}

    /* if either the input min or max has not been given, or
     * if we are at the first frame, or if the "recalc
     * for every frame" flag is set, then get frame min and max
     */

    if (first || eflag)
	{	/* set up in-window to out-window mapping scale factors */
    	first = FALSE;

	if (biflag)
	    {	/* find min - max from image, if input window not given */
    	    in_pixel_max_byte = in_pixel_min_byte = start_in_image_buf_byte[0];

    	    for (i=1; i < image_size; i++)	/* find min max  */
                {
                if (*++in_image_byte > in_pixel_max_byte)
			in_pixel_max_byte = *in_image_byte;
                if (*in_image_byte < in_pixel_min_byte)
			in_pixel_min_byte = *in_image_byte;
                }
    	    in_image_byte = start_in_image_buf_byte; /* reset image pointer */
	    in_pixel_min = in_pixel_min_byte;
	    in_pixel_max = in_pixel_max_byte;
	    }

	if (siflag)
	    {
    	    in_pixel_max_short = in_pixel_min_short = start_in_image_buf_short[0];

    	    for (i=1; i < image_size; i++)	/* find min max  */
                {
                if (*++in_image_short > in_pixel_max_short)
			in_pixel_max_short = *in_image_short;
                if (*in_image_short < in_pixel_min_short)
			in_pixel_min_short = *in_image_short;
                }
    	    in_image_short = start_in_image_buf_short; /* reset image pointer */
	    in_pixel_min = in_pixel_min_short;
	    in_pixel_max = in_pixel_max_short;
	    }

	if (iiflag)
	    {
    	    in_pixel_max_int = in_pixel_min_int = start_in_image_buf_int[0];

    	    for (i=1; i < image_size; i++)	/* find min max  */
                {
                if (*++in_image_int > in_pixel_max_int)
			in_pixel_max_int = *in_image_int;
                if (*in_image_int < in_pixel_min_int)
			in_pixel_min_int = *in_image_int;
                }
    	    in_image_int = start_in_image_buf_int; /* reset image pointer */
	    in_pixel_min = in_pixel_min_int;
	    in_pixel_max = in_pixel_max_int;
	    }

	if (fiflag)
	    {
    	    in_pixel_max_float = in_pixel_min_float = start_in_image_buf_float[0];

    	    for (i=1; i < image_size; i++)	/* find min max  */
                {
                if (*++in_image_float > in_pixel_max_float)
			in_pixel_max_float = *in_image_float;
                if (*in_image_float < in_pixel_min_float)
			in_pixel_min_float = *in_image_float;
                }
    	    in_image_float = start_in_image_buf_float; /* reset image pointer */
	    in_pixel_min = in_pixel_min_float;
	    in_pixel_max = in_pixel_max_float;
	    }


    	if (!min_in_flag) /* not user spec., so set */
            min_in = in_pixel_min;
    	if (!max_in_flag) /* not user spec., so set */
            max_in = in_pixel_max;

    	if ((!quad_flag && (max_out - min_out)==0) || (max_in - min_in)==0 )
	    {
            fprintf(stderr, "scale_gray: max_out = min_out, or max_in = min_in");
	    exit (1);
	    }

    	if (!quad_flag) Scal_i_o = (max_out - min_out)/(max_in - min_in);

        if (biflag) { int_temp1=in_pixel_min+.5; int_temp2=in_pixel_max+.5;
	  fprintf(stderr,
	  "scale_gray: input frame is byte pixel format, frame number is %d, min = %d, max = %d\n",
		frame_num+1, int_temp1, int_temp2);}

        if (siflag) { int_temp1=in_pixel_min+.5; int_temp2=in_pixel_max+.5;
	  fprintf(stderr,
	  "scale_gray: input frame is short pixel format, frame number is %d, min = %d, max = %d\n",
		frame_num+1, int_temp1, int_temp2);}

        if (iiflag) { int_temp1=in_pixel_min+.5; int_temp2=in_pixel_max+.5;
	fprintf(stderr,
	  "scale_gray: input frame is integer pixel format, frame number is %d, min = %d, max = %d\n",
		frame_num+1, int_temp1, int_temp2);}

        if (fiflag) { fprintf(stderr,
	  "scale_gray: input frame is float pixel format, frame number is %d, min = %f, max = %f\n",
		frame_num+1, in_pixel_min, in_pixel_max);}

        fprintf(stderr,
		"scale_gray: input window min = %f, max = %f\n",
			min_in, max_in);
	if (!quad_flag)
        fprintf(stderr,
		"scale_gray: output window min = %f, max = %f\n",
			min_out, max_out);
    	}	/* end scale factor setup */

/*
 *Scale factors: (They are set in the i/o routines in case min_in
 *and/or max_in are gotten from the pixel limits of the frame.)
 *
 *Pix_out = (Pix_in - min_in) * (max_out - min_out) + min_out
 *           ----------------
 *           (max_in - min_in)
 */


    if (debug)
        {
        if (quad_flag)
	    {
	    fprintf (stderr, "scale_gray(D): A, %f, B, %f, C, %f\n",A, B, C);
	    }
        else
	    {
	    fprintf (stderr, "scale_gray(D): min_in= %f; max_in= %f; min_out= %f; max_out= %f\n",
	    min_in, max_in, min_out, max_out);
	    }
	}
/*    debug_first = TRUE; */
    for (i=0; i < image_size; i++)
	{ 	/* process the frame */
        if (biflag) Pix_in = *in_image_byte++;
        if (siflag) Pix_in = *in_image_short++;
        if (iiflag) Pix_in = *in_image_int++;
        if (fiflag) Pix_in = *in_image_float++;

    /*  pixel less than lower window bound	*/
	if( Pix_in < min_in)
	    {
/*            if (debug & debug_first) fprintf (stderr, "scale_gray(D):loop: pixel less than lower window bound\n"); */
	    if (clip_flag)
		{
    		/* set output to zero if clipping */
		Pix_out = 0.;
		}
	    else
		{
    		  /* set to imin if pixel is less than imin */
                Pix_in = min_in;
                Pix_out = min_out;

    		  /* do quadratic, if necessary */
		if (quad_flag)
		    {
		    if (!foflag)
			Pix_out = A * Pix_in*Pix_in + B * Pix_in + C + 0.5;
		    else 	/* don't round for float output */
			Pix_out = A * Pix_in*Pix_in + B * Pix_in + C;
		    }
    	        }
	    goto next_pixel;
	    }

    /*  pixel greater than upper window bound	*/
 	if( Pix_in > max_in)
            {
/*            if (debug & debug_first) fprintf (stderr, "scale_gray(D):loop: pixel greater than upper window bound\n"); */
            if (clip_flag)
                {
                /* set output to zero if clipping */
                Pix_out = 0.;
                }
            else
                {
                  /* set output to max if it is greater than a given value */
                Pix_in = max_in;
                Pix_out = max_out;

                  /* do quadratic, if necessary */
                if (quad_flag)
                    {
		    if (!foflag)
                        Pix_out = A * Pix_in*Pix_in + B * Pix_in + C + 0.5;
                    else        /* don't round for float output */
                        Pix_out = A * Pix_in*Pix_in + B * Pix_in + C;
                    }
                }
            goto next_pixel;
            }

    /*  pixel is within input window bound	*/
        if (quad_flag)
    	    {
		 /* do quadratic transformation*/
            if (!foflag)
                Pix_out = A * Pix_in*Pix_in + B * Pix_in + C + 0.5;
            else        /* don't round for float output */
                Pix_out = A * Pix_in*Pix_in + B * Pix_in + C;
    	    }
	else
        	/* do linear window mapping */
    	    {
            if (!foflag)
                {
    	        Pix_out = (Pix_in - min_in) * Scal_i_o + min_out + 0.5;
/*                if (debug & debug_first) fprintf (stderr, "scale_gray(D):loop: Pix_in=%f, Pix_out=%f\n", Pix_in, Pix_out); */
                }
            else        /* don't round for float output */
    	        Pix_out = (Pix_in - min_in) * Scal_i_o + min_out;
    	    }

    /*  finish up	*/
        next_pixel:

	if (quad_flag && !foflag)
	    {
                /* for fixed point pixel formats limit quadratic transformed
			pixels to valid value */
            if (Pix_out < 0. )
                {
                Pix_out = 0.;
                lcount++;
                }
            if (Pix_out > Pixel_max_value)
                {
                Pix_out = Pixel_max_value;
                hcount++;
                }
	    }

	if (boflag) *out_image_byte++ = Pix_out;
	if (soflag) *out_image_short++ = Pix_out;
	if (ioflag) *out_image_int++ = Pix_out;
	if (foflag) *out_image_float++ = Pix_out;

/*        debug_first = FALSE; */
/*        if (i%1000 == 0) debug_first=TRUE; */

        } /* frame complete */

    
    if (boflag)
        if (fwrite(start_out_image_buf_byte, out_image_size_bytes,1,stdout)
						!= 1)
    	    fprintf(stderr,"scale_gray: Error during write of frame number %d.\n",frame_num);
	/* reset image buffer pointer for each frame */
	out_image_byte = start_out_image_buf_byte;
    if (soflag)
        if (fwrite(start_out_image_buf_short, out_image_size_bytes,1,stdout)
						!= 1)
    	    fprintf(stderr,"scale_gray: Error during write of frame number %d.\n",frame_num);
	out_image_short = start_out_image_buf_short;
    if (ioflag)
        if (fwrite(start_out_image_buf_int, out_image_size_bytes,1,stdout)
						!= 1)
    	    fprintf(stderr,"scale_gray: Error during write of frame number %d.\n",frame_num);
	out_image_int = start_out_image_buf_int;
    if (foflag)
        if (fwrite(start_out_image_buf_float, out_image_size_bytes,1,stdout)
						!= 1)
    	    fprintf(stderr,"scale_gray: Error during write of frame number %d.\n",frame_num);
	out_image_float = start_out_image_buf_float;

    if (lcount > 0 || hcount > 0)
        fprintf(stderr, "scale_gray: %d underflows and %d overflows generated \
	by: %f * pix**2 + %f * pix + %f  in frame number %d\n",
    	lcount,hcount, A, B, C, frame_num);

    lcount = 0;
    hcount = 0;

    } /*  seq complete  */
return (0);
}
