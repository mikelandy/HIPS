/**************************************************************************
 *                                                                        * 
 * Function: max_Rroi()						                        *
 *                                                                        *
 * Usage:	max_Rroi [rows [cols [initialrow [initialcol]]]] > filename     *
 * Returns:  none							                        *
 * Defaults: rows: 512, cols: 512, initialrow: 0, initialcol: 0           *
 * Loads: cc -o -DPOLLED max_Rroi max_Rroi.c -lhipl -ldgplib.a -lrsplib.a *
 *               -lmvlib.a -Imaxvideo/include -Ilocaldir/include          *
 * Modified:TK 20-III-88                                                  *
 *                                                                        *
 * Description:Reads a frame from the Datacube Roistore LS starting at    *
 *             (initialrow,initialcol)with size rows x cols.              *
 *             HIPS header is created for frame, but without title.       * 
 *             FOR POLLED MODE ONLY        	     				    *
 *             Connections should be as follows	     			    *
 *              DIGI             ROI                                      * 
 *              P3       ->      P3                                       * 
 *    CAMERA -> P12                                                       * 
 *    TV     <- P13                                                       * 
 *              P7       <-      P7                                       * 
 *                       ->      P9                                       * 
 *              P10      ->      P10                                      * 
 **************************************************************************
 *                    Copyright (c) 1988                                  *
 *                    Captain Chaos     V.3 MAX_WARE                      *
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
char *argv[] ;
{
     RS_DESC *rsfd;	/* ROISTORE device descriptor */
     DG_DESC *dgfd;	/* DIGIMAX device descriptor */
	char *picbuf,*inp,*outp;
	int	r,c,ir,ic,or,oc,n /* ,RS_T50 */ ;  
	struct header hd;

     /* initialize parameters */
	Progname = strsave(*argv);
	r=c=512 ; ir=ic=0 ; /* RS_T50 = 0; */

	if(argv[argc-1][0]=='-')argc--;
	if(argc>1)r=atoi(argv[1]);	/* assign parameters to variables */
	if(argc>2)c=atoi(argv[2]);
	if(argc>3)ir=atoi(argv[3]);
	if(argc>4)ic=atoi(argv[4]);

	or=(511-ir+1);or=r<or?r:or;	/* check for out of range initial r & c*/
	oc=(511-ic+1);oc=c<oc?c:oc;	/* or oc become frame sizes */
	n= r*c;					/* n is size of frame */

	/* display error message on terminal(stderr) */
	if((or<1)||(oc<1)) {
		 fprintf(stderr,"max_rframe: wrong dimensions.\n");
		 exit(1) ;
		}
	fprintf(stderr,"max_Rroi:frame size: %d rows : %d cols:\n",r,c);
	fprintf(stderr,"max_Rroi:initial start point: %d %d\n",ir,ic);

	if ( (picbuf = (char *) calloc(n,sizeof(char))) == 0) { 
			fprintf(stderr,"max_Rroi:can't allocate core\n");
			exit(1);
		}
	inp = outp = picbuf;

     /* Allocate memory for the boards' register structures */
     rsfd = rsOpen(RS_RBASE, RS_MBASE, RS_M2048, RS_WVECTOR, DQ_ON);
     dgfd = dgOpen(DG_BASE, DG_VECTOR, DQ_ON);

	/*create HIPS header for frame */
	init_header(&hd,"","",1,"",or,oc,PFBYTE,1,"");
	update_header(&hd,argc,argv);
	write_header(&hd);
    
	/* Calling ROISTORE read function */
	rsVmBitBlt(rsfd,MOVE,RS_LS,ic,ir,inp,c,r,512);

	/* Writing out frame to filename */
	fwrite(outp,n,1,stdout);

	/*closing all open file devices*/
     if (rsClose(rsfd) == NOOK)
	 printf("ROISTORE, problems with freeing the memory\n");
     if (dgClose(dgfd) == NOOK)
	 printf("DIGIMAX, problems with freeing the memory\n");
	return(0);
}


