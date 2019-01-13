#include <stdio.h>

magpic(srcbuf, snu, snv, xlo, xhi, ylo, yhi, dstbuf, dnu, dnv,
					m_xscl, m_yscl, sm_xscl, sm_yscl)
unsigned char srcbuf[], dstbuf[];
int *m_xscl, *m_yscl, sm_xscl, sm_yscl;
{
/*
 *	Try to magnify image in src subwindow by sm_xscl, sm_yscl.
 *	If to large to fit window return new scale to fit dst window.
 *	In either case the scale used is returned via m_xscl, m_yscl.
 *	xlo, xhi, ylo, yhi are src subwindow limits.
 *	snu, snv are dimensions of src image.
 *	dnu, dnv are dimensions of dst image.
 */
	register int i, j, is, js;
	register int snx, dbnx, sny, dbny;
	register unsigned char data, *srcP, *dstP, *dstlimP;
	int dy0, sk, snxXdbnx, dnuXdbny;
	int m_tobig = 0;

	snx  = xhi - xlo + 1;
	dbnx = dnu / snx;
	sny  = yhi - ylo + 1;
	dbny = dnv / sny;

		/* return x & y magnification scale factor if out of range */
	if(sm_xscl > dbnx){
		*m_xscl = dbnx;
		m_tobig = 1;
	}else{
		*m_xscl = sm_xscl;
		dbnx = *m_xscl;
	}
	if(sm_yscl > dbny){
		*m_yscl = dbny;
		m_tobig = 1;
	}else{
		*m_yscl = sm_yscl;
		dbny = *m_yscl;
	}
	if(m_tobig)
		printf("sm_scale(%d %d) to large: ",sm_xscl,sm_yscl,dbnx,dbny); 
	printf("magnifying image area with m_scale(%d %d)\n", dbnx, dbny);
	fflush(stdout);

	snxXdbnx = snx * dbnx;
	dnuXdbny = dnu * dbny;
	sk  = xlo + ylo*snu;
	dy0 = 0;
	for(js=ylo; js<=yhi; js++) {

			/* form first line of dbny by (snx+rt_filler) blocks */
		srcP = &srcbuf[sk];
		dstP = &dstbuf[dy0];
		for(is=0; is<snx; is++) {
			data = *srcP++;
			for(i=0; i<dbnx; i++)
				*dstP++ = data;
		}
			/* right edje filler block set to 0 */
		for(i=snxXdbnx; i<dnu; i++)
			*dstP++= 0;

			/* copy first line to remaining dbny-1 lines */
		dstP =	&dstbuf[dy0+dnu];
		for(j=1; j<dbny; j++) {
			srcP =	&dstbuf[dy0];
			for(i=0; i<dnu; i++)
				*dstP++ = *srcP++;
		}
		sk  += snu;
		dy0 += dnuXdbny;
	}

			/* fill remaining bottom lines with 0 */
	dstlimP = &dstbuf[dnu*dnv];
	while(dstP < dstlimP)
		*dstP++ = 0;
}
