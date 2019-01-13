/*
**       Copyright (c) 1990 David Wilson, The Turing Institute.
**
** Disclaimer:  No guarantees of performance accompany this software,
** nor is any responsibility assumed on the part of the authors.  All the
** software has been tested extensively and every effort has been made to
** insure its reliability.
**
*/


/*
**
** Filename: datacube.h
**
** Description:	Header file containing useful definitons for a Datacube
**              DIGIMAX/FRAMESTORE system. Note that this files includes
**              the required Datacube header files for both the DIGIMAX
**              and FRAMESTORE boards. It should really be extended to
**              cope with both RS-170 and CCIR signals. This is quite
**              simple and it will be done soon when I get access to
**              RS-170 signals to test it out. It goes hand in hand with
**              libray routines in file datacube_lib.c
**
** To load: See the accompanying Makefile.
**
** Author: David Wilson, The Turing Institute, September 1991.
**
*/


/*
** Include the Datacube definitions.
*/

#include <dgHead.h>		/* Datacube DIGIMAX Board Header File */
#include <fsHead.h>		/* Datacube FRAMESTORE Board Header File */



/*
** Define a mapping for the analogue input channels.
*/

#define NUM_DG_CHANNELS  8		/* DIGIMAX has 8 analogue inputs */

static ENUM  dgchannels [NUM_DG_CHANNELS] =
{
    DG_P1P12,
    DG_P3P12,
    DG_P5P12,
    DG_P7P12,
    DG_P1P11,
    DG_P3P11,
    DG_P5P11,
    DG_P7P11
};




/*
** Specify the CCIR operating parameters for DIGIMAX/FRAMESTORE.
*/

#define MAX_IMAGE_ROWS  512
#define MAX_IMAGE_COLS  512
#define FIELD_RATE      50




/*
** Function declarations for utilities in datacube_lib.
*/

extern int  initialise_Datacube ();
extern void  freeze_display ();
extern void  live_display ();
extern void  clear_display ();
extern void  delay_fields ();
extern void  grab_frame ();
extern void  load_frame ();
extern int  close_Datacube ();
