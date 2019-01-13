/*
 * wframe.c - write a frame on the matrox vip-1024 frame grabber
 *
 * Usage:	wframe [initialrow [initialcolumn]] [-cCOLORPLANE] < frame
 *
 * Defaults:	centered in the first 480x512
 *
 * Load:	cc -o wframe wframe.c viplib3.o -lhips
 *
 * Michael Landy - 6/22/85
 * Edmond Mesrobian - 10/11/87 Modified to run on the matrox vip-1024
 *                           frame grabber
 * (initialrow, initialcolumn) specifies the screen position for 
 * frame coordinate (0,0).  Effective off-screen coordinates are lost,
 * so there is no wraparound.  Note that for bit-packed images, the image
 * will be extended with 0's to fill out the last byte of the image line.
 * Only bit-packed images with 1 bit per pixel in LSBFIRST format will
 * written to the screen.
 * The -c option is for displaying an 8 bit image on a user specified colorplane
 * (green, red, or blue colorboards). COLORPLANE should be replaced
 * by one of the following letters <R,r,G,g,B,b> indicating which color board
 * should be used. If the -c option is not used, the default is for the 
 * frame to be sent to all three colorplanes.
 *
 * If the specified file has more than 1 frame, it is assume to be a 24 bit
 * image and each frame is sent to a different colorboard. The frames are
 * assumed to be in the following order: RED, GREEN, AND BLUE.
 */

#include <hipl_format.h>
#include <stdio.h>

#define SIZE	(480 * 512)

/* vip board numbers for the coressponding colors */
#define GREEN	0
#define RED	1
#define BLUE	2
#define ALLPLANES	3

#define RESET_REGS	1     /* 0 ==> leave vip registers alone */
#define DEFINE_LUTS     1     /* 0 ==> leave vip luts alone      */
#define CLEAR_MEMORY    0     /* 1 ==> erase vip frame memory (to 0) */

/* Global data structures */  

extern unsigned char *vip_base[];

main(argc,argv)

int argc;
char **argv;
{
	int i,j,r,c,cb,ocb,ir,irr,ic;
	unsigned char *fr,*ofr,*pfr,*pofr;
	struct header hd;
        int hp98710;
        int bit;
	int count;
        int lchar = 0;
        int hchar = 255;
        int colorboard = ALLPLANES;  /* default is to send the frame to */
                                     /* all of the color boards.        */


        extern mpl_vip_setup();  /* routine to initialize the matrox board */
        extern mpl_vip_close_device();  /* routine to initialize the matrox board */
        extern mpl_vip_write_rect();

	Progname = strsave(*argv);
	read_header(&hd);
	r = hd.orows;
	cb = c = hd.ocols;
	if (hd.pixel_format == PFLSBF)
		cb = (c+7)/8;
	if ((fr = (unsigned char *) calloc(r*cb,sizeof(unsigned char))) == 0)
		perr(HE_MSG,"can't allocate core");
	ir=(480-r)/2; ic=(512-c)/2;
        
        /* check to see if the user has requested to output a color image. If */
        /* so, get the colorgun involved.				      */

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
                   default:	perror("Illegal COLORPLANE option -- llegal values are <r,R,g,G,b,B>");
				break;
                  }
		argc--;
	   }

	if (hd.num_frame == 3) colorboard = ALLPLANES; /* 24 bit image */

	if(argv[argc-1][0]=='-')argc--;
	if(argc>1) ir=atoi(argv[1]);
	if(argc>2) ic=atoi(argv[2]);
	if (hd.pixel_format != PFBYTE && hd.pixel_format != PFLSBF)
		perr(HE_MSG,"frame must be in byte or LSBF format");

        /* initalize the appropriate matrox board */
        if (colorboard == ALLPLANES)
           {
             if (!mpl_vip_setup(GREEN,RESET_REGS,CLEAR_MEMORY,DEFINE_LUTS))
     		perror("Matrox VIP-1024 GREEN board initialization failed");

             if (!mpl_vip_setup(RED,RESET_REGS,CLEAR_MEMORY,DEFINE_LUTS))
     		perror("Matrox VIP-1024 RED board initialization failed");

             if (!mpl_vip_setup(BLUE,RESET_REGS,CLEAR_MEMORY,DEFINE_LUTS))
     		perror("Matrox VIP-1024 BLUE board initialization failed");
           }
        else if (!mpl_vip_setup(colorboard,RESET_REGS,CLEAR_MEMORY,DEFINE_LUTS))
     		perror("Matrox VIP-1024 initialization failed");


	for (count = 0; count < hd.num_frame; count++) {
		if (fread(fr,r*cb*sizeof(unsigned char),1,stdin) != 1)
			perr(HE_MSG,"error during read");

		if (hd.pixel_format == PFLSBF) {
      	    		if ((ofr = (unsigned char *) calloc(r*c,sizeof(unsigned char))) == 0)
					perr(HE_MSG,"can't allocate output frame core");
             		pofr = ofr; pfr = fr;
             		for (i=0;i<r;i++) {
                   		bit = 0;
                   		for (j=0;j<c;j++) {
                         		*pofr++ = ((*pfr & (01 << bit)) != 0)
                                       				? hchar : lchar;
                         		if (++bit == 8) {
                              			bit =0;
                              			pfr++;
                         		}
                   		}
                   		if (bit !=0)
                      			pfr++;
		 	}

			if (hd.num_frame == 3) {
				switch (count) {
				   case 0:
                   			mpl_vip_write_rect(RED,ic,ir,c,r,ofr);
					break;
				   case 1:
                   			mpl_vip_write_rect(GREEN,ic,ir,c,r,ofr);
					break;
				   case 2:
                   			mpl_vip_write_rect(BLUE,ic,ir,c,r,ofr);
					break;
				}
			} else {
                		if (colorboard == ALLPLANES) {
                     			mpl_vip_write_rect(GREEN,ic,ir,c,r,ofr);
                     			mpl_vip_write_rect(RED,ic,ir,c,r,ofr);
                     			mpl_vip_write_rect(BLUE,ic,ir,c,r,ofr);
                  		} else mpl_vip_write_rect(colorboard,ic,ir,c,r,ofr);
			}

		} else {
			if (c & 01) {
				ocb = c + 1;
				if ((ofr = (unsigned char *) calloc(r*ocb,sizeof(unsigned char))) == 0)
					perr(HE_MSG,"can't allocate output frame core");
				pofr = ofr; pfr = fr;
				for (i=0;i<r;i++) {
					for (j=0;j<ocb;j++) {
						if (j < c)
							*pofr++ = *pfr++;
						else *pofr++ = 0;
					}
				}
			} else {
				ocb = c;
                        	ofr = fr;
			}

			if (hd.num_frame == 3) {
				switch (count) {
				   case 0:
                   			mpl_vip_write_rect(RED,ic,ir,ocb,r,ofr);
					break;
				   case 1:
                   			mpl_vip_write_rect(GREEN,ic,ir,ocb,r,ofr);
					break;
				   case 2:
                   			mpl_vip_write_rect(BLUE,ic,ir,ocb,r,ofr);
					break;
				}
			} else {
                		if (colorboard == ALLPLANES) {
                     			mpl_vip_write_rect(GREEN,ic,ir,ocb,r,ofr);
                     			mpl_vip_write_rect(RED,ic,ir,ocb,r,ofr);
                     			mpl_vip_write_rect(BLUE,ic,ir,ocb,r,ofr);
                  		} else mpl_vip_write_rect(colorboard,ic,ir,ocb,r,ofr);
			}
                
		} 

	} /* all frames */

        if (colorboard == ALLPLANES)
           {
             mpl_vip_close_device(GREEN);
             mpl_vip_close_device(RED);
             mpl_vip_close_device(BLUE);
           }
        else mpl_vip_close_device(colorboard);
	return(0);
}
