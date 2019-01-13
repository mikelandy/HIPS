/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * mask.c - filter an image by applying one or more masks and then
 *          applying another function to the various mask outputs.
 *
 * usage:	mask [-f filter-number | -m filter-file] <iseq >oseq
 *
 * where filter-number specifies:
 *
 *	filter number   mask type			function
 *		1	3-level (3x3)			maxabs
 *		2	3-level (5x5)			maxabs
 *		3	3-level (7x7)			maxabs
 *		4	3-level (9x9)			maxabs
 *		5	5-level (3x3)			maxabs
 *		6	Prewitt (3x3)			mean-square
 *		7	Prewitt (3x3)			sumabs
 *		8	Prewitt (5x5)			mean-square
 *		9	Prewitt (5x5)			sumabs
 *		10	Prewitt (7x7)			mean-square
 *		11	Prewitt	(7x7)			sumabs
 *		12	Prewitt (9x9)			mean-square
 *		13	Prewitt (9x9)			sumabs
 *		14	Roberts (2x2)			mean-square
 *		15	Roberts (2x2)			sumabs
 *		16	Sobel (3x3)			mean-square
 *		17	Sobel (3x3)			sumabs
 *		18	compass (3x3)			max
 *		19	Kirsch (3x3)			max
 *		20	pyramid (5x5)			maxabs
 *		21	pyramid (7x7)			maxabs
 *		22	pyramid (9x9)			maxabs
 *		23	4x4				mean-square
 *		24	Laplacian - a			Identity
 *		25	Laplacian - b			Identity
 *		26	Laplacian - c			Identity
 *		27	Laplacian - d			Identity
 *		28	Kasvand-Lapl			Identity
 *		29	Kasvand-line			max-floor
 *		30	Eberlein			max-abs-sub-floor
 *		31	Extended Laplacian-c (5x5)	Identity
 *		32	Two channel Laplacian		Product of Floors
 *		100	4x4 smoother			normalize
 *		101	5x5 smoother			normalize
 *		102	4x4 smoother			normalize
 *		103	3x3 smoother			normalize
 *		104	2x2 smoother			normalize
 *		105	4x4 smoother			normalize
 *		106	4x4 normalizer	image	 	normalized mask output
 *		107	5x5 normalizer	image 		normalized mask output
 *		161	Sobel (3x3)			orientation
 *		165	Sobel (5x5)			orientation
 *		200	1x2 x differentiation		identity
 *		201	2x1 y differentiation		identity
 *		202	1x3 x 2nd differentiation	identity
 *		203	3x1 y 2nd differentiation	identity
 *		261	Extended Laplacian-c 5x5	Identity
 *		...	etc. (see masks directory)
 *
 * The default filter is 1.  The definition for each of these filters
 * is to be found in $(MASKDIR)/maskn, where n is the filter number and
 * $(MASKDIR) is set at HIPS installation time.
 *
 * The -m switch allows a new filter to be supplied by the user.
 * A convolution mask set consists of 1 or more masks
 * (arrays of floats or ints), each with associated size and row and column
 * offsets (to denote which mask pixel is centered on the image pixel before
 * cross-correlating (***important note***, h_mask cross-correlates with the
 * masks rather than convolves; in other words, the rows and columns are not
 * first reflected;  for the usual mirror symmetric masks this poses no
 * complications).
 *
 * The format of the filter definition file is as follows:
 *
 *	"filter name"
 *	number-of-masks function-number mask-format
 *	
 *	mask-1-rows mask-1-cols mask-1-rowoffset mask-1-coloffset
 *	mask-1-values
 *	  .
 *	  .
 *	  .
 *	mask-(number-of-masks)-rows cols rowoffset coloffset
 *	mask-(number-of-masks)-values
 *
 * mask-format is either 2 (i.e. PFINT, for integer mask values) or 3 (i.e.
 * PFFLOAT, for floating point mask values).
 * mask-rows and mask-cols give the side lengths of the rectangular mask.
 * mask-rowoffset and mask-coloffset identify the pixel which overlaps a given
 * image position to produce the mask value corresponding to that image
 * position.  Note that the earlier mask values are applied to earlier image
 * values.  Thus, the interpretation of the mask orientation depends on the
 * definition of ULORIG (the HIPS installation flag which specifies whether
 * images have an upper-left or lower-left origin).  If ULORIG is defined
 * (images have their origin at
 * the upper-left), then the first mask row is topmost relative to the image.
 * Otherwise, the last mask row is topmost relative to the image.  Mask
 * values are given as a sequence of integers or floats in column-fastest
 * order.
 *
 * Mask sets can include 1 or more masks.  The mask program applies a
 * function to the set of mask output values which results in the single
 * pixel value placed in a given position in the output image.  The second
 * line of the mask definition identifies which function is to be used,
 * chosen from:
 *
 *	1	MASKFUN_MAXABS	- the maximum absolute value of all mask outputs
 *	2	MASKFUN_MEANSQ  - the square root of the sum of the squares of
 *				  all masks
 *	3	MASKFUN_SUMABS  - the sum of the absolute value of all mask
 *				  outputs
 *	4	MASKFUN_MAX	- the maximum mask output
 *	5	MASKFUN_MAXFLR	- the maximum mask output, floored at zero
 *	6	MASKFUN_MXASFLR	- the larger of |mask-1| and |mask-2|, minus
 *				  |mask-3|, floored at zero
 *	7	MASKFUN_MUL	- the product of the mask outputs, each floored
 *				  at zero
 *	8	MASKFUN_NORM	- the first mask output normalized by the sum
 *				  of the mask entries
 *	9	MASKFUN_DIFF	- the value of the pixel minus the normalized
 *				  mask output
 *	10	MASKFUN_ORIENT	- compute orientation:
 *				  360*atan(mask1/mask2)/2*PI
 *	11	MASKFUN_IDENT	- the value of the first mask output (simple
 *				  convolution, well... cross-correlation)
 *
 * pixel formats handled directly: BYTE, INT, FLOAT
 * output pixel format: FLOAT
 *
 * to load: cc -o mask mask.c -lhipsh -lhips -lm
 *
 * Michael Landy - 4/21/82
 * Hips 2 - msl - 7/13/91
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {
	{"f",{"m",LASTFLAG},1,{{PTINT,"1","filter-number"},LASTPARAMETER}},
	{"m",{"f",LASTFLAG},1,{{PTBOOLEAN,"FALSE"},
		{PTFILENAME,"","filter-file"},LASTPARAMETER}},
	LASTFLAG};
int types[] = {PFBYTE,PFINT,PFFLOAT,LASTTYPE};

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd,hdp,hdo;
	struct hips_mask mask;
	int method,fr,f,filterno;
	Filename filename,maskfile;
	FILE *fp;
	h_boolean mflag,imagecopy;
	struct hips_roi roi;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&filterno,&mflag,&maskfile,FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	method = fset_conversion(&hd,&hdp,types,filename);
	if (mflag)
		read_mask(&mask,maskfile);
	else
		read_num_mask(&mask,filterno);
	if (mask.pixel_format == PFINT && hdp.pixel_format == PFFLOAT)
		mask_itof(&mask);
	imagecopy = FALSE;
	if (hdp.rows != hdp.orows || hdp.cols != hdp.ocols)
		imagecopy = TRUE;
	dup_headern(&hdp,&hdo);
	setformat(&hdo,PFFLOAT);
	alloc_image(&hdo);
	write_headeru(&hdo,argc,argv);
	getroi(&hdp,&roi);
	fr = hdp.num_frame;
	fprintf(stderr,"%s: applying filter:	%s\n",Progname,mask.name);
	for (f=0;f<fr;f++) {
		fprintf(stderr,"%s: starting frame #%d\n",Progname,f);
		fread_imagec(fp,&hd,&hdp,method,f,filename);
		if (imagecopy) {
			if (hdp.pixel_format == PFFLOAT) {
				clearroi(&hdp);
				clearroi(&hdo);
				h_copy(&hdp,&hdo);
				setroi2(&hdp,&roi);
				setroi2(&hdo,&roi);
			}
			else
				h_tof(&hdp,&hdo);
		}
		h_mask(&hdp,&mask,&hdo);
		write_image(&hdo,f);
	}
	return(0);
}
