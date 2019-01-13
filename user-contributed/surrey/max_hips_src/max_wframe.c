/**************************************************************************
 *                                                                        * 
 * Function: max_wframe()					                        *
 *                                                                        *
 * Usage:	max_wrframe [initialrow [initialcolumn]] < frame                *
 * Returns:  none							                        *
 * Defaults: centered                                                     *
 * Loads: cc -o -DDG max_wframe max_wframe.c -lhipl  -lfslib.a -lldgib.a  *
 *                        -Imaxvideo/include -Ilocaldir/include           *
 * Modified:TK 24-VIII-87                                                 *
 *                                                                        *
 * Description:Writes a frame onto the Datacube Framestore fs0 at screen  *
 *             position (initialrow,initialcol)for frame coordinate (0,0).*
 *             There is no wraparound so off-screen coordinates are lost. *
 *             Connections should be as follows	     			    *
 *              DIGI             FRM                                      * 
 *              P3       ->      P3                                       * 
 *    CAMERA -> P12                                                       * 
 *    TV     <- P13                                                       * 
 *              P7       <-      P7                                       * 
 *              P10      ->      P10                                      * 
 **************************************************************************
 *                    Copyright (c) 1987                                  *
 *                    Captain Chaos    V.2.1 MAX_WARE                     *
 **************************************************************************
 */
#include <fsHead.h>
#include <dgHead.h>
#include <maxdefs.h>
#include <hipl_format.h>

main(argc,argv)
int argc;
char **argv;
{
     FS_DESC *fsfd;	/* FRAMSTORE device descriptor */
     DG_DESC *dgfd;	/* DIGIMAX device descriptor */
     int verbose;	/* verbosity flag - set for printf's */
     int max_wframe();
	int  ir,ic;
	struct header hd;

     Progname = strsave(*argv);
     /* initialize parameters */
     verbose = 0;
	read_header(&hd);
	ir=(512-hd.orows)/2; ic=(512-hd.ocols)/2; /* centre display of frame */

	if(argv[argc-1][0]=='-')argc--; /* assign parameters */
	if(argc>1) ir=atoi(argv[1]) ;
	if(argc>2) ic=atoi(argv[2]) ;

	/* display error message on terminal(stderr) */
	if (hd.pixel_format != PFBYTE) {
		fprintf(stderr,"max_wframe: frame must be in byte format\n");
		exit(1);
	}

     /* Allocate memory for the boards' register structures */
     fsfd = fsOpen(FS_BASE, verbose);
     dgfd = dgOpen(DG_BASE, verbose);

	/* Calling read function */
	max_wframe(fsfd->fs0_base,hd.orows,hd.ocols,ir,ic) ;

	/*closing all open file devices*/
     if (fsClose(fsfd) == NOOK)
	 printf("FRAMESTORE, problems with freeing the memory\n");
     if (dgClose(dgfd) == NOOK)
	 printf("DIGIMAX, problems with freeing the memory\n");
	return(0) ;
}


max_wframe(picaddr,r,c,ir,ic) 
unsigned char *picaddr;
int r,c,ir,ic ;
{
	int i,j,irr,icc ;

	irr=ir+r ; icc=ic+c ;/* final row and column */

	/* write file from stdin (0) to Datacube Framestore in single bytes */
	for (i=ir;i<irr;i++) {
		if(i<0||i>511) continue ;		
		for (j=(i*512)+ic;j<(i*512)+icc;j++) { 
			   if(i<0||i>511||j<0||j>(i*512)+511) continue ;
			   fread((picaddr+j), 1, 1, stdin);
			}
		}
	return(0) ;
}
