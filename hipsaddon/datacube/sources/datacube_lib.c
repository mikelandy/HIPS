/*
**       Copyright (c) 1991 David Wilson, The Turing Institute.
**
** Disclaimer:  No guarantees of performance accompany this software,
** nor is any responsibility assumed on the part of the authors.  All the
** software has been tested extensively and every effort has been made to
** insure its reliability.
**
*/


/*
**
** Filename: datacube_lib.c
**
** Description:	Library modules for a Datacube DIGIMAX/FRAMESTORE
**              system. These are simple routines to avoid a user
**              having to learn the intrincacies of Datacube MaxWare.
**              The configuration required for the boards is the
**              default connecions and all display/capture is via
**              FS0 on the FRAMESTORE. 
**
** To load: See the accompanying Makefile
**
** Author: David Wilson, The Turing Institute, September 1991.
**
*/


/*
** Include the Datacube definitions.
*/

#include <stdio.h>			/* UNIX Standard I/O Header File */
#include "datacube_lib.h"		/* Local Datacube Library Header File */




/*******************************************************************************
**
** Name: initialise_Datacube
**
** Purpose: To open the Datacube hardware and initialise it to a standard
**	    state displaying live video from the selected analogue input.
**
** Parameters: The following parameters are recognised
**		   digimax_ptr    - **DG_DESC, is set to point to the
**				    descriptor for the opened DIGIMAX
**				    device.
**
**		   framestore_ptr - **FS_DESC, is set to point to the
**				    descriptor for the opened FRAMESTORE
**				    device.
**
**		   input_channel  - int, the selected analogue input channel.
**
** Return Value: An error signal of NOOK  will be returned if a fatal error
**		 occurs trying to open either the DIGIMAX or FRAMESTORE.
**		 Otherwise, the return value of OK is used to indicate
**		 success. These are Datacube defined values.
**
** Functions Called: None
**
** External References:
**    datacube_lib.h - Local Datacube utilites header file
**
*******************************************************************************/


int
initialise_Datacube (digimax_ptr,
		     framestore_ptr,
		     input_channel)

DG_DESC	 **digimax_ptr;
FS_DESC	 **framestore_ptr;
int	 input_channel;

{

    /*
    ** Open descriptors for the boards.
    */

    *digimax_ptr = dgOpen (DG_BASE,
			   DG_VECTOR,
			   DQ_OFF);

    if (*digimax_ptr == (DG_DESC *) NULL)
    {
	return (NOOK);
    }

    *framestore_ptr = fsOpen (FS_BASE, DQ_OFF);

    if (*framestore_ptr == (FS_DESC *) NULL)
    {
	(void) dgClose (*digimax_ptr);
	return (NOOK);
    }

    /*
    ** Set the DIGIMAX as the master interrupter.
    */

    dgMaster (*digimax_ptr);

    /*
    ** Initialise the boards to a standard state.
    */

    dgInit (*digimax_ptr, DG_UNSGD);

    fsInit (*framestore_ptr, FS_T50);

    /*
    ** Select the appropriate analogue input for the camera and display
    ** live video.
    */

    dgAnInput (*digimax_ptr, dgchannels [input_channel]);
    fs0AcquireMd (*framestore_ptr, FS_ACQUIRE);

    /*
    ** This delay is a timing fix to let things get into sync for a
    ** damaged piece of kit. Note that it not generally required.
    */

    dgWaitFld (*digimax_ptr, 2);

    /*
    ** Flush the queued primitives
    */

    mvRefresh ();
    mvEmpty ();

    return (OK);

}




/*******************************************************************************
**
** Name: freeze_video
**
** Purpose: To freeze the video input to a Datacube FRAMESTORE.
**
** Parameters: The following parameters are recognised
**		   framestore_ptr - *FS_DESC, a pointer to the
**				    descriptor for the opened FRAMESTORE
**				    device.
**
** Return Value: None
**
** Functions Called: None
**
** External References:
**    datacube_lib.h - Local Datacube utilites header file
**
*******************************************************************************/


void
freeze_video (framestore_ptr)

FS_DESC	 *framestore_ptr;

{

    fs0AcquireMd (framestore_ptr, FS_FREEZE);
    mvRefresh ();
    mvEmpty ();
    return;

}




/*******************************************************************************
**
** Name: live_video
**
** Purpose: To enable live video display via a Datacube FRAMESTORE.
**
** Parameters: The following parameters are recognised
**		   framestore_ptr - *FS_DESC, a pointer to the
**				    descriptor for the opened FRAMESTORE
**				    device.
**
** Return Value: None
**
** Functions Called: None
**
** External References:
**    datacube_lib.h - Local Datacube utilites header file
**
*******************************************************************************/


void
live_video (framestore_ptr)

FS_DESC	 *framestore_ptr;

{

    fs0AcquireMd (framestore_ptr, FS_ACQUIRE);
    mvRefresh ();
    mvEmpty ();
    return;

}




/*******************************************************************************
**
** Name: clear_display
**
** Purpose: To clear the display from a Datacube FRAMESTORE to a single
**          greylevel.
**
** Parameters: The following parameters are recognised
**		   framestore_ptr - *FS_DESC, a pointer to the
**				    descriptor for the opened FRAMESTORE
**				    device.
**
**		   greylevel - int, the greylevel to which the background
**			       is cleared.
**
** Return Value: None
**
** Functions Called: None
**
** External References:
**    datacube_lib.h - Local Datacube utilites header file
**
*******************************************************************************/


void
clear_display (framestore_ptr,
	       greylevel)

FS_DESC	 *framestore_ptr;
int      greylevel;

{

    fsFastClear (framestore_ptr,
		 FS_FS0,
		 greylevel);
    mvRefresh ();
    mvEmpty ();
    return;

}




/*******************************************************************************
**
** Name: delay_fields
**
** Purpose: To pause for a given number of field times.
**
** Parameters: The following parameters are recognised
**		   digimax_ptr - *DG_DESC, a pointer to the
**				 descriptor for the opened DIGIMAX
**				 device.
**
**		   delay - int, the number of fields to delay.
**
** Return Value: None
**
** Functions Called: None
**
** External References:
**    datacube_lib.h - Local Datacube utilites header file
**
*******************************************************************************/


void
delay_fields (digimax_ptr,
	      delay)

DG_DESC	 *digimax_ptr;
int      delay;

{

    dgWaitFld (digimax_ptr, delay);
    mvRefresh ();
    mvEmpty ();
    return;

}




/*******************************************************************************
**
** Name: grab_frame
**
** Purpose: To grab a frame via the Datacube DIGIMAX and FRAMESTORE.
**	    A frame may be of any size and may be taken from any
**	    arbitrary position provided that the full extent of the
**	    frame lies within the boundary of the FRAMESTORE. Note
**	    that the FRAMESTORE can handle a maximum image size of
**	    512  x 512 pixels. The window given by the orign and size
**	    coordinates  can either be captured at full or reduced
**	    resolution.
**
** Parameters: The following parameters are recognised
**		   framestore_device - *FS_DESC, a descriptor for the
**				       opened FRAMESTORE device.
**
** 		   buffer	     - unsigned char *, a preallocated
**				       block of memory to hold the captured
**				       frame.
**
** 		   row_buffer	     - unsigned char *, a preallocated
**				       block of memory to hold the captured
**				       row for reduced resolution frames.
**
** 		   x_origin 	     - int, the x origin of the window from
**				       which the image is to be captured.
**				       This ranges from 0 to 511.
**
** 		   y_origin 	     - int, the y origin of the window from
**				       which the image is to be captured.
**				       This ranges from 0 to 511.
**
** 		   x_size	     - int, the x size of the window from
**				       which the image is to be captured.
**				       This ranges from 1 to 512.
**
** 		   y_size	     - int, the y size of the window from
**				       which the image is to be captured.
**				       This ranges from 1 to 512.
**
** 		   x_resolution      - int, the x resolution at which the
**				       image is to be captured.
**				       This ranges from 1 (equivalent to
**				       full resolution) and must be exactly
**				       divisible into the frame x size.
**
** 		   y_resolution      - int, the y resolution at which the
**				       image is to be captured.
**				       This ranges from 1 (equivalent to
**				       full resolution) and must be exactly
**				       divisible into the frame x size.
**
** Return Value: None
**
** Functions Called: None
**
** External References:
**    datacube_lib.h - Local Datacube utilites header file
**
*******************************************************************************/


void
grab_frame (framestore_device,
	    buffer,
	    row_buffer,
	    x_origin,
	    y_origin,
	    x_size,
	    y_size,
	    x_resolution,
	    y_resolution)

FS_DESC  *framestore_device;
unsigned char	 *buffer;
unsigned char	 *row_buffer;
int	 x_origin;
int	 y_origin;
int	 x_size;
int	 y_size;
int	 x_resolution;
int	 y_resolution;

{

    unsigned char  *pixel_ptr;

    int		   x, y;


    /*
    ** If a full resolution view is required, capture it in one go.
    ** Otherwise, it is necessary to capture each row in turn and extract
    ** the required pixels from it.
    */

    if (x_resolution == 1 &&
	y_resolution == 1)
    {
	fsVmBitBlt (framestore_device,
		    MOVE,
		    FS_FS0,
		    x_origin,
		    y_origin,
		    buffer,
		    x_size,
		    y_size);
    }
    else
    {
	for (y = y_origin; y < y_size; y += y_resolution)
	{
	    fsVmRowMove (framestore_device,
			 FS_FS0,
			 x_origin,
			 y,
			 row_buffer,
			 x_size);

	    pixel_ptr = row_buffer;
	    for (x = x_origin; x < x_size; x += x_resolution)
	    {
		*buffer = *pixel_ptr;
		pixel_ptr += x_resolution;
		buffer++;
	    }
	}
    }
    return;

}




/*******************************************************************************
**
** Name: load_frame
**
** Purpose: To load a frame into a Datacube FRAMESTORE at an  arbitrary
**          position.
**
** Parameters: The following parameters are recognised
**		   framestore_device - *FS_DESC, a descriptor for the
**				       opened FRAMESTORE device.
**
** 		   buffer	     - unsigned char *, a block of memory
**				       holding the frame.
**
** 		   x_origin 	     - int, the x origin at which the
**				       the frame is to be loaded.
**				       This ranges from 0 to 511.
**
** 		   y_origin 	     - int, the y origin at which the
**				       the frame is to be loaded.
**				       This ranges from 0 to 511.
**
** 		   x_size	     - int, the x size of the frame
**				       which is to be loaded.
**				       This ranges from 1 to 512.
**
** 		   y_size	     - int, the y size of the frame
**				       which is to be loaded.
**				       This ranges from 1 to 512.
**
**
** Return Value: None
**
** Functions Called: None
**
** External References:
**    datacube_lib.h - Local Datacube utilites header file
**
*******************************************************************************/


void
load_frame (framestore_device,
	    buffer,
	    x_origin,
	    y_origin,
	    x_size,
	    y_size)

FS_DESC  *framestore_device;
unsigned char	 *buffer;
int	 x_origin;
int	 y_origin;
int	 x_size;
int	 y_size;

{

    fsMvBitBlt (framestore_device,
		MOVE,
		buffer,
		FS_FS0,
		x_origin,
		y_origin,
		x_size,
		y_size);

     return;
}




/*******************************************************************************
**
** Name: close_Datacube
**
** Purpose: To close the Datacube hardware descriptors.
**
** Parameters: The following parameters are recognised
**		   digimax_ptr    - *DG_DESC, a pointer to the
**				    descriptor for the opened DIGIMAX
**				    device.
**
**		   framestore_ptr - *FS_DESC, a pointer to the
**				    descriptor for the opened FRAMESTORE
**				    device.
**
**
** Return Value: An error signal of NOOK  will be returned if either the
**		 DIGIMAX or FRAMESTORE can not be closed. Otherwise a
**		 return value of OK is used to indicate success.
**		 These are Datacube defined values.
**
** Functions Called: None
**
** External References:
**    datacube_lib.h - Local Datacube utilites header file
**
*******************************************************************************/


int
close_Datacube (digimax_device,
		framestore_device)

DG_DESC	 *digimax_device;
FS_DESC	 *framestore_device;

{

    /*
    ** Close the descriptors for both boards.
    */

    if (dgClose (digimax_device) == NOOK ||
	fsClose (framestore_device) == NOOK)
    {
	return (NOOK);
    }

    return (OK);

}



