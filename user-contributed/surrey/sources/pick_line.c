/**************************************************************************
 *                                                                        * 
 * Function: pick_line()					                             *
 *                                                                        *
 * Usage:	pick_line [-nN] [-v] <inseq >outseq                             *
 * Returns:  none							                        *
 * Defaults: line is a row: line positioned in the middle of th image     *
 * Loads: cc -o  pick_line pick_line.c -lhipl                             *
 * Modified:TK 7-IX-87                                                    *
 *                                                                        *
 * Description:to pick a line out from byte-formatted images.             *
 *             -n specifies the position of the line to be picked out.    *
 *             -v specifies a column is picked from an image.             *
 *             output is in byte format with header updated to reflect    *
 *             the size of the line.                                      * 
 **************************************************************************
 *                    Copyright (c) 1987                                  *
 *                    Captain Chaos                                       *
 **************************************************************************
 */

#include <hipl_format.h>
#include <stdio.h>

int vflag = 0;
#define N   512				/* max. size of array */


int main(argc,argv)
int argc;
char *argv[] ;
{
	int		fr,f,r,c,argcp;
	int		i,n=0,nn;
	char		*frci,*frcip,*frco,*frcop;
	struct	header	hd;

	Progname = strsave(*argv);
	/* initialise parameters from input header */
	read_header (&hd) ;
	if(hd.pixel_format != PFBYTE) 
	{
		fprintf(stderr,"pick_line: pixel format must be byte\n");
		exit(1) ;
	}
	r = hd.orows ;
	c = hd.ocols ;
	f = hd.num_frame;
	update_header (&hd,argc,argv) ;

	/* assign input arguments */
	argcp = argc;
	while (--argcp > 0)
		switch (argv[argcp][1])
		{
		case 'n':
			if (n)
			{
				fprintf(stderr,"command usage: pick_line [-nN]\n");
				exit(1);
			}
			else 
			{
				argv[argcp][0] = argv[argcp][1] = '0';
				nn = atoi(argv[argcp]);
				n = 1;
			} 
		continue;
		case 'v':
			vflag++;               /* set col. flag to 1 (on) */
			continue;
		default:
				fprintf(stderr,"command usage: pick_line [-nN]\n");
				exit(1);
		}

	/* check for input image max 512x512 */
	if(r > N || c > N) 
	{
		fprintf(stderr,"pick_line: size of input image too large\n");
		exit(1) ;
	}

	/* check for row or column pick */
	if (!vflag){
		if(!n ) nn = r / 2;
		if(nn < 0 || nn > r - 1){
			fprintf(stderr, "pick_line:  row outside of image\n");
			exit(1);
		}
    	 	hd.orows = hd.rows = 1;
	 }
	if (vflag){
		if(!n ) nn = c / 2;
		if(nn < 0 || nn > c - 1){
			fprintf(stderr, "pick_line:  column outside of image\n");
			exit(1);
		}
    	 	hd.ocols = hd.cols = 1;
	 }


	write_header (&hd) ;

	if (!vflag){
		if ((frci = (char *) calloc(r*c,sizeof (char))) == 0 || 
		    (frco = (char *) calloc(c,sizeof (char))) == 0) 
		{
			fprintf(stderr,"pick_line: can't allocate core\n");
			exit(1);
		}
	 }

	if (vflag){
		if ((frci = (char *) calloc(r*c,sizeof (char))) == 0 || 
		    (frco = (char *) calloc(r,sizeof (char))) == 0) 
		{
			fprintf(stderr,"pick_line: can't allocate core\n");
			exit(1);
		}
	 }

	/* processing begins pick line  for each frame*/

	for(fr=0;fr<f;fr++) { 	
		if (fread(frci,r*c*sizeof(char),1,stdin) != 1) {
			fprintf(stderr,"pick_line: error during read\n");
			exit(1);
		}
		if (!vflag){	/* read pixel values from row nn*c */
			for(i = 0,frcip = frci + c * nn,frcop = frco; i < c; i++)
				*frcop++ = *frcip++;	/* inc. both input and output */
	
			if (fwrite(frco,c*sizeof(char),1,stdout) != 1)
				perr(HE_MSG,"error during write");
		}
		if (vflag){	/* read pixel values from col nn and then succ. cols */
			for(i = 0,frcip = frci + nn,frcop = frco; i < r; i++){
				*frcop = *frcip;
				frcip += c;		/* jump to next column position */
				frcop++;			/* increment output line */
			}
			if (fwrite(frco,r*sizeof(char),1,stdout) != 1) 
				perr(HE_MSG,"error during write");
		}
	}
	return(0);
}
