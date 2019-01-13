/*      Copyright (c) 1987, 1988 UCLA Machine Perception Laboratory
Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.  
*/

/*
 * mlive.c - set up a continuous grab (digitization) of video input 
 *	     through the matrox vip-1024 frame grabber board(s).
 *
 * Usage:	mlive [-vINPUT_VIDEO_CHANNEL] [-c]
 *
 * Defaults:	Continuously grabs video input from channel 0 using just
 *		the GREEN COLORPLANE BOARD (i.e., the master).
 *
 * Load:	cc -o mlive mlive.c -lhips -MplVip
 *
 * Edmond Mesrobian - 11/19/87 
 *
 * The -v option is for continuously grabbing video input from a video
 * channel INPUT_VIDEO_CHANNEL (other than channel 0). INPUT_VIDEO_CHANNEL 
 * should be replaced by one of the following numbers <0,1,2> indicating 
 * which video channel to use. If the -v option is not used, the default 
 * is to use channel 0.
 * The -c option is used to indicate that the video source is a color
 * camera (i.e., use all three colorplanes).
 * This continuous digitization stops when any other Matrox command is given.
 */

#include <hipl_format.h>
#include <stdio.h>

/* vip board numbers for the coressponding colors */
#define GREEN	0
#define RED	1
#define BLUE	2
#define ALLPLANES	3

/* vip board input video channels */
#define INPUT_VIDEO_CHANNEL0	0
#define INPUT_VIDEO_CHANNEL1	1
#define INPUT_VIDEO_CHANNEL2	2

/* video input image types */
#define GRAY	0
#define COLOR	1

#define RESET_REGS	1	/* set vip regs to a friendly state */
#define CLEAR_MEMORY	1	/* erase vip frame memory (set to 0) */
#define DEFINE_LUTS	1	/* set vip luts to a linear scale */

main(argc,argv)

int argc;
char **argv;
{
        int video_input_channel = INPUT_VIDEO_CHANNEL0;
        int input_image_type = GRAY;


        extern mpl_vip_setup();  /* routine to initialize the matrox board */
        extern mpl_vip_cont_grab();  /* routine to contimuously grab from */
				    /* a video source */

        

        /* check to see if the user has requested to continuously grab  */
        /* a particular type of image: a gray scale (GREEN colorplane)  */
        /* or color image (all colorplanes). If so, get the image type  */

        if ( (argv[argc-1][0] == '-') && (argv[argc-1][1] == 'c') )
           {
             input_image_type = COLOR;
	     argc--;
	   }

        /* check to see if the user has requested to continuously grab  */
        /* from a particular video channel. If so, get the channel involved. */

        if ( (argv[argc-1][0] == '-') && (argv[argc-1][1] == 'v') )
           {
                switch (argv[argc-1][2])
                  {
                   case '0':	video_input_channel = INPUT_VIDEO_CHANNEL0;
				break;
     		   case '1':	video_input_channel = INPUT_VIDEO_CHANNEL1;
				break;
	           case	'2':	video_input_channel = INPUT_VIDEO_CHANNEL2;
				break;
                   default:	perror("Illegal INPUT_VIDEO_CHANNEL option -- llegal values are <0,1,2>");
				break;
                  }
		argc--;
	   }

	if(argv[argc-1][0]=='-')argc--;

        /* initalize the appropriate matrox board(s) */
        if (input_image_type == COLOR)
           {
	    if ( mpl_vip_setup(GREEN,RESET_REGS,CLEAR_MEMORY,DEFINE_LUTS) == 0)
     		    perror("Matrox VIP-1024 (GREEN) initialization failed");

	    if ( mpl_vip_setup(RED,RESET_REGS,CLEAR_MEMORY,DEFINE_LUTS) == 0)
     		    perror("Matrox VIP-1024 (RED) initialization failed");

	    if ( mpl_vip_setup(BLUE,RESET_REGS,CLEAR_MEMORY,DEFINE_LUTS) == 0)
     		    perror("Matrox VIP-1024 (BLUE) initialization failed");

            if (!mpl_vip_cont_grab_frame(video_input_channel,GREEN))
     		    perror("Matrox VIP-1024 grab failed (GREEN)");
            if (!mpl_vip_cont_grab_frame(video_input_channel,RED))
     		    perror("Matrox VIP-1024 grab failed (RED)");
            if (!mpl_vip_cont_grab_frame(video_input_channel,BLUE))
     		    perror("Matrox VIP-1024 grab failed (BLUE)");

           }
	else if ( mpl_vip_setup(GREEN,RESET_REGS,CLEAR_MEMORY,DEFINE_LUTS) == 0)
     		  perror("Matrox VIP-1024 (GREEN) initialization failed");
             else if (!mpl_vip_cont_grab_frame(video_input_channel,GREEN))
     		      perror("Matrox VIP-1024 grab failed (GREEN)");


	return(0);
}

perr(s)

char *s;

{
	fprintf(stderr,"mlive: %s\n",s);
	exit(1);
}




