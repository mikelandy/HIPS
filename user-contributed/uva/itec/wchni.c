/*	PROGRAM
 *		wrchn_itc
 *
 *	PURPOSE
 *		write chain coded outlines on Image Technology 
 *		frame buffers.
 *
 *	SYNOPSIS
 *		wrchn_itc(tr,lc)
 *
 *		a chain coded file is expected from standard input.
 *
 *	AUTHOR
 *		Thao Le 
 *		for
 *		Merickel Imaging Lab
 *		Biomedical Engineering
 *		University of Virginia
 *		Charlottesville, Va.  22903
 *
 *	REVISIONS
 *		5/13/86		Allowed for standard input. 
 *		Stuart Ware	Used Itec library routines.
 */
#include <stdio.h>
#include <hipl_format.h>
#include "image.h"

wrchn_itc(tr,lc,pixval)
	short tr, lc;
	unsigned short pixval;
{
	extern int CC_DIR[2][8];
	int strnum,chnnum,quad,filflg,counter,drflg,cons;
	int x1,y1;

	unmask(BW);

	counter = 0;
	quad = 0;
	drflg = 1;
	while (drflg)   {
		cons = fscanf(stdin,"%d %d %d %d\n",&strnum,&chnnum,&x1,&y1);

		/* Check to see if at the end of chain coded file 	*/
		if (cons == EOF) 
			drflg = 0;

		/* not at end of file					*/ 
		else
		{
			x1 += lc;
			y1 += tr;
			bwwrpt (pixval,(unsigned short)x1,(unsigned short)y1);
			quad = 0;
			while (quad!=8) 
			{
				counter++;
				if (counter>40) 
				{
					counter = 1;
					fscanf (stdin, "\n");
				}
				cons = fscanf (stdin,"%d ",&quad);
				if (cons==EOF) 
				{
					printf ("*\n");
					return(1);
				}
				if (quad == 8) {
					cons = fscanf (stdin,"\n");
				}
				else {
					x1 += CC_DIR[0][quad];
					y1 += CC_DIR[1][quad];
					bwwrpt (pixval,(unsigned short)x1,
						(unsigned short)y1);
				}
			}/*end while quad!=8*/
		}/*end else*/
	}/*end while drflg*/

	return(0);
 }
