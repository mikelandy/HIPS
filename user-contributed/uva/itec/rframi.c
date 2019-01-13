/*
 * rframi.c - read a frame from the itec
 *
 *	Charles Carman 10/22/86
 */

#include <stdio.h>
#include "image.sh"

rframi(c,r,ic,ir,color)
int	r,c,ir,ic;
char 	color;
{
	int		row;
	int		bytes_written;
	unsigned char	buf[COLMAX];
	
	for (row=ir;row<ir+r;row++)
	{
		fbrdln(color,buf,c,(short)ic,(short)row);
		bytes_written = fwrite((char *)buf,(unsigned)c,1,stdout);
		if (bytes_written != 1)
		{
			fprintf(stderr,"rframi: write error\n");
			exit(1);
		}
	}
	return(0);
}

