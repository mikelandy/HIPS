/*      Copyright (c) 1987, 1988 UCLA Machine Perception Laboratory
Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.  
*/

/*
 * msnap.c - capture a frame of video input through the matrox 
 *	     vip-1024 frame snapber board(s).
 *
 * Usage:	msnap [-c]
 *
 * Defaults:	captures a frame from video input using just
 *		the GREEN COLORPLANE BOARD (i.e., the master).
 *
 * Load:	cc -o msnap msnap.c -lhips -MplVip
 *
 * Edmond Mesrobian - 11/19/87 
 *
 * The -c option is used to indicate that the video source is a color
 * camera (i.e., use all three colorplanes).
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

#define RESET_REGS	0	/* do not reset the vip registers */
#define CLEAR_MEMORY    0	/* do not clear vip frame memory */
#define DEFINE_LUTS	0	/* do not reset vip luts         */

main(argc,argv)

int argc;
char **argv;
{
        int input_image_type = GRAY;


        extern mpl_vip_snap();  /* routine to capture a frame from */
				    /* a video source */

        

        /* check to see if the user has requested to snap               */
        /* a particular type of image: a gray scale (GREEN colorplane)  */
        /* or color image (all colorplanes). If so, get the image type  */

        if ( (argv[argc-1][0] == '-') && (argv[argc-1][1] == 'c') )
           {
             input_image_type = COLOR;
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

            if (!mpl_vip_snap(RED))
     		    perror("Matrox VIP-1024 snap failed (RED)");
            if (!mpl_vip_snap(BLUE))
     		    perror("Matrox VIP-1024 snap failed (BLUE)");
            if (!mpl_vip_snap(GREEN))
     		    perror("Matrox VIP-1024 snap failed (GREEN)");

           }
	else if ( mpl_vip_setup(GREEN,RESET_REGS,CLEAR_MEMORY,DEFINE_LUTS) == 0)
     		    perror("Matrox VIP-1024 (GREEN) initialization failed");
             else if (!mpl_vip_snap(GREEN))
     		      perror("Matrox VIP-1024 snap failed (GREEN)");


	return(0);
}

perr(s)

char *s;

{
	fprintf(stderr,"msnap: %s\n",s);
	exit(1);
}




