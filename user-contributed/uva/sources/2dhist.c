/*      PROGRAM
 *              2dhist
 *
 *      SYNOPSIS
 *              2dhist [<x_image><y_image>] [-zhb] | pipeout
 *
 *      DESCRIPTION
 *              Creates a 256x256 hips image output representing
 *              the two dimensional histogram of the image pair.
 *		Input images may be from a pipe or 2 specified files.
 *              Input images may be any size but must match. For
 *              example, input images may be in register views of
 *              the same source, masked to a region of interest.
 *              Output image is in integer format, ready to be
 *              SCALEd as desired.  By default, bin (0,0), which 
 *              represents paired zeros from background, is set
 *              to zero so as not to tip the scaling.  The [-z]
 *              option activates bin (0,0) when zero suppression
 *              is not desired.  A border is automatically added
 *		around the outside, inless the [-b] option is specified.
 *
 *      AUTHOR
 *              Justin D. Pearlman, M.D. M.E.
 *		Stuart Ware & Charles Carman
 *              for
 *              Merickel Imaging Laboratory
 *              Biomedical Engineering
 *              University of Virginia
 *              Charlottesville, VA 22908
 *
 */

#include <ctype.h>
#include <hipl_format.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>

#define SIDE	256
#define HSIZE   SIDE * SIDE

int main(ac,av)
int ac;
char *av[];
{
	char tmp[100];
	int sum, max;
	int zeroflag=0, helpflag=0, sequence=1, boardflag=0;
    	int h[SIDE][SIDE];
	FILE *fp1,*fp2;
   	int row, col;
   	unsigned int size;
	register int i, *ph;
    	register unsigned char *buf1, *buf2;
    	struct header hd;
	void exit();

	Progname = strsave(*av);
	for (i=1;i<ac;i++)
	{
		if(av[i][0]=='-')
		{
			boardflag = (int)strchr(av[i],'b');
			helpflag = (int)strchr(av[i],'h');
			zeroflag = (int)strchr(av[i],'z');
		}
		else
			sequence = 0;
	}	

    	if (helpflag)
	{
		fprintf(stderr,
		"%s [<x_image> <y_image>] [-zh] | pipe\n",av[0]);
		fprintf(stderr,"-z allow zero bin in histogram\n");
		fprintf(stderr,"-h gives this help message\n");
		exit(1);
	}

	/* Using standard input */
	if (sequence)
	{
		read_header(&hd);
		if (hd.pixel_format != PFBYTE)
			perr(HE_MSG,"pixel_format must be BYTE");
		if (hd.num_frame<2)
			perr(HE_MSG,"must have a sequence of images greater than 1");
		row = hd.orows;
		col = hd.ocols;
		size = row*col;
		buf1 = (unsigned char *)halloc(size,1);
		buf2 = (unsigned char *)halloc(size,1);
		if (fread(buf1,row*col*sizeof(unsigned char),1,stdin) != 1)
				perr(HE_MSG,"cannot read images");
		if (fread(buf2,row*col*sizeof(unsigned char),1,stdin) != 1)
				perr(HE_MSG,"cannot read images");
	}

	/* Using file names */
	else
	{
    		if ((fp1 = fopen(av[1],"r")) == NULL) {
			sprintf(tmp,"error opening %s",av[1]);
			perr(HE_MSG,tmp);
		}

    		fread_header(fp1, &hd, av[1]);

		/* Make sure this is correct format		*/
    		if (hd.pixel_format!=PFBYTE) {
        		sprintf(tmp,"must use PFBYTE format %s",av[1]);
			perr(HE_MSG,tmp);
		}

    		row = hd.orows; 
   		col = hd.ocols;

    		if ((fp2 = fopen(av[2],"r")) == NULL) {
			sprintf(tmp,"error opening %s",av[2]);
			perr(HE_MSG,tmp);
		}

    		fread_header(fp2, &hd, av[2]);

    		if (hd.pixel_format!=PFBYTE) {
        		sprintf(tmp,"must use PFBYTE format: %s",av[2]);
			perr(HE_MSG,tmp);
		}

		if (hd.ocols != col || hd.orows != row)
			perr(HE_MSG,"unequal image size");

		size = row*col;
		buf1 = (unsigned char *)halloc(size,1);
		buf2 = (unsigned char *)halloc(size,1);
		if (fread(buf1,row*col*sizeof(unsigned char),1,fp1) != 1)
			perr(HE_MSG,"cannot read images");
		if (fread(buf2,row*col*sizeof(unsigned char),1,fp2) != 1)
			perr(HE_MSG,"cannot read images");
	}

	/* Init bins */
	for (ph=h[0], i=0; i<HSIZE; i++) *ph++ = 0;

    	/*Fill bins*/
    	for (max=0,i=0;i<size;i++,buf1++,buf2++) 
	{
        	sum = ++h[*buf2][*buf1];
        	if (*buf1 != 0 || *buf2 != 0)
            		max = MAX(max, sum);
    	}

	if (zeroflag)
        	max = MAX(max, h[0][0]);
    	else 
		h[0][0]=0;

	/* add a border */
	if (!boardflag) {
	for (i=0; i<SIDE; i++) {
		if (h[i][0] == 0) h[i][0]++;
		if (h[0][i] == 0) h[0][i]++;
		if (h[i][SIDE-1] == 0) h[i][SIDE-1]++;
		if (h[SIDE-1][i] == 0) h[SIDE-1][i]++;
	} }

    	/*Unload results*/
	if (!sequence)
    		init_header(&hd,"","",1,"",SIDE,SIDE,PFINT,1,"");
	else {
		for (i=2; i<hd.num_frame; i++)
			fread(buf1,row*col*sizeof(unsigned char),1,stdin);
		hd.num_frame = 1;
		hd.rows = hd.cols = hd.orows = hd.ocols = SIDE;
		hd.pixel_format = PFINT;
	}
    	update_header(&hd,ac,av);
    	write_header(&hd);

	if (fwrite(h,SIDE*SIDE*sizeof(int),1,stdout) != 1)
		perr(HE_MSG,"error writing 2d histogram file");
	return(0);
}
