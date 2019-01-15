/*
 *	region.c - for use with SEGAL 3d
 *
 *	By Bryan Skene
 *
 */

#include "common.h"

/**************************************/
void
traverse_region(roi, x, y, z, f)
int roi;
int x, y, z;
void (*f)();
{
/* traverses the region specified by roi and feeds the coordinates to the
 * funciton f().  Pass in initial values for x, y, z.  This func will vary
 * the appropriate coordinates.  f() must take as arguments x, y, z.
 */
	set_watch_cursor();

	switch(roi) {
	case R2d_WHOLE :
		for(y = 0; y < win[WIN_PAINT].img_r; y++)
		for(x = 0; x < win[WIN_PAINT].img_c; x++)
			(*f)(x, y, z);
		break;
	case R2d_CROP :
		for(y = region.y1; y < region.y2; y++)
		for(x = region.x1; x < region.x2; x++)
			(*f)(x, y, z);
		break;
	case R2d_PT_LIST :
		for(y = 0; y < win[WIN_PAINT].img_r; y++)
		for(x = 0; x < win[WIN_PAINT].img_c; x++)
			if(BIT_IS_ON(win[WIN_PAINT].m_data[y][x], m[BUF_PTS].bit_key))
				(*f)(x, y, z);
		break;
	case R3d_WHOLE :
		for(z = 0; z < segal.f; z++)
		for(y = 0; y < segal.r; y++)
		for(x = 0; x < segal.c; x++)
			(*f)(x, y, z);
		break;
	case R3d_CUBE :
		for(z = region.f1; z < region.f2; z++)
		for(y = region.y1; y < region.y2; y++)
		for(x = region.x1; x < region.x2; x++)
			(*f)(x, y, z);
		break;
	case R3d_PT_LIST :
		for(z = 0; z < segal.f; z++)
		for(y = 0; y < segal.r; y++)
		for(x = 0; x < segal.c; x++)
			if(BIT_IS_ON(mbuf[z][y][x], m[MASK_ROI_3d].bit_key))
				(*f)(x, y, z);
		break;
	default :
		break;
	}

	unset_watch_cursor();
}
