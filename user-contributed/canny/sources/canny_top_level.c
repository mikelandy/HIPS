#include <math.h> 
#include <stdio.h>
#include "canny_top_level.h"

void copyimage(),gauss_filter(),magnitude(),non_max_supp(),find_edges();

/* ------------------------------------------------------------------ */
/* The top level routine performing a Canny edge detection.           */
/* The parameters are described below.                                */
/* ------------------------------------------------------------------ */

/*   void canny(magmax,hthresh,lthresh,image,xsize,ysize,shortim,windowsize,sigma,bordermode,hfrac,lfrac,pflag,  */
void canny(magmax,hthresh,lthresh,image,xsize,ysize,shortim,windowsize,sigma,bordermode,hfrac,lfrac,pflag,
      gx,gy,mag,hist,histsize,nms,edgemap,gm,gmp,temp)
/* INPUT PARAMETERS */
    int *image;    /* The image array as an array of unsiged char. */
    int *xsize;             /* The x-dimension of the image */
    int *ysize;             /* The y-dimension of the image */
    short *shortim,*temp;   /* scratch space */
    int *windowsize;        /* The size of the window to be used in the filtering MUST BE ODD!  */
    float *gm,*gmp;         /* arrays to hold masks */
    double *sigma;          /* The sigma of the gaussian in pixels. g(x) = (1/(sqrt(two_PI)*sigma))*exp(x*x/(2*sigma*sigma) */ 
                            /* RANGE -- (0.0,somebignumber) */
    int *bordermode;        /* Specifies how the filtering routine treats th borders.   */
                            /* The options for bordermode are 0: Use zero outside image bounds,,  */
                            /* 1: WRAP, 3: Extends the boudary pixels, */
                            /* 4: Masks the invalid boundary region to zero.*/ 
    double *hfrac;           /* Chooses the upper threshold in hysteretic thresholding to cause the  seed pixels to be chosen from */
                            /* the upper (hfrac*100) percent of the pixels in the magnitiude (edginess) image */
                            /* RANGE -- [0.0,1.0] */
    double *lfrac;           /* Mutiplies the upper threshold to get the lower hysteretic threshold */
                            /* RANGE -- [0.0, 1.0] */
    int *pflag;              /* 1 -- print progress. 0 -- suppress printing of progress. */
    
/* Output Parameters */
    unsigned char *edgemap;  /* An array of char with 255 values at the edgepoints and 0 elsewhere */
    
/* Input/Output parameters */
    short *gx;              /* Pointer to array of short.  If not nil, will be used to compute x-gradient */
    short *gy;              /* Same as gx except its the y gradient.  If the pointer is nil, canny routine will allocate */
                            /* temporary storage for these routines. */
    short *mag;             /* Same as gx and gy except for its for the magnitude image. */
    int  *magmax;           /* storage  for the maximum gradient magnitude. */
    int  *hist;              /* An array of 256 integers used for historgramming which will contain a histogram of the mag image. */
    int  *histsize;          /* Size of the histogram. */
    int *hthresh, *lthresh; /* If not nil, the values of the actual thresholds used in the hysteretic thresholding will be returned. */
    unsigned char  *nms;    /* Same as above except it is a pointer to an array of unsigned char. The result of non-maximal */
                            /* supression applied to the magnitude image is placed here, if not nil. */
{    
    short *mgx,*mgy,*mmag,*shortimage;
    int *mhist,mhthresh,mlthresh, mhistsize,dummyoutx,dummyouty,mxsize,mysize,mmagmax;
    /* unsigned char *iptr, *optr; */
    short *temp_image;
    float *gmask,*gpmask;
    

    if(*pflag)
    {
        fprintf(stderr,"The non-array input arguments are: \n\nxsize - %d\nysize - %d\nsigma - %f\nwindowsize - %d\n",*xsize,*ysize,*sigma,*windowsize);
        fprintf(stderr,"lfrac - %f\nhfrac - %f\n\n",*lfrac,*hfrac);
        fprintf(stderr,"\nbordermode - %d\nhistsize - %d\n\n",*bordermode,*histsize);
    }

    mxsize = *xsize;
    mysize = *ysize;

    mhistsize = *histsize;

    /* array arguments are assumed to have the following sizes  */
    /*                                                          */
    /*      pointer     bytes       type        # elements      */
    /*      -------     -----       ----        ----------      */
    /*      shortim     2*npixels   short       nrows*ncols     */
    /*      gx          2*npixels   short       nrows*ncols     */
    /*      gy          2*npixels   short       nrows*ncols     */
    /*      gy          2*npixels   short       nrows*ncols     */
    /*      mag         2*npixels   short       nrows*ncols     */
    /*      nms         npixels     unsign ch   nrows*ncols     */
    /*      hist        4*histsize  int         histsize        */
    /*      gm          4*windowsz  float       windowsize      */
    /*      gmp         4*windowsz  float       windowsize      */
    /*      temp        2*npixels   short       nrows*ncols     */

    shortimage = shortim;
    mgx = gx;
    mgy = gy;
    mmag = mag;
    mhist = hist;
    temp_image = temp;
    gmask = gm;
    gpmask = gmp;

    /* BEGIN COMPUTATION */ 

    copyimage(image,mxsize,mysize,shortimage);   

    shortimage = shortim;
                           
    if(*pflag)
        fprintf(stderr,"Beginning computation of the x gradient image.\n");

    gauss_filter(shortimage,mxsize,mysize,XDIR,
                 *bordermode,*windowsize,*sigma,mgx,
                 &dummyoutx,&dummyouty,gmask,gpmask,temp_image);

    shortimage = shortim;
    temp_image = temp;
    gmask = gm;
    gpmask = gmp;

    if(*pflag)
    {
        fprintf(stderr,"Done with computation of the x gradient image.\n\n");
        fprintf(stderr,"Beginning computation of the y gradient image.\n");
    }

    gauss_filter(shortimage,mxsize,mysize,YDIR,
                 *bordermode,*windowsize,*sigma,mgy,
                 &dummyoutx,&dummyouty,gmask,gpmask,temp_image);
         
    shortimage = shortim;
    temp_image = temp;
    gmask = gm;
    gpmask = gmp;

    if(*pflag)
    {
        fprintf(stderr,"Done with computation of the y gradient image.\n\n");
        fprintf(stderr,"Beginning computation of the magnitude image from the two gradient images.\n");
    }

    magnitude(mgx,mgy,mxsize,mysize,mmag,&mmagmax);

    if(*pflag)
        fprintf(stderr,"Done computing magnitude.\n\nStarting non-maximal supression.\n");

    non_max_supp(mmag,mgx,mgy,mxsize,mysize,edgemap);

/*	COMMENTED OUT BY JMM Apr.7, 1989 - since the nms array is
currently never printed out.


	/  * This code (following) originally copied edgemap from the
	non_max_supp (above) to nms to save the results before the
	final edge pixels were chosen.  It can be commented out IF
	noone wants to use this nms mask later. *  /

    if (nms != (unsigned char *) 0L)
    {
fprintf(stderr,"JOE copying NMS NOW FOR DEBUG or OTHER PURPOSES\n");
        for(i=0, iptr = edgemap, optr = nms; i<mxsize*mysize;i++, iptr++, optr++)
            *optr = *iptr;
    }
*/
    if(*pflag)
        fprintf(stderr,"Done with non-maximal supression.\n\nStarting hysteretic thresholding.\n");

    find_edges(edgemap,mmag,mxsize,mysize,mmagmax,*hfrac,*lfrac,
               mhist,mhistsize,&mhthresh,&mlthresh);

    if(*pflag)
        fprintf(stderr,"Done with hysterectic thresholding.\n\nDone with Canny edge detection.\n");

    /* set output vars */
    
    if (magmax != (int *) 0L)
    {
        *magmax = mmagmax;
    }


    if (hthresh != (int *) 0L)
    {
        *hthresh =  mhthresh;
    }

    if (lthresh != (int *) 0L) 
    {
        *lthresh =  mlthresh; 
    }

}



/* ------------------------------------------------------------------ */   
/* Mag takes two images and produces their magnitude                  */
/* ------------------------------------------------------------------ */

void magnitude(gx,gy,xsize,ysize,mag, max)
    short *gx,*gy,*mag;
    int xsize,ysize,*max;
{
    short *xpixelptr,*ypixelptr,*magpixelptr,nearestshort();
    int pixelcount,themax = 0;
    float gradx,grady;


    for(pixelcount = 0, xpixelptr = gx, ypixelptr = gy, magpixelptr = mag;
        pixelcount < xsize*ysize;
        pixelcount++, xpixelptr++,ypixelptr++,magpixelptr++)
    {
        gradx = (float) *xpixelptr;
        grady = (float) *ypixelptr;

        if ((*magpixelptr = nearestshort(sqrt((float) (gradx*gradx + grady*grady)))) > themax)
            themax = *magpixelptr;
    }
    *max = themax;
}
 
/* ------------------------------------------------------------------ */
/* The following procedure calculates the non-maximal suppression of  */
/* an image.  The required input is the magnitude image, and the two  */
/* gradient images. Edgemap is and image for the result.              */
/* ------------------------------------------------------------------ */

void non_max_supp(mag,gradx,grady,ncols,nrows,result) 
    short *mag, *gradx, *grady;
    int   ncols, nrows; 
    unsigned char *result;
{
    int rowcount, colcount,count;
    short *magrowptr,*magptr;
    short *gxrowptr,*gxptr;
    short *gyrowptr,*gyptr,z1,z2;
    short m00,gx,gy;
    float mag1,mag2,xperp,yperp;
    unsigned char *resultrowptr, *resultptr;
    

    /* Zero the edges of the result image */

    for(count = 0,resultrowptr = result, resultptr = result + ncols*(nrows - 1); 
        count < ncols;
        resultptr++,resultrowptr++,count++)
    {
        *resultrowptr = *resultptr = (unsigned char) 0;
    }

    for(count = 0, resultptr = result, resultrowptr = result + ncols - 1;
        count < nrows;
        count++, resultptr += ncols, resultrowptr += ncols)
    {
        *resultptr = *resultrowptr = (unsigned char) 0;
    }

    /* Suppress non-maximum points */

    for(rowcount = 1, magrowptr = mag + ncols + 1, gxrowptr = gradx + ncols + 1, gyrowptr = grady + ncols + 1, resultrowptr = result + ncols + 1;
        rowcount < nrows - 2; 
        rowcount++, magrowptr += ncols, gyrowptr += ncols, gxrowptr += ncols, resultrowptr += ncols)
    {   
        for(colcount = 1, magptr = magrowptr, gxptr = gxrowptr, gyptr = gyrowptr, resultptr = resultrowptr; 
            colcount < ncols - 2; 
            colcount++, magptr++,gxptr++,gyptr++, resultptr++)
        {   
            m00 = *magptr;
            if(m00 == 0)
            {
                *resultptr = (unsigned char) NOEDGE; 
   /* Commented out by Joe Miller Nov. 89 - a BUG ?  Reported to me by
	ramin@scotty.Standfor.edu...     at least its a potential error
 	I don't know where it came from.
                xperp = -(gx = *gxptr)/((float)m00);
                yperp = (gy = *gyptr)/((float)m00);
 */
            }
            else
            {
                xperp = -(gx = *gxptr)/((float)m00);
                yperp = (gy = *gyptr)/((float)m00);
            }

            if (gx >= 0)
            {
                if (gy >= 0) 
                {
                    if (gx >= gy)
                    {  
                        /* 111 */
                        /* Left point */
                        z1 = *(magptr - 1);
                        z2 = *(magptr - ncols - 1);

                        mag1 = (m00 - z1)*xperp + (z2 - z1)*yperp;
                        
                        /* Right point */
                        z1 = *(magptr + 1);
                        z2 = *(magptr + ncols + 1);

                        mag2 = (m00 - z1)*xperp + (z2 - z1)*yperp;
                        /* JOE CHANGED THIS ERROR  mag2 = (m00 - z1)*xperp + (z1 - z2)*yperp; */
                    }
                    else
                    {    
                        /* 110 */
                        /* Left point */
                        z1 = *(magptr - ncols);
                        z2 = *(magptr - ncols - 1);

                        mag1 = (z1 - z2)*xperp + (z1 - m00)*yperp;

                        /* Right point */
                        z1 = *(magptr + ncols);
                        z2 = *(magptr + ncols + 1);

                        mag2 = (z1 - z2)*xperp + (z1 - m00)*yperp; 
                    }
                }
                else
                {
                    if (gx >= -gy)
                    {
                        /* 101 */
                        /* Left point */
                        z1 = *(magptr - 1);
                        z2 = *(magptr + ncols - 1);

                        mag1 = (m00 - z1)*xperp + (z1 - z2)*yperp;
            
                        /* Right point */
                        z1 = *(magptr + 1);
                        z2 = *(magptr - ncols + 1);

                        mag2 = (m00 - z1)*xperp + (z1 - z2)*yperp;
                    }
                    else
                    {    
                        /* 100 */
                        /* Left point */
                        z1 = *(magptr + ncols);
                        z2 = *(magptr + ncols - 1);

                        mag1 = (z1 - z2)*xperp + (m00 - z1)*yperp;

                        /* Right point */
                        z1 = *(magptr - ncols);
                        z2 = *(magptr - ncols + 1);

                        mag2 = (z1 - z2)*xperp  + (m00 - z1)*yperp; 
                    }
                }
            }
            else
            {
                if ((gy = *gyptr) >= 0)
                {
                    if (-gx >= gy)
                    {          
                        /* 011 */
                        /* Left point */
                        z1 = *(magptr + 1);
                        z2 = *(magptr - ncols + 1);

                        mag1 = (z1 - m00)*xperp + (z2 - z1)*yperp;

                        /* Right point */
                        z1 = *(magptr - 1);
                        z2 = *(magptr + ncols - 1);

                        mag2 = (z1 - m00)*xperp + (z2 - z1)*yperp;
                    }
                    else
                    {
                        /* 010 */
                        /* Left point */
                        z1 = *(magptr - ncols);
                        z2 = *(magptr - ncols + 1);

                        mag1 = (z2 - z1)*xperp + (z1 - m00)*yperp;

                        /* Right point */
                        z1 = *(magptr + ncols);
                        z2 = *(magptr + ncols - 1);

                        mag2 = (z2 - z1)*xperp + (z1 - m00)*yperp;
                    }
                }
                else
                {
                    if (-gx > -gy)
                    {
                        /* 001 */
                        /* Left point */
                        z1 = *(magptr + 1);
                        z2 = *(magptr + ncols + 1);

                        mag1 = (z1 - m00)*xperp + (z1 - z2)*yperp;

                        /* Right point */
                        z1 = *(magptr - 1);
                        z2 = *(magptr - ncols - 1);

                        mag2 = (z1 - m00)*xperp + (z1 - z2)*yperp;
                    }
                    else
                    {
                        /* 000 */
                        /* Left point */
                        z1 = *(magptr + ncols);
                        z2 = *(magptr + ncols + 1);

                        mag1 = (z2 - z1)*xperp + (m00 - z1)*yperp;

                        /* Right point */
                        z1 = *(magptr - ncols);
                        z2 = *(magptr - ncols - 1);

                        mag2 = (z2 - z1)*xperp + (m00 - z1)*yperp;
                    }
                }
            } 

            /* Now determine if the current point is a maximum point */

            if ((mag1 >= 0.0) || (mag2 > 0.0))
            {
                *resultptr = (unsigned char) NOEDGE;
            }
            else
            {
                *resultptr = (unsigned char) POSSIBLE_EDGE;
            }
        } 
    }
}

