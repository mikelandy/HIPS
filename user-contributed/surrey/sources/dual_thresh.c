/**************************************************************************
 *                                                                        * 
 * Function: dual_thresh.c()                                              *
 * Usage:dual_thresh [-pP] [-gN] < inseq byte> outseq byte                *
 * Returns:  none                                                         *
 * Defaults:Thresh percentage 10: greyscale output                        *
 * Loads: cc -o -DDG dual_thresh dual_thresh.c -lhipl                     *
 * Modified:TK 2-III-88                                                   *
 *                                                                        *
 * Description:Dual threshold function for a single frame                 *
 *             compute the histogram of values in a frame  calculate the  *
 *             two intensity values which give N% of pixels above, and N% *
 *             of pixels below, apply dual threshold to the frame.        *
 *             -p upper and lower percentage value P on the histogram     *
 *             -g set middle greyscale values to constant value N         *
 *                if not set output is greyscale                          *
 **************************************************************************
 *                    Copyright (c) 1988                                  *
 *                    Captain Chaos                                       *
 **************************************************************************
 */
#include <hipl_format.h>
#include <stdio.h>

int pflag = 0;int gflag = 0;
int hchar = 255,lchar = 0;

int main(argc,argv)
int argc;
char *argv[] ;
{
	int k,r,c,fr,f,ii,mchar,fpix,min,max,sum,val;
	int percent,lthresh,uthresh,hist[1001],upper,lower;	
	char *inpic,*tin,*outpic,*tout,tmp[100];
	struct header hd;

	Progname = strsave(*argv);
	/* initialise parameters from input header */
	read_header(&hd);
	update_header(&hd,argc,argv);
	r=hd.orows; c=hd.ocols; fr=hd.num_frame;fpix = r * c; 
	if (hd.pixel_format != PFBYTE)
		perr(HE_MSG,"pixel format must be byte");	
	/* assign input arguments */
	while (--argc > 0) {                  /* while arguments remain */
		if ((*++argv)[0] == '-') {       /* check first char of first argv*/
			switch((*argv)[1]) {
			case 'D':
				continue;
			case 'p':
				pflag++;               
				(*argv)[0] = (*argv)[1] = '0';
				percent = atoi(*argv); continue;
			case 'g':
				gflag++;               
				(*argv)[0] = (*argv)[1] = '0';
				mchar = atoi(*argv); continue;
			default:
				sprintf(tmp,"unknown switch %s\n",argv[0]);
				perr(HE_MSG,tmp);
			}
		}
	}
	/* set all default values and check input parameters */
	if (!pflag) percent = 10;		
	else if ((percent) >= 50)
		perr(HE_MSG," value of percent outside limits :use 0 -> 49%");
	fprintf(stderr,"dual_thresh:frame size: %d x %d\n",r,c);
	fprintf(stderr,"dual_thresh:thresh vals:%d & %d\n",percent,(100-percent));
	write_header(&hd);

	/* memory allocation for frame in & out and window */
	if ((inpic = (char *) calloc(r*c,sizeof (char))) == 0 ||
	    (outpic = (char *) calloc(r*c,sizeof (char))) == 0 )
		perr(HE_MSG,"can't allocate core ");
	
     /* for each frame in inseq */
	for (f=0;f < fr;f++) {
		if (fread(inpic,r*c*sizeof(char),1,stdin) != 1) 
			perr(HE_MSG,"error during read ");
		tin = inpic; tout = outpic;
		/* calculate the int histogram for frame */
		for (ii=0;ii <= 255;ii++) hist[ii] = 0;
		sum = 0;
		for (ii=0;ii < fpix;ii++){
			val = *tin++ & 0377;
			hist[val]++;
			sum += val ;
			if (ii == 0) min = max = val;
			else if (val > max) max = val;
			else if (val < min) min = val;
		} 
		/* find upper and lower threshold limits */
		lower = percent * sum / 100;
		upper = (100 - percent) * sum / 100;
		k = 0 ;
		for (ii=min;ii < max;ii++) {	
			k += ( ii * hist[ii]);
			if (k >= upper) break;
		}
		uthresh = ii; k = 0;
		for (ii=min;ii < max;ii++) {	
			k += ( ii * hist[ii]);
			if (k >= lower) break;
		}
		lthresh = ii; tin = inpic;	
		/* apply threshold to window */
		for (ii=0;ii < fpix;ii++) {	
			val = *tin++ & 0377;
			if (val <= lthresh) val = lchar;
			else if (val >= uthresh) val = hchar;
			else if(gflag) val = mchar;
			*tout++ = val;
		} 
	}
	if (fwrite(outpic,fpix*sizeof(char),1,stdout) != 1)
		perr(HE_MSG,"error during write");
	return(0);
}
