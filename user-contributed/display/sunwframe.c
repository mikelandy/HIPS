/*	Copyright (c) 1982 Michael Landy, Yoav Cohen, and George Sperling

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/*
 * sunwframe.c - write a frame on the Adage
 *
 * Usage:	sunwframe [initialrow [initialcolumn]] < frame
 *
 * Defaults:	centered in the frame
 *
 * Load:	cc -o sunwframe sunwframe.c -lhips -lpixrect
 * for a 386i:  cc -DLSB -o sunwframe sunwframe.c -lhips -lpixrect
 *
 * Michael Landy - 6/22/85
 *
 * Revised for Sun 4 by David Burr - 10/10/88
 * Modified for binary, etc., Mike Landy - 10/24/88
 *
 * (initialrow, initialcolumn) specifies the screen position for 
 * frame coordinate (0,0).  Effective off-screen coordinates are lost,
 * so there is no wraparound.  This works for both binary and 8-bit images.
 * For 8-bit images, the program displays the image, then waits for a 
 * carriage-return, after which it restores a binary lookup table and
 * returns.
 */

#include <hipl_format.h>
#include <pixrect/pixrect_hs.h>
#include <stdio.h>
int binary = 0;


main(argc,argv)

int argc;
char **argv;

{
	int i,j,r,c,ir,ic,irb,x_lim,y_lim;
	char *fr,*ofr,*pfr,*pofr,tmp[100];
	unsigned char col[256],str[20];
	struct header hd;
	struct pixrect *screen,*mem_screen;
	FILE *fp;

	Progname = strsave(*argv);
	screen=pr_open("/dev/fb");
	x_lim=screen->pr_size.x;
	y_lim=screen->pr_size.y;
	read_header(&hd);
	r = hd.orows;
	c = hd.ocols;
	if(r>y_lim || c>x_lim) {
		sprintf(tmp,"image must be less than %d X %d",y_lim,x_lim);
		perr(HE_MSG,tmp);
	}
	ir=(y_lim-r)/2; ic=(x_lim-c)/2;
	if(argv[argc-1][0]=='-')argc--;
	if(argc>1) ir=atoi(argv[1]);
	if(argc>2) ic=atoi(argv[2]);
	if (hd.pixel_format != PFBYTE && hd.pixel_format != PFMSBF &&
	    hd.pixel_format != PFLSBF)
		perr(HE_MSG,"frame must be in byte format");
#ifdef LSB
	if (hd.pixel_format == PFMSBF)
		perr(HE_MSG,"Packed images must be LSBFIRST");
	else if (hd.pixel_format == PFLSBF)
#else
	if (hd.pixel_format == PFLSBF)
		perr(HE_MSG,"Packed images must be MSBFIRST");
	else if (hd.pixel_format == PFMSBF)
#endif
		binary++;
	if (!binary && ((screen -> pr_depth) < 8))
		perr(HE_MSG,"can't view an 8-bit image on a 1-bit display");
	irb = binary ? ((c+7)/8) : c;
	if (((fr = (char *) calloc(r*irb,sizeof(char)))  == 0))
		perr(HE_MSG,"can't allocate core");
	if (fread(fr,r*irb*sizeof(char),1,stdin) != 1)
		perr(HE_MSG,"error during read");

	if (irb % 2) {
		if (((ofr = (char *) calloc(r*(irb+1),sizeof(char)))  == 0))
			perr(HE_MSG,"can't allocate core");
		pfr = fr;
		pofr = ofr;
		for (i=0;i<r;i++) {
			for (j=0;j<irb;j++)
				*pofr++ = *pfr++;
			*pofr++ = 0;
		}
	}
	else
		ofr = fr;
	mem_screen=mem_point(c,r,binary ? 1 : 8,ofr);
	pr_rop(screen,ic,ir,c,r,PIX_SRC,mem_screen,0,0);
	if (!binary) {
		for(i=0;i<256;i++)		/* Black and white */ 
			col[i]=i;
		pr_putcolormap(screen,0,256,col,col,col);
		fp = fopen("/dev/tty","r");
		fgets(str,20,fp);
		fclose(fp);
		for(i=0;i<256;i++)		/* Black and white */ 
			col[i]=(i%2) ? 0 : 255;
		pr_putcolormap(screen,0,256,col,col,col);
	}
	pr_close(screen);
	return(0);
}
