/* count blobs in a binary image */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "satts.h"
#define	true 1
#define false 0

int find_blob(),delete_blob();
void copy_frame();

/* if area = 0 all blobs are counted else only those with
 * area >= area_argument ( & < for a neg arg). */ 

int blobcnt(pic,xd,yd,area,debug)

unsigned char *pic;
int xd,yd,area,debug;

{
	int blobs,x,y,blobarea ;
	unsigned char *tempic ;

	blobs=0; 
	tempic = (unsigned char *)malloc(xd*yd);
	copy_frame(pic,tempic,xd,yd);
	while (find_blob(tempic,xd,yd,&x,&y)) {
		blobarea = delete_blob(tempic,xd,yd,x,y,debug);
		if (area)
			if (area < 0) {
				if (blobarea < -area)
					blobs++;
			}
			else {
				if (blobarea >= area)
					blobs++;
			}	
		else 
			blobs++;
	}
	return(blobs);
}

/* find the biggest blob in a binary image */

int biggest_blob(pic,xd,yd,maxx,maxy)

unsigned char *pic;
int xd,yd;
int *maxx,*maxy;

{
	int area,maxarea,x,y ;
	unsigned char *tpic,*getpicbuf();

	maxarea = *maxx = *maxy = x = y = 0 ;
	tpic = getpicbuf(xd,yd);
	copy_frame(pic,tpic,xd,yd);
	while ( find_blob(tpic,xd,yd,&x,&y) ) {
		if ((area=delete_blob(tpic,xd,yd,x,y,false)) > maxarea) {
			maxarea = area;
			*maxx= x;
			*maxy= y;
		}
	}
	free(tpic);
	return(maxarea) ;
}

void blobstats(pic,xd,yd,areaband,bandcnt,debug)

unsigned char *pic;
int xd,yd,debug;
int areaband[],bandcnt[] ;	/* delimited by 0 */

{
	int blobs,x,y,blobarea,i ;
	unsigned char *tempic ;

	blobs=0; 
	i = 0 ;
	while (areaband[i])
		bandcnt[i++] = 0 ; 
	bandcnt[i] = 0 ;	/* >= area */ 
	tempic = (unsigned char *)malloc(xd*yd);
	copy_frame(pic,tempic,xd,yd);
	while (find_blob(tempic,xd,yd,&x,&y)) {
			blobarea = delete_blob(tempic,xd,yd,x,y,debug);
			if (debug)
			 	printf("area was %d\n",blobarea);
			i = 0 ;
			while ((blobarea >= areaband[i])&&(areaband[i])) i++;
			bandcnt[i] += 1 ;
	}
}
