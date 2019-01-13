/*
 *	PROGRAM
 *		map
 *
 *	PURPOSE
 *		maps specified grayscale range of an image 
 *		into the grayscale range of 0-255 
 *		using a linear transformation.
 *
 *	SYNTAX
 *	    map [-s scale] [in_LOval in_HIval [out_LOval out_HIval]]
 *		argument parameter definitions:
 *		   <scale>:	scale constant
 *		   <in_LOval>:  low value of input grayscale range
 *     		   <in_HIval>:  high value of input grayscale range
 *		   <out_LOval>:  low value of output grayscale range
 *     		   <out_HIval>:  high value of output grayscale range
 *
 *	DEFAULTS
 *		All grayscale values less than in_LOval and greater than
 *		in_HIval will be mapped into the greyscale values of
 *		out_LOval and out_HIval, respectively.
 *		If in_LOval and in_HIval are not specified they default to 
 *		0 and the 2048 or the maximum for the input pixel size.
 *		if out_LOval and out_HIval are not specified they default to
 *		0 and 255 respectively.
 *		If scale is defined, then the input is multiplied by scale
 *		and truncated into a BYTE image.
 *	!!! map works on INT, SHORT, and BYTE input images.  The output
 *		image is BYTE format.
 *
 *	AUTHOR
 *		Stuart Ware & Charles Carman
 *		for
 *		Merickel Imaging Lab
 *		Biomedical Engineering
 *		University of Virginia
 *		Charlottesville, VA   22903
 */
#include <stdio.h>
#include <hipl_format.h>
#define COLMAX 10000

int main(argc, argv)
int argc;
char *argv[];
{
	struct header hd;
	double atof();
	register int offset;
	register float scale;
	int in_LOval;		/* the lowest value of input to be mapped */
	int in_HIval;		/* the highest value of input to be mapped */
	int out_LOval;		/* the lowest value of output */
	int out_HIval;		/* the highest value of output */
	int arg_vals[2][2];
	int bytes;		/* byte depth of input image */
	int nrows, ncols;
	int i, n;			
	int outflg = 0, scl_flg = 0;
	register int j;
	register unsigned char *op;
	int inbuf[COLMAX];
	unsigned char outbuf[COLMAX];
	char tmp[100];

	Progname = strsave(*argv);
	/*  check argument list  */
	if (argc == 2 || argc > 7)  
	 perr(HE_MSG,
	    "usage:  %s [-s scale] [in_LOval in_HIval [out_LOval out_HIval]]");

	/* read in the header */
	read_header(&hd);

	/* Set up for new picture header */
	switch (hd.pixel_format) {
	case PFBYTE:
		in_LOval = 0;
		in_HIval = 255;
		bytes = sizeof(char);
		break;
	case PFSHORT:
		in_LOval = 0;
		in_HIval = 4095;
		bytes = sizeof(short);
		hd.pixel_format = PFBYTE;
		break;
	case PFINT:
		in_LOval = 0;
		in_HIval = 4095;
		bytes = sizeof(int);
		hd.pixel_format = PFBYTE;
		break;
	default:
		perr(HE_MSG,"input must be ints, shorts, or bytes");
	}
	ncols = hd.ocols;
	nrows = hd.orows;

	update_header(&hd,argc,argv);
	write_header(&hd);

	/* obtain the high and low values from the arguments */
	out_LOval = 0;
	out_HIval = 255;
	arg_vals[0][0] = in_LOval;
	arg_vals[0][1] = in_HIval;
	arg_vals[1][0] = out_LOval;
	arg_vals[1][1] = out_HIval;

	for (i=1; i<argc; i++) {
		if (argv[i][0] == '-') {
			switch (argv[i][1]) {
			case 'i':
				outflg = 0;
				arg_vals[outflg][0] = atoi(argv[++i]);
				arg_vals[outflg][1] = atoi(argv[++i]);
				break;
			case 'o':
				outflg = 1;
				arg_vals[outflg][0] = atoi(argv[++i]);
				arg_vals[outflg][1] = atoi(argv[++i]);
				break;
			case 's':
				scale = atof(argv[++i]);
				scl_flg++;
				break;
			}
		} else {
			arg_vals[outflg][0] = atoi(argv[i]);
			arg_vals[outflg][1] = atoi(argv[++i]);
			outflg++;
		}
	}
	in_LOval = arg_vals[0][0];
	in_HIval = arg_vals[0][1];
	out_LOval = arg_vals[1][0];
	out_HIval = arg_vals[1][1];

	if (!scl_flg) {
		scale = (float)(out_HIval - out_LOval) / 
			(float)(in_HIval - in_LOval);
		offset = in_LOval;
	} else
		offset = 0;
fprintf(stderr,"%s: scale = %f, offset = %d\n",argv[0],scale,offset);

	for (n=0; n<hd.num_frame; n++) {  /* repeat for each image */
		for (i=0; i<nrows; i++)  {
		
			if(fread(inbuf,ncols*bytes,1,stdin) != 1) {
				sprintf(tmp,"ERROR reading image %d",n);
				perr(HE_MSG,tmp);
			}

			op = outbuf;
			if (bytes == sizeof(char)) {
			  register unsigned char *cip = (unsigned char *)inbuf;
				register short tmp;
				for (j=0; j<ncols; j++,cip++,op++) {
					if (*cip < in_LOval) 
						*op = out_LOval;
					else  if (*cip > in_HIval ) 
						*op = out_HIval;
					else {
						tmp = (*cip - offset) * scale;
				*op = (tmp > out_HIval) ? out_HIval : tmp;
					}
				}
	        	} else if (bytes == sizeof(short)) {
				register short *sip = (short *)inbuf;
				register short tmp;
				for (j=0; j<ncols; j++,sip++,op++) {
					if (*sip < in_LOval) 
						*op = out_LOval;
					else  if (*sip > in_HIval ) 
						*op = out_HIval;
					else {
						tmp = (*sip - offset) * scale;
				*op = (tmp > out_HIval) ? out_HIval : tmp;
					}
				}
	        	} else if (bytes == sizeof(int)) {
				register int *iip = inbuf;
				register int tmp;
				for (j=0; j<ncols; j++,iip++,op++) {
					if (*iip < in_LOval) 
						*op = out_LOval;
					else  if (*iip > in_HIval ) 
						*op = out_HIval;
					else {
						tmp = (*iip - offset) * scale;
				*op = (tmp > out_HIval) ? out_HIval : tmp;
					}
				}
	        	}

			if (fwrite(outbuf,ncols,1,stdout) != 1)
				perr(HE_MSG,"error writing image");
		} /* end of row for loop */	
	} /* end of image for loop */
}
