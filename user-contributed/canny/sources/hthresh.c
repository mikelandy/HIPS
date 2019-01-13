#include "canny_top_level.h"
#include <stdio.h>
#include <math.h>

int x[8] = {1,1,0,-1,-1,-1,0,1},y[8] = {0,1,1,1,0,-1,-1,-1};
int xdim,ydim;
int thelothresh;
/* debug*/
int bugflag = 0;  /* A bug flag of 1 causes all kinds of debug info..  */

void histogram(),get_histogram(),cleanup(),get_histogram_threshold();
int nearestint();

/* -------------------------------------------------------------------------- */
/* clear borders sets the edge pixels of a character array to zero            */
/* -------------------------------------------------------------------------- */                                                                              

void clear_borders(charimage,xsize,ysize)
    unsigned char *charimage;
    int xsize,ysize;
{
    unsigned char *rowptr,*rowendptr, *colptr, *colendptr;
    int i;

    for(i=0, rowptr = charimage, rowendptr = charimage + xsize - 1;
        i < ysize;
        i++,rowptr += xsize, rowendptr += xsize)
    {
        *rowptr = (unsigned char) NOEDGE;
        *rowendptr = (unsigned char) NOEDGE;
    }

    for(i=0,colptr = charimage, colendptr = charimage + xsize*(ysize - 1);
        i < xsize;
        i++, colptr++, colendptr++)
    {
        *colptr = (unsigned char) NOEDGE;
        *colendptr = (unsigned char) NOEDGE;
    } 
}


/* ------------------------------------------------------------------------ */
/* follow_edges is a recursive routine that traces edgs along all paths     */
/* whose magnitude values remain above some specifyable lower threshhold.   */
/* This lower threshold is a floating number between 0.0 and 1.0 and        */
/* defines the threshold in terms of the fraction of the largest value of   */
/* the magnitude.                                                           */
/* ------------------------------------------------------------------------ */


void follow_edges(edgemapptr,edgemagptr)
    unsigned char *edgemapptr;
    short *edgemagptr;
{   
    short *tempmagptr;
    unsigned char *tempmapptr;
    int i;


/*debug*/      if (bugflag) 
/*debug*/      {  
/*debug*/          printf("%d %d %d %d %d\n%d %d %d %d %d\n%d %d %d %d %d\n%d %d %d %d %d\n%d %d %d %d %d\n\n",
/*debug*/                            *(edgemapptr-2*xdim-2),*(edgemapptr-2*xdim-1),*(edgemapptr-2*xdim),*(edgemapptr-2*xdim+1),*(edgemapptr-2*xdim+2),
/*debug*/                      *(edgemapptr-xdim-2),*(edgemapptr-xdim-1),*(edgemapptr-xdim),*(edgemapptr-xdim+1),*(edgemapptr-xdim+2),
/*debug*/                      *(edgemapptr-2),*(edgemapptr-1),*(edgemapptr),*(edgemapptr+1),*(edgemapptr+2),
/*debug*/                      *(edgemapptr+xdim-2),*(edgemapptr+xdim-1),*(edgemapptr+xdim),*(edgemapptr+xdim+1),*(edgemapptr+xdim+2),
/*debug*/                      *(edgemapptr+2*xdim-2),*(edgemapptr+2*xdim-1),*(edgemapptr+2*xdim),*(edgemapptr+2*xdim+1),*(edgemapptr+2*xdim+2));
    
/*debug*/          printf("%d %d %d %d %d\n%d %d %d %d %d\n%d %d %d %d %d\n%d %d %d %d %d\n%d %d %d %d %d\n\n",
/*debug*/                            *(edgemagptr-2*xdim-2),*(edgemagptr-2*xdim-1),*(edgemagptr-2*xdim),*(edgemagptr-2*xdim+1),*(edgemagptr-2*xdim+2),
/*debug*/                      *(edgemagptr-xdim-2),*(edgemagptr-xdim-1),*(edgemagptr-xdim),*(edgemagptr-xdim+1),*(edgemagptr-xdim+2),
/*debug*/                      *(edgemagptr-2),*(edgemagptr-1),*(edgemagptr),*(edgemagptr+1),*(edgemagptr+2),
/*debug*/                      *(edgemagptr+xdim-2),*(edgemagptr+xdim-1),*(edgemagptr+xdim),*(edgemagptr+xdim+1),*(edgemagptr+xdim+2),
/*debug*/                      *(edgemagptr+2*xdim-2),*(edgemagptr+2*xdim-1),*(edgemagptr+2*xdim),*(edgemagptr+2*xdim+1),*(edgemagptr+2*xdim+2));
 
/*debug*/      }




    for(i=0;i<8;i++)
    {   
        tempmapptr = edgemapptr - y[i]*xdim + x[i];
        tempmagptr = edgemagptr - y[i]*xdim + x[i];
                                  
        if( (*tempmapptr == POSSIBLE_EDGE) && (*tempmagptr > thelothresh))
        {    
            *tempmapptr = (unsigned char) EDGE;
 
/*debug*/      if (bugflag) 
/*debug*/      {  
/*debug*/          printf("%d %d %d %d %d\n%d %d %d %d %d\n%d %d %d %d %d\n%d %d %d %d %d\n%d %d %d %d %d\n\n",
/*debug*/                            *(tempmapptr-2*xdim-2),*(tempmapptr-2*xdim-1),*(tempmapptr-2*xdim),*(tempmapptr-2*xdim+1),*(tempmapptr-2*xdim+2),
/*debug*/                      *(tempmapptr-xdim-2),*(tempmapptr-xdim-1),*(tempmapptr-xdim),*(tempmapptr-xdim+1),*(tempmapptr-xdim+2),
/*debug*/                      *(tempmapptr-2),*(tempmapptr-1),*(tempmapptr),*(tempmapptr+1),*(tempmapptr+2),
/*debug*/                      *(tempmapptr+xdim-2),*(tempmapptr+xdim-1),*(tempmapptr+xdim),*(tempmapptr+xdim+1),*(tempmapptr+xdim+2),
/*debug*/                      *(tempmapptr+2*xdim-2),*(tempmapptr+2*xdim-1),*(tempmapptr+2*xdim),*(tempmapptr+2*xdim+1),*(tempmapptr+2*xdim+2));
                                    
/*debug*/          printf("Came from (%d, %d).\n",-x[i],-y[i]);
/*debug*/          printf("%d %d %d %d %d\n%d %d %d %d %d\n%d %d %d %d %d\n%d %d %d %d %d\n%d %d %d %d %d\n\n",
/*debug*/                            *(tempmagptr-2*xdim-2),*(tempmagptr-2*xdim-1),*(tempmagptr-2*xdim),*(tempmagptr-2*xdim+1),*(tempmagptr-2*xdim+2),
/*debug*/                      *(tempmagptr-xdim-2),*(tempmagptr-xdim-1),*(tempmagptr-xdim),*(tempmagptr-xdim+1),*(tempmagptr-xdim+2),
/*debug*/                      *(tempmagptr-2),*(tempmagptr-1),*(tempmagptr),*(tempmagptr+1),*(tempmagptr+2),
/*debug*/                      *(tempmagptr+xdim-2),*(tempmagptr+xdim-1),*(tempmagptr+xdim),*(tempmagptr+xdim+1),*(tempmagptr+xdim+2),
/*debug*/                      *(tempmagptr+2*xdim-2),*(tempmagptr+2*xdim-1),*(tempmagptr+2*xdim),*(tempmagptr+2*xdim+1),*(tempmagptr+2*xdim+2));
 
/*debug*/      }



            follow_edges(tempmapptr,tempmagptr);
        }
    } 
}

/* -------------------------------------------------------------------------- */
/* This routine finds edges that are above some hi threshhold and calls       */
/* follow edges.                                                              */
/* -------------------------------------------------------------------------- */

void find_edges(map,mag,xsize,ysize,maxmag,hpixel_fraction,lpixel_fraction,hgram,hsize,actual_hthresh,actual_lthresh)
    unsigned char *map;
    short *mag;
    int xsize, ysize,maxmag,*hgram,hsize,*actual_hthresh,*actual_lthresh;
    float hpixel_fraction,lpixel_fraction;
{
    int hthresh;
    int i;
    float hithresh,lothresh;
    short *magptr;
    unsigned char *mapptr;

/*	DEBUG code - JMM
	fprintf(stderr,"find_edges: START EDGE is %d NOEDGE %d\n",EDGE,NOEDGE);
*/

    xdim = xsize;
    ydim = ysize;

    /* Set the borders of the edge map to zero as this will insure that the recusive */
    /* procedure follow_edges will terminate without overrunning the edges           */
                              
/*debug    printf("\n clear borders \n");*/

    clear_borders(map,xsize,ysize);
                                    

    /* Histogram the magenitude image to find whare the pixel values lie */ 

/*debug    printf("\n histogram \n"); */

    histogram(mag,xsize,ysize,maxmag,0,hgram,hsize); 
    
    /* Use the histogram to get a pixel threshold below which h_pixel_fraction  */
    /* pixels lie */

/*debug    printf("\n hist threshold \n"); */

    get_histogram_threshold(hgram,hsize,maxmag,0,hpixel_fraction,1,&hithresh,&lothresh);

    *actual_hthresh = hthresh = nearestint(hithresh);

    /* Compute the lower threshold as l_pixel_fraction fraction of the hi threshold */
    /* computed from the histogram */

/*	DEBUG code - JMM
fprintf(stderr,"find_edges: before assigned thelothresh is %d\n",thelothresh);
*/
    *actual_lthresh = thelothresh = nearestint(hithresh*lpixel_fraction);
/*	DEBUG code - JMM
fprintf(stderr,"find_edges: thelothresh is %d\n",thelothresh);
*/

    /* This loop looks for pixels above hthresh and then calls follow_edges on that pixel. */

/*debug    printf("\n follow_edges \n"); */

    for(i = 0, magptr = mag, mapptr = map;i < xsize*ysize; i++, magptr++, mapptr++) 
    {
        if ((*mapptr == POSSIBLE_EDGE) && (*magptr >= hthresh))
        {
            *mapptr = EDGE;
/*	DEBUG code - JMM
fprintf(stderr,"fin_edges:  EDGE FOUND at offset i = %d\n",i);
*/
            follow_edges(mapptr,magptr);
        }
    }

    /* Cleanup sets the remaining possible edges to nonedges */ 

/*debug    printf("\n clear borders \n"); */

    cleanup(map,xsize,ysize);
}


/* ------------------------------------------------------------------------------- */
/* Cleanup sets the remaining possible edges to nonedges                           */
/* ------------------------------------------------------------------------------- */

void cleanup(map,xsize,ysize)
    unsigned char *map;
    int xsize,ysize;
{
    unsigned char *mapptr;
    int i;

    for(i=0, mapptr = map; i<xsize*ysize; i++, mapptr++)
    {
        if (*mapptr != (unsigned char) EDGE)
            *mapptr= (unsigned char) NOEDGE;
    }
}
