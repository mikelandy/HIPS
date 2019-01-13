/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/* cut_frame.c - to cut a frame by a rectangular mask;
 *		 only the points that ovelap the mask are saved
 *		 in the output frame.
 *		 Returns the number of bytes in the new frame.
 *
 * Yoav Cohen 11/11/82
 * modified for HIPS 2 - msl - 1/3/91
 */

#include <hipl_format.h>
#define inrange(x,y)  ((x<x0 || x>xn || y<y0 || y>yn)?0:1)

int cut_frame(inbuf,nbytes,outbuf,limit,x0,y0,xn,yn)

char *inbuf , *outbuf;
double	x0,y0,xn,yn;
int nbytes,limit;

{
	int in,out;
	int i,nsect,ibw,casen;
	double lastx,lasty,lastz,lastb,sectx[4],secty[4],sx,sy,bx,by;
	double slope,intercept;
	double x1,y1,z1,x2,y2,z2,br;
	int op,nbw;

	for(in=0,out=0; in<nbytes; ) {
		in=getplot(inbuf,in,&op,&br,&x1,&y1,&z1,&x2,&y2,&z2);
		casen=0;
		switch(op) {
		case	'p':	lastb=br; lastx=x1; lasty=y1; lastz=z1;
				if (inrange(x1,y1))
					out=addpoint(outbuf,out,limit,br,
						x1,y1,z1);
				break;
		case	'v':	lastb=br; lastx=x2; lasty=y2; lastz=z2;
				if ((inrange(x1,y1)) && (inrange(x2,y2))) {
					out=addvec(outbuf,out,limit,br,x1,y1,
						z1,x2,y2,z2);
					break;
				}
				else
		inter:		/* find intersections */
				if ((y1>yn && y2>yn)||(y1<y0 && y2<y0)
				  ||(x1>xn && x2>xn)||(x1<x0 && x2<x0))
					break;
		inter2:		nsect=0;
				if (y2-y1 != 0.0) {
					slope=(x2-x1)/(y2-y1);
					intercept=x1-y1*slope;
					sectx[nsect]=sx=y0*slope+intercept;
					secty[nsect]=y0;
					if (sx>=x0 && sx<=xn) nsect++;
					sectx[nsect]=sx=yn*slope+intercept;
					secty[nsect]=yn;
					if (sx>=x0 && sx<=xn) nsect++;
				}
				if (nsect<2 && x2-x1 !=0.0) {
					slope=(y2-y1)/(x2-x1);
					intercept=y1-x1*slope;
					secty[nsect]=sy=x0*slope+intercept;
					sectx[nsect]=x0;
					if (sy>=y0 && sy<=yn) nsect++;
					secty[nsect]=sy=xn*slope+intercept;
					sectx[nsect]=xn;
					if (sy>=y0 && sy<=yn) nsect++;
				}
				if (nsect==0)
					break;
				if (nsect==1)
					return(perr(HE_CUT1));
				/* check betweeness */
				nbw=0;
				for(i=0;i<2;i++) {
					bx=sectx[i]; by=secty[i];
					if (((x1<=bx && bx<=x2) ||
						(x1>=bx && bx>=x2)) &&
						((y1<=by && by<=y2) ||
						(y1>=by && by>=y2))) {
							nbw++; ibw=i;
					}
				}
				if (nbw==0)
					return(perr(HE_CUTI));
				if (nbw==1) {
					if (inrange(x1,y1)) {
						x2=sectx[ibw]; y2=secty[ibw];
						if (casen) goto back;
					}
					else {
						x1=sectx[ibw]; y1=secty[ibw];
					}
					out=addvec(outbuf,out,limit,br,x1,y1,
						z1,x2,y2,z2);
					break;
				}
				else
					out=addvec(outbuf,out,limit,br,
						sectx[0],secty[0],z1,sectx[1],
						secty[1],z2);
				break;
		case	'n':	x1=lastx; y1=lasty; lastx=x2; lasty=y2;
				z1=lastz; lastz=z2; br=lastb;
				if (inrange(x1,y1)) {
					if (inrange(x2,y2)) {
						out=addend(outbuf,out,limit,
							x2,y2,z2);
						break;
					}
				/* find intersection */
					casen=1; goto inter2;
				back:	casen=0;
					out=addend(outbuf,out,limit,x2,y2,z2);
					break;
				}
				else goto inter;
		default:	return(perr(HE_CODE,"cut_frame"));
		}
	}
	return(out);
}
