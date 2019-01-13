/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * luminance.c - convert a color or pseudocolor image to black-and-white
 *
 * usage:	luminance [-f colormapfile | -g [gammar [gammag [gammab]]]]
 *			[-p redweight greenweight blueweight]
 *
 * Luminance is used to apply a colormap to an image replacing pixels with
 * the corresponding lookup table entries, and then compute a weighted sum of
 * the three color values to compute a corresponding brightness.
 * The lookup table can come from
 * a file (-f), can be a standard inverse gamma table (-g) or come from the
 * `cmap' entry in the image header (the default).  Any such `cmap' entry is
 * then deleted from the header.  For -f, the colormap is formatted as for the
 * subroutine readcmap.  -g is used to specify an inverse gamma lookup table,
 * where gammar defaults to 2, gammag defaults to gammar and gammab defaults to
 * gammag.  If the image has one color plane, then that image is put through
 * all three lookup tables.
 * If the input has three color planes, the first is put through the red
 * color table, the second is put through the green, and the third through the
 * blue.  The pixels are then multiplied by the supplied weights (which
 * default to .299 (red), .587 (green) and .114 (blue).
 *
 * to load:	cc -o luminance luminance.c -lhipsh -lhips -lm
 *
 * Mike Landy - 8/17/91
 */

#include <stdio.h>
#include <hipl_format.h>
#include <math.h>

static Flag_Format flagfmt[] = {
	{"f",{"g",LASTFLAG},1,{{PTBOOLEAN,"FALSE"},{PTFILENAME,"","cmapfile"},
		LASTPARAMETER}},
	{"g",{"f",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},{PTDOUBLE,"2","gammar"},
		{PTDOUBLE,"-1","gammag"},{PTDOUBLE,"-1","gammab"},
		LASTPARAMETER}},
	{"p",{LASTFLAG},3,{{PTDOUBLE,".299","redweight"},
		{PTDOUBLE,".587","greenweight"},{PTDOUBLE,".114","blueweight"},
		LASTPARAMETER}},
	LASTFLAG};
int types[] = {PFBYTE,LASTTYPE};

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd,hdr,hdg,hdb;
	int method,fr,f,ff;
	Filename mapfile,imagefile;
	FILE *fp;
	int count,i,tlclip=0,thclip=0,tlwclip=0,thwclip=0;
	byte r[256],g[256],b[256],*pr,*pg,*pb;
	double gammar,gammag,gammab,wtr,wtg,wtb;
	h_boolean fflag,gflag,read3,cmapflag;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&fflag,&mapfile,&gflag,&gammar,&gammag,
		&gammab,&wtr,&wtg,&wtb,FFONE,&imagefile);
	fp = hfopenr(imagefile);
	fread_hdr_a(fp,&hd,imagefile);
	pr = r; pg = g; pb = b;
	cmapflag = TRUE;
	if (gflag) {
		if (gammag < 0)
			gammag = gammar;
		if (gammab < 0)
			gammab = gammag;
		gammar = 1./gammar;
		gammag = 1./gammag;
		gammab = 1./gammab;
		count = 256;
		r[0] = g[0] = b[0] = 0;
		for (i=1;i<256;i++) {
			r[i] = 255.*pow((double) i/255.,gammar) + .5;
			g[i] = 255.*pow((double) i/255.,gammag) + .5;
			b[i] = 255.*pow((double) i/255.,gammab) + .5;
		}
	}
	else if (fflag)
		readcmap(mapfile,256,&count,r,g,b);
	else if (findparam(&hd,"cmap") != NULLPAR) {
		count = 768;
		getparam(&hd,"cmap",PFBYTE,&count,&pr);
		if (count % 3)
			perr(HE_MSG,"colormap length not a multiple of 3");
		count /= 3;
		pg = pr + count;
		pb = pg + count;
	}
	else if (hd.numcolor == 3 || type_is_col3(&hd))
		cmapflag = FALSE;
	else if (hd.numcolor == 1)
		perr(HE_MSG,"no colormap provided and only one color plane");
	else
		perr(HE_MSG,"number of colors must be 1 or 3");
	hd.paramdealloc = FALSE;	/* don't deallocate the cmap! */
	if (findparam(&hd,"cmap") != NULLPAR)
		clearparam(&hd,"cmap");
	method = fset_conversion(&hd,&hdr,types,imagefile);
	if (hdr.numcolor == 1) {
		fr = hdr.num_frame;
		read3 = FALSE;
	}
	else {
		fr = hd.num_frame = hdr.num_frame = hdr.num_frame/3;
		hd.numcolor = hdr.numcolor = 1;
		read3 = TRUE;
	}
	if (type_is_col3(&hd))
		hips_convback = FALSE;
	write_headeru2(&hd,&hdr,argc,argv,hips_convback);
	dup_headern(&hdr,&hdg);
	alloc_image(&hdg);
	dup_headern(&hdr,&hdb);
	alloc_image(&hdb);
	ff = 0;
	for (f=0;f<fr;f++) {
		fread_imagec(fp,&hd,&hdr,method,ff++,imagefile);
		if (read3) {
			fread_imagec(fp,&hd,&hdg,method,ff++,imagefile);
			fread_imagec(fp,&hd,&hdb,method,ff++,imagefile);
			if (cmapflag) {
				h_applylut(&hdr,&hdr,count,pr);
				tlclip += hips_lclip; thclip += hips_hclip;
				h_applylut(&hdg,&hdg,count,pg);
				tlclip += hips_lclip; thclip += hips_hclip;
				h_applylut(&hdb,&hdb,count,pb);
				tlclip += hips_lclip; thclip += hips_hclip;
			}
		}
		else {
			h_applylut(&hdr,&hdb,count,pb);
			tlclip += hips_lclip; thclip += hips_hclip;
			h_applylut(&hdr,&hdg,count,pg);
			tlclip += hips_lclip; thclip += hips_hclip;
			h_applylut(&hdr,&hdr,count,pr);
			tlclip += hips_lclip; thclip += hips_hclip;
		}
		h_wtsum3(&hdr,&hdg,&hdb,&hdr,wtr,wtg,wtb);
		tlwclip += hips_lclip; thwclip += hips_hclip;
		write_imagec(&hd,&hdr,method,hips_convback,f);
	}
	if (tlclip || thclip) {
		fprintf(stderr,"Total of ");
		if (tlclip)
			fprintf(stderr,"%d underflows ",tlclip);
		if (tlclip && thclip)
			fprintf(stderr,"and ");
		if (thclip)
			fprintf(stderr,"%d overflows ",thclip);
		fprintf(stderr,"detected applying lookup table\n");
	}
	if (tlwclip || thwclip) {
		fprintf(stderr,"Total of ");
		if (tlwclip)
			fprintf(stderr,"%d underflows ",tlwclip);
		if (tlwclip && thwclip)
			fprintf(stderr,"and ");
		if (thwclip)
			fprintf(stderr,"%d overflows ",thwclip);
		fprintf(stderr,"detected computing weighted sum\n");
	}
	return(0);
}
