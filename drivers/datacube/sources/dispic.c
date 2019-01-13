/*
**             Copyright (c) 1990 The Turing Institute
**
** Disclaimer:  No guarantees of performance accompany this software,
** nor is any responsibility assumed on the part of the authors.  All the
** software has been tested extensively and every effort has been made to
** insure its reliability.
**
*/


/*
**
** Filename: dispic.c
**
** Description:	HIPS filter to display an image via a Datacube DIGIMAX
**		FRAMESTORE configuration. Note that this module assumes
**              the default physical routing between DIGIMAX and
**              FRAMESTORE.
**
** Usage: dispic [-b background]
**               [-o row col]
**               [-d delay]
**               < imagefile
**
**        where
**              background - specifies the background greylevel on which
**              the image is displayed and defaults to 0.
**
**              row, col - specifies an offset at which the image is
**              displayed and defaults to (0,0).
**
**              delay - specifies a delay in seconds between the display
**              of successive frames in a sequence and defaults to 0.
**
**              imagefile - is the name of the image to be displayed.
**
** To load: See the accompanying makefile.
**
** Author: David Wilson, The Turing Institute,, September 1991.
*/


/*
** Specify include files.
*/

#include <stdio.h>		/* UNIX Standard I/O Header File */
#include <math.h>		/* UNIX Standard Maths Header File */
#include <hipl_format.h>	/* HIPS Header File */
#include "datacube_lib.h"	/* Local Datacube Utilities Header File */


/*
** Define the command line options accepted by this filter.
*/

static Flag_Format flag_format[] =
{
    {"b", {LASTFLAG}, 1, {{PTINT, "0", "background"}, LASTPARAMETER}},
    {"d", {LASTFLAG}, 1, {{PTDOUBLE, "0.0", "delay"}, LASTPARAMETER}},
    {"o", { LASTFLAG}, 2, {{PTINT, "0", "from_row"}, {PTINT, "0", "from_col"}, LASTPARAMETER}},
    LASTFLAG
};

/*
** Only PFBYTE is handled directly as yet.
*/

int types[] = {PFBYTE, LASTTYPE};




/*******************************************************************************
**
** Name: main
**
** Purpose: Main control routine
**
** Parameters: The following parameters are recognised
**		   argc - int, the number of command line arguments passed to
**			  the program.
**
**		   argv - **char, the command line which invoked the program.
**
** Return Value: The function returns 0 to the invoking shell upon successful
**		 completion. If an error condition is met, the function
**		 returns -1 to the invoking shell.
**
** Functions Called: None
**
** External References:
**    stdio.h  - UNIX standard I/O header file
**    math.h  - UNIX standard maths/O header file
**    datacube.h - Local Datacube utilities header file
**
*******************************************************************************/


int
main (argc,
      argv)

int   argc;
char  **argv;

{

    int	           background;
    int	           row_offset,  col_offset;
    int	           field_delay;
    int            frame, method;

    double         delay;

    Filename       image_file;
    FILE           *image_file_ptr;
    DG_DESC        *digimax_device;
    FS_DESC        *framestore_device;
    struct header  input_image, converted_image;


    Progname = strsave (argv [0]);

    /*
    ** Set the required operating mode.
    */

    parseargs (argc,
	       argv,
	       flag_format,
	       &background,
	       &delay,
	       &row_offset,
	       &col_offset,
	       FFONE,
	       &image_file);

    image_file_ptr = hfopenr (image_file);

    /*
    ** Read the image header.
    */

    fread_hdr_a (image_file_ptr,
		 &input_image,
		 image_file);

    /*
    ** Check the size of the image.
    */

    if (input_image.rows > MAX_IMAGE_ROWS || input_image.cols > MAX_IMAGE_COLS)
    {
	perr (HE_MSG, "image is too big for display in this framestore");
    }

    if (row_offset < 0 || col_offset < 0)
    {
	perr (HE_MSG, "display origin coordinates must be positive");
    }

    if (row_offset + input_image.rows > MAX_IMAGE_ROWS ||
	col_offset + input_image.cols > MAX_IMAGE_COLS)
    {
	perr (HE_MSG, "display origin is too far over");
    }

    /*
    ** Check and convert the delay to field periods.
    */

    if (delay < 0.0)
    {
	perr (HE_MSG, "inter-frame delay must be positive");
    }

    field_delay = (int) ceil (delay * FIELD_RATE);


    /*
    ** Initialise the DATACUBE boards 
    */

    if (initialise_Datacube(&digimax_device,
			    &framestore_device,
			    0)
			    == NOOK)
    {
	perr (HE_MSG, "error initialising the Datacube hardware");
    }

    /*
    ** Freeze the display and clear to the background colour.
    */

    freeze_video (framestore_device);

    clear_display (framestore_device, background);

    /*
    ** Set up for conversion if this is needed.
    */

    method = fset_conversion (&input_image,
			      &converted_image,
			      types,
			      image_file);

    /*
    ** Read each frame in turn and load it into the framestore.
    */

    for (frame = 0; frame < converted_image.num_frame; frame++)
    {
	fread_imagec (image_file_ptr,
		      &input_image,
		      &converted_image,
		      method,
		      frame,
		      image_file);

	load_frame (framestore_device,
		    converted_image.firstpix,
		    col_offset,
		    row_offset,
		    converted_image.cols,
		    converted_image.rows);

	if (delay > 0)
	{
	    delay_fields (digimax_device, field_delay);

	}
    }

    /*
    ** Close the DIGIMAX and FRAMESTORE boards.
    */

    if (close_Datacube(digimax_device,
		       framestore_device)
		       == NOOK)
    {
	perr (HE_MSG, "error closing the Datacube hardware");
    }

    return (0);

}
