/*
 *	PROGRAM
 *		getquad
 *
 *	PURPOSE
 *		to recursively compute the coordinates of the upper left corner
 *		of a "quadrant" in the image, using the letters "abcd" to 
 *		designate the quadrant selected in each division by 2.
 *			-------------
 *			!  a  !  b  !
 *			-------------
 *			!  c  !  d  !
 *			-------------
 *
 *	AUTHOR
 *		Charles Carman
 *	for	Merickel Imageing Lab
 *		Biomedical Engineering
 *		University of Virginia
 *		Charlottesville, VA 22903
 */
getquad(pv, px, py, ix, iy)
unsigned short ix, iy, *px, *py;
char *pv;
{
	char c;

	if ((c = *pv) >= '0' && c <= '9') return(2);
	switch (c)  {
		case '\0':
			break;
		case 'a':
			getquad(++pv, px, py, ix/2, iy/2);
			break;
		case 'b':
			*px += ix;
			getquad(++pv, px, py, ix/2, iy/2);
			break;
		case 'c':
			*py += iy;
			getquad(++pv, px, py, ix/2, iy/2);
			break;
		case 'd':
			*px += ix;
			*py += iy;
			getquad(++pv, px, py, ix/2, iy/2);
			break;
		default:
			return(0);
	}
	return(1);
}
