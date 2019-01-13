/*	Copyright (c) 1982 Michael Landy, Yoav Cohen, and George Sperling

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 * rframe.c - read a frame from the Matrox VIP-1024
 *
 * Usage:	rframe [rows [cols [initialrow [initialcol]]]] [-cCOLORPLANE] 
 *			[-mac]
 *
 * Defaults:	rows: 480, cols: 512, initialrow: 0, initialcol: 0
 *		COLORPLANE: green
 *
 * Load:	cc -o rframe rframe.c -lhips -MplVip
 *
 * Michael Landy - 6/22/85
 * Modified to work on Matrox VIP-1024 by Edmond Mesrobian - 11/15/87
 *
 * Reads a frame from the Matrox VIP-1024 frame memory starting at 
 * (initialrow,initialcol) with size rows x cols. There is no wraparound 
 * so large sizes will be truncated.
 *
 * The -c colorplane option is used to indicate which colorplane the frame
 * should be grabbed from. Possible values for COLORPLANE are <a,A,r,R,g,G,b,B>.
 * Specifing the colorplane option -cA, causes rframe to read a frame from
 * each colorplane and to create a 24 bit color image. The hips image file will
 * contain 3 image frames (one for red, green, and blue).
 *
 * The -mac option causes rframe to create an image file for the Apple
 * Mac. It is assumed that the user wishes to have a 24 bit color image saved.
 */

#include <hipl_format.h>
#include <stdio.h>

#define SIZE	(480 * 512)

/* vip board numbers for the coressponding colors */
#define GREEN	0
#define RED	1
#define BLUE	2
#define ALLPLANES	3

#define RESET_REGS	1	/* 0 ==> leave vip registers alone */
#define CLEAR_MEMORY	0	/* 1 ==> set vip frame memory to 0 */
#define DEFINE_LUTS     1       /* 0 ==> leave vip luts alone      */

#define HIPS	0
#define MAC	1

/* Global data structures */  

extern unsigned char *vip_base[];

main(argc,argv)
char *argv[];
{
	int r,c,ir,ic,or,oc,i,argcc;
	unsigned char *fr;
	unsigned char *red_fr, *green_fr, *blue_fr;
        int hp98710;
	struct header hd;

	Progname = strsave(*argv);
	int format = HIPS;
        int colorboard = GREEN;  /* default is to read a frame from     */
                                 /* the GREEN COLORPLANE.               */


        extern mpl_vip_alt_setup(); /* routine to initialize the matrox board */
        extern mpl_vip_close_device();  /* routine to initialize the matrox board */
        extern mpl_vip_read_rect();

	argcc=argc;
	r=480;c=512; ir=ic=0;

        /* check to see if the user has requested to output a mac image. */
        if ( (argv[argc-1][0] == '-') && (argv[argc-1][1] == 'm') ) {
	    	format = MAC;
		argc--;
	}

        if ( (argv[argc-1][0] == '-') && (argv[argc-1][1] == 'c') ) {
                switch (argv[argc-1][2]) {
                   case 'r':
                   case 'R':	colorboard = RED;
				break;
                   case 'g':
     		   case 'G':	colorboard = GREEN;
				break;
                   case 'b':
	           case	'B':	colorboard = BLUE;
				break;
                   case 'a':
	           case	'A':	colorboard = ALLPLANES;
				break;
                   default:	perror("Illegal COLORPLANE option -- llegal values are <a,A,r,R,g,G,b,B>");
				break;
                }
		argc--;
	}

	if(argv[argc-1][0]=='-')
		argc--;

	if(argc>1)
		r=atoi(argv[1]);
	if(argc>2)
		c=atoi(argv[2]);
	if(argc>3)
		ir=atoi(argv[3]);
	if(argc>4)
		ic=atoi(argv[4]);

	or=480-ir;or=r<or?r:or;
	oc=(512-ic) & (~01);oc=c<oc?c:oc;
	if((or<1)||(oc<1))
		perr(HE_MSG,"wrong dimensions");
	if (oc & 01)
		perr("number of columns must be even");

	if (colorboard != ALLPLANES) {
		if ((fr = (unsigned char *) calloc(or*oc,sizeof(unsigned char))) == 0)
			perr(HE_MSG,"can't allocate core");
	} else {
		if ((red_fr = (unsigned char *) calloc(or*oc,sizeof(unsigned char))) == 0)
			perr(HE_MSG,"can't allocate core");
		if ((green_fr = (unsigned char *) calloc(or*oc,sizeof(unsigned char))) == 0)
			perr(HE_MSG,"can't allocate core");
		if ((blue_fr = (unsigned char *) calloc(or*oc,sizeof(unsigned char))) == 0)
			perr(HE_MSG,"can't allocate core");
	}





	switch (colorboard) {

		case GREEN :
		case BLUE :
		case RED :

        			/* initalize the appropriate matrox board */
				if (!mpl_vip_setup(colorboard,RESET_REGS,
						   CLEAR_MEMORY,DEFINE_LUTS)) 
     					perror("Matrox VIP-1024 board initialization failed");
        			mpl_vip_read_rect(colorboard,ic,ir,oc,or,fr);
        			mpl_vip_close_device(colorboard);
				if (format == MAC) {
					for (i=0; i<or*oc; i++) {
						putchar((int) (*fr) );
						putchar((int) (*fr) );
						putchar((int) (*fr) );
						fr++;
					}
				} else {
					init_header(&hd,"rframe","8 bit image",1,"",or,oc,PFBYTE,1,"");
					update_header(&hd,argcc,argv);
					write_header(&hd);
					if (fwrite(fr,
					    or*oc*sizeof(unsigned char),
					    1,stdout) != 1)
						perr(HE_MSG,"error during write");
				}
				break;

		case ALLPLANES:
        			/* initalize the matrox boards */
				if (!mpl_vip_setup(GREEN ,RESET_REGS,
						   CLEAR_MEMORY,DEFINE_LUTS)) 
     					perror("Matrox VIP-1024 board initialization failed");
				if (!mpl_vip_setup(BLUE ,RESET_REGS,
						   CLEAR_MEMORY,DEFINE_LUTS)) 
     					perror("Matrox VIP-1024 board initialization failed");
				if (!mpl_vip_setup(RED ,RESET_REGS,
						   CLEAR_MEMORY,DEFINE_LUTS)) 
     					perror("Matrox VIP-1024 board initialization failed");
				/* get the data from the frame buffers */
        			mpl_vip_read_rect(RED ,ic,ir,oc,or,red_fr);
        			mpl_vip_read_rect(GREEN ,ic,ir,oc,or,green_fr);
        			mpl_vip_read_rect(BLUE ,ic,ir,oc,or,blue_fr);

				if (format == MAC) {
					for (i=0; i<or*oc; i++) {
						putchar((int) (*red_fr) );
						putchar((int) (*green_fr) );
						putchar((int) (*blue_fr) );
						red_fr++;
						blue_fr++;
						green_fr++;
					}
				} else {
					init_header(&hd,"rframe","24 bit color image",3,"",or,oc,PFBYTE,3,"");
					update_header(&hd,argcc,argv);
					write_header(&hd);
					if (fwrite(red_fr,
					    or*oc*sizeof(unsigned char),
					    1,stdout) != 1)
						perr(HE_MSG,"error during write");
					if (fwrite(green_fr,or*oc*sizeof(unsigned char),1,stdout) != 1)
						perr(HE_MSG,"error during write");
					if (fwrite(blue_fr,or*oc*sizeof(unsigned char),1,stdout) != 1)
						perr(HE_MSG,"error during write");
				}

        			mpl_vip_close_device(GREEN );
        			mpl_vip_close_device(RED );
        			mpl_vip_close_device(BLUE );
	}
	return(0);
}
