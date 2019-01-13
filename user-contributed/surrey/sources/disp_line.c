/**************************************************************************
 *                                                                        * 
 * Function: disp_line()					                             *
 *                                                                        *
 * Usage:	disp_line [-m]  < hist > graphimage                             *
 * Returns:  none							                        *
 * Defaults: maximum is the first histogram bin used                      *
 * Loads: cc -o -DULORIG disp_line disp_line.c -lhipl                     *
 * Modified:TK 2-IX-87                                                    *
 *          TK 7-IX -87                                                   *
 * Description:display grey-level histograms for a single line histogram  *
 *             created by histo_line, may be row or column histogram      *
 *             -m flag specifies an initial maximum bincount for use in   *
 *             scaling the displays.Max line dimensions 512x1.            *
 **************************************************************************
 *                    Copyright (c) 1987                                  *
 *                    Captain Chaos                                       *
 **************************************************************************
 */

#include <hipl_format.h>
#include <stdio.h>

int mflag = 0;
int vflag = 0;
int maxcnt = 1;

int main(argc,argv)
int argc;
char *argv[] ;
{
	int i,j,k,r,c,f,fr,binleft,numbin,binwidth,*hist,*hp,one=1;
	char out[256*512],*op;         /* max line length 512x1 */
	struct header hd;

	Progname = strsave(*argv);
	/* initialise parameters from input header */
	read_header(&hd);
	r=hd.orows;
	c=hd.ocols;
	f=hd.num_frame;
	update_header(&hd,argc,argv);

	/* assign input arguments */
	while (--argc > 0) {
		argv++;
		if (argv[0][0] == '-') {
			switch(argv[0][1]) {
			case 'D':
				continue;
			case 'm':
				mflag++;
				maxcnt = atoi(*(++argv));
				continue;
			default:
				fprintf(stderr,"disphist: unknown switch %s\n",
					argv[0]);
				exit(1);
			}
		}
	}

	/* check input in histo format set output byte format */
	if (hd.pixel_format != PFHIST)
		perr(HE_MSG,"image must be in byte histogram format");
	hd.pixel_format = PFBYTE;
	/* check for row or column line input */
	if(r != 1 && c !=1)
		perr(HE_MSG,"image must contain only one line");
	if(c == 1 ) {
		hd.ocols = hd.cols = r;
		vflag++;
	}
	hd.orows = hd.rows = 256;

	getparam(&hd,"imagepixfmt",PFINT,&one,&i);
	if (i != PFBYTE)
		perr(HE_MSG,"image format has to have been byte");
	getparam(&hd,"numbin",PFINT,&one,&numbin);
	getparam(&hd,"binleft",PFINT,&one,&binleft);
/*	getparam(&hd,"binwidth",PFINT,&one,&i); */

	binwidth = hd.ocols/numbin;
	fprintf(stderr,"numbin: %d\n",numbin);
	fprintf(stderr,"hd.ocols: %d\n",hd.ocols);
	if (binwidth != 1 )
		perr(HE_MSG,"strange number of bins!!?");

	write_header(&hd);

	if ((hist = (int *) calloc(numbin+2,sizeof(int))) == 0)
		perr(HE_MSG,"can't allocate core");

	if (vflag)
		fprintf(stderr,"disp_line: column size was %d x %d\n",r,c);
  	else
		fprintf(stderr,"disp_line: row size was %d x %d\n",r,c);

	fprintf(stderr,"disp_line: first bin starts at %d, and there are %d bins\n",binleft,numbin);
	
	for(fr=0;fr<f;fr++) { 	/* draw border and clear bins for each frame*/
		op = out;
		for (i=0;i<256;i++)
			for (j=0;j<numbin;j++) {
				if (j==0 || j==(numbin - 1) || i==0 || i==255)
					*op++ = 127;
				else
					*op++ = 0;
			}
		if (fread(hist,numbin*sizeof(int),1,stdin) != 1)
			perr(HE_MSG,"error during read");
		/* print underflow/overflow here, when add this */
		hp = hist+1;
		j = 0;
		for (i=0;i<numbin;i++) {
			if (*hp > maxcnt) {
				j++;
				maxcnt = *hp++;
			}
			else
				hp++;
		}
		if (j)
			fprintf(stderr,"disphist: frame %d maxcnt increased to %d\n",
				fr,maxcnt);
		for (i=0;i<numbin;i++) {
			j = hist[i+1]*254/maxcnt;
#ifdef ULORIG
			op = out + i + 255*numbin;
#else
			op = out + i + numbin;
#endif
			for (k=0;k<j;k++) {
				*op = 255;		/* set value to 255(white) */
#ifdef ULORIG
				op -= numbin;		/* move pointer backwards */
#else
				op += numbin;		/* or forwards in out[]   */
#endif
			}
		}
		if (fwrite(out,256*numbin*sizeof(char),1,stdout) != 1)
			perr(HE_MSG,"error during write");
	}
}
