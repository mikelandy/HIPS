			      
/*      Copyright (c) 1990 Rasmus Larsen, IMSOR
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 *
 * 08/12/92 HIPS-2 Rasmus Larsen
 */
#include <hipl_format.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <math.h>

int main(argc,argv)
int argc;
char *argv[];
{
   struct header hd;
	

   float          *bbfr;  /* internal representation of frames*/

   int           i,j,k;  /* loop counters              */


   int           dim;    /* number of frames                 */
   int           rows;   /* number of rows                   */
   int           cols;   /* number of columns                */
   double        konstant,twopi;
   int           alfainv;

   Progname = strsave(*argv); /* save program name to be used by perr */
   
   read_header(&hd);
   if (hd.pixel_format != PFFLOAT) perr(HE_MSG,"Image must be in float format");
   rows  = hd.orows;
   cols  = hd.ocols;
   dim   = hd.num_frame;
   update_header(&hd,argc,argv);

/******************************************************************************/
/* Space is allocated for internal representation of framesequence.           */
/* The input framesequence is loaded into bfr (buffer)                        */
/******************************************************************************/

bbfr = (float *) malloc(cols*sizeof(float));

write_header(&hd);
twopi = 8*atan(1.0);
alfainv = 4;
konstant = twopi/(cols/alfainv);


for (k=0;k<dim;k++) {
   for (i=0;i<rows;i++) {
      if (fread(bbfr,sizeof(float),cols,stdin) != cols)
	 perr(HE_MSG,"error during read");

      for(j=0;j< cols/(2*alfainv);j++)
	 bbfr[j] *= (float) 0.5*(1-cos(konstant*j));

      for(j= (cols-cols/(2*alfainv));j<cols;j++)
	 bbfr[j] *= (float) 0.5*(1-cos(konstant*(cols-j)));

      if (i< rows/(2*alfainv)) 
	 for (j=0;j<cols;j++) bbfr[j] *= (float) 0.5*(1-cos(konstant*i));

      if (i>= (int) (rows-rows/(2*alfainv)))
	 for (j=0;j<cols;j++) bbfr[j] *= (float) 0.5*(1-cos(konstant*(rows-i)));

      if (fwrite(bbfr,sizeof(float),cols,stdout) != cols)
	 perr(HE_MSG,"error during write");
   }
}

return 0;
}
