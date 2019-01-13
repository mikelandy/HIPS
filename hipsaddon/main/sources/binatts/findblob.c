/* Return the first pixel in the first blob encountered
 * in a frame when scaning top to bottom & left to right.
 *
 * B.A.Shepherd, 1984.
 */

#define POSPIC 0
#define NEGPIC 1

int find_blob(bf,xdim,ydim,x,y)

unsigned char *bf ;
int *x,*y,xdim,ydim;

{
	register unsigned char *rbp,*picbot ;	

	rbp = bf ;
	picbot = rbp + xdim*ydim ;
	while ((*rbp <= 0 ) && (rbp < picbot)) rbp++ ;

	if (rbp >= picbot)
		return(0);	/* no blob found! */
	*x = (rbp-bf) % xdim ;
	*y = (rbp-bf) / xdim ;
	return(1); 		/* blob found */
}		


/* same as above but starts its search from (x,y) */
int next_blob(bf,xdim,ydim,x,y)

unsigned char *bf ;
int *x,*y,xdim,ydim;

{
	register unsigned char *rbp,*picbot ;	

	picbot = bf + xdim*ydim ;
	rbp = bf + (*y) * xdim + (*x) ;
	while ((*rbp <= 0 ) && (rbp < picbot)) rbp++ ;

	if (rbp >= picbot)
		return(0);	/* no blob found! */
	*x = (rbp-bf) % xdim ;
	*y = (rbp-bf) / xdim ;
	return(1); 		/* blob found */
}		
