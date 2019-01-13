/**************************************************************************
 *                                                                        * 
 * Function: max_Wroi()					                   		    *
 *                                                                        *
 * Usage:	max_Wroi [initialrow [initialcolumn]] < frame              	    *
 * Returns:  none							                        *
 * Defaults: centered                                                     *
 * Loads: cc -o -DPOLLED max_Wroi max_Wroi.c -lhipl -lrsplib.a -ldgplib.a *
 *              -lmvlib.a -Imaxvideo/include -Ilocaldir/include           *
 * Modified:TK 20-III-88                                                  *
 *                                                                        *
 * Description:Writes a frame onto the Datacube Roistore LS at screen     *
 *             position (initialrow,initialcol)for frame coordinate (0,0).*
 *              FOR POLLED MODE ONLY        	     				    *
 *              Connections should be as follows	     			    *
 *              DIGI             ROI                                      * 
 *              P3       ->      P3                                       * 
 *    CAMERA -> P12                                                       * 
 *    TV     <- P13                                                       * 
 *              P7       <-      P7                                       * 
 *                       ->      P9                                       * 
 *              P10      ->      P10                                      * 
 **************************************************************************
 *                    Copyright (c) 1988                                  *
 *                    Captain Chaos   V.3 MAX_WARE                        *
 **************************************************************************
 */
#include <dqHead.h>
#include <mvIntDefs.h>
#include <dgHead.h>
#include <rsHead.h>
#include <maxdefs.h>
#include <hipl_format.h>
#include <stdio.h>

main(argc,argv)
int argc;
char **argv;
{
     RS_DESC *rsfd;	/* ROISTORE device descriptor */
     DG_DESC *dgfd;	/* DIGIMAX device descriptor */
	char *picbuf,*inp,*outp;
	int  ir,ic,r,c,n /* ,RS_T50 */ ;
	struct header hd;

     /* initialize parameters */
	Progname = strsave(*argv);
	read_header(&hd);
	ir=(512-hd.orows)/2; ic=(512-hd.ocols)/2; /* centre display of frame */
	/* RS_T50 = 0; */ r = hd.orows; c = hd.ocols;n = r*c ;

	if(argv[argc-1][0]=='-')argc--; /* assign parameters */
	if(argc>1) ir=atoi(argv[1]) ;
	if(argc>2) ic=atoi(argv[2]) ;

	/* display error message on terminal(stderr) */
	if (hd.pixel_format != PFBYTE) {
		fprintf(stderr,"max_Wroi: frame must be in byte format\n");
		exit(1);
	}
	fprintf(stderr,"max_Wroi:frame size: %d rows : %d cols:\n",r,c);
	fprintf(stderr,"max_Wroi:initial start point: %d %d\n",ir,ic);

	if ( (picbuf = (char *) calloc(n,sizeof(char))) == 0) { 
			fprintf(stderr,"max_Wroi:can't allocate core\n");
			exit(1);
		}
	outp = inp = picbuf;

     /* Allocate memory for the boards' register structures */
     rsfd = rsOpen(RS_RBASE, RS_MBASE, RS_M2048, RS_WVECTOR, DQ_ON);
     dgfd = dgOpen(DG_BASE, DG_VECTOR, DQ_ON);

	/* Reading file from stdin */
	fread(inp,n,1,stdin);

	/* Calling ROISTORE write function to LS */
	rsMvBitBlt(rsfd,MOVE,outp,RS_LS,ic,ir,c,r,512);

	/*closing all open file devices*/
     if (rsClose(rsfd) == NOOK)
	 printf("ROISTORE, problems with freeing the memory\n");
     if (dgClose(dgfd) == NOOK)
	 printf("DIGIMAX, problems with freeing the memory\n");
	return(0) ;
}
