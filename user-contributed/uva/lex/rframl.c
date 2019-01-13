/*
 * rframl.c - read a frame from the Lexidata
 *
 * Usage:	rframl (cols, rows, left_col, top_row) 
 *
 * Load:	cc -c rframl.c 
 *
 */

#include <stdio.h>

rframl(cols,rows,left_col,top_row) 
	int cols,rows,left_col,top_row;
{
	int i, size, iter, iter_flg, end_size;
	short *buf, buflen;
	short err, chan, count, plns;
	short right_col, bottom_row, sleft_col, stop_row;
	char *outbuf, *malloc();

	right_col = left_col + cols - 1;
	bottom_row = top_row + rows - 1;
	sleft_col = left_col; stop_row = top_row;
	size = cols * rows;
	if (size <= 8192) {
		count = size;
		iter = 1;
		iter_flg = 0;
	} else {
		count = 8192;
		iter = size / 8192;
		end_size = size % 8192;
		if (end_size != 0) {
			iter++;
			iter_flg = 1;
		}
	}
	plns = 255;

	buf = (short *) malloc((unsigned) (count + 8));
	outbuf = malloc((unsigned) (count + 8));

	dsopn_(&err, &chan);
	dslim_(&sleft_col,&stop_row,&right_col,&bottom_row);

	for (i=0; i<iter; i++) {
		if (iter_flg && i == iter - 1)
			count = end_size;
		dsppr_(&plns,&count,buf);
		sleep((unsigned)1);
		swab((char *)buf,outbuf,(int)count);
		fwrite(outbuf,(int)count,1,stdout);
	}
	dscls_();
}
