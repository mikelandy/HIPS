/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/* view_frame.c - to generate a conic perspective of a 3D graph.
 *		 the "dist" argument (must be positive) is the distance
 *		 from the (0,0,0) point to the the picture plane on which the
 *		 graph is projected.
 *		 The picture is on a plane which is parallel to the x-y plane.
 *		 any point which is located behind dist/100 is discarded.
 *		 Likewise, any vector which intersects the plane (parallel
 *		 to the x-y plane) at z=dist/100 is projected up to that
 *		intersection point.
 *		 It is assumed that the input frame is "transformed".
 *
 * Load with:  -lhips
 * 		note: the function calls a "perror" function which must be
 *		supplied with the main program.
 *
 * Yoav Cohen 11/11/82
 * modified for HIPS 2 - msl - 1/3/91
 */

#include <hipl_format.h>

int view_frame(inbuf,nbytes,outbuf,limit,dist)

char *inbuf,*outbuf;
double dist;
int nbytes,limit;

{
	int in,out;
	double lastx,lasty,lastz,lastb,z,zz1,zz2;
	double newx,newy;
	double x1,y1,z1,x2,y2,z2,br,d100;
	int op;

	d100=dist/100.;
	for(in=0,out=0; in<nbytes;) {
		in=getplot(inbuf,in,&op,&br,&x1,&y1,&z1,&x2,&y2,&z2);
		switch(op) {
		case	'p':	lastb=br; lastx=x1; lasty=y1; lastz=z1;
				if(z1<d100)break;
				z=dist/z1;
				x1=x1*z; y1=y1*z;
					out=addpoint(outbuf,out,limit,br,x1,
						y1,z1);
				break;
		case	'v':	lastb=br; lastx=x2; lasty=y2; lastz=z2;
		compute:	if(z1<d100 && z2<d100) break;
				if(z1<d100 || z2<d100) {
					zz2=z1-z2;
					zz1=z1-d100;
					newx=x1+zz1*(x2-x1)/zz2;
					newy=y1+zz1*(y2-y1)/zz2;
					if(z1<d100) {
						x1=newx; y1=newy; z1=d100;
					}
					else {
						x2=newx; y2=newy; z2=d100;
					}
				}
				zz1=dist/z1; x1=x1*zz1; y1=y1*zz1;
				zz1=dist/z2; x2=x2*zz1; y2=y2*zz1;
				out=addvec(outbuf,out,limit,br,x1,y1,z1,x2,y2,
					z2);
				break;
		case	'n':	x1=lastx; y1=lasty; lastx=x2; lasty=y2;
				z1=lastz; lastz=z2; br=lastb;
				if(z1>=d100 && z2>=d100) {
					zz1=dist/z2; x2=x2*zz1; y2=y2*zz1;
					out=addend(outbuf,out,limit,x2,y2,z2);
						break;
				}
				goto compute;
		default:	return(perr(HE_CODE,"view_frame"));
		}
	}
	return(out);
}
