/*
 *	PROGRAM
 *		cle_lex
 *
 *	PURPOSE
 *		to clear the Lexidata screen
 */

cle_lex(xul,yul,cols,rows,color,full_scr)
int full_scr;
unsigned short xul, yul, cols, rows;
unsigned char color;
{
	short err, chan, temp, rectlim[8], scolr, count;

	scolr = color;

	/* open the I/O channel to the lexidata */
	dsopn_(&err, &chan);

	if (full_scr && color == 0) {
		/* clear everything */
		temp = -1;
		dsclr_(&temp);
	} else {
		rectlim[0] = xul;
		rectlim[1] = yul;
		rectlim[2] = 0x5000 + cols;
		rectlim[3] = 0x6000 + rows;
		rectlim[4] = 0x5fff & -(short)cols;
		count = 5;
		dspoly_(&scolr,rectlim,&count);
	}

	/* close the I/O channel */
	dscls_();
}

