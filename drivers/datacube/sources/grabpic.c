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
** Filename: grabpic.c
**
** Description:	HIPS filter to capture an image from a Datacube DIGIMAX
**		FRAMESTORE configuration. Note that this module assumes
**              the default physical routing between DIGIMAX and
**              FRAMESTORE.
**
** Usage: grabpic [-c channel]
**                [-d num_rows num_cols]
**                [-o from_row from_col]
**                [-r row_resolution col_resolution]
**                [-s num_frames [delay]]
**                imagefile
**
**        where
**              channel - specifies the DIGIMAX i/p from which the image
**              is captured and defaults to 0.
**
**              num_rows, num_cols - specifies the size of the window
**              from which the image is captured and defaults to the full
**              video frame.
**
**              from_row, from_col - specifies the origin of this window
**              in the framestore and defaults to (0,0).
**
**              row_resolution, col_resolution - specifies the resolution
**              of the captured image taken from this window and defaults
**              to 4,4, ie a quarter sized view.
**
**              num_frames, delay - specifies that a sequence of
**              num_frames are grabbed with an inter-frame delay of
**              delay in seconds. These default to a single frame and
**              no delay between capturing successive frames.
**
**              imagefile - is the name of the file to which the image is
**              output.
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
    {"c", {LASTFLAG}, 1, {{PTINT, "0", "channel"}, LASTPARAMETER}},
    {"d", {LASTFLAG}, 2, {{PTINT, "512", "rows"}, {PTINT, "512", "cols"}, LASTPARAMETER}},
    {"o", {LASTFLAG}, 2, {{PTINT, "0", "from_row"}, {PTINT, "0", "from_col"}, LASTPARAMETER}},
    {"r", {LASTFLAG}, 2, {{PTINT, "4", "row_resolution"}, {PTINT, "4", "col_resolution"}, LASTPARAMETER}},
    {"s", {LASTFLAG}, 1, {{PTINT, "1", "num_frames"}, {PTDOUBLE, "0.0", "delay"}, LASTPARAMETER}},
    LASTFLAG
};




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
**    math.h  - UNIX standard maths header file
**    dgHead.h - Datacube main board header file for DIGIMAX
**    fsHead.h - Datacube main board header file for FRAMESTORE
**    datacube.h - Local Datacube utilities header file
**
*******************************************************************************/


int
main (argc,
      argv)

int   argc;
char  **argv;

{

    int	     channel;
    int	     num_rows,  num_cols;
    int	     from_row,  from_col;
    int	     row_resolution,  col_resolution;
    int	     num_frames, frame;
    int      field_delay;

    double   delay;

    byte     *row_buffer;

    DG_DESC  *digimax_device;
    FS_DESC  *framestore_device;

    struct header  header;


    Progname = strsave (argv [0]);

    /*
    ** Set the required operating mode.
    */

    parseargs (argc,
	       argv,
	       flag_format,
	       &channel,
	       &num_rows,
	       &num_cols,
	       &from_row,
	       &from_col,
	       &row_resolution,
	       &col_resolution,
	       &num_frames,
	       &delay,
	       FFNONE);

    /*
    ** Check that the capture parameters make sense.
    */

    if (channel < 0 || channel >= NUM_DG_CHANNELS)
    {
	perr (HE_MSG, "invalid channel - DIGIMAX supports channels 0 to 7");
    }

    if (num_rows <= 0 || num_cols <= 0 ||
	from_row < 0 || from_col < 0 ||
	from_row + num_rows > MAX_IMAGE_ROWS ||
	from_col + num_cols > MAX_IMAGE_COLS)
    {
	perr (HE_MSG, "parameters for the capture window are out of range");
    }

    if (row_resolution <= 0 || col_resolution <= 0)
    {
	perr (HE_MSG, "can't have a negative resolution");
    }

    if (num_rows % row_resolution != 0 ||
	num_cols % col_resolution != 0)
    {
	perr (HE_MSG, "can't get that resolution from that window");
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
			    channel)
			    == NOOK)
    {
	perr (HE_MSG, "error initialising the Datacube hardware");
    }

    /*
    ** Create and output the header.
    */

    init_header (&header,
		 "",
		 "",
		 num_frames,
		 "",
		 num_rows / row_resolution,
		 num_cols / col_resolution,
		 PFBYTE,
		 1,
		 "");

    write_headeru (&header,
		   argc,
		   argv);

    alloc_image (&header);

    if ((row_buffer = (byte *) memalloc (num_cols, sizeof (byte)))
		    == (byte *) HIPS_ERROR)
    {
	exit (HIPS_ERROR);
    }

    /*
    ** Capture and output the frames.
    */

    for (frame = 0; frame < num_frames; frame++)
    {
	freeze_video (framestore_device);

	grab_frame (framestore_device,
		    header.firstpix,
		    row_buffer,
		    from_col,
		    from_row,
		    num_cols,
		    num_rows,
		    col_resolution,
		    row_resolution);

        live_video (framestore_device);

	write_image (&header);

	if (delay > 0.0)
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
