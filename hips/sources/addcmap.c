/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * addcmap.c - add a colormap to a HIPS image header
 *
 * usage:	addcmap [-f colormapfile | -g gammar [gammag [gammab]]] < iseq
 *
 * Addcmap is used to add a colormap to an image, saving it in the header
 * parameter `cmap'.  The colormap is formatted as for the subroutine
 * readcmap.  -g is used to specify an inverse gamma lookup table, where
 * gammag defaults to gammar and gammab defaults to gammag.  The overall
 * default is -g 2.
 *
 * to load:	cc -o addcmap addcmap.c -lhips -lm
 *
 * Mike Landy - 8/16/91
 */

#include <stdio.h>
#include <hipl_format.h>
#include <math.h>

static Flag_Format flagfmt[] = {
	{"f",{"g",LASTFLAG},1,{{PTFILENAME,"","cmapfile"},LASTPARAMETER}},
	{"g",{"f",LASTFLAG},1,{{PTBOOLEAN,"TRUE"},{PTDOUBLE,"2","gammar"},
		{PTDOUBLE,"-1","gammag"},{PTDOUBLE,"-1","gammab"},
		LASTPARAMETER}},
	LASTFLAG};

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd;
	Filename mapfile,imagefile;
	FILE *fp;
	int count,i,c,*fmts,fmtssize;
	byte colormap[768],r[256],g[256],b[256];
	double gammar,gammag,gammab;
	h_boolean gflag;
	hsize_t currsize;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&mapfile,&gflag,&gammar,&gammag,&gammab,
		FFONE,&imagefile);
	fp = hfopenr(imagefile);
	fread_hdr_a(fp,&hd,imagefile);
	if (gflag) {
		if (gammag < 0)
			gammag = gammar;
		if (gammab < 0)
			gammab = gammag;
		gammar = 1./gammar;
		gammag = 1./gammag;
		gammab = 1./gammab;
		count = 256;
		colormap[0] = colormap[256] = colormap[512] = 0;
		for (i=1;i<256;i++) {
			colormap[i] = 255.*pow((double) i/255.,gammar) + .5;
			colormap[i+256] = 255.*pow((double) i/255.,gammag) +
				.5;
			colormap[i+512] = 255.*pow((double) i/255.,gammab) +
				.5;
		}
	}
	else {
		readcmap(mapfile,256,&count,r,g,b);
		for (i=0;i<count;i++) {
			colormap[i] = r[i];
			colormap[i+count] = g[i];
			colormap[i+2*count] = b[i];
		}
	}
	setparam(&hd,"cmap",PFBYTE,3*count,colormap);
	write_headeru(&hd,argc,argv);
	if (hd.sizeimage) {
		for (i=0;i<hd.num_frame;i++) {
			fread_image(fp,&hd,i,imagefile);
			write_image(&hd,i);
		}
	}
	else if (hd.pixel_format == PFMIXED) {
		fmtssize = hd.num_frame;
		getparam(&hd,"formats",PFINT,&fmtssize,&fmts);
		if (fmtssize != hd.num_frame)
			perr(HE_FMTSLEN,imagefile);
		setformat(&hd,fmts[0]);
		alloc_image(&hd);
		currsize = hd.sizeimage;
		for (i=0;i<hd.num_frame;i++) {
			setformat(&hd,fmts[i]);
			if (hd.sizeimage > currsize) {
				free(hd.image);
				alloc_image(&hd);
				currsize = hd.sizeimage;
			}
			fread_image(fp,&hd,i,imagefile);
			write_image(&hd,i);
		}
	}
	else {
		while ((c=getc(fp)) != EOF) putchar(c);
	}
	return(0);
}
