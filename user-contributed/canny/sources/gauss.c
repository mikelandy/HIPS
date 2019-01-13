#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define TWO_PI 2*3.1415927


#include "canny_top_level.h" 

void crop_image();

/* This routine convolves an image with a filter along one direction */
/* ie. x or y.   */

void correlate_image(image_ptr, incols, inrows, filter, windowsize, direction, boundery, result, outcols, outrows, maxval, minval, status)
    short *image_ptr; 
    short *result;
    int inrows, incols, windowsize, boundery, direction;
    float filter[20];
    int *outcols, *outrows, *maxval, *minval, *status;
{
    int rowcount,colcount,findex,halfwindow,currentpixel,thecol,zeroflag = 0;
    int therow,totalpixels, windowcolpixels,themaxval = 0, theminval = 0;
    short *inbegrowptr, *inendrowptr, nearestshort(); 
    short *inbegcolptr, *inendcolptr, *inposptr1, *inposptr2; 
    short *outrowptr, *outcolptr, *outposptr; 
    float currentresult;

/* DEBUG PRINTOUT HERE... - JMM
fprintf(stderr,"COR_IMAGE: col %d row %d \n",incols,inrows);
	To get cols and rows passed */

    if (windowsize % 2 == 0) 
    {  
        /* Do not allow even sized filters */
        *status = EVENWINDOWSIZE;
        return;
    } 
    
    /* Calculate the values of some variables */

    halfwindow = windowsize/2;  /* half the size of the filter window */
    totalpixels = inrows * incols;    /* The total number of pixels in the image */
    windowcolpixels = halfwindow * incols;  /* The number of pixels in the scan lines between two pixels that  */
                                                /* are halfwindow apart vertically. */


    switch(direction)
    {                        
        case XDIR: 
        /* Filter is in the X direction */
            /* Convolution in the x direction, convolve across cols */
            for(rowcount = 0, inbegrowptr = image_ptr, inendrowptr = image_ptr + incols - 1, outrowptr = result; 
                rowcount < inrows; rowcount++, inbegrowptr += incols, outrowptr += incols)
            {
                for(colcount = 0, inposptr1 = inbegrowptr, outposptr = outrowptr; 
                    colcount < incols ; 
                    colcount++, inposptr1++, outposptr++)
                { 
                    currentresult = 0;
                    zeroflag = 0;
                    /* scan filter */
                    for(findex = 0, inposptr2 = inposptr1 - halfwindow, thecol = colcount - halfwindow; 
                        findex < windowsize; 
                        findex++, inposptr2++, thecol++)
                    {
				/* This section works on the LEFT side
				of the image - JMM Apr. 6, 89 */
                        if(thecol < 0)
                        {      
                            switch(boundery)
                            {
                                case ZERO:
                                    currentpixel = 0;
                                    break;
                                case WRAP:
                                    currentpixel = (int) (*(inposptr2 + incols));
                                    break;
                                case MAKESMALL:
                                    currentpixel = 0;
                                    break; 
                                case EXTEND:
                                    currentpixel = (int) *(inbegrowptr); /* lefthand pixel of the current row */
                                    break;
                                case MASKIT:
                                    currentpixel = 0;
                                    zeroflag = 1;
                                    break;
                                default:
                                    *status = NOSUCHBOUNDERY;
                                    return;
                            }
                        }
                        else
                        {
				/* This section works on the right side
				of the image - JMM Apr. 6, 89 */
                            if(thecol >= incols)
                            {
                                switch(boundery)
                                {
                                case ZERO:
                                    currentpixel = 0;
                                    break;
                                case WRAP:
                                   currentpixel = (int) (*(inposptr2 - incols));
                                    break;
                                case MAKESMALL:
                                    currentpixel = 0;
                                    break; 
                                case EXTEND:
                                    currentpixel = (int) *(inendrowptr); /* lefthand pixel of the current row */
                                    break; 
                                case MASKIT:
                                    currentpixel = 0;
                                    zeroflag = 1;
                                    break;
                                default:
                                    *status = NOSUCHBOUNDERY;
                                    return;
                                }
                            }
                            else
                                currentpixel = (int) *inposptr2;
                        }  
                        if (!zeroflag) 
                            currentresult += filter[findex]*currentpixel;
                    }   /* End filter scan loop */
                    if (themaxval < currentresult) themaxval = currentresult;
                    if (theminval > currentresult) theminval = currentresult;
                    *outposptr = nearestshort(currentresult);
                }   /* End column loop */
            }   /* End row loop */ 

            *outrows = inrows;
            *outcols = incols; 
            *maxval = themaxval;
            *minval = theminval;




            if (boundery == MAKESMALL)
            {
                crop_image(result,outcols,outrows,halfwindow,0,incols - halfwindow,inrows - 1);
            }
            break;  /* Break out of direction switch statement */


        /* Filter in the Y direction */
        case YDIR: 
            /* Convolution in the Y direction, convolve across rows */
            for(colcount = 0,inbegcolptr = image_ptr,inendcolptr = image_ptr + totalpixels - incols, outcolptr = result; 
                colcount < incols; 
                colcount++, inbegcolptr++, inendcolptr++, outcolptr++)
            {
                for(rowcount = 0,inposptr1 = inbegcolptr,outposptr = outcolptr; 
                    rowcount < inrows; 
                    rowcount++, inposptr1 += incols, outposptr += incols)
                {    
                    zeroflag = 0;
                    currentresult = 0;
                    /* scan filter */
                    for(findex = 0, inposptr2 = inposptr1 - windowcolpixels, therow = rowcount - halfwindow; 
                        findex < windowsize; 
                        findex++, inposptr2 += incols, therow++)
                    {
                        if(therow < 0)
                        {
				/*  TOP of IMAGE is processed here -JMM */
                            switch(boundery)
                            {
                                case ZERO:
                                    currentpixel = 0;
                                    break;
                                case WRAP:
                                    currentpixel = (int) (*(inposptr2 + totalpixels));   /* totalpixels = nrow*ncols */
                                    break;
                                case MAKESMALL:
                                    currentpixel = 0;
                                    break; 
                                case EXTEND:
                                    currentpixel = (int) (*(inbegcolptr)); /* top pixel of the current column */
                                    break;
                                case MASKIT:
                                    currentpixel = 0;
                                    zeroflag = 1;
                                    break;
                                default:
                                    *status = NOSUCHBOUNDERY;
                                    return;
                            }
                        } /* end of IF */
                        else
                        {
                            if(therow >= inrows)
                            {
                                switch(boundery)
                                {
                                case ZERO:
                                    currentpixel = 0;
                                    break;
                                case WRAP:
                                    currentpixel = (int) (*(inposptr2 - totalpixels));   /* totalpixels = nrow*ncols */
                                    break;
                                case MAKESMALL:
                                    currentpixel = 0;
                                    break; 
                                case EXTEND:
                                    currentpixel = (int) (*(inendcolptr));  /* bottom pixel of the current column */
                                    break;
                                case MASKIT:
                                    currentpixel = 0;
                                    zeroflag = 1;
                                    break;
                                default:
                                    *status = NOSUCHBOUNDERY;
                                    return;
                                }
                            }

			/* THEY FORGOT THIS ELSE !!  - JMM */
			    else
                                currentpixel = (int) *inposptr2;
                        } /* end of else */ 
                        if (!zeroflag)
                            currentresult += filter[findex]*currentpixel;
                    }   /* End filter scan loop */
                    if (themaxval < currentresult) themaxval = currentresult;
                    if (theminval > currentresult) theminval = currentresult;
                    *outposptr = nearestshort(currentresult);
                }   /* End row loop */
            }   /* End column loop */

            *outrows = inrows;
            *outcols = incols;
            *maxval = themaxval;
            *minval = theminval;

            if (boundery == MAKESMALL)
            {    
                /* shift image to upper left corner and set the outrow and outcols parameters appropriately */

                crop_image(result,outcols,outrows,0,halfwindow,incols - 1,inrows - halfwindow);
            }      
            break;

        default:
            *status = NOSUCHDIRECTION; 
            return;
    }   /* End direction switch */
}
                                 
/*-----------------------------------------------------*/    
/* This routine loads gmask with a Gaussian mask with  */
/* sigma in pixels as the spreading factor.  It also   */
/* loads gprimemask with the derivative of a Gaussian. */
/* The masksize must be odd. Maxresponse is the        */ 
/* maximum response desired to a perfect unit step     */
/* edge.                                               */
/* --------------------------------------------------- */

#define NINTEGRATEPTS 11  /* Number of points to integrate over. */

void make_gaussian_mask(gmask, gprimemask, masksize, sigma, maxresponse)
    float gmask[20],gprimemask[20],sigma, maxresponse;
    int masksize;
{
    int i, maskcenter, count, findex;
    float delta, currentx,gconst,gprimeconst;
    
    if (masksize % 2 == 0)
    {
        printf("Even masksize in make_gaussian_mask (in gauss.c).\n");
        exit(-1); 
    }

    maskcenter = masksize/2; 
    delta = 1.0/(NINTEGRATEPTS - 1);
    
    gconst = 1.0*maxresponse/((sqrt(TWO_PI)) * sigma);
    gprimeconst = maxresponse/(sigma*sigma);


    for(i = -maskcenter, findex = 0;findex < masksize;i++, findex++)
    {   
        gmask[findex] = 0;
        gprimemask[findex] = 0;
        
        /* Loop NINTEGRATEPTS times to perform the integration. */
        for(count = 1, currentx = i - .5; count <= NINTEGRATEPTS; count++, currentx += delta)
        {
            gmask[findex] += gconst * exp(-currentx*currentx/(2*sigma*sigma));
            gprimemask[findex] += gprimeconst * currentx * exp(-currentx*currentx/(2*sigma*sigma)); 
        }
        gmask[findex] /= NINTEGRATEPTS;
        gprimemask[findex] /= NINTEGRATEPTS;
    }
}


void gauss_filter(inimage, inx, iny, direction, boundary, masksize,
             sigma, grad, outx, outy,
             gmask,gprimemask,tempimage)
    short *inimage,*grad;
    int inx,iny,direction,masksize,*outx,*outy,boundary;
    float sigma;
    float *gmask, *gprimemask;
    short *tempimage;
{
    int orthogdir,status,max,min;

    /* Allocate space for the mask off the stack */
    /* and load mask with Gaussian and its derivative */ 

    make_gaussian_mask(gmask, gprimemask, masksize, sigma, 4.0);

    /* Load orthogdir with the direction orthoganal to direction */

    switch(direction)
    {
        case XDIR:
            orthogdir = YDIR;
            break;
        case YDIR:
            orthogdir = XDIR;
            break;
        default:
            fprintf(stderr,"No such direction in gauss_filter (in gauss.c).\n");
            exit(-1);
            break;
    } 
status=0;
    

    correlate_image(inimage,inx,iny,gmask,masksize,orthogdir,boundary,tempimage,outx,outy,&max,&min,&status);


    correlate_image(tempimage,*outx,*outy,gprimemask,masksize,direction,boundary,grad,outx,outy,&max,&min,&status);
}
    
/* ----------------------------------------------------- */
/* copyimage copys an array of INTEGER to an array - JMM */
/* of short.                                             */
/* JMM: Modified Mar. 2, 1989 to copy integers - NOT u.c.*/
/* ----------------------------------------------------- */

void copyimage(charimage,ncols,nrows,shortimage)
    int *charimage;
    short *shortimage;
    int ncols, nrows;
{
    int i;
    int *inptr;
    short *outptr;

    for(i=0,inptr = charimage,outptr = shortimage;i<ncols*nrows;i++,inptr++,outptr++)
    {
        *outptr = (short) *inptr;
    }
}

