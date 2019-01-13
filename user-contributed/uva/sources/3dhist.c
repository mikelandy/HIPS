/*      PROGRAM
 *              3dhist
 *
 *      SYNOPSIS
 *              3dhist [-bhrz] | pipeout
 *
 *      DESCRIPTION
 *              Creates three 256x256 hips output images representing
 *              the two dimensional histogram of each pair of images.
 *		Input images may be from a pipe.
 *              Input images may be any size but must match. For
 *              example, input images may be in register views of
 *              the same source, masked to a region of interest.
 *              Output image is in integer format, ready to be
 *              SCALEd as desired.
 *		By default, bin (0,0), which represents paired zeros
 *		from background, is set to zero so as not to tip
 *		the scaling.  The [-z] option activates bin (0,0)
 *		when zero suppression is not desired. 
 *		A boarder can be added at the edges of the histogram
 *		(default) or not added if the -b option is specified.
 *		By default, the origin of the histogram is located in the
 *		upper-left of the image (image location (0,0)).  The [-r]
 *		option rotates the histogram so that the origin is
 *		placed in the lower left corner.
 *
 *      AUTHOR
 *              Justin D. Pearlman, M.D. M.E.
 *		Stuart Ware, and Charles Carman 
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
	int zeroflag=0, helpflag=0, rotaflag=0, boardflag=0;
    	int h[SIDE][SIDE];
   	int row, col;
   	unsigned int size;
	register int i, j, *ph;
    	register unsigned char *pbx, *pby;
    	unsigned char *buf1, *buf2, *buf3;
    	struct header hd;
	void exit();

	Progname = strsave(*av);
	for (i=1;i<ac;i++)
	{
		if(av[i][0]=='-')
		{
			boardflag = (int)strchr(av[i],'b');
			helpflag = (int)strchr(av[i],'h');
			rotaflag = (int)strchr(av[i],'r');
			zeroflag = (int)strchr(av[i],'z');
		}
		else helpflag++;
	}	

    	if (helpflag)
	{
		fprintf(stderr,
		"%s [-bhrz] | pipe\n",av[0]);
		fprintf(stderr,"-b disables the addition of a boundary\n");
		fprintf(stderr,"-z allow zero bin in histogram\n");
		fprintf(stderr,"-r places the origin in the lower-left\n");
		fprintf(stderr,"-h gives this help message\n");
		exit(1);
	}

	/* Using standard input */
	read_header(&hd);
	if (hd.pixel_format != PFBYTE)
		perr(HE_MSG,"pixel_format must be BYTE");
	if (hd.num_frame!=3)
		perr(HE_MSG,"must be a sequence of 3 images");
	row = hd.orows;
	col = hd.ocols;
	size = row*col;
	buf1 = (unsigned char *)halloc(size,1);
	buf2 = (unsigned char *)halloc(size,1);
	buf3 = (unsigned char *)halloc(size,1);
	if (fread(buf1,row*col*sizeof(unsigned char),1,stdin) != 1)
		perr(HE_MSG,"cannot read images");
	if (fread(buf2,row*col*sizeof(unsigned char),1,stdin) != 1)
		perr(HE_MSG,"cannot read images");
	if (fread(buf3,row*col*sizeof(unsigned char),1,stdin) != 1)
		perr(HE_MSG,"cannot read images");

	hd.orows = hd.ocols = hd.rows = hd.cols = SIDE;
	hd.pixel_format = PFINT;
    	update_header(&hd,ac,av);
    	write_header(&hd);

    	/*Fill bins*/
	for (j=0; j<3; j++) {
		switch(j) {
		case 0:
			pbx = buf2;
			pby = buf1;
			break;
		case 1:
			pbx = buf3;
			pby = buf1;
			break;
		case 2:
			pbx = buf2;
			pby = buf3;
			break;
		}
		for (ph=(&h[0][0]),i=0;i<HSIZE;i++,ph++)
			*ph = 0;

    		if (rotaflag) {
			for (i=0;i<size;i++,pbx++,pby++)
        			++h[SIDE-*pbx-1][*pby];
		} else {
    			for (i=0;i<size;i++,pbx++,pby++)
        			++h[*pby][*pbx];
		}

		if (!zeroflag) {
			if (rotaflag) h[SIDE-1][0] = 0;
			else h[0][0]=0;
		}

		/* add a border */
		if (!boardflag) {
			for (i=0; i<SIDE; i++) {
				if (h[i][0] == 0) h[i][0]++;
				if (h[0][i] == 0) h[0][i]++;
				if (h[i][SIDE-1] == 0) h[i][SIDE-1]++;
				if (h[SIDE-1][i] == 0) h[SIDE-1][i]++;
			}
		}

    		/*Unload results*/
		if (fwrite(h,256*256*sizeof(int),1,stdout) != 1)
			perr(HE_MSG,"error writing 3d histogram file");
	}
	exit(0);
}
