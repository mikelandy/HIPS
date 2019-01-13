#include <math.h>

void order();

/* This routine will take an image and return a ptr (in the same block of memory */
/* a pointer to a specified subimage.  cropcorner/ax/ay/bx/by specify opposing corners */
/* of the cropping rectangle.  The new image size is inserted in rows and cols. */

void crop_image(imageptr,cols,rows, cropcornerax, cropcorneray, cropcornerbx, cropcornerby)
    unsigned char *imageptr;
    int *rows, *cols, cropcornerax, cropcorneray, cropcornerbx, cropcornerby;
{

    int rowindex, colindex;
    unsigned char *srcrowptr, *destrowptr,*srcptr,*destptr;
    int vstepsrc, vstepdest;
    
    /* Make corner a be the one closest to the origin, and b the opposite. */
    order(&cropcornerax,&cropcornerbx);
    order(&cropcorneray,&cropcornerby);

    srcrowptr = imageptr + (*cols)*cropcorneray + cropcornerax;
    destrowptr = imageptr;

    vstepsrc = *cols;
    vstepdest = cropcornerbx - cropcornerax;
    
    /* Replace *rows and *cols by the values of the cropped image. */

    *rows = cropcornerby - cropcorneray;
    *cols = vstepdest;

    /* copy the subimage to the low memory portion of the original storage area. */

    for(rowindex = 0; rowindex < *rows; rowindex++, srcrowptr += vstepsrc, destrowptr += vstepdest)
    {
        for(colindex = 0, srcptr = srcrowptr, destptr = destrowptr; colindex < *cols; colindex++, srcptr++, destptr++)
        {
            *destptr = *srcptr;                         

        }
    }
}
          

/* ------------------------------------------------------------------------------------------------------- */
/* This routine computes a histogram of theimage and places it in thehistogram, a 256 element integer      */
/* array.                                                                                                  */
/* ------------------------------------------------------------------------------------------------------- */

void histogram(theimage,xsize,ysize,pixelmax,pixelmin,hgram,histsize)
    short *theimage;
    int xsize,ysize,hgram[],histsize,pixelmax,pixelmin;
{
    int i,index;
    short *iptr;
    float scale;

    order(&pixelmin,&pixelmax); 

    scale = ((float) histsize)/(pixelmax - pixelmin + 1);     
                   
    /* Zero the histogram */

/*
#debug    printf("\n histogram: xsize = %d, ysize = %d, max = %d, min = %d,",
#debug            xsize, ysize, pixelmax, pixelmin);
#debug    printf(" scale = %d, histsize = %d\n", scale, histsize);
*/
    for(i=0;i<histsize;i++)
        hgram[i] = 0;

    for(i=0, iptr = theimage; i< xsize*ysize;i++,iptr++) 
    {     
        index = (int) ((*iptr - pixelmin) * scale); 

        hgram[index]++ ;
    }
  
}

/* ------------------------------------------------------------------------------------------------------ */
/* This routine finds a threshold above below which the given fraction of pixels lie                      */
/* ------------------------------------------------------------------------------------------------------ */

void get_histogram_threshold(hgram,histsize,pixelmax,pixelmin,fraction,zflag,ht,lt)
    int hgram[],histsize,pixelmax,pixelmin,zflag;
    float fraction,*ht,*lt;
{
    int i,j, sum, partsum, halfindex,lstart,hstart;


    /* find total area */

    for(i = 0,sum = 0;i < histsize; sum += hgram[i],++i);

    /* Now find thresholds */

    if(zflag)
    {
        /* find the correct index */ 

        for(i=0, partsum=0; partsum < fraction*sum; partsum += hgram[i], i++);

        /* Compute the threshold */

        *ht = ((((float) (pixelmax - pixelmin))/histsize) * i)  + pixelmin; 
        *lt = 0.0;
    }
    else
    {   
        /* Find halfway index */
        for(halfindex = 0,partsum = 0;partsum < .5*sum; partsum += hgram[halfindex],halfindex++);

        halfindex--;

        lstart = (int) ((sum/2.0) - ((float)partsum) + hgram[halfindex] + 1.0);
        hstart = hgram[halfindex] - lstart - 1;



        /* now go down then up to find the required fraction */
        for(i = halfindex - 1, partsum = lstart;(partsum < .5*fraction*sum) && (i >= 0);partsum += hgram[i],i--);
        for(j = halfindex + 1, partsum = hstart;(partsum < .5*fraction*sum) && (j < histsize);partsum += hgram[j],j++);

        *lt = ((((float) (pixelmax - pixelmin))/histsize) * (i + 1))  + pixelmin; 
        *ht = ((((float) (pixelmax - pixelmin))/histsize) * (j - 1))  + pixelmin;
    }
}
