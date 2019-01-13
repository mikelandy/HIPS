/*      Copyright (c) 1987, 1988 UCLA Machine Perception Laboratory
Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.  
*/

/*
 * mclear.c - clear the matrox vip-1024 frame grabber board(s).
 *
 * Usage:	mclear [-cCOLORPLANE]
 *
 * Defaults:	clears ALL COLOR PLANES
 *
 * Load:	cc -o mclear mclear.c -lhips -MplVip
 *
 * Edmond Mesrobian - 10/11/87 
 *
 * The -c option is for clearing a particular color plane. COLORPLANE 
 * should be replaced by one of the following letters <R,r,G,g,B,b> 
 * indicating which color plane should be cleared. 
 * If the -c option is not used, the default is for all three colorplanes
 * to be cleared.
 */

#include <hipl_format.h>
#include <stdio.h>

/* vip board numbers for the coressponding colors */
#define GREEN	0
#define RED	1
#define BLUE	2
#define ALLPLANES	3

#define RESET_REGS	1	/* set vip registers to a friendly state */
#define CLEAR_MEMORY	1       /* erase vip frame memory (set to 0)     */
#define DEFINE_LUTS	1	/* set vip luts to a linear scale        */

/* Global data structures */  

main(argc,argv)

int argc;
char **argv;
{
        int colorboard = ALLPLANES;  /* default is to send the frame to */
                                     /* all of the color boards.        */


        extern mpl_vip_setup();  /* routine to initialize the matrox board */
        extern mpl_vip_close_device();  /* routine to initialize the matrox board */

        
        /* check to see if the user has requested to clear a particular */
        /* color plane. If so, get the colorplane involved.             */

        if ( (argv[argc-1][0] == '-') && (argv[argc-1][1] == 'c') )
           {
                switch (argv[argc-1][2])
                  {
                   case 'r':
                   case 'R':	colorboard = RED;
				break;
                   case 'g':
     		   case 'G':	colorboard = GREEN;
				break;
                   case 'b':
	           case	'B':	colorboard = BLUE;
				break;
                   default:	perror("Illegal COLORGUN option -- llegal values are <r,R,g,G,b,B>");
				break;
                  }
		argc--;
	   }

	if(argv[argc-1][0]=='-')argc--;

        /* initalize the appropriate matrox board */
        if (colorboard == ALLPLANES)
           {
	     if ( mpl_vip_setup(GREEN,RESET_REGS,CLEAR_MEMORY,DEFINE_LUTS) == 0)
     		    fprintf("Matrox VIP-1024 initialization failed\n");
             else mpl_vip_close_device(GREEN);

	     if ( mpl_vip_setup(RED,RESET_REGS,CLEAR_MEMORY,DEFINE_LUTS) == 0)
     		    fprintf("Matrox VIP-1024 initialization failed\n");
             else mpl_vip_close_device(RED);

	     if ( mpl_vip_setup(BLUE,RESET_REGS,CLEAR_MEMORY,DEFINE_LUTS) == 0)
     		    fprintf("Matrox VIP-1024 initialization failed\n");
             else mpl_vip_close_device(BLUE);
           }
	else if ( mpl_vip_setup(colorboard,RESET_REGS,CLEAR_MEMORY,
                                                             DEFINE_LUTS) == 0)
     		  fprintf("Matrox VIP-1024 initialization failed\n");
             else mpl_vip_close_device(colorboard);


	return(0);
}

perr(s)

char *s;

{
	fprintf(stderr,"mclear: %s\n",s);
	exit(1);
}




