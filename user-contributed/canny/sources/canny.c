/*	Copyright (c) 1989 Joe Miller Michigan State University
	Department of Computer Science
	East Lansing Michigan 48824

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   (Original 'hips' disclaimer shell)  See the
README file for more disclaimer information.

	canny.c			Joseph M. Miller	Feb. 8,1989

	This is an implementation of the canny edge detector.  It is
    based on the style and format of the dog.c hips filtering program.
    The idea was to make the final product easily integrated into the
    hips package.

	Currently this is only the shell - the Makefile specifies all
   the other modules by name.  Lots of fun items.

*/

#define MAX_MASK 256		/* Mask Mask size - 1D */
/*		DEFINE the DEFAULT values for input parameters... 
		that are used if those parameters AREN'T SPECIFIED */
#define DEFAULT_SIGMA 1.0
#define DEFAULT_MASKSIZE 9
#define DEFAULT_LFRAC 0.5
#define DEFAULT_HFRAC 0.9


/*
 * canny.c - filter an image by applying Dr. John Francis Canny's edge
 *	and line finding algorithm as described in the paper "A
 *	Computational Approach to Edge Detection" in PAMI-8 No. 6
 *	November 1986.  His MIT technical report # 720 was used
 *	as well for a reference.
 *
 *	    The input is in byte or float format, and the output is in 
 *		floating point or in integer format.
 *
 * usage:  canny [sigma [windowsize [lfrac]]] [ -pn ] [-i [-c]] [ <input_seq ] >out_seq 
 *
 *	windowsize [9] is the size of the mask (an integer).  MUST be 9
 *		FOR THIS TEST SET-UP
 *
 *	-i implies output in PFINT format.
 *	-c if -i is specified, causes checking of input to be in the
 *	 range [-1000 to 1000].
 *	-m output the Gaussian linear arrays only, do not convolve.
 
 *	If input file is  not redirected, the filtering is done on an impulse
 *		response in a 128X128 frame.
 *
 * to load:	cc -O -o canny canny.c -lm -lhipl
 *    note:	The efficiency of convolution depends heavily on optimization by
 *		the compiler, hence option -O.
 *
 *
 *		REVISIONS:  (JMM Apr. 1989)
 *	- adding a missing else in gauss.c (correlate_image routine).
 *	  this fixed the improper NON-extend around behavior of the 
 *	  extend option.
 *
 *	- switched improper order of z1 and z2 from (z1 -z2) to (z2-z1)
 *	  in a dot product calculation in gauss.c (nms routine).  It
 *	  caused several lower right hand corner parts to be dropped
 *	  until it was fixed.
 *
 *	- modified the final EDGE check in gauss.c (nms routine again)
 *	  so that rather than not finding a maximum pixel in cases of
 *	  ties - it chooses the 'right hand' pixel all the time.
 *
 *	- modified canny_top_level.c so that after the non_max_supp
 *	  (non maximal suppresion routine) the nms array IS NOT saved
 *	  since currently we don't print it out - it really isn't
 *	  needed.
 *
 *	- ~May 23, 1989.  Canny result is sent through thin() in thin.c
 *	  to create 8 connected lines (not 4 connected as before) by
 *	  default.  To override this default the user must type the "-e"
 *	  option.  The thin.c code includes the header information that
 *	  used to be in its header.h file and was hacked by Deb Trytten
 *	  in ways lightly documented.  Do a diff between this thin.c and the
 *	  original hips thin.c to get an idea of what she did.
 *
 *	- November 1989
 *	Commented out some potentially error causing lines in the routine
 *	non_max_supp() in canny_top_level.c  (they caused
 *	a divid by zero and didn't appear to serve any useful function.
 *	The 'BUG' was reported to me (Joe) by Ramin Samadani (EE Dept Stanford)
 *       ramin@scotty.Standfor.edu...
 *
 */

#include <fcntl.h>
#include <hipl_format.h>
#include <stdio.h>
#include <math.h>
#include "canny_top_level.h"

void canny(),thin();

int main(argc,argv)

int argc;
char **argv;
{
	/*  Files are used for multiple output filenames i.e. the -m
		-x and -y options... */
	FILE 	*fopen();   /* holds file pointers */
	struct 	header 	hd,hdorig;
	int	argcc,synthetic,informat,rows,cols,nrc,fr;
	int	i,fsize;
	FILE	*output_file;

		/* Flags for which arrays to save */
	int	magnitude_save, x_save, y_save, d_save;	
		/* Flag for whether or not 8 connected edge thinning
			should be performed */
	int	edge_thinning;
	char	mag_filename[80],x_filename[80],d_filename[80],y_filename[80];
	char	*binpic,*bp;
	float	*inpic,*conv1,*opic,*pinpic;
	int	*intinpic,*intpinpic;
	double	atof(),gauss_mask();
	float  	*emask,*imask;
	int	*intemask,*intimask;
    short *shortim,*temp;   /* scratch space */
/* Output Parameters */
    unsigned char *edgemap;  /* An array of char with 255 values at the edgepoints and 0 elsewhere */
    
/* Input/Output parameters */
    short *gx;              /* Pointer to array of short.  If not nil, will be used to compute x-gradient */
    short *gy;              /* Same as gx except its the y gradient.  If the pointer is nil, canny routine will allocate */
                            /* temporary storage for these routines. */
    short *mag;             /* Same as gx and gy except for its for the magnitude image. */
    int  *temp_out;	/* Used to write out gx, gy, or mag arrays */
    float  *temp_float;/* Used to write out Direction array
					which is a combination of gx and gy */
    unsigned char  *nms;    /* Same as above except it is a pointer to an array of unsigned char. The result of non-maximal */
                            /* supression applied to the magnitude image is placed here, if not nil. */


/* INPUT PARAMETERS FOR canny CALL... */
    int xsize;             /* The x-dimension of the image */
    int ysize;             /* The y-dimension of the image */
    int windowsize;        /* The size of the window to be used in the filtering MUST BE ODD!  */
    float gm[MAX_MASK],gmp[MAX_MASK];         /* arrays to hold masks */
    double sigma;          /* The sigma of the gaussian in pixels. g(x) = (1/(sqrt(two_PI)*sigma))*exp(x*x/(2*sigma*sigma) */ 
                            /* RANGE -- (0.0,somebignumber) */
    int bordermode;        /* Specifies how the filtering routine treats th borders.   */
                            /* The options for bordermode are 0: Use zero outside image bounds,,  */
                            /* 1: WRAP, 3: Extends the boudary pixels, */
                            /* 4: Masks the invalid boundary region to zero.*/ 
    double hfrac;           /* Chooses the upper threshold in hysteretic thresholding to cause the  seed pixels to be chosen from */
                            /* the upper (hfrac*100) percent of the pixels in the magnitiude (edginess) image */
                            /* RANGE -- [0.0,1.0] */
    double lfrac;           /* Mutiplies the upper threshold to get the lower hysteretic threshold */
                            /* RANGE -- [0.0, 1.0] */
    int pflag;              /* 1 -- print progress. 0 -- suppress printing of progress. */
	/* DEBUG - lastint variable */
    int lastint;	    /* Last integer passed to make sure args match */
    

    int  magmax;           /* storage  for the maximum gradient magnitude. */
    int  hist[257];              /* An array of 256 integers used for historgramming which will contain a histogram of the mag image. */
    int  histsize;          /* Size of the histogram. */
    int hthresh, lthresh; /* If not nil, the values of the actual thresholds used in the hysteretic thresholding will be returned. */


	Progname = strsave(*argv);
	if (argv[argc-1][0]=='-' && argv[argc-1][1]=='D') argc--;
	argcc=argc;
	/*	pflag=0 means NO debug (progression) print out
		pflag=1 means YES give me the good ol debug
			/progress output */
	pflag=0;
	/*	By default we want 8 connected edge thinning to be done
		Joe Miller May 23, 1989 */
	edge_thinning=1;

	while (argv[argcc-1][0]=='-') {
		switch(argv[argcc-1][1]) {
/*
	INSTEAD p = print flag...
*/
		case 'p':	pflag=1;
				break;
/*
	Edge thinning flag...  if   "-e" is present then NO 8-connected
   edge thinning is done.  (default is 8 edge connected...) */
		case 'e':	edge_thinning=0;
				/*  =0 means DON't do the edge thinning
				    =1 means DO do 8 connected edge thinning */
				break;
	
		case 'h':	/* The user would like 'HELP' on the format */
fprintf(stderr,"\n   canny  VERSION 3.0: The Canny edge detector (Last mod 4/25/89 - JMM)\n");
fprintf(stderr,"   --------------------------------------------------------------------\n");
fprintf(stderr,"\n	Call the canny program as follows (hips format):\n\n");
fprintf(stderr,"canny [SIGMA [MASKSIZE [LFRAC [HFRAC]]]] [-OPTIONS] <INFILENAME >OUTFILENAME\n");
fprintf(stderr,"\n	Where the arguments are:\n\n");
fprintf(stderr,"SIGMA - Smaller sigma's generally find higher frequency edges (default=%3.1f) \n",(float) DEFAULT_SIGMA);
fprintf(stderr,"MASKSIZE - Size of 1D gausian masks used (MUST be odd - default=%2d)\n",DEFAULT_MASKSIZE);
fprintf(stderr,"LFRAC - Low threshold value (edge pixels > this fraction - default=%3.1f)\n",DEFAULT_LFRAC);
fprintf(stderr,"HTHRESH - Each edge must have a pixel > this fractional value (default=%3.1f)\n",DEFAULT_HFRAC);
fprintf(stderr,"\n	and the OPTIONS are:\n");
fprintf(stderr,"\n   -p	: causes the program to print its parameter values and its progress.\n");
fprintf(stderr,"\n   -e	: means DON'T do 8 connected edge thinning.  (done by default)\n");
fprintf(stderr,"   -d, -m, -x, -y	: request for additional output data from internal\n");
fprintf(stderr,"information regarding the DIRECTION (-180 to 180), MAGNITUDE, X gradient,\n");
fprintf(stderr,"and Y gradients respectively.  See the canny manual page for more information.\n");
fprintf(stderr,"   -h	: causes this help message to be printed\n\n");
				exit(0);
				break;
		case 'm':	magnitude_save=1; 
				strcpy(mag_filename,argv[argcc-1]+2);
if (pflag==1)
fprintf(stderr," m  option to save magnitude to file %s\n",mag_filename);
				break;
		case 'x':	x_save=1; 
				strcpy(x_filename,argv[argcc-1]+2);
if (pflag==1)
fprintf(stderr," x  option to save X gradient to file %s\n",x_filename);
				break;
		case 'y':	y_save=1; 
				strcpy(y_filename,argv[argcc-1]+2);
if (pflag==1)
fprintf(stderr," y  option to save Y gradient to file %s\n",y_filename);
				break;
		case 'd':	d_save=1; 
				strcpy(d_filename,argv[argcc-1]+2);
if (pflag==1)
fprintf(stderr," d  option to save EDGE DIRECTION to file %s\n",d_filename);
				break;
		default:	perr(HE_MSG,"unrecognized option");
				break;
		}
		argcc--;
	}

	/* Define the default input paraments now (in order
	  of their expected appearance  - JMM */

	/* Sigma and window size defaults */
	sigma=DEFAULT_SIGMA;
	windowsize=DEFAULT_MASKSIZE;

	/*	Low and high fraction default values */
	lfrac=DEFAULT_LFRAC;
	hfrac=DEFAULT_HFRAC;

	if (argcc>1) sigma=atof(argv[1]);
	if (argcc>2) windowsize=atoi(argv[2]);
	if (argcc>3) lfrac=atof(argv[3]);
	if (argcc>4) hfrac=atof(argv[4]);


	/*  If no input file is given - create a default one... - JMM */

	if (system("test -t 0")==0) {	/* no input file */
			/* Make fsize the biggest power of 2 <=windowsize - JMM */
		fsize=1; while(fsize<=windowsize)fsize *= 2;
		fprintf(stderr,"canny: output frame size = %dX%d.\n", fsize,fsize);
		init_header(&hd,"","CANNY mask",1,"",fsize,fsize,PFFLOAT,1,"");
		synthetic=1;
	}
	else {
		read_header(&hd);
		if (hd.pixel_format != PFBYTE && hd.pixel_format !=PFFLOAT) 
			perr(HE_MSG,"canny: image pixel format must be bytes or floats - not PFFLOAT");
		synthetic=0;
	}
	informat=hd.pixel_format;
	rows=hd.orows; cols=hd.ocols;
	/* Calculate the number of data elements - JMM */
	nrc=rows*cols;

	if (informat==PFBYTE)
		binpic = (char * ) halloc(nrc,sizeof (char));
	inpic = (float *) halloc(nrc,sizeof (float));
	conv1 = (float *) halloc(nrc,sizeof(float));
	opic = (float *) halloc(nrc,sizeof(float));
	emask = (float *) halloc(windowsize,sizeof(float));
	imask = (float *) halloc(windowsize,sizeof(float));
	gx = (short *) malloc(nrc*sizeof(short));
	gy = (short *) malloc(nrc*sizeof(short));
	shortim = (short *) malloc(nrc*sizeof(short));
	temp = (short *) malloc(nrc*sizeof(short));
	mag = (short *) malloc(nrc*sizeof(short));
	edgemap = (unsigned char *) malloc(nrc*sizeof(char));
	nms = (unsigned char *) malloc(nrc*sizeof(char));
	temp_out = (int *) malloc(nrc*sizeof(int));

	intinpic=(int *)inpic;
	intemask = (int *) halloc(windowsize,sizeof(int));
	intimask = (int *) halloc(windowsize,sizeof(int));
	setformat(&hd,PFBYTE);

	/* Add the current operations to the saved header... - JMM */

	update_header(&hd,argc,argv);
	write_header(&hd);

	for(i=0;i<windowsize;i++) {
		intemask[i]=emask[i]*1000+.5;
		intimask[i]=imask[i]*1000+.5;
	}

	for(fr=0;fr<hd.num_frame;fr++) {
		if (hd.num_frame>1)
			fprintf(stderr,"canny: starting frame #%d\n",fr);
		/*	If synthetic data is to be used - JMM */
		if (synthetic) {
			for(i=0;i<nrc;i++)
				intinpic[i]=0;
			intinpic[(rows/2)*cols+cols/2]=1;
		}
		else {
		/*	If NO synthetic data is to be used READ IT IN - JMM */
			if (informat==PFBYTE) {
				if (fread(binpic,nrc,1,stdin)!=1)
					perr(HE_MSG,"unexpected EOF");
				for(i=0,bp=binpic,intpinpic=intinpic;i<nrc;i++)
					*intpinpic++=(*bp++)&0377;
			}
			else {
				if (fread(intinpic,nrc*sizeof(int),1,stdin)!=1)
					perr(HE_MSG,"unexpected EOF");
				for(i=0,intpinpic=intinpic,pinpic=inpic;i<nrc;i++)
					*intpinpic++ = *pinpic++;
						
			}
		}
		/* IF CHECKMODE is set then we want to count the number
			of under and overflows.		- JMM */

	/* Set up all the values... */

	/* INPUT parameters */
	xsize=cols;
	ysize=rows;
	bordermode=1;	/* Use 1=WRAP 0 = outer zeros, 3 extend bounries*/
			/* 4: masks the invalid boundary to 0's */

	/* OUTPUT parameters */

	/* INPUT / OUTPUT parameters */
	/* gx and gy... - don't supply gradient masks... */
	*gx=0;
	*gy=0; 
	magmax=1;
	histsize=256;	/* size of the histogram array */
	hthresh=0;
	lthresh=0;
	lastint=32767;

   canny(&magmax,&hthresh, &lthresh ,intinpic,&xsize,&ysize,shortim,&windowsize,
   &sigma, &bordermode,&hfrac,&lfrac,&pflag,gx,gy,mag,hist,&histsize,nms,edgemap,gm,gmp,temp,&lastint);

	/*	PERFORM EDGE THINNING TO GET 8 connected THIN EDGES */
	/*	Added at Deb Trytten's request by Joe Miller May 23, 89 */
	if (edge_thinning == 1)
		{
		thin(edgemap,ysize,xsize);
		} /* End of edge_thinning == 1 for 8 connected edges only */


	/* Write out the resulting edge image... */

fwrite(edgemap,nrc*sizeof(char),1,stdout);


		/*	IFF THE MAGNITUDE FLAG IS SET - SAVE IT !! */
		if (magnitude_save == 1)
				{
			if (pflag == 1)
				fprintf(stderr,"MAG save started...\n");
	/*  Open the file with OWNER rwx (why this works I don't know)
		UNIX documentation is for the S#@$(*%Y@ and I tried all
		kinds of things before this worked... */
if ((output_file=fopen(mag_filename,"a")) == NULL)
	{
	/* output file as specified couldn't be opened */
	fprintf(stderr,"Error - the output filename :%s:(-m argument) \n",mag_filename);
	fprintf(stderr,"couldn't be opened for write ! - Check file protection.\n\n");
	exit(-1);
	}
	/* Add the current operations to the header to be saved,
	first duplicate the hd header, then update its header
	and place the string "MAGNITUDE IMAGE IN IT"... - JMM */
				if (fr == 0) {
					dup_headern(&hd,&hdorig);
					desc_append(&hd,"\"MAGNITUDE IMAGE\"");
					setformat(&hdorig,PFINT);

					/*	Write out the MAGNITUDE header */
					fwrite_header(output_file,&hdorig,
						mag_filename);
				}

				/*	Copy the mag array for output */
				/* i.e. change from short to int size */
				for (i=0;i<nrc;i++)
					temp_out[i]=(int) mag[i];
				fwrite(temp_out,nrc*sizeof(int),1,output_file);
				fclose(output_file);
				}

		/*	IFF THE X FLAG IS SET - SAVE THE X array !! */
		if (x_save == 1)
				{
			if (pflag == 1)
				fprintf(stderr,"X save started...\n");
	/*  Open the file with OWNER rwx (why this works I don't know)
		UNIX documentation is for the S#@$(*%Y@ and I tried all
		kinds of things before this worked... */
if ((output_file=fopen(x_filename,"a")) == NULL)
	{
	/* output file as specified couldn't be opened */
	fprintf(stderr,"Error - the output filename :%s:(-m argument) \n",x_filename);
	fprintf(stderr,"couldn't be opened for write ! - Check current directory protection.\n\n");
	exit(-1);
	}
	if (fr == 0) {
		dup_headern(&hd,&hdorig);
		desc_append(&hd,"\"X GRADIENTS\"");
		setformat(&hdorig,PFINT);

		/*	Write out the X GRADIENTS header */
		fwrite_header(output_file,&hdorig,
			x_filename);
	}

				for (i=0;i<nrc;i++)
					temp_out[i]=(int) gx[i];
				fwrite(temp_out,nrc*sizeof(int),1,output_file);
				fclose(output_file);
				}

		/*	IFF THE Y FLAG IS SET - SAVE THE GY array !! */
		if (y_save == 1)
				{
			if (pflag == 1)
				fprintf(stderr,"Y save started...\n");
	/*  Open the file with OWNER rwx (why this works I don't know)
		UNIX documentation is for the S#@$(*%Y@ and I tried all
		kinds of things before this worked... */
if ((output_file=fopen(y_filename,"a")) == NULL)
	{
	/* output file as specified couldn't be opened */
	fprintf(stderr,"Error - the output filename :%s:(-m argument) \n",y_filename);
	fprintf(stderr,"couldn't be opened for write ! - Check current directory protection.\n\n");
	exit(-1);
	}
	if (fr == 0) {
		dup_headern(&hd,&hdorig);
		desc_append(&hd,"\"Y GRADIENTS\"");
		setformat(&hdorig,PFINT);

		/*	Write out the Y GRADIENTS header */
		fwrite_header(output_file,&hdorig,y_filename);
	}
				for (i=0;i<nrc;i++)
					temp_out[i]=(int) gy[i];
				fwrite(temp_out,nrc*sizeof(int),1,output_file);
				fclose(output_file);
				}
		/*	IFF THE d FLAG IS SET - SAVE THE DIRECTION array !! */
		if (d_save == 1)
				{
			if (pflag == 1)
				fprintf(stderr,"DIRECTION save started...\n");
	/*  Open the file with OWNER rwx (why this works I don't know)
		UNIX documentation is for the S#@$(*%Y@ and I tried all
		kinds of things before this worked... */
	if (fr == 0) {
		dup_headern(&hd,&hdorig);
		desc_append(&hd,"\"Y GRADIENTS\"");
		setformat(&hdorig,PFINT);

		/*	Write out the Y GRADIENTS header */
		fwrite_header(output_file,&hdorig,y_filename);
	}
if ((output_file=fopen(d_filename,"a")) == NULL)
	{
	/* output file as specified couldn't be opened */
	fprintf(stderr,"Error - the output filename :%s:(-m argument) \n",d_filename);
	fprintf(stderr,"couldn't be opened for write ! - Check current directory protection.\n\n");
	exit(-1);
	}
	if (fr == 0) {
		dup_headern(&hd,&hdorig);
		desc_append(&hd,"\"DIRECTION\"");
		setformat(&hdorig,PFFLOAT);

		/*	Write out the Y GRADIENTS header */
		fwrite_header(output_file,&hdorig,d_filename);
	}

				/* Calculate the direction of the edge
				   using the formula:
					DIR=atan2(gy,gx)*180.0/PI;
				   to get the value between -180.0 and 180.0
				*/
					
		temp_float = (float *) malloc(nrc*sizeof(float));

				for (i=0;i<nrc;i++)
					{
					if (gx[i]==0 || gy[i]==0)
						temp_float[i]=0.0;
					else
						temp_float[i]=(float) atan2((double) gy[i],(double) gx[i])* 57.2974;
					}
				fwrite(temp_float,nrc*sizeof(float),1,
					output_file);
				fclose(output_file);
				}
	}
	exit(0);

}
