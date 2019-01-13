/*
 * mframl.c - write a frame on the Lexidata
 *
 * Charles Carman 9/23/86
 */

#include <stdio.h>

mframl(cols,rows,left_col,top_row,thresh) 
	int cols,rows,left_col,top_row,thresh;
{
	register short *pb;
	short buf[520];
	short err, chan, count, mode, zero;
	short i, j, right_col, bottom_row, sleft_col, stop_row;

	zero = 0;
	mode = 0x3000;

	right_col = left_col + cols - 1;
	bottom_row = top_row + rows - 1;
	sleft_col = left_col; stop_row = top_row;
	count = cols;

	dsopn_(&err, &chan);
	dsdisp_(&mode, &zero, &zero);
	dslim_(&sleft_col,&stop_row,&right_col,&bottom_row);

	for (i=stop_row;i<=bottom_row;i++) {
		pb = buf;
		for (j=sleft_col;j<=right_col;j++,pb++) {
			*pb = 0377 & getchar();
			if (*pb <= thresh)  *pb = 0;
		}
		dsput_(buf,&count);
		dsowt_();
	}

	dsdisp_(&zero, &zero, &zero);
	dscls_();
}
