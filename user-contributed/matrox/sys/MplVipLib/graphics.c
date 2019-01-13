/*      Copyright (c) 1987, 1988 UCLA Machine Perception Laboratory
Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.  
*/


#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sundev/vipreg.h>

#include <errno.h>

#define FALSE		0
#define TRUE		1

#define DISPLAY_WIDTH	512
#define DISPLAY_HEIGHT	480

#define MIN_START_LOC	0
#define MAX_START_LOC	255
#define MIN_LUT_SIZE	1
#define MAX_LUT_SIZE	256

#define GREEN_LUT	0
#define RED_LUT		1
#define BLUE_LUT	2
#define INPUT_LUT	3

#define WRITE_TO_LUT	0

#define READ_FROM_LUT	1

extern int vip_fd[3];
extern unsigned char *vip_base[3];


mpl_vip_write_vert_line(board, line_color, xstart, ystart, length)
int board;
int line_color;
int xstart, ystart, length;
{
   unsigned char *optr;
   int i, j;

   if (xstart < DISPLAY_WIDTH) {
        optr = vip_base[board] + ( ystart * DISPLAY_WIDTH) + xstart;
	j = ystart + length;
        for (i=ystart; i < j; i++) {
            if ( i < DISPLAY_HEIGHT ) {
                 *optr = line_color;
                 optr += DISPLAY_WIDTH;
	    }
	}
   }

}


mpl_vip_write_line(board, input_data, xstart, ystart, nbytes)
int board;
unsigned char *input_data;
int xstart, ystart, nbytes;
{
   unsigned char *optr;
   int i;

   if (ystart < DISPLAY_HEIGHT)
      {
        optr = vip_base[board] + ( ystart * DISPLAY_WIDTH) + xstart;
        for (i=0; i < nbytes; i++)
            if ( (xstart + i) < DISPLAY_WIDTH )
                 *optr++ = *input_data++;
      }

}

mpl_vip_write_rect(board, xstart, ystart, xlen, ylen, input_data)
int board;
int xstart, ystart, xlen, ylen;
unsigned char *input_data;
{
   unsigned char *optr;
   int i;

   optr = input_data;
   for (i=ystart; i < (ystart+ ylen); i++) {
        mpl_vip_write_line(board,optr, xstart, i, xlen);
        optr += xlen;
   }
}

mpl_vip_read_line(board, output_data, xstart, ystart, nbytes)
int board;
unsigned char *output_data;
int xstart, ystart, nbytes;
{
   unsigned char *iptr;
   int i;

   if (ystart < DISPLAY_HEIGHT)
      {
        iptr = vip_base[board] + ( ystart * DISPLAY_WIDTH) + xstart;
        for (i=0; i < nbytes; i++)
            if ( (xstart + i) < DISPLAY_WIDTH )
                 *output_data++ = *iptr++;
            else *output_data++ = 0;
      }

}

mpl_vip_read_rect(board, xstart, ystart, xlen, ylen, output_data)
int board;
int xstart, ystart, xlen, ylen;
unsigned char *output_data;
{
   unsigned char *optr;
   int i;

   optr = output_data;
   for (i=ystart; i < (ystart+ ylen); i++) {
        mpl_vip_read_line(board,optr, xstart, i, xlen);
        optr += xlen;
   }
}




int mpl_vip_access_lut(board,read_write_flag,lut,start,count,lut_data)
int board;	/* which vip board */
int read_write_flag; /* read or write to lut */
int lut;	/* which lut */
int start;	/* specifies where in the lut table you are going to */
		/* start writing the lut entries                     */
int count;	/* how many entries are to be written via this call  */
unsigned char lut_data[];	/* lut entries */
{

	int w;
	long l,f;

	if ( (count < MIN_LUT_SIZE) || (count > MAX_LUT_SIZE) ) 
           {
		fprintf(stderr,"mpl_vip_access_lut: Illegal number of lut entries.\n Maximum number of lut entries is %d, you specified %d\n",MAX_LUT_SIZE,count);
		return(FALSE);
	   };

	if ( (start < MIN_START_LOC) || (start > MAX_START_LOC) ) 
           {
		fprintf(stderr,"mpl_vip_access_lut: Illegal starting location for lut entries.\n Starting location must be in the range of 0 to 255\n. You specified %d\n",start);
		return(FALSE);
	   };

	if ( (start + count) > MAX_LUT_SIZE) 
           {
		fprintf(stderr,"mpl_vip_access_lut: Not enough room in lut table to define %d entries starting from position %d\n",count,start);
		return(FALSE);
	   };

	switch(lut) /* which lut */
	 {
		case GREEN_LUT:	l = lseek(vip_fd[board], VIP_GREEN_LUT + start, L_SET);
				f = VIP_GREEN_LUT + start;
				if (l == -1)
					{
					fprintf(stderr,"mpl_vip_access_lut: failed to lseek <%ld> bytes into lut <%d> on board <%d>.",f,lut,board);
					return(FALSE);
					}

				if (read_write_flag == WRITE_TO_LUT)
					w = write(vip_fd[board],lut_data,count);
				else w = read(vip_fd[board],lut_data,count);
				break;

		case RED_LUT:	l = lseek(vip_fd[board], VIP_RED_LUT + start, L_SET);
				f = VIP_RED_LUT + start;
				if (l == -1)
					{
					fprintf(stderr,"mpl_vip_access_lut: failed to lseek <%ld> bytes into lut <%d> on board <%d>.",f,lut,board);
					return(FALSE);
					}

				if (read_write_flag == WRITE_TO_LUT)
					w = write(vip_fd[board],lut_data,count);
				else w = read(vip_fd[board],lut_data,count);
				break;

		case BLUE_LUT:	l = lseek(vip_fd[board], VIP_BLUE_LUT + start, L_SET);
				f = VIP_BLUE_LUT + start;
				if (l == -1)
					{
					fprintf(stderr,"mpl_vip_access_lut: failed to lseek <%ld> bytes into lut <%d> on board <%d>.",f,lut,board);
					return(FALSE);
					}

				if (read_write_flag == WRITE_TO_LUT)
					w = write(vip_fd[board],lut_data,count);
				else w = read(vip_fd[board],lut_data,count);
				break;

		case INPUT_LUT:	l = lseek(vip_fd[board], VIP_INPUT_LUT + start, L_SET);
				f = VIP_INPUT_LUT + start;
				if (l == -1)
					{
					fprintf(stderr,"mpl_vip_access_lut: failed to lseek <%ld> bytes into lut <%d> on board <%d>.",f,lut,board);
					return(FALSE);
					}

				if (read_write_flag == WRITE_TO_LUT)
					w = write(vip_fd[board],lut_data,count);
				else w = read(vip_fd[board],lut_data,count);
				break;

		default:	fprintf(stderr,"mpl_vip_access_lut: non-existent lut table\n");
				return(FALSE);
				break;
	}


/* kernel routine vip<write,read> will pass back the actual number of     */
/* bytes written to or read out from the lut. Otherwise, it will pass back */
/* ENXIO or ENOSPC.      				                  */ 

	switch (w)
	 {	
		case 0: /* this implies that everything went ok */
			return(count);
		case ENXIO:
			fprintf(stderr,"mpl_vip_access_lut: ENXIO set while attempting to write to lut <%d> on board <%d>\n",lut,board);
			return(FALSE);
			break;
		case ENOSPC:
			fprintf(stderr,"mpl_vip_access_lut: ENOSPC (attempting to write past lut boundary) set while writing to lut <%d> on board <%d>\n",lut,board);
			return(FALSE);
			break;
		default:
			if (read_write_flag == WRITE_TO_LUT)
				fprintf(stderr,"mpl_vip_access_lut: only <%d> written to lut <%d> on board <%d>\n",w ,lut,board);
			else fprintf(stderr,"mpl_vip_access_lut: only <%d> read from lut <%d> on board <%d>\n",w ,lut,board);

			return(FALSE);
			break;
	 }	

}
