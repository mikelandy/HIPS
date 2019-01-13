#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned char *getpicbuf(xdim,ydim)

int xdim,ydim;

{
	unsigned char *pic;
	void *malloc();
	if ( (pic = malloc(xdim*ydim)) == NULL) { 
		fprintf(stderr,"difficulty in mallocing a pic buf!\n");
		exit(-1);
	}
	return(pic);
}

