/*	PROGRAM
 *		shrink
 *
 *	PURPOSE
 *		to shrink an image by one quarter (i.e., 1/2 in
 *		the x and 1/2 in the y directions) by median filtering.  
 *
 *	SYNOPSIS
 *		shrink [-tz] < filename
 *	-  the input sequence is read from stdin and the result goes out 
 *		stdout
 *      -  the shrinking operation is performed by calculating the median
 *	   	value of 2x2 neighborhoods 
 *	the -t flag is specified if white text is to be shrunk correctly
 *	the -z flag is specified if ROIs are to be processed correctly
 *
 *	AUTHOR
 *		Charles Carman
 *		for
 *		Merickel Imaging Lab
 *		Biomedical Engineering
 *		University of Virginia
 *		Charlottesville, VA   22903 
 *
 */
#include <stdio.h>
#include <hipl_format.h>
#define WHITE 255
#define BLACK 0
#define NOT_BLACK 1

int t_flg, z_flg;
void srt(),shrnkr();

int main (argc, argv)
	int argc;
	char *argv[];
{
	struct header hd;
	int inrows, incols, insize, outrows, outcols, outsize;
	int i, m;
	char *pobuf, *pibuf, *outbuf, *inbuf,tmp[100];

	Progname = strsave(*argv);
	if (argc > 2) {
		sprintf(tmp,"Syntax: %s [-tz]",argv[0]);
		perr(HE_MSG,tmp);
	}
	t_flg = z_flg = 0;
	if (argc == 2 && argv[1][0] == '-') {
		switch (argv[1][1]) {
		case 't':
			t_flg++;
			break;
		case 'z':
			z_flg++;
			break;
		default:
			sprintf(tmp,"Syntax: %s [-tz]",argv[0]);
			perr(HE_MSG,tmp);
		}
	}

	/* read in the header (structure) information */
	read_header(&hd);

	if (hd.pixel_format != PFBYTE)
		perr(HE_MSG,"input must be a byte sequence");

	/* get the needed data from it */
	inrows = hd.orows;
	incols = hd.ocols;
	insize = inrows * incols;
	
	/* set up the output header */
	outrows = hd.orows = hd.rows = inrows/2;
	outcols = hd.ocols = hd.cols = incols/2;
	outsize = outrows * outcols;

	update_header(&hd,argc,argv);

	/* write the new header (structure) */
	write_header(&hd);

	inbuf = (char *) halloc((unsigned) insize,1);
	outbuf = (char *) halloc((unsigned) outsize,1);
	/* main loop */
    for (m=0; m<hd.num_frame; m++)  {
	/* read in the input frame */
	if (fread(inbuf, insize,1,stdin) != 1)
		perr(HE_MSG,"invalid read");
	pibuf = inbuf; pobuf = outbuf;
	for (i=0; i<outrows; i++, pibuf+=2*incols, pobuf+=outcols)  {
		/* run the shrinker down the rows */
		shrnkr(pibuf, pobuf, incols, outcols);
	}
	/* write out the file */
	if (fwrite(outbuf, outsize,1,stdout) != 1)
		perr(HE_MSG,"invalid write");
    }
}

/* subroutine to shrink the large image by 1/4 using a median replacement */

void shrnkr (pinb, poutb, incols, outcols)
	int incols, outcols;
	char *poutb, *pinb;
{
	int i, k, sum;
	unsigned char temp[5];
	register unsigned char *pob, *pib1, *pib2, *ptemp;

	/* initialize the pointer arrays */
	pob = (unsigned char *) poutb;
	pib1 = (unsigned char *) pinb;
	pib2 = (unsigned char *) pinb+incols;

	/* do once for each column in the output image */
	for (k=0; k<outcols; k++)  {
		/* put the four elements into temp */
		for (ptemp=temp, i=0; i<2; i++)  {
			*ptemp++ = *pib1++;
			*ptemp++ = *pib2++;
		}
		/* calculate the average */
		for (i=0, sum=2; i<4; i++)
			sum += temp[i];
		temp[4] = sum / 4;
		/* sort the temp array */
		srt(temp, 5);
		/* output the median value */
		if (t_flg && temp[4] == WHITE) temp[2] = WHITE;
		if (z_flg && temp[2] == BLACK && temp[4] != BLACK)
			temp[2] = NOT_BLACK;
		*pob++ = temp[2];
	}
}

/* subroutine to sort the temp array */

void srt (ibuf, numb)
	int numb;
	unsigned char *ibuf;
{
	register unsigned char *pta, *ptb, temp;
	register int i, j;

	/* sort the temp array */
	for (i=0, pta=ibuf; i<numb-1; i++, pta++)  {
		for (j=i+1, ptb=pta+1; j<numb; j++, ptb++)  {
			if (*pta > *ptb)  {
				temp = *ptb;
				*ptb = *pta;
				*pta = temp;
			}
		}
	}
}
