/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/* pix_code.c -	To generate a byte-formatted pixel area from a PLOT3D buffer.
 *		It is assummed that all the coordinates are already
 *		in the range of the display area, and are incremented
 *		(by .5) so that truncation would result in rounding.
 *
 *
 * Mike Landy - 10/19/83
 * modified for HIPS 2 - msl - 1/3/91
 */

#include <hipl_format.h>

double lastx,lasty;
int lastib,codepoint(),codevec(),fillvect(),codeendp();

int pix_code(buf,nbytes,pic,rows,nlp)

char *buf;
int nbytes,rows,nlp;
byte *pic;

{
	int op,in;
	double br,x1,y1,z1,x2,y2,z2;

	lastib= -1;

	for(in=0;in<nbytes;) {
		in=getplot(buf,in,&op,&br,&x1,&y1,&z1,&x2,&y2,&z2);
		switch(op) {
		case 'p':	codepoint(br,x1,y1,pic,rows,nlp); break;
		case 'v':	codevec(br,x1,y1,x2,y2,pic,rows,nlp); break;
		case 'n':	codeendp(x2,y2,pic,rows,nlp); break;
		default :	return(perr(HE_CODE,"pix_code"));
		}
	}
	return(HIPS_OK);
}



int codepoint(b,x,y,pic,rows,nlp)

byte *pic;
double b,x,y;
int rows,nlp;

{
int ib,r,c;
	ib=b+.5;
	if(ib<0)ib=0; else if(ib>0)ib=255;
	r = (int)x & 0777;
	c = (int)y & 0777;
#ifdef ULORIG
	pic[(rows-(1+r))*nlp+c] = ib;
#else
	pic[r*nlp+c] = ib;
#endif
	return(HIPS_OK);
}


int codevec(b,x1,y1,x2,y2,pic,rows,nlp)

double b,x1,y1,x2,y2;
byte *pic;
int rows,nlp;

{
	int ib;

	ib=b+.5;
	if(ib<0)ib=0; else if(ib>0)ib=255;
	fillvect(ib,x1,y1,x2,y2,pic,rows,nlp);
	lastx=x2; lasty=y2; lastib=ib;
	return(HIPS_OK);
}


int codeendp(x,y,pic,rows,nlp)

byte *pic;
double x,y;
int rows,nlp;

{
	fillvect(lastib,lastx,lasty,x,y,pic,rows,nlp);
	lastx=x; lasty=y;
	return(HIPS_OK);
}

#ifdef ULORIG
#define setpnt(x,y)	pic[(rows-(1+y))*nlp+x]=ib
#else
#define setpnt(x,y)	pic[y*nlp+x]=ib
#endif

int dx[9] = {0,1,1,0,-1,-1,-1,0,1};
int dy[9] = {0,0,1,1,1,0,-1,-1,-1};

int fillvect(ib,fx0,fy0,fx1,fy1,pic,rows,nlp)

float fx0,fy0,fx1,fy1;
byte *pic;
int ib,rows,nlp;

{
	int x0,y0,x1,y1;
	int dmv,hmv,nd,nh,dh,rem,i,x,y,mv;

	x0 = fx0 + .5;
	y0 = fy0 + .5;
	x1 = fx1 + .5;
	y1 = fy1 + .5;

	if (x0 < x1) {
		if (y0 < y1) {
			if ((x1-x0) > (y1-y0)) {
				dmv = 2;
				hmv = 1;
				nd = y1 - y0;
				nh = (x1-x0) - nd;
			}
			else {
				dmv = 2;
				hmv = 3;
				nd = x1 - x0;
				nh = (y1-y0) - nd;
			}
		}
		else {
			if ((x1-x0) > (y0-y1)) {
				dmv = 8;
				hmv = 1;
				nd = y0 - y1;
				nh = (x1-x0) - nd;
			}
			else {
				dmv = 8;
				hmv = 7;
				nd = x1 - x0;
				nh = (y0-y1) - nd;
			}
		}
	}
	else {
		if (y0 < y1) {
			if ((x0-x1) > (y1-y0)) {
				dmv = 4;
				hmv = 5;
				nd = y1 - y0;
				nh = (x0-x1) - nd;
			}
			else {
				dmv = 4;
				hmv = 3;
				nd = x0 - x1;
				nh = (y1-y0) - nd;
			}
		}
		else {
			if ((x0-x1) > (y0-y1)) {
				dmv = 6;
				hmv = 5;
				nd = y0 - y1;
				nh = (x0-x1) - nd;
			}
			else {
				dmv = 6;
				hmv = 7;
				nd = x0 - x1;
				nh = (y0-y1) - nd;
			}
		}
	}

	setpnt(x0,y0);
	x = x0;
	y = y0;
	dh = nd + nh;
	rem = dh/2;
	
	for (i=0;i<dh;i++) {
		rem += nh;
		if (rem >= dh) {
			rem -= dh;
			mv = hmv;
		}
		else
			mv = dmv;
		x += dx[mv];
		y += dy[mv];
		setpnt(x,y);
	}
	return(HIPS_OK);
}
