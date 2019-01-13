/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * wrapimg - shift and wrap around an image sequence
 *
 * usage:	wrapimg [-s x-shift [y-shift [t-shift]]]
 *
 * Wrapimg shifts an input sequence with wrap-around.  In other words, pixels
 * which are shifted off one edge of the region reappear at the opposite
 * edge.  The x-shift/y-shift wraparound refer to pixels within the region
 * of interest.  The t-shift allows wraparound of frames, and applies to the
 * order frames are written regardless of any spatial shift and/or region of
 * interest.  Positive values of x-shift and y-shift move the image rightward
 * and upward, respectively.  The x-shift defaults to 1 and the other shifts
 * to 0.
 *
 * Pixel formats handled directly:  BYTE, SHORT, INT, FLOAT, DOUBLE, COMPLEX,
 *					DBLCOM
 *
 * to load:	cc -o wrapimg wrapimg.c -lhipsh -lhips
 *
 * Mike Landy - 11/13/92
 * added depths - msl - 3/8/94
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {
	{"s",{LASTFLAG},1,{{PTINT,"1","x-shift"},{PTINT,"0","y-shift"},
		{PTINT,"0","t-shift"},LASTPARAMETER}},
	LASTFLAG};
int types[] = {PFBYTE,PFSHORT,PFINT,PFFLOAT,PFDOUBLE,PFCOMPLEX,PFDBLCOM,
		LASTTYPE};

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd,hdp1,hdp2;
	int method,nc,f,fr,xshift,yshift,tshift,col,ncol,i,ndepth;
	Filename filename;
	FILE *fp;
	byte *pimage;
	hsize_t sizeim;
	struct hips_roi roi;
	h_boolean imagecopy = FALSE;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&xshift,&yshift,&tshift,
		FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	if (hd.rows != hd.orows || hd.cols != hd.ocols)
		imagecopy = TRUE;
	method = fset_conversion(&hd,&hdp1,types,filename);
	write_headeru2(&hd,&hdp1,argc,argv,hips_convback);
	dup_headern(&hdp1,&hdp2);
	alloc_image(&hdp2);
	fr = hdp1.num_frame;
	ndepth = hgetdepth(&hdp1);
	ncol = hdp1.numcolor;
	getroi(&hdp1,&roi);
	if (tshift) {
		nc = hd.ocols;
		sizeim = hdp1.sizeimage;
		if (method == METH_IDENT) {
			setsize(&hdp1,hdp1.orows,nc*fr);
			alloc_image(&hdp1);
			pimage = hdp1.image;
			fread_image(fp,&hdp1,0,filename);
			setsize(&hdp1,hdp1.orows,nc);
		}
		else {
			free_image(&hdp1);
			setsize(&hdp1,hdp1.orows,nc*fr);
			alloc_image(&hdp1);
			pimage = hdp1.image;
			setsize(&hdp1,hdp1.orows,nc);
			for (f=0;f<fr;f++) {
				hdp1.image = pimage + f*sizeim;
				fread_imagec(fp,&hd,&hdp1,method,f,filename);
			}
		}
		fr /= (ncol*ndepth);
		for (f=0;f<fr;f++) {
			for (col=0;col<ncol*ndepth;col++) {
				i = (f-tshift)%fr;
				if (i < 0)
					i += fr;
				hdp1.image = pimage +
					(i*ncol*ndepth+col)*sizeim;
				if (imagecopy) {
					clearroi(&hdp1);
					clearroi(&hdp2);
					h_copy(&hdp1,&hdp2);
					setroi2(&hdp2,&roi);
				}
				setroi2(&hdp1,&roi);
				h_translate(&hdp1,&hdp2,xshift,yshift,TRUE);
				write_imagec(&hd,&hdp2,method,hips_convback,f);
			}
		}
	}
	else {
		for (f=0;f<fr;f++) {
			fread_imagec(fp,&hd,&hdp1,method,f,filename);
			if (imagecopy) {
				clearroi(&hdp1);
				clearroi(&hdp2);
				h_copy(&hdp1,&hdp2);
				setroi2(&hdp1,&roi);
				setroi2(&hdp2,&roi);
			}
			h_translate(&hdp1,&hdp2,xshift,yshift,TRUE);
			write_imagec(&hd,&hdp2,method,hips_convback,f);
		}
	}
	return(0);
}
