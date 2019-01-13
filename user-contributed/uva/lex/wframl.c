/*
 * wframl.c - write a frame on the Lexidata
 *
 * Usage:	wframl (cols, rows, left_col, top_row, buf) 
 *
 * Load:	cc -c wframl.c 
 *
 */

#include <stdio.h>

wframl(cols,rows,left_col,top_row,ibuf) 
	int cols,rows,left_col,top_row;
	unsigned char *ibuf;
{
	register unsigned char *pib;
	register short *pob;
	register int j, chr_tmp;
	int i;
	short obuf[640];
	short err, chan, count;
	short right_col, bottom_row, sleft_col, stop_row;

	right_col = left_col + cols - 1;
	bottom_row = top_row + rows - 1;
	sleft_col = left_col; stop_row = top_row;
	count = cols;

	dsopn_(&err, &chan);
	dslim_(&sleft_col,&stop_row,&right_col,&bottom_row);

	pib = ibuf;
	for (i=stop_row;i<=bottom_row;i++) {
		pob = obuf;
		for (j=sleft_col;j<=right_col;j++)
			*pob++ = *pib++;

		dsput_(obuf,&count);
	}
	dscls_();
}
