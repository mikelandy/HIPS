/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * discedge2.c - a discrete domain regional edge detector with offsets
 *
 * usage:	discedge2 [-s size] [-c variance-crit] [-t edgethresh]
 *
 * Where size is the length of a side of the nonoverlapping domains in which
 * the algorithm operates, variance-crit is the threshold on the variance
 * in the normalized region (normalized by mean only) below which no
 * edge is sought in that region, and edgethresh is a threshold applied to 
 * edge values. Size defaults to 7, variance-crit to 0, and edgethresh to 1.
 *
 * This program is an implementation of the discrete domain regional operator
 * described by G. B. Shaw (Comp. Graph. and Image Proc., 9, pp. 135-149 (1979).
 * The algorithm outlined therein is sketchy and contains errors, which
 * hopefully are corrected here.  Also, the article does not clarify what to do
 * with edges which appear to travel along a border of the region (the algorithm
 * purports to be symmetric with respect to horizontal and vertical edges, but
 * isn't really).  In this implementation, the first pixels on the light
 * side of a light/dark edge are marked, and when the light/dark boundary
 * travels along the boundary of the region, only the "middlemost" pixel is
 * marked, since otherwise horizontal edges will all include little "tails" at
 * an edge of each region.  Lastly, note that any excess after multiples of
 * size in rows and columns is not edge detected.
 * The computations are done with integer arithmetic with all pixels scaled by
 * size*size in order that the normalization by the mean can be exact.
 *
 * This program is a modification of discedge.c in which the algorithm
 * operates twice on an image, at offsets of (0,0), (1,1),...,(size-1,size-1).
 * The edge value which the algorithm would give to a given pixel at a given
 * offset is thresholded by edge-thresh, and if above threshold, then a bit is
 * set in the output image at that pixel. Bit 0 is set for offset (0,0), bit 1
 * for offset (1,1), and so on. Thus, the output can be thresholded at 1 or 
 * (2**size)-1 to give the "or" or "and" of the two offset images, respectively.
 * The output image is in byte format.
 *
 * to load:	cc -o discedge2 discedge2.c -lhipsh -lhips -lm
 *
 * Mike Landy 6/1/82
 * HIPS 2 - msl - 8/8/91
 */

#include <stdio.h>
#include <hipl_format.h>

int types[] = {PFBYTE,LASTTYPE};
static Flag_Format flagfmt[] = {
    {"s",{LASTFLAG},1,{{PTINT,"7","size"},LASTPARAMETER}},
    {"c",{LASTFLAG},1,{{PTDOUBLE,"0","variance-crit"},LASTPARAMETER}},
    {"t",{LASTFLAG},1,{{PTINT,"1","edgethresh"},LASTPARAMETER}},
    LASTFLAG};

int main(argc,argv)

int argc;
char **argv;

{
	int f,fr,method,size,thresh;
	struct header hd,hdp,hdo;
	Filename filename;
	FILE *fp;
	h_boolean imagecopy;
	struct hips_roi roi;
	double varcrit;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&size,&varcrit,&thresh,FFONE,&filename);
	fp = hfopenr(filename);
	if (varcrit < 0 || size < 2 || size > 15)
		perr(HE_MSG,"bad variance criterion or size");
	fread_hdr_a(fp,&hd,filename);
	method = fset_conversion(&hd,&hdp,types,filename);
	dup_headern(&hdp,&hdo);
	alloc_image(&hdo);
	write_headeru2(&hd,&hdo,argc,argv,hips_convback);
	fr = hdp.num_frame;
	imagecopy = (hdp.rows != hdp.orows || hdp.cols != hdp.ocols) ?
		TRUE : FALSE;
	getroi(&hdp,&roi);
	for (f=0;f<fr;f++) {
		fread_imagec(fp,&hd,&hdp,method,f,filename);
		if (imagecopy) {
			clearroi(&hdp);
			clearroi(&hdo);
			h_copy(&hdp,&hdo);
			setroi2(&hdp,&roi);
			setroi2(&hdo,&roi);
		}
		h_discedge2(&hdp,&hdo,size,(float) varcrit,thresh);
		write_imagec(&hd,&hdo,method,hips_convback,f);
	}
	return(0);
}
