/*
**             Copyright (c) 1991 The Turing Institute
**
** Disclaimer:  No guarantees of performance accompany this software,
** nor is any responsibility assumed on the part of the authors.  All the
** software has been tested extensively and every effort has been made to
** insure its reliability.
**
*/


/*
**
** Filename: livepic.c
**
** Description:	Filter to set a Datacube DIGIMAX/FRAMESTORE configuration
**		to display live video from a selected input channel.
**		Note that this modules assumes the default routing
**              both DIGIMAX and FRAMESTORE.
**
** Usage: livepic [-c channel]
**
**        where "channel" should be between 0 and 7 and is the analogue
**        input to DIGIMAX which is displayed.
**
** To load: see the accompanying Makefile
**
** Author: David Wilson, The Turing Institute, September 1991.
*/


/*
** Specify include files.
*/

#include <stdio.h>		/* UNIX Standard I/O  Header File */
#include <hipl_format.h>	/* HIPS Header File */
#include "datacube_lib.h"	/* Local Datacube Utilities Header File */


/*
** Define the command line options accepted by this filter.
*/

static Flag_Format flag_format[] =
{
    {"c", {LASTFLAG}, 1, {{PTINT, "0", "channel"}, LASTPARAMETER}},
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

    DG_DESC  *digimax_device;
    FS_DESC  *framestore_device;


    Progname = strsave (argv [0]);

    /*
    ** Set the required operating mode.
    */

    parseargs (argc,
	       argv,
	       flag_format,
	       &channel,
	       FFNONE);

    /*
    ** Check that the selected channel is valid - DIGIMAX supports
    ** channels 0 to 7.
    */

    if (channel < 0 || channel >= NUM_DG_CHANNELS)
    {
	perr (HE_MSG, "invalid channel invalid - DIGIMAX supports channels 0 to 7");
    }

    /*
    ** Initialise the DATACUBE boards for live video from the selected
    ** channel.
    */

    if (initialise_Datacube(&digimax_device,
			    &framestore_device,
			    channel)
			    == NOOK)
    {
	perr (HE_MSG, "error initialising the Datacube hardware");
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
