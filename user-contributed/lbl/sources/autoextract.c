/* This program generates a script for extracting a number of images
 * from a larger hips image.  The input image is specified on the
 * command line by using the `-f' flag: "-f infile."  
 * 
 * Image extraction size may be specified with the -s option
 * or the -p option.  The -s option allows the user to specify
 * the size, in pixels, of the extracted images.  Its format is
 * "-s x y," where `x' is the number of pixels in the horizontal 
 * dimension, and `y' is the number of pixels in the vertical dim-
 * ension.
 *
 * The -p option allows the user to break the input image into
 * a specified number of panels.  Its format is "-p r c," where
 * `r' and `c' specify the number of rows and columns to break
 * the image into.
 *
 * If the -q option is selected, then a line is added to the 
 * script to generate a sequence of the smaller images.  This file
 * will be named using the convention "infile.seq."
 *
 * The output script is written to the standard output.
 *
 * Regardless of input specifications, the output images
 * created by executing the script will be named with the
 * following name convention:
 * 	"infile.row.col,"
 * where row and col represent the location of the sub-image
 * on the original image.  By HIPS convention, the upper left 
 * represents a row and col of zero.
 *
 *
 * Fritz Renema   
 * Advanced Development Projects Group 
 * Lawrence Berkeley Laboratory    
 */
#include <stdio.h>
#include <fcntl.h>
#include <hipl_format.h>
#define	ROUND_UP(x)  ((x-(int)x)>0) ? (1+(int)x):x

void usage();

int main (argc, argv)
	int	argc;
	char	*argv[];
{
	char	*infl;		/* input image file	*/
	char	otfl[50];	/* output file name	*/
	char	**ar=argv;	/* pointer to argument array	*/
	register int	i, j;	/* loop counters		*/
	int	x, y, r, c;	/* input parameters		*/
	int	xplc, yplc;	/* current x, y place holders	*/
	FILE	*fp;		/* file descriptor		*/
	enum 	flag	{enable = 1, disable = 0}; 
	enum flag	fl, size, panel, sequence;/* input options 	*/
	struct header	hd;		/* hips header structure	*/

	Progname = strsave(*argv);
	ar++; 	/* skip first argument, which is the program name */

	/* must at least specify input file. */
	if (argc < 2)
		usage (argv[0]);
	
	/* initialize flags */
	fl = size = panel = sequence = disable;

	/* parse command line */
	while (*ar != '\0')
	{
		if (*(*ar)++ != '-')
			usage (argv[0]);

		switch (**ar++)
		{
			case 'f':
			  fl = enable;
			  infl = *ar++;
			  break;
			
			case 's':
			  size = enable;
			  x = atoi (*ar++);
			  y = atoi (*ar++);
			  break;

			case 'p':
			  panel = enable;
			  r = atoi (*ar++);
			  c = atoi (*ar++);
			  break;

			case 'q':
			  sequence = enable;
			  break;

			default:
			  usage (argv[0]);
			  break;
		}
	} /* while */


	/* make sure command line parameters were appropriate */
	if (!fl || (size && panel) || (!size && !panel))
		usage (argv[0]);

	/* open input image file, read header (need image size) */
	if ((fp = fopen (infl, "r")) == NULL)
	{
		perror ("open");
		fprintf(stderr,"%s: unable to open \"%s\"\n", argv[0], infl);
		exit(-1);
	}
	fread_header (fp, &hd, infl);

	/* compute pixel dimensions if panel option selected, or 
	 * compute number of panel rows and columns */
	if (panel)
	{
		y = (int) ((float)hd.orows/(float)r);
		x = (int) ((float)hd.ocols/(float)c);
	}
	else
	{
		r = ROUND_UP((float)((float)hd.orows/(float)y));
		c = ROUND_UP((float)((float)hd.ocols/(float)x));
	}

	/* sanity check */
	if ((y > hd.orows) || (x > hd.ocols))
		fprintf (stderr, "Warning: input image size is smaller than\
 selected extraction size.\n");

	/* output shell script */
	printf ("#csh\n");
	printf ("set nohup\n");
	for (i = yplc = xplc = 0; i < r; i++, xplc = 0, yplc += y)
	  for (j = 0; j < c; j++, xplc += x)
	  {
		sprintf (otfl, "%s.%d.%d", infl, i, j);
		printf ("extract -s %d %d -p %d %d <%s >%s\n", y, x,
			 yplc, xplc, infl, otfl);
	  }
	if (sequence)
	{
	  printf ("# put the frames into a sequence\n");
	  printf ("catframes ");
	  for (i = 0; i < r; i++)
	    for (j = 0; j < c; j++)
		  printf ("%s.%d.%d ", infl, i, j);
	  printf (">%s.seq\n", infl);
	}

	/* close input image file */
	fclose (fp);

	return(0);
}


/* display proper usage message and abort 
 */
void	usage (cmd)
	char *cmd;
{
	fprintf(stderr,"Usage: %s <-f file> <-s x y OR -p r c> [-q]\n", cmd);
	exit(-1);
}

