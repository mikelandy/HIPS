/* This is the 4 board version of loadimgl, for systems with 4 DR256
	memory boards */

/*
 * loadimgl - download movie frames into Adage memory in lores mode
 *
 * calling sequence:
 *
 *	loadimgl(cuesw,cueval,blanksw,startframe,byteno,file)
 *
 * cuesw adds a blank frame and a cue frame after the last movie frame.
 * cueval is the pixel value of the cue spot. blanksw adds a blank frame
 * after the last movie frame.  startframe is the ordinal position in the
 * frame buffer where the first frame will be placed.  byteno is the byte
 * number (or board number) of the 32-bit lores pixels that will be written
 * by these images.  Finally, file is the filename of the movie.
 * Note that this routine is used by vaximgld.c in this directory, and by
 * the 68000/host communication program dmaconn, so after modifying it, both
 * vaximgld and dmaconn should be rebuilt.  dmaconn is to be found in
 * xp:/usr/spool/68000/nrtx/src/lib/dmahost, and 
 * hipl:/image/NRTX/68000/nrtx/src/lib/dmahost.
 *
 * loadimg returns 0 when successful.  Otherwise it returns an error code
 * indicating the reason for failure:
 *	1	can't open file
 *	2	frame must be in byte format
 *	3	UNDEFINED
 *	4	can't fit sequence in Adage memory
 *	5	not enough core
 *	6	can't allocate input frame core
 *	7	error reading primary input
 *	8	frame must not be bit-packed
 *
 * 03-17-88 PH  lores images should not cross 512 pixel column boundary
 *		
 */

#include <hipl_format.h>
#include <stdio.h>
#include <sys/ikio.h>
#include <graphics/ik_const.h>
#include <ctype.h>

#define	strow(imageno) ((((imageno) / numfcol) % numfrow) * outr)
#define	stcol(imageno) ( ( ( ((imageno) % numfcol) < numfcol2 ) ? 0 : 512 ) + \
			((imageno) % numfcol2) * outc)

char *store,*ostore;
int nistore = 0;
int nostore = 0;

loadimgl(cuesw,cueval,blanksw,startframe,byteno,file)

char *file;

{
	char *st,*ost;
	int i,j,ii,jj,jj4,frame,oldmode;
	int r,c,rcb,f,blankptr,cueptr;
	int numfcol,numfcol2,numfrow,fcol,frow,outr,outc,outrcb,numoutf;
	int ocb,currarg;
	struct header hd;
	FILE *fp;

	if ((fp = fopen(file,"r")) == NULL)
		return(1);
	fread_header(fp,&hd,file);
	if (hd.pixel_format == PFLSBF || hd.pixel_format == PFMSBF)
		return(8);
	else if (hd.pixel_format != PFBYTE)
		return(2);

	r=hd.orows; c=hd.ocols;
/*	This rounding doesn't appear to be needed in lores mode:
	outr = (r + 1) & ~01;
*/
	outr = r;
	outc = ocb = (c+1) & (~01);	/* the Adage only transfers
						an even number of
						bytes in one op */
	ocb *= 4;
	rcb=r*c;
	outrcb=outr*ocb;
	numoutf=f=hd.num_frame;

	numoutf += ((blanksw || cuesw) ? 1 : 0) + (cuesw ? 1 : 0);

	/* images must not cross 512 column boundary in lores mode */
	/* # images in half row of frame buffer i.e. 512 pixels */
	numfcol2 = (512/outc);
	/* # images in complete row of frame buffer i.e. 1024 pixels */
	numfcol = numfcol2 * 2;
	numfrow = 1024/outr;

	if (numfcol*numfrow < startframe+numoutf)
		return(4);
	if (nostore < outrcb) {
		if (nostore)
			cfree(ostore);
		nostore = outrcb;
		if ((ostore=(char *)calloc(outrcb,sizeof(char))) == 0) 
			return(5);
	}
	if (nistore < rcb) {
		if (nistore)
			cfree(store);
		nistore = rcb;
		if ((store=(char *)calloc(rcb,sizeof(char))) == 0) 
			return(6);
	}
	blankptr = startframe + f;
	cueptr = startframe + f + 1;

	ost = ostore;
	for (i=0;i<outrcb;i++)
		*ost++ = 0;

	oldmode = Ik_set_mode(SET_32_BIT_MODE);
	Ik_mask_set(0,(0xff << (8*byteno)));

	if (blanksw || cuesw) {
		fcol = stcol(blankptr);
		frow = strow(blankptr);
		Ik_windowdma(fcol,ocb,IK_XY_ADDR);
		Ik_dmawr8(IK_XY_ADDR,fcol,frow,ostore,outrcb);
	}
	if (cuesw) {
		ii = (r < 8) ? r : 8;
		jj = (c < 8) ? c : 8;
		jj4 = jj*4;
		for (i=0;i<ii;i++) {
			ost = ostore + ((r/2)-(ii/2)+i)*ocb
				+ ((c/2)-(jj/2))*4;
			for (j=0;j<jj4;j++)
				*ost++ = cueval;
		}
		fcol = stcol(cueptr);
		frow = strow(cueptr);
		Ik_windowdma(fcol,ocb,IK_XY_ADDR);
		Ik_dmawr8(IK_XY_ADDR,fcol,frow,ostore,outrcb);
	}

	for (frame=0;frame<f;frame++) {
		if (fread(store,rcb*sizeof(char),1,fp) != 1)
			return(7);
		st = store;
		ost = ostore;
		for (i=0;i<r;i++) {
			for (j=0;j<c;j++) {
				*ost++ = *st;
				*ost++ = *st;
				*ost++ = *st;
				*ost++ = *st++;
			}
			ost += 4*(outc-c);
		}
		fcol = stcol(startframe+frame);
		frow = strow(startframe+frame);
		Ik_windowdma(fcol,ocb,IK_XY_ADDR);
		Ik_dmawr8(IK_XY_ADDR,fcol,frow,ostore,outrcb);
	}
	Ik_mask_set(0,0xffffffff);
	Ik_set_mode(oldmode);
	fclose(fp);
	return(0);
}
