/* mahe.c		Max Rible
 * Program that wraps around some useful adaptive histogram
 * routines and makes them useful for us at the graphics lab.
 */

#include <stdio.h>
#include <math.h>
#include <hipl_format.h>
#include "ahe.h"
/*#define DEBUG*/

#define Calloc(a,b) (b *) calloc((unsigned)(a), sizeof(b))
#define Fread(a,b,c,d) fread((char *)(a), b, (int)(c), d)
#define Fwrite(a,b,c,d) fwrite((char *)(a), b, (int)(c), d)
#define Cfree(x,y,z) cfree((char *)(x), (unsigned)(y), z)

float clip;
static int dim[3], nreg[3], minmax[2], imfmt;
static struct header hipshead;

main(argc, argv)
     int argc;
     char *argv[];
{
    FILE *input, *output;
    int i, j, imsize, err;
    unsigned int *intbuf ;
    unsigned char *inptr ;
    unsigned short *in_short_ptr ;
    unsigned char *in_char_image ;
    unsigned short *in_short_image ;
    unsigned char *outimage ;
    float scale_io ;

    int min_int = 2000000000 ;
    int max_int = 0 ;
    unsigned char min_char = 255 ;
    unsigned char max_char = 0 ;
    unsigned short min_short = 65535 ;
    unsigned short max_short = 0 ;
    int tmpint ;
    int num_frames ;
    int frame_index ;
    char *inname,*outname;

    Progname = strsave(*argv);
    input = stdin;
    output = stdout;
    inname = "<stdin>";
    outname = "<stdout>";
    dim[0] = 1; dim[1] = dim[2] = 512;
    nreg[0] = 1; nreg[1] = nreg[2] = 4;
    clip = 0.0;

    parse_args(argc, argv);

	fread_header(input, &hipshead,inname);
	imfmt = hipshead.pixel_format;
	dim[2] = hipshead.ocols;
	dim[1] = hipshead.orows;
	num_frames = hipshead.num_frame ;
	hipshead.pixel_format = PFBYTE;
	update_header(&hipshead, argc, argv);

    imsize = dim[2]*dim[1];

    if((outimage = Calloc(imsize, unsigned char)) == NULL)
	perror("outimage");
    fprintf (stderr, "mahe begun\n");

for (frame_index = 0 ; frame_index < num_frames ; ++frame_index)
{
    switch(imfmt) {
    case PFINT:
        if((in_short_image = Calloc(imsize, unsigned short)) == NULL)
	    perror("in_short_image");

	if((intbuf = Calloc(dim[2], unsigned int)) == NULL)
	    perror("int buffer");
	for(j = 0; j < dim[1]; j++)
	{
	    Fread(intbuf, sizeof(int), dim[2], input);
	    for(i = 0; i < dim[2]; i++)
	    {
		if (intbuf[i] > max_int)
		    max_int = intbuf[i] ;
		if (intbuf[i] < min_int)
		    min_int = intbuf[i] ;
	    }
	}
        scale_io = ((float) MAXSHORT/((float) (max_int - min_int)));
#ifdef DEBUG
	fprintf (stderr,"%d %d %f\n",min_int,max_int,scale_io) ;
#endif
	/* CHANGE! */
	for(j = 0; j < dim[1]; j++)
	{
	    Fread(intbuf, sizeof(int), dim[2], input);
	    for(i = 0; i < dim[2]; i++)
    	        in_short_image[j*dim[2] + i] = (unsigned short)((float)(in_short_image[j*dim[2] + i] - min_int)) * scale_io + 0.5;
	}
	Cfree(intbuf, dim[2], sizeof(unsigned char));
	break;
    case PFBYTE:
        if((in_char_image = Calloc(imsize, unsigned char)) == NULL)
	    perror("in_char_image");

	if(Fread(in_char_image, sizeof(char), imsize, input) != imsize)
	    perror("read");
	inptr = in_char_image ;
	for (i = 0 ; i < imsize ; i++)
	{
	    if (*inptr > max_char)
	       max_char = *inptr ;
	    if (*inptr < min_char)
	       min_char = *inptr ;
	    ++inptr ;
	}
        scale_io = ((float) MAXCHAR/((float) (max_char - min_char)));
	inptr = in_char_image ;
	for(i = 0; i < imsize ; i++)
	{
    	        tmpint = (int) (((float)(*inptr - min_char)) * scale_io + 0.5);
		*inptr++ = (unsigned char) tmpint ;
	}
        minmax[0] = 0 ;
        minmax[1] = 255 ;
	break;
    case PFSHORT:
        if((in_short_image = Calloc(imsize, unsigned short)) == NULL)
	    perror("in_short_image");

	if(Fread(in_short_image, sizeof(short), imsize, input) != imsize)
	    perror("read");
	in_short_ptr = in_short_image ;
	for (i = 0 ; i < imsize ; i++)
	{
	    if (*in_short_ptr > max_short)
	       max_short = *in_short_ptr ;
	    if (*in_short_ptr < min_short)
	       min_short = *in_short_ptr ;
	    ++in_short_ptr ;
	}
        scale_io = ((float) MAXSHORT/((float) (max_short - min_short)));
#ifdef DEBUG
	fprintf (stderr,"%d %d %f\n",min_short,max_short,scale_io) ;
#endif
	in_short_ptr = (unsigned short *) in_short_image ;
	for(i = 0; i < imsize ; i++)
	{
    	        tmpint = (int) (((float)(*in_short_ptr - min_short)) * scale_io + 0.5);
		*in_short_ptr++ = (unsigned short) tmpint ;
	}
        minmax[0] = 0 ;
        minmax[1] = MAXSHORT ;
	break;
    case PFFLOAT:
    case PFCOMPLEX:
    default:
	fprintf(stderr, "unsupported file format\n");
	exit(-1);
    }

    if (imfmt == PFBYTE)
	ahecalc(in_char_image, outimage, dim, minmax, nreg, clip, argv);
    else
	ahecalc_short(in_short_image, outimage, dim, minmax, nreg, clip, argv);

    if (frame_index == 0)
        fwrite_header(output, &hipshead,outname);

    fwrite((char *)outimage, sizeof(char), imsize, output);
    if (imfmt == PFBYTE)
	free(in_char_image) ;
    else
	free(in_short_image) ;
    fprintf(stderr, "Done with frame %d of %d\n",frame_index+1,num_frames);
}    /* frame_index loop */
    free (outimage) ;
}

parse_args(argc, argv)
     int argc;
     char *argv[];
{
    int i;
    int rconflict = 0 ;

    for(i = 1; i < argc; i++)
	if(argv[i][0] == '-') switch(argv[i][1]) {
	case 'r':		/* number of regions */
	    if (rconflict)
	    {
		fprintf (stderr,"conficting options -r and -W\n") ;
		exit(0) ;
	    }
	    nreg[2] = atoi(argv[++i]);
	    nreg[1] = atoi(argv[++i]);
	    if ((nreg[2] < 2) || (nreg[1] < 2))
	    {
		fprintf (stderr,"number of regions must be at least 2\n") ;
		exit(0) ;
	    }
	    ++rconflict ;
	    break;
	case 'W':		/* region dimensions */
	    if (rconflict)
	    {
		fprintf (stderr,"conficting options -r and -W\n") ;
		exit(0) ;
	    }
	    nreg[2] = dim[2]/atoi(argv[++i]);
	    nreg[1] = dim[1]/atoi(argv[++i]);
	    if ((nreg[2] < 2) || (nreg[1] < 2))
	    {
		fprintf (stderr,"region dimensions must be at least 2\n") ;
		exit(0) ;
	    }
	    ++rconflict ;
	    break;
	case 'c':		/* clipping */
	    clip = (float) atof(argv[++i]);
	    if (clip < 1.0)
	    {
		fprintf (stderr,"clip must be greater than 1.0\n") ;
		exit(0) ;
	    }
#ifdef DEBUG
		fprintf (stderr, "mahe: clip %d\n", clip);
#endif
	    break;
	    /* following cases for backwards compatibility with
	       Max's original interface */
	case 'd':
	case 'm':
	    i += 2 ;
	    break ;
	case 'i':
	case 'o':
	case 'w':
	    i++ ;
	    break ;
	case 'H':
	case 'M':
	case 'f':
	case 'p':
	    break ;
	case 'h':
	default:
#ifdef LATER
puts("-m min max    Minima and maxima of data (integer).");
#endif
puts("-r x y        X, Y number of regions (integer), default 4, 4.");
puts("-W x y        X, Y dimensions of a region.");
puts("-c f          Clipping limit f (float), default 0.0.");
puts("-h            Help!");
	    exit(0);
	}
}
