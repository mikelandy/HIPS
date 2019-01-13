/*	PROGRAM
 *		wrchn_lex
 *
 *	PURPOSE
 *		write chain coded outlines on Lexidata
 *
 *	SYNOPSIS
 *		wrchn_lex(tr,lc,pixval)
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

#define X_MASK 0x0fc0
#define Y_MASK 0x03f

wrchn_lex(tr,lc,pixval)
	short tr, lc;
	unsigned short pixval;
{
	extern int CC_DIR[2][8];
	int strnum,chnnum,quad,filflg,counter,drflg,cons;
	int x1,y1;
	short err, chan, count;
	short ot_buf[1024];

	dsopn_(&err,&chan);
	count = 0;
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
			ot_buf[count++] = x1 + lc;
			ot_buf[count++] = y1 + tr;

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
					dscvec_(&pixval,ot_buf,&count);
					count = 0;
				}
				else {
					ot_buf[count++] = 0x7000 |
					    (X_MASK & (CC_DIR[0][quad] << 6)) |
					    (Y_MASK & CC_DIR[1][quad]);
					if (count >= 1024) {
fprintf(stderr,"wrchain_lex: too many points for internal buffer\n");
						exit(2);
					}
				}
			}/*end while quad!=8*/
		}/*end else*/
	}/*end while drflg*/

	dscls_();
	return(0);
 }
