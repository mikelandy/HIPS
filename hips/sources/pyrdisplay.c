/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * pyrdisplay - convert an image pyramid to a single image for display
 *
 * usage:	pyrdisplay [-c] [-m margin]
 *
 * to load:	cc -o pyrdisplay pyrdisplay.c -lhips
 *
 * Pyrdisplay converts an image pyramid to a single image for display
 * purposes.  There are two formats, either the default side-by-side format:
 *
 * 11111111 2222 33 4
 * 11111111 2222 33
 * 11111111 2222
 * 11111111 2222
 * 11111111
 * 11111111
 * 11111111
 * 11111111
 *
 * or a `compressed' format (specified with -c):
 *
 * 11111111 2222
 * 11111111 2222
 * 11111111 2222
 * 11111111 2222
 * 11111111
 * 11111111 33 4
 * 11111111 33
 * 11111111
 *
 * The only difference is that the third and higher levels are shifted around
 * to a second row.  The user may specify the number of margin pixels between
 * each image (using -m, the default is zero) and the pixel value for
 * background pixels (using the standard switch -UL).  Floating pyramids
 * result in floating point images, and integer pyramids result in integer
 * pyramids.
 *
 * Mike Landy - 3/6/89
 * Hips 2 - msl - 7/19/91
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {
    {"c",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
    {"m",{LASTFLAG},1,{{PTINT,"0","margin"},LASTPARAMETER}},
    LASTFLAG};
int types[] = {PFINTPYR,PFFLOATPYR,LASTTYPE};

int main(argc,argv)

int argc;
char **argv;

{
	struct header hdi,hdo;
	int f,fr,nr,nc,i,j,toplev,margin,nor,noc,noctop,nocbottom,one=1;
	FPYR fpyr;
	IPYR ipyr;
	Filename filename;
	FILE *fp;
	h_boolean cflag;
	Pixelval val;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&cflag,&margin,FFONE,&filename);
	Image_border = 0;
	fp = hfopenr(filename);
	fread_hdr_cpf(fp,&hdi,types,filename);
	getparam(&hdi,"toplev",PFINT,&one,&toplev);
	nr = hdi.orows; nc = hdi.ocols; fr = hdi.num_frame;
	dup_headern(&hdi,&hdo);
	setformat(&hdo,(hdi.pixel_format == PFINTPYR) ? PFINT : PFFLOAT);
	if (cflag) {
		nor = nr;
		if (toplev > 1)
			nor = MAX(nor,((nr+1)/2)+((nr+3)/4)+margin);
		if (toplev == 0)
			noc = nc;
		else if (toplev == 1)
			noc = nc + ((nc+1)/2) + margin;
		else {
			noctop = nc + ((nc+1)/2) + margin;
			nocbottom = nc;
			for (i=2,j=(nc+3)/4;i<=toplev;i++,j=(j+1)/2)
				nocbottom += margin + j;
			noc = MAX(noctop,nocbottom);
		}
	}
	else {
		nor = nr;
		noc = nc;
		for (i=1,j=(nc+1)/2;i<=toplev;i++,j=(j+1)/2)
			noc += margin + j;
	}
	setsize(&hdo,nor,noc);
	alloc_image(&hdo);
	if (hdo.pixel_format == PFFLOAT) {
		def_fpyr(fpyr,0,nr,nc);
		alloc_fpyr(fpyr,0,toplev);
		val.v_float = hips_lchar;
	}
	else {
		def_ipyr(ipyr,0,nr,nc);
		alloc_ipyr(ipyr,0,toplev);
		val.v_int = hips_lchar;
	}
	h_setimage(&hdo,&val);
	write_headeru(&hdo,argc,argv);
	for (f=0;f<fr;f++) {
		if (hdo.pixel_format == PFFLOAT) {
			read_fpyr(fp,fpyr,0,toplev,f,filename);
			h_pyrdisp(fpyr,0,toplev,&hdo,cflag,margin);
		}
		else {
			read_ipyr(fp,ipyr,0,toplev,f,filename);
			h_pyrdisp(ipyr,0,toplev,&hdo,cflag,margin);
		}
		write_image(&hdo,f);
	}
	return(0);
}
