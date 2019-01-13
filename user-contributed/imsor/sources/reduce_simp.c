
/*      Copyright (c) 1992  Karsten Hartelius, IMSOR.

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   


 reduce_thin - removes every second row and coloumn from an HIPS-image.
		By default the first line and coloumn are kept. 
		
 to load:
	cc -o reduce_thin reduce_thin.c  -L /gsnk1/hips2/lib -lm -lhips

 to run:
     reduce_thin parameters < HIPS-file  > HIPS-file 

 include-files:  util.h
*/


#include <stdio.h>
#include <hipl_format.h>
#include "util.h"

int main(argc,argv)
int argc;
char *argv[];
{

	Dvector	in, out, outptr;	
	struct 	header hd;
	int		i, j, k, 
			format, 		/* format of input-sequence */
			frames, 		/* number of frames in input */
			rows, 		/* number of rows in input */
			cols, 		/* number of coloumns in input */
			Npix,		/* number of pixels in input */
			rows2, 		/* number of rows in output */
			cols2, 		/* number of coloumns in output */
			Npix2, 		/* number of pixels in output */
			hfactor, 		/* horizontal reduction-factor */
			vfactor, 		/* vertical reduction-factor */
			hfirst, 		/* first coloumn to be included  in output*/
			vfirst;		/* first row to be included in output */

	/* default definitions */
	hfactor = vfactor = 2;
	hfirst = vfirst = 0;
	for (i=1;i<argc;i++) if (argv[i][0]=='-')
	switch (argv[i][1]){
		case 'h':	hfactor=atoi(argv[++i]);
				vfactor=1;
				if ((i+1<argc) && (argv[i+1][0]!='-'))
					hfirst=atoi(argv[++i]);
				break;
		case 'v': vfactor=atoi(argv[++i]);
				hfactor=1;
				if ((i+1<argc) && (argv[i+1][0]!='-'))
					vfirst=atoi(argv[++i]);
				break;
		case 's': hfactor=vfactor=atoi(argv[++i]);
				if ((i+1<argc) && (argv[i+1][0]!='-'))
					hfirst=vfirst=atoi(argv[++i]);
				break;
		default : fprintf(stderr,"usage: reduce_simp [-h][-v][-s] <i >o");
		}
				
	read_header(&hd);
	Progname=strsave(*argv);
	frames=hd.num_frame;
	format=hd.pixel_format;
	rows=hd.orows;
	cols=hd.ocols;
	Npix=rows*cols;
	cols2 = (cols - hfirst - 1)/hfactor + 1; 
	rows2 = (rows - vfirst - 1)/vfactor + 1; 
	Npix2=rows2*cols2;

	hd.orows=rows2;
	hd.ocols=cols2;
	hd.rows=rows2;
	hd.cols=cols2;
	update_header(&hd,argc,argv);
	write_header(&hd);
	in = dvector(Npix);
	out = dvector(Npix2);

	for (k=0;k<frames;k++){
		fread_to_dvec(stdin,in,Npix,format);
		outptr=out;
		for (i=0;i<rows2;i++) for (j=0;j<cols2;j++)
			*(outptr++) = in[(vfactor*i+vfirst)*cols + (hfactor*j+hfirst)];
		fwrite_from_dvec(stdout,out,Npix2,format);
		}
	return(0);
}
		
