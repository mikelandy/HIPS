
void copy_frame(sbf,dbf,xdim,ydim)

unsigned char *sbf,*dbf ;
int xdim,ydim;

{
	register unsigned char *sbp,*dbp,*picend ;

	sbp = sbf ;
	dbp = dbf ;
	picend = sbf + xdim*ydim ;
	while ( sbp < picend)
		*dbp++ = *sbp++ ;
}
