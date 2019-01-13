/**************************************************************************
 *                                                                        * 
 * Function: averages()					                             *
 *                                                                        *
 * Usage:	averages [batch-length]  < inseq > outseq                       *
 * Returns:  none							                        *
 * Defaults: batch-length is the number of frames in inseq                *
 * Loads: cc -o -DULORIG averages averages.c -lhipl                       *
 * Modified:TK 2-XII-87                                                   *
 *          TK 7-XII -87                                                  *
 * Description:average batches of frames pixel-by-corresponding-pixel     *
 *             without normalization                                      *
 *             where batch-length is the length of subsequence to be      *
 *             averaged.                                                  *
 **************************************************************************
 *                    Copyright (c) 1987                                  *
 *                    Captain Chaos                                       *
 **************************************************************************
 */

#include <hipl_format.h>
#include <stdio.h>

int main(argc,argv)
int argc;
char *argv[] ;
{
	int	fr,r,c,i,j,l,n,num,len;
	struct	header hd;
	char	*ifr,*ofr,*ip,*op;
	int	*sum,*sp;

	Progname = strsave(*argv);
	/* initialise parameters from input header */
	read_header (&hd) ;
	/* check input in inseq format  */
	if(hd.pixel_format != PFBYTE) {
		fprintf(stderr,"average: pixel format must be byte\n");
		exit(1) ;
	}
	r = hd.orows ;
	c = hd.ocols ;
	fr = hd.num_frame;
	update_header (&hd,argc,argv) ;
	
	/* assign input arguments */
	if (argv[argc-1][0] == '-' && argv[argc-1][1] == 'D')
		argc--;
	if (argc > 1)
		len = atoi(argv[1]);
	else
		len = fr;
	/* check for batch-length and inseq length */
	if (len < 1 || len > fr) {
		fprintf(stderr,"average: unreasonable batch-length specified\n");
		exit(1);
	}
	/* update outseq header */
	num = fr / len;
	hd.num_frame = num;
	write_header (&hd) ;

	/* allocate memory for in-frame out-frame and summation of frames */
	if ((ifr = (char *) calloc(r*c,sizeof (char))) == 0 ||
	    (ofr = (char *) calloc(r*c,sizeof (char))) == 0 ||
	    (sum = (int *) calloc(r*c,sizeof (int))) == 0) {
		fprintf(stderr,"average: can't allocate core\n");
		exit(1);
	}
	/* read in frames into ifr store result in sum write out from ofr */
	for (n=0;n<num;n++) {	/* no. of batches */
		for (l=0;l<len;l++) {	/* frames per batch */
			if (fread(ifr,r*c*sizeof(char),1,stdin) != 1)
				perr(HE_MSG,"error during read");
			ip = ifr;
			sp = sum;
			if (l == 0) {
				for (i=0;i<r;i++)
					for (j=0;j<c;j++)
						*sp++ = *ip++ & 0377 ;
			}
			else	{
				for (i=0;i<r;i++) 
					for (j=0;j<c;j++) 
						*sp++ += *ip++ & 0377;
			}
		}
		sp = sum;
		op = ofr;
		for (i=0;i<r;i++) 
			for (j=0;j<c;j++) 
				*op++ = ((*sp++) / len);

		if (fwrite(ofr,r*c*sizeof(char),1,stdout) != 1)
			perr(HE_MSG,"error during write");
	}
	return(0);
}
