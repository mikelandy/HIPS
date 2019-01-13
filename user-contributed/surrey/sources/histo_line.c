/**************************************************************************
 *                                                                        * 
 * Function: histo_line()					                             *
 *                                                                        *
 * Usage:	histo_line [-v] [-c] <inseq >outhist                             *
 * Returns:  none							                        *
 * Defaults: row histogram : single image output:                         *
 * Loads: cc -o -DDG histo_line histo_line.c -lhipl                         *
 * Modified:TK 2-IX-87                                                    *
 *                                                                        *
 * Description:compute grey-level histograms for byte-formatted images    *
 *             -v specifies that the histogram be made from a column input*
 *             -c causes multiple frame sequences to collapse into a      *
 *             single normalised histogram,instead of a separate histogram*
 *             being generated for each input frame.                      * 
 **************************************************************************
 *                    Copyright (c) 1987                                  *
 *                    Captain Chaos                                       *
 **************************************************************************
 */

#include <hipl_format.h>
#include <stdio.h>

int cflag = 0;
int vflag = 0;

int main(argc,argv)
int argc;
char *argv[] ;
{
	int i,j,r,c,f,fr,numbin,*hist;
	char *ifr,*ip;
	struct header hd;

	Progname = strsave(*argv);
	/* initialise parameters from input header */
	read_header(&hd);
	r=hd.orows;
	c=hd.ocols;
	f=hd.num_frame;

	update_header(&hd,argc,argv);

	/* assign input arguments */
	while (--argc > 0) {                  /* while arguments remain */
		argv++;
		if (argv[0][0] == '-') {         /* check first char of first argv*/
			switch(argv[0][1]) {
			case 'D':
				continue;
			case 'v':
				vflag++;               /* set col. flag to 1 (on) */
				continue;
			case 'c':
				cflag++;               /* set compact flag to 1 (on) */
				continue;
			default:
				fprintf(stderr,"histo_line: unknown switch %s\n",
					argv[0]);
				exit(1);
			}
		}
	}

	/* check for single line input */
	if (!vflag){
		if(r != 1 ){
			fprintf(stderr,"histo_line: input must contain one row\n");
			exit(1) ;
	    	 }
	    	 else {
	    	 	hd.orows = hd.rows = 1;
	    	 	numbin = c;
	    	 }
	 }
	if (vflag){
		if(c != 1 ){
			fprintf(stderr,"histo_line: input must contain one column\n");
			exit(1) ;
	    	 }
	    	 else {
	    	 	hd.ocols = hd.cols = 1;
	    	 	numbin = r;
	    	 }
	 }

	/* check input in byte format set output pixel format */
	if (hd.pixel_format != PFBYTE)
		perr(HE_MSG,"image must be in byte format");
	hd.pixel_format = PFHIST;
	if (cflag)
		hd.num_frame = 1;
	setparam(&hd,"numbin",PFINT,1,numbin);
	setparam(&hd,"imagepixfmt",PFINT,1,PFBYTE);
	setparam(&hd,"binleft",PFINT,1,0);
	setparam(&hd,"binwidth",PFINT,1,1);
	write_header(&hd);

	if ((hist = (int *) calloc(numbin+2,sizeof(int))) == 0 ||
		(ifr = (char *) calloc(numbin,sizeof(char))) == 0)
		perr(HE_MSG,"can't allocate core");

	/* for each frame in the sequence */
	for(fr=0;fr<f;fr++) { 
		if (fread(ifr,numbin*sizeof(char),1,stdin) != 1)
			perr(HE_MSG,"error during read");
		ip = ifr;
		for (i=0;i<numbin;i++) {
			j = *ip++ & 0377;
			hist[i+1] += j ;
				/* DEPENDS ON CALLOC INITIALIZING hist TO 0 */
		}
		if (!cflag) {
			if (fwrite(hist,(numbin+2)*sizeof(int),1,stdout) != 1)
					perr(HE_MSG,"error during write");
			for (i=0;i<numbin;i++)
				hist[i+1] = 0;
		}
	}
	if (cflag) {
		for (i=0;i<numbin;i++)
			hist[i+1] = hist[i+1]/fr;
					/* normalise the collapsed histo */
		if (fwrite(hist,(numbin+2)*sizeof(int),1,stdout) != 1)
				perr(HE_MSG,"error during write");
	}
}
