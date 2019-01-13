/*****************************************************************************/
/*****************************************************************************/
/**                                                                         **/
/**                                RANK.C                                   **/
/**                                                                         **/
/*****************************************************************************/
/*****************************************************************************/
/*
**
**             Copyright (c) 1991 The Turing Institute
**
** Disclaimer:  No guarantees of performance accompany this software,
** nor is any responsibility assumed on the part of the authors.  All the
** software has been tested extensively and every effort has been made to
** insure its reliability.
**
** rank.c - rank order filter
**
** usage: rank [-s hor_size [ver_size]] 
**	       [ -r [hor_rank [ver_rank]] | -min | -max | med]
**             < inseq > outseq
**
** where: hor_size and ver_size specify the size of the horizontal and vertical
**	  filtering windows respectively. ver_size defaults to hor_size and
**	  hor_size defaults to 3. Specifying a size of 1 causes no filtering
**	  to be carried out in that dimension, so that 1D horizontal rank order
**	  filtering can be carried out by specifying a vertical size of 1 and
**	  1D horizontal filtering can be carried out by specifying a horizontal
**	  size of 1.
**
**	  hor_rank and ver_rank specify the rank to be used when filtering in
**        the horizontal and vertical directions respectively. hor_rank and 
**	  ver_rank default to the median rank. Instead of using the -r flag,
**	  -min, -max, or -med can be used to specify that the minimum, maximum,
**        or median rank respectivly should be used for filtering in both
**	  dimensions.
**
** to load: cc -o rank rank.c -lhipsh -lhips -lm
**
** Name: rank.c
**
** Author: Colin Urquhart, The Turing Institute, 22 September 1991 
**
** History: Modified from frank.c for HIPS2
**
** Purpose: to perform rank-order filtering on a sequence of HIPS2 images.
**
** Inputs: Frank accepts a standard HIPS2 sequence of images. 
**         It also accepts flags specifying window size, rank position
**         required, etc., as outlined above.
**
** Outputs: Frank outputs a standard HIPS sequence of images.
**
** Method: Frank performs rank-order filtering with a 1D mask, first in the
**         horizontal direction, and then  in the vertical direction. 
*/

#include <hipl_format.h>

static Flag_Format flag_format[] = 
{
    {"s",{LASTFLAG},1,{{PTINT,"3","hor_size"},
		       {PTINT,"-1","ver_size"},
		       LASTPARAMETER}},
    {"r",{"min","max","med",LASTFLAG},0,{{PTINT,"-1","hor_rank"},
					 {PTINT,"-1","ver_rank"},
					 LASTPARAMETER}},
    {"min", {"r","max","med",LASTFLAG},0,{{PTBOOLEAN, "FALSE"},
					  LASTPARAMETER}},
    {"max", {"r","min","min",LASTFLAG},0,{{PTBOOLEAN, "FALSE"},
					  LASTPARAMETER}},
    {"med", {"r","max","min",LASTFLAG},0,{{PTBOOLEAN, "FALSE"},
					  LASTPARAMETER}},
    LASTFLAG
};

int types[] = {PFBYTE,PFSHORT,PFINT,PFFLOAT,PFDOUBLE,LASTTYPE};
void check_params();

int main (argc,argv)

int  argc;   /* the number of command line arguements */
char **argv; /* the array of command line arguements */
{
    struct	header 	input_hdr;
    struct 	header	output_hdr;
    struct	header	process_hdr;
    int 		method;
    int			frame;
    Filename 		filename;
    FILE 		*file_ptr;

    int			hor_size;
    int			ver_size;
    int			hor_rank;
    int			ver_rank;
    h_boolean		min_flag;
    h_boolean		max_flag;
    h_boolean		med_flag;

    
    /* Save the program name for use with perr */
    Progname = strsave (*argv);

    /* Parse the arguments */
    parseargs (argc,
	       argv,
	       flag_format,
	       &hor_size,
	       &ver_size,
	       &hor_rank,
	       &ver_rank,
	       &min_flag,
	       &max_flag,
	       &med_flag,
	       FFONE,      
	       &filename);

    check_params (&hor_size,
		  &ver_size,
		  &hor_rank,
		  &ver_rank,
		  min_flag,
		  max_flag,
		  med_flag);

    /* Get the file pointer coping with filename = "<stdin>" */
    file_ptr = hfopenr (filename);
    
    /* Read the input file's header allocating core for one frame */
    fread_hdr_a (file_ptr,
		 &input_hdr,
		 filename);

    /* Find the nearest acceptable pixel format to the input pixel   *
     * format and set up a header with this format and allocate core *
     * for an image of this format.					 */
    method = fset_conversion (&input_hdr,
			      &process_hdr,
			      types,
			      filename);

    /* Set output_hdr = process_hdr with no duplication of image ptr */
    dup_headern (&process_hdr,
		 &output_hdr);
	    
    /* Allocate core for a frame in the output sequence */
    alloc_image (&output_hdr);

    /* Update the history in output_hdr and write it to stdout */
    write_headeru2 (&input_hdr,
		    &output_hdr,
		    argc,
		    argv,
		    hips_convback);

    for (frame = 0; frame < process_hdr.num_frame; frame++) 
    {
	/* Read in the frame from the file into input_header and convert *
	 * appropriately for process_hdr				 */
	fread_imagec (file_ptr,
		      &input_hdr,
		      &process_hdr,
		      method,
		      frame,
		      filename);

	h_rank (&process_hdr,
		&output_hdr,
		hor_size,
		ver_size,
		hor_rank,
		ver_rank);

	/* write the output image to stdout */
	write_imagec (&input_hdr,
		      &output_hdr,
		      method,
		      hips_convback,
		      frame);
    }	
    return(0);	
}

void check_params (hor_size,
	      ver_size,
	      hor_rank,
	      ver_rank,
	      min_flag,
	      max_flag,
	      med_flag)

int	*hor_size;
int	*ver_size;
int	*hor_rank;
int	*ver_rank;
h_boolean	min_flag;
h_boolean	max_flag;
h_boolean	med_flag;
{
    if (*ver_size == -1)
    {
	*ver_size = *hor_size;
    }
    
    if ((*hor_size < 1) || (*ver_size < 1))
    {
	perr (HE_MSG,
	      "filter sizes must be greater than 0!");
    }
	
    /* check that sizes are odd */

    if ((*hor_size % 2 == 0) || (*ver_size % 2 == 0))
    {
	perr (HE_MSG,
	      "filter sizes must be odd");
    }

    if (min_flag == TRUE)
    {
	*hor_rank = *ver_rank = 1;
    }
    else if (max_flag == TRUE)
    {
	*hor_rank = *hor_size;
	*ver_rank = *ver_size;
    }
    else
    {
	if ((med_flag == TRUE) || (*hor_rank == -1))
	{
	    *hor_rank = ((*hor_size) + 1) / 2;
	}
	if ((med_flag == TRUE) || (*ver_rank == -1))
	{
	    *ver_rank = ((*ver_size) + 1) / 2;
	}
    }
    /* check that sensible ranks have been specified */
    
    if (*hor_rank < 1 || 
	*hor_rank > *hor_size || 
	*ver_rank < 1 || 
	*ver_rank > *ver_size)
    {
	perr (HE_MSG, 
	      "invalid rank specification");
    }

    if (*hor_size > 1)
    {
	fprintf (stderr, "rank: horizontal size = %d, rank = %d\n",
		 *hor_size,
		 *hor_rank);
    }
    if (*ver_size > 1)
    {
	fprintf (stderr, "rank: vertical size   = %d, rank = %d\n",
		 *ver_size,
		 *ver_rank);
    }
    return;
}

