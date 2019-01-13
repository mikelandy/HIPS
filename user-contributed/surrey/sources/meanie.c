/**************************************************************************
 * Function: meanie.c()                                                   *
 * Usage:meanie [-sN]  < inseq byte> outseq byte  	                   *
 * Returns:  none                                                         *
 * Defaults: size 3:                                                      *
 * Loads: cc -o -DDG meanie meanie.c -lhipl                               *
 * Modified:TK 22-IV-88                                                   *
 *                                                                        *
 * Description:Apply a mean filter to an image                            *
 *             -s side of the widow on each pixel value ( 3 ,5 ,7 or 9)   *
 **************************************************************************
 *                    Copyright (c) 1988                                  *
 *                    Captain Chaos                                       *
 **************************************************************************
 */
#include <hipl_format.h>
#include <stdio.h>

int sflag = 0; 
int hval = 255,lval = 0;

int main(argc,argv)
int argc;
char *argv[] ;
{
	int i,j,r,c,fr,f,ii,jj;
	int size,sizesq,ir,ic,sum;
	int minus,plus,top,bot,left,right;
	char *inpic,*tin,*ttin,*outpic,*tout;
	struct header hd;

	Progname = strsave(*argv);
	/* initialise parameters from input header */
	read_header(&hd);
 	update_header(&hd,argc,argv);
	r=hd.orows; c=hd.ocols; fr=hd.num_frame; 
	if (hd.pixel_format != PFBYTE)
		perr(HE_MSG,"pixel format must be byte");	

	/* assign input arguments */
	while (--argc > 0) {                  /* while arguments remain */
		if ((*++argv)[0] == '-') {       /* check first char of first argv*/
			switch((*argv)[1]) {
			case 'D':
				continue;
			case 's':
				sflag++;               
				(*argv)[0] = (*argv)[1] = '0';
				size = atoi(*argv); continue;       
			default:
				fprintf(stderr,"Max_min: unknown switch %s\n",argv[0]);
				exit(1);
			}
		}
	}
	/* set all default values and check input parameters */
	if (!sflag) size = 3;
	else if (size < 3 || size > 9)
		perr(HE_MSG," window size should be 3, 5, 7 or 9 ");
	write_header (&hd) ;
	
	/* set parameters for window */
	sizesq = size*size;
	plus = size / 2; minus = plus - size + 1;
	top = -minus; bot = r - plus;
	left = -minus; right = c - plus;
	/* memory allocation for frame in & out and window */
	if ((inpic = (char *) calloc(r*c,sizeof (char))) == 0||
	    (outpic = (char *) calloc(r*c,sizeof (char))) == 0) 
		perr(HE_MSG,"can't allocate core ");
     /* for each frame in inseq */
	for (f=0;f<fr;f++) {
		if (fread(inpic,r*c*sizeof(char),1,stdin) != 1) 
			perr(HE_MSG,"error during read ");
		tin = inpic;tout = outpic;
		/* place window centre at every pixel in the frame */
		for (i=0;i<r;i++) {
			for (j=0;j<c;j++) {
				sum = 0;
				if (i<top || i>=bot || j<left || j>=right) {
					for (ii=minus;ii<=plus;ii++){
						for (jj=minus;jj<=plus;jj++) {
						    ir = i + ii;
						    ic = j + jj;
						    ir = ir<0?0:(ir>=r)?r-1:ir;
						    ic = ic<0?0:(ic>=c)?c-1:ic;
						    sum += inpic[ir*c+ic] & 0377;
						}
					}
				}
				else {
					ttin = tin + minus*c + minus;
					for (ii=minus;ii<=plus;ii++) {
						for (jj=minus;jj<=plus;jj++){
							sum += *ttin++ & 0377;
							}
						ttin += c - size;
					}
				}
				*tout++ = (sum / sizesq) ; tin++ ;
			}
		}
		if (fwrite(outpic,r*c*sizeof(char),1,stdout) != 1) 
			perr(HE_MSG,"error during write");
	}
	return(0);
}
