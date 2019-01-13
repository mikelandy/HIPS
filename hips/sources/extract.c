/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * extract - extract a subpicture
 *
 * usage: extract [-s rows [cols]] [-p from-row [from-col]] < in > out
 *
 * defaults:	rows,cols: 1/2 of input dimensions
 *		from-row,from-col: such that sub-picture is centered
 * 
 * Extract extracts a subpicture from the input image.  If ``rows'' is
 * negative then from-row is taken to be the final row of the subpicture.
 * Similarly if ``cols'' is negative then from-col is taken to be the final
 * columns of the subpicture.  Only those elements of the subpicture which
 * actually lie in the input image are output.
 *
 * Types handled directly: all raster formats (see h_copy.c)
 *
 * to load:	cc -o extract extract.c -lhipsh -lhips -lm
 *
 * HIPS 2 - msl - 1/16/91
 */

#include <stdio.h>
#include <hipl_format.h>
int types[] = {PFMSBF,PFLSBF,PFSBYTE,PFBYTE,PFSHORT,PFUSHORT,PFINT,PFUINT,
		PFFLOAT,PFDOUBLE,PFCOMPLEX,PFDBLCOM,PFMIXED,LASTTYPE};
static Flag_Format flagfmt[] = {
    {"s",{LASTFLAG},1,{{PTINT,"-999999","rows"},{PTINT,"-999999","cols"},
	LASTPARAMETER}},
    {"p",{LASTFLAG},1,{{PTINT,"-999999","fromrow"},{PTINT,"-999999","fromcol"},
	LASTPARAMETER}},
    LASTFLAG};

int main(argc,argv)

int argc;
char **argv;

{
	int fr,f,r,c,nr,nc,ir,ic,nrspec,ncspec,frspec,fcspec;
	int *fmts,fmtssize,method;
	hsize_t currsize;
	struct header hd,hdp,hdo;
	struct hips_roi roi;
	Filename filename;
	FILE *fp;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&nrspec,&ncspec,&frspec,&fcspec,
		FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	method = fset_conversion(&hd,&hdp,types,filename);
	r=hd.orows; c=hd.ocols;
	nr=r/2; nc=c/2;
	if (nrspec != -999999)
		nr = nrspec;
	ir = (r - ((nr<0) ? -nr : nr))/2;
	if (ncspec != -999999)
		nc = ncspec;
	ic = (c - ((nc<0) ? -nc : nc))/2;
	if (frspec != -999999) {
		ir = frspec;
		if (nr < 0)
			ir += nr + 1;
	}
	if (fcspec != -999999) {
		ic = fcspec;
		if (nc < 0)
			ic += nc + 1;
	}
	if (nr<0)
		nr = -nr;
	if (ir<0) {
		nr += ir;
		ir = 0;
	}
	if (ic<0) {
		nc += ic;
		ic = 0;
	}
	if (ir+nr > r)
		nr =  r - ir;
	if (ic+nc > c)
		nc =  c - ic;
	if (nr<=0 || nc<=0)
		 perr(HE_ZNEG);
	dup_headern(&hdp,&hdo);
	setsize(&hdo,nr,nc);
	getroi(&hdp,&roi);
	roi.frow -= ir;
	if (roi.frow < 0) {
		roi.rows += roi.frow;
		roi.frow = 0;
	}
	roi.fcol -= ic;
	if (roi.fcol < 0) {
		roi.cols += roi.fcol;
		roi.fcol = 0;
	}
	if (roi.frow + roi.rows > nr)
		roi.rows = nr - roi.frow;
	if (roi.fcol + roi.cols > nc)
		roi.cols = nc - roi.fcol;
	setroi2(&hdo,&roi);
	alloc_image(&hdo);
	write_headeru(&hdo,argc,argv);
	clearroi(&hdp);
	clearroi(&hdo);
	fr = hdp.num_frame;
	if (hd.pixel_format != PFMIXED) {
		for (f=0;f<fr;f++) {
			fread_imagec(fp,&hd,&hdp,method,f,filename);
			h_extract(&hdp,&hdo,ir,ic,nr,nc);
			write_image(&hdo,f);
		}
	}
	else {
		fmtssize = hd.num_frame;
		getparam(&hd,"formats",PFINT,&fmtssize,&fmts);
		if (fmtssize != hd.num_frame)
			perr(HE_FMTSLEN,filename);
		setformat(&hd,fmts[0]);
		alloc_image(&hd);
		setformat(&hdo,fmts[0]);
		alloc_image(&hdo);
		currsize = hd.sizeimage;
		for (f=0;f<hd.num_frame;f++) {
			setformat(&hd,fmts[f]);
			setformat(&hdo,fmts[f]);
			if (hd.sizeimage > currsize) {
				free(hd.image);
				alloc_image(&hd);
				free(hdo.image);
				alloc_image(&hdo);
				currsize = hd.sizeimage;
			}
			fread_image(fp,&hd,f,filename);
			h_extract(&hd,&hdo,ir,ic,nr,nc);
			write_image(&hdo,f);
		}
	}
	return(0);
}
