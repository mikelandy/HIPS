/* This is the 4 board version of bps_viewport, for systems with 4 DR256
	memory boards */

/*
 * loadimg - download movie frames into Adage memory
 *
 * cuesw adds a blank frame and a cue frame after the last movie frame.
 * cueval is the pixel value of the cue spot (effective for 8-bit movies
 * only).  blanksw adds a blank frame after the last movie frame.  Binary
 * acts as if the movie is binary for frame placement - image widths are
 * rounded up to a multiple of 32.  nocol1024 avoids overlapping the 1024
 * column boundary, and norow1024 avoids overlapping the 1024 row boundary.
 * binary movies are always written using nocol1024 because of problems with
 * incrementing the word mode address of pixels across that boundary.  It
 * turns out that there are problems from the BPS with both boundaries in
 * word mode, and with one boundary in 8-bit mode.  Thus, when movies will
 * be processed by bit-slice routines, nocol1024 and norow1024 should be
 * used. startframe is the ordinal position in the frame buffer where the
 * first frame will be placed.  Finally, file is the filename of the movie.
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
 *	3	binary movies must be LSBFIRST
 *	4	can't fit sequence in Adage memory
 *	5	not enough core
 *	6	can't allocate input frame core
 *	7	error reading primary input
 */

#include <hipl_format.h>
#include <stdio.h>
#include <sys/ikio.h>
#include <graphics/ik_const.h>
#include <ctype.h>

#define	strowb(imageno) ((((((imageno) / numfcol) % numfrow) < numfrow2) ? \
			0 : 1024) + \
			((((imageno) / numfcol) % numfrow2) * outr))
#define	strown(imageno) ((((imageno) / numfcol) % numfrow) * outr)
#define strow(imageno) (norow1024 ? strowb((imageno)) : strown((imageno)))
#define	stcolb(imageno) (((((imageno) % numfcol) < numfcol2) ? 0 : 1024) + \
			(((imageno) % numfcol2) * movoutc))
#define	stcoln(imageno) (((imageno) % numfcol) * movoutc)
#define stcol(imageno) (nocol1024 ? stcolb((imageno)) : stcoln((imageno)))
#define	stbitplane(imageno) ((imageno) / (numfrow*numfcol))
#define	stplane(imageno) (stbitplane((imageno)) % 2)
#define	stcard(imageno) (02 + (stbitplane((imageno)) / 2))

char *store,*sfr;
int nstore = 0;
int nsfr = 0;

loadimg(cuesw,cueval,blanksw,Binary,nocol1024,norow1024,startframe,file)

char *file;

{
	char *st,*ifr,*pifr;
	int i,j,ii,jj,numf,frame,binary,oldmode;
	int r,c,rcb,f,blankptr,cueptr;
	int numfcol,numfrow,fcol,frow,outr,outc,movoutc,outrcb,numoutf,conv;
	int numfcol2,numfrow2,cb,ocb,currarg;
	struct 	header hd;
	FILE *fp;

	binary = 0;
	if ((fp = fopen(file,"r")) == NULL)
		return(1);
	fread_header(fp,&hd,file);
	if (hd.pixel_format == PFLSBF) {
		binary++;
		Binary++;
		nocol1024++;
	}
	else if (hd.pixel_format == PFMSBF)
		return(3);
	else if (hd.pixel_format != PFBYTE)
		return(2);

	r=hd.orows; cb=c=hd.ocols;
	outr = (r + 1) & ~01;
	if (binary) {
		cb = (c + 7)/8;
		outc = (c + 037) & ~037;
		ocb = outc/8;
	}
	else {
		cb = c;
		outc = ocb = (c+1) & (~01);	/* the Adage only transfers
							an even number of
							bytes in one op */
	}
	movoutc = Binary ? ((c + 037) & ~037) : outc;
	rcb=r*cb;
	outrcb=outr*ocb;
	numoutf=f=hd.num_frame;

	numoutf += ((blanksw || cuesw) ? 1 : 0) + (cuesw ? 1 : 0);
	numfcol = nocol1024 ? ((1024/movoutc)*2) : (2048/movoutc);
	numfcol2 = numfcol/2;
	numfrow = norow1024 ? ((1024/outr)*2) : (2048/outr);
	numfrow2 = numfrow/2;
	if (numfcol*numfrow*(binary ? 8 : 1) < startframe+numoutf)
		return(4);
	if (nstore < outrcb) {
		nstore = outrcb;
		if ((store=(char *)calloc(outrcb,sizeof(char))) == 0) 
			return(5);
	}
	blankptr = startframe + f;
	cueptr = startframe + f + 1;

	st = store;
	for (i=0;i<outrcb;i++)
		*st++ = 0;

	if (cb!=ocb) {
		if (nsfr < rcb) {
			nsfr = rcb;
			if ((sfr = (char *) calloc(rcb,sizeof(char))) == 0)
				return(6);
		}
		ifr = sfr;
		conv = 1;
	}
	else {
		conv = 0;
		ifr = store;
	}


	if (binary)
		oldmode = Ik_set_mode(SET_32_BIT_MODE);
	else
		oldmode = Ik_set_mode(SET_8_BIT_MODE);

	if (blanksw || cuesw) {
		fcol = stcol(blankptr);
		frow = strow(blankptr);
		if (binary) {
			Ik_windowdma(fcol,ocb,IK_WDHXY_ADDR);
			i = HI_WORDXYH(stcard(blankptr),stplane(blankptr),
				(fcol>>5),frow);
			j = LO_WORDXYH(stcard(blankptr),stplane(blankptr),
				(fcol>>5),frow);
			Ik_dmawr8(IK_WD_ADDR,i,j,store,outrcb);
		}
		else {
			Ik_windowdma(fcol,ocb,IK_HXY_ADDR);
			Ik_dmawr8(IK_HXY_ADDR,fcol,frow,store,outrcb);
		}
	}
	if (cuesw) {
		if (binary) {
		    ii = (r < 8) ? r : 8;
		    for (i=0;i<ii;i++)
			store[((r/2)-(ii/2)+i)*ocb+(ocb/2)]=255;
		}
		else {
		    ii = (r < 8) ? r : 8;
		    jj = (c < 8) ? c : 8;
		    for (i=0;i<ii;i++)
			for (j=0;j<jj;j++)
			    store[((r/2)-(ii/2)+i)*ocb+((c/2)-(jj/2)+j)]=cueval;
		}
		fcol = stcol(cueptr);
		frow = strow(cueptr);
		if (binary) {
			Ik_windowdma(fcol,ocb,IK_WDHXY_ADDR);
			i = HI_WORDXYH(stcard(cueptr),stplane(cueptr),
				(fcol>>5),frow);
			j = LO_WORDXYH(stcard(cueptr),stplane(cueptr),
				(fcol>>5),frow);
			Ik_dmawr8(IK_WD_ADDR,i,j,store,outrcb);
		}
		else {
			Ik_windowdma(fcol,ocb,IK_HXY_ADDR);
			Ik_dmawr8(IK_HXY_ADDR,fcol,frow,store,outrcb);
		}
	}

	for (frame=0;frame<f;frame++) {
		if (fread(ifr,rcb*sizeof(char),1,fp) != 1)
			return(7);
		if (conv) {
			st = store;
			pifr = ifr;
			for (i=0;i<r;i++) {
				for (j=0;j<cb;j++) {
					*st++ = *pifr++;
				}
				st += ocb-cb;
			}
		}
		fcol = stcol(startframe+frame);
		frow = strow(startframe+frame);
		if (binary) {
			Ik_windowdma(fcol,ocb,IK_WDHXY_ADDR);
			i = HI_WORDXYH(stcard(startframe+frame),
				stplane(startframe+frame), (fcol>>5),frow);
			j = LO_WORDXYH(stcard(startframe+frame),
				stplane(startframe+frame), (fcol>>5),frow);
			Ik_dmawr8(IK_WD_ADDR,i,j,store,outrcb);
		}
		else {
			Ik_windowdma(fcol,ocb,IK_HXY_ADDR);
			Ik_dmawr8(IK_HXY_ADDR,fcol,frow,store,outrcb);
		}
	}
	Ik_set_mode(oldmode);
	fclose(fp);
	return(0);
}
