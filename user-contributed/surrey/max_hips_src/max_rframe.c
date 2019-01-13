/**************************************************************************
 *                                                                        * 
 * Function: max_rframe()					                        *
 *                                                                        *
 * Usage:	max_rframe [rows [cols [initialrow [initialcol]]]] > filename   *
 * Returns:  none							                        *
 * Defaults: rows: 512, cols: 512, initialrow: 0, initialcol: 0           *
 * Loads: cc -o -DDG max_rframe max_rframe.c -lhipl  -lfslib.a -lldgib.a  *
 *                        -Imaxvideo/include -Ilocaldir/include           *
 * Modified:TK 20-VIII-87                                                 *
 *                                                                        *
 * Description:Reads a frame from the Datacube Framestore fs0 starting at *
 *             (initialrow,initialcol) with size rows x cols.             *
 *             There is no wraparound so large sizes will be truncated.   *
 *             HIPS header is created for frame, but without title.       * 
 *             Connections should be as follows	     			    *
 *              DIGI             FRM                                      * 
 *              P3       ->      P3                                       * 
 *    CAMERA -> P12                                                       * 
 *    TV     <- P13                                                       * 
 *              P7       <-      P7                                       * 
 *              P10      ->      P10                                      * 
 **************************************************************************
 *                    Copyright (c) 1987                                  *
 *                    Captain Chaos     V.2.1 MAX_WARE                    *
 **************************************************************************
 */

#include <fsHead.h>
#include <dgHead.h>
#include <maxdefs.h>
#include <hipl_format.h>

main(argc,argv)
int argc;
char *argv[] ;
{
     FS_DESC *fsfd;	/* FRAMSTORE device descriptor */
     DG_DESC *dgfd;	/* DIGIMAX device descriptor */
     int verbose;	/* verbosity flag - set for printf's */
     int max_rframe();
	int	r,c,ir,ic,or,oc;  
	struct header hd;

     Progname = strsave(*argv);
     /* initialize parameters */
     verbose = 0;
	r=c=512 ; ir=ic=0 ;

	if(argv[argc-1][0]=='-')argc--;
	if(argc>1)r=atoi(argv[1]);	/* assign parameters to variables */
	if(argc>2)c=atoi(argv[2]);
	if(argc>3)ir=atoi(argv[3]);
	if(argc>4)ic=atoi(argv[4]);

	or=(511-ir+1);or=r<or?r:or;	/* check for out of range initial r & c*/
	oc=(511-ic+1);oc=c<oc?c:oc;	/* or oc become frame sizes */

	/* display error message on terminal(stderr) */
	if((or<1)||(oc<1)) {
		 fprintf(stderr,"max_rframe: wrong dimensions.\n");
		 exit(1) ;
		}

     /* Allocate memory for the boards' register structures */
     fsfd = fsOpen(FS_BASE, verbose);
     dgfd = dgOpen(DG_BASE, verbose);

	/*create HIPS header for frame */
	init_header(&hd,"","",1,"",or,oc,PFBYTE,1,"");
	update_header(&hd,argc,argv);
	write_header(&hd);
    
	/* Calling read function */
	max_rframe(fsfd->fs0_base,r,c,ir,ic);

	/*closing all open file devices*/
     if (fsClose(fsfd) == NOOK)
	 printf("FRAMESTORE, problems with freeing the memory\n");
     if (dgClose(dgfd) == NOOK)
	 printf("DIGIMAX, problems with freeing the memory\n");
	return(0);
}


max_rframe(picaddr,r,c,ir,ic)
unsigned char *picaddr;
int	r,c,ir,ic;
{
	int	irr,icc,i,j;

	irr=ir+r ; if(irr>512)irr=512 ; /* final row and column */
	icc=ic+c ; if(icc>512)icc=512 ;

	/* write file of sides r&c to stdout (1), c bytes at at time   */
	for (i=ir;i<irr;++i) {
		  j=(i*512)+ic;
		  fwrite((picaddr+j), 1, c, stdout);
		}
	return(0);
}
