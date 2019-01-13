static char *SccsId = "@(#)prncpl.c     1.0      15/1/91";

/*      Copyright (c) 1991 Carsten Kruse Olsson

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/* prncpl.c     This procedure estimates the principal components of a
 *              multi frame image.
 *
 *              In the input all pixels are concidered to be a multivariate 
 *              observation of the same size as the number of frames.
 *              The output of this procedure is the principal components
 *              of the input frames in decreasing order, the first output 
 *              frame corresponds to the largest eigenvalue, the second
 *              frame to the second largest eigenvalue, etc..
 *
 *              Input is in float format and output is in float format.
 *
 * usage:       prncpl [-v [verbose]] [-c] [-t context_image] < inseq > outseq
 *
 * to load:     cc -o prncpl prncpl.c -lhips -lm
 *
 * Carsten Kruse Olsson 15/1-91
 *
*/


#include <stdio.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <hipl_format.h>

void eigenvectors();

int main(argc,argv)
int  argc;
char *argv[];

{       int             verbose={0};
        char            *trainingfile={NULL};
        int             covarflag={TRUE};
        int             i, j, k, r, c, nband, indx;
        int             rows, cols, imagesize;
        double          *mean, *s2, *d, *eigenvect, *eigenval, *evp, dval;
        unsigned char   *tfr;
        float           *infr, *inp, *outfr, *outp;
        struct header   hd, thd;
        FILE            *fileptr;

        Progname = strsave(*argv);
        for (i=1; i<argc; i++){
                if (argv[i][0]!='-'){
                        fprintf(stderr,"%s: Illegal argument %s\n",Progname,argv[i]);
                }
                else{
                        switch (argv[i][1]){
                        case 'v': /* verbose [val] - information level */
                                {
                                if (i+1<argc && (verbose=atoi(argv[i+1]))>0) 
                                        i++;
                                else
                                        verbose = 1;
                                break;
                                }
                        case 'c': /*    Use correlation matrix (- default is
                                        covariance matrix) */
                                covarflag = FALSE;
                                break;
                        case 't': /*    Image file (byte) containing training 
                                        areas. 
                                        If a pixel has the value 0 it is
                                        ignored, otherwise it is included. */
                                {
                                if (i+1<argc){
                                        trainingfile = argv[i+1];
                                        i++;
                                }
                                else{
                                        fprintf(stderr,"%s: Missing filename. Near %s\n",Progname,argv[i]);
                                        trainingfile = NULL;
                                }
                                break;
                                }
                        default:{
                                fprintf(stderr,"%s: Illegal argument %s\n",Progname,argv[i]);
                                }
                        }
                }
        }

        if (verbose>1){
                fprintf(stderr,"%s:\n\tVerbose = %d\n",Progname,verbose);
                fprintf(stderr,"\tCoVar flag = %d\n",covarflag);
                fprintf(stderr,"\tTraining file = %s\n",trainingfile);
                fprintf(stderr,"\n");
        }

        read_header (&hd) ;
        update_header(&hd,argc,argv);
        write_header(&hd);
        if(hd.pixel_format != PFFLOAT) {
                fprintf(stderr,"%s: pixel format must be float\n",Progname);
                exit(1) ;
        }

        rows = hd.orows ; cols = hd.ocols ;
        imagesize = rows*cols;
        nband = hd.num_frame;
        if (verbose>4)
                fprintf(stderr,"  Allocate\n");
        if ((infr = (float *) malloc(nband*rows*cols*sizeof(float))) == 0) 
                perr(HE_MSG,"Can't allocate core\n");
        if ((trainingfile!=NULL) && 
                ((tfr = (unsigned char *)malloc(rows*cols*sizeof(char)))==0))
                perr(HE_MSG,"Can't allocate core\n");

        if (verbose>4)
                fprintf(stderr,"  Read input file (from stdin)\n");
        if (fread(infr,nband*rows*cols*sizeof(float),1,stdin) != 1) {
                fprintf(stderr,"%s: error during read\n",Progname);
                exit(1);
        }

        if (trainingfile!=NULL){
                if (verbose>1)
                        fprintf(stderr,"  Read %s\n",trainingfile);

                /* read training area file */
                if ((fileptr = fopen(trainingfile,"r"))==NULL){
                        fprintf(stderr,
                        "%s: Cannot open %s\n", Progname,trainingfile);
                        exit(1);
                }
                fread_header(fileptr,&thd,trainingfile);
                if (thd.pixel_format!=PFBYTE || 
                                thd.orows!=hd.orows || thd.ocols!=hd.ocols){
                        fprintf(stderr,"%s: Error in header of %s\n",
                                        Progname,trainingfile);
                        fprintf(stderr,"\twant pixel type = %d, got %d\n",
                                        PFBYTE,thd.pixel_format);
                        fprintf(stderr,"\twant image size = %dx%d, got %dx%d\n",
                                        hd.orows,hd.ocols,thd.orows,thd.ocols);
                        exit(1);
                }
                if (fread(tfr,rows*cols*sizeof(char),1,fileptr) != 1) {
                        fprintf(stderr,"Error reading training file\n");
                        exit(1);
                }
                fclose(fileptr);
        }


        mean = (double *)calloc(nband,sizeof(double));
        s2   = (double *)calloc(nband*nband,sizeof(double));
        d    = (double *)calloc(nband,sizeof(double));

        if (verbose>1)
                fprintf(stderr,"  Collecting data\n");
        for (r=0; r<rows; r++){
                for (c=0; c<cols; c++){
                        if (trainingfile==NULL || tfr[r*cols+c]!=0){
                                for (i=0; i<nband; i++)
                                        d[i] = infr[r*cols+c+i*imagesize] 
                                                                - mean[i];
                                for (i=0; i<nband; i++)
                                        mean[i] += d[i]/(r*cols+c+1);
                                for (i=0; i<nband; i++)
                                        for (j=i; j<nband; j++)
                                                s2[i*nband+j] += d[i]*d[j];
                        }
                }

        }

        /* Use covariance or correlation (default) */
        if (covarflag==FALSE){
                for (i=0; i<nband; i++){
                        for (j=nband-1; j>i; j--){
                                if (s2[i*nband+i]*s2[j*nband+j]!=0){
                                        s2[i*nband+j] /= s2[i*nband+i]
                                                                *s2[j*nband+j];
                                }
                                else{
                                        s2[i*nband+j] = 1; /* Singular ! ! ! */
                                        fprintf(stderr,
                                        "%s: Covarince matrix is singular\n",
					Progname);
                                }
                        }
                        s2[i*nband+i] = 1;
                }
        }

        /* find eigenvectors of s2 */
        eigenvect = (double *)malloc(nband*nband*sizeof(double));
        eigenval  = (double *)malloc(nband*sizeof(double));

        /* If you got access to IMSL or EISPACK library please use one
           of the routines. The routine supplied with this program is working
           perfectly in the examples tested so far (i), but it is slightly 
           slower than the others.

           i)   If s2 is ill-conditioned it works just as bad as other routines
                for finding eigenvectors
        */
        eigenvectors(s2,nband,eigenvect,eigenval);
        
        outfr     = (float *)malloc(imagesize*sizeof(float));
        for (i=0; i<nband; i++){
                for (j=0, indx=0; j<nband; j++)
                        if (eigenval[j]>eigenval[indx])
                                indx = j;
                for (j=0, inp=infr, outp=outfr; j<imagesize; j++, inp++){
                        for (k=0, dval=0, evp= &(eigenvect[indx*nband]); 
                                                                k<nband; k++)
                                dval += inp[k*imagesize]* *evp++;
                        *outp++ = dval;
                }
                if (fwrite(outfr,imagesize*sizeof(float),1,stdout) != 1){
                        perr(HE_MSG,"Error during write\n");
                }
                eigenval[indx] = -1;
        }
        return(0);
}

