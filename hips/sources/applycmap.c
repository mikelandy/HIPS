/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * applycmap.c - apply a colormap to an image
 *
 * usage:	applycmap [-f colormapfile | -g [gammar [gammag [gammab]]]]
 *
 * Applycmap is used to apply a colormap to an image replacing pixels with
 * the corresponding lookup table entries.  The lookup table can come from
 * a file (-f), can be a standard inverse gamma table (-g) or come from the
 * `cmap' entry in the image header (the default).  Any such `cmap' entry is
 * then deleted from the header.  For -f, the colormap is formatted as for the
 * subroutine readcmap.  -g is used to specify an inverse gamma lookup table,
 * where gammar defaults to 2, gammag defaults to gammar and gammab defaults to
 * gammag.  If the image has one color plane, then that image is put through
 * all three lookup tables, resulting in an output with three color planes.
 * If the input has three color planes, the first is put through the red
 * color table, the second is put through the green, and the third through the
 * blue.
 *
 * to load:	cc -o applycmap applycmap.c -lhips -lm
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
	LASTFLAG};
int types[] = {PFBYTE,LASTTYPE};

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd,hdp,hdo;
	int method,fr,f,ff;
	Filename mapfile,imagefile;
	FILE *fp;
	int count,i,tlclip=0,thclip=0;
	byte r[256],g[256],b[256],*pr,*pg,*pb;
	double gammar,gammag,gammab;
	h_boolean fflag,gflag,read3;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&fflag,&mapfile,&gflag,&gammar,&gammag,
		&gammab,FFONE,&imagefile);
	fp = hfopenr(imagefile);
	fread_hdr_a(fp,&hd,imagefile);
	pr = r; pg = g; pb = b;
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
	else {
		count = 768;
		getparam(&hd,"cmap",PFBYTE,&count,&pr);
		if (count % 3)
			perr(HE_MSG,"colormap length not a multiple of 3");
		count /= 3;
		pg = pr + count;
		pb = pg + count;
	}
	hd.paramdealloc = FALSE;	/* don't deallocate the cmap! */
	if (findparam(&hd,"cmap") != NULLPAR)
		clearparam(&hd,"cmap");
	method = fset_conversion(&hd,&hdp,types,imagefile);
	if (hdp.numcolor == 1) {
		fr = hdp.num_frame;
		hd.num_frame *= 3;
		hdp.num_frame *= 3;
		hd.numcolor = 3;
		hdp.numcolor = 3;
		read3 = FALSE;
	}
	else if (hdp.numcolor == 3) {
		fr = hdp.num_frame/3;
		read3 = TRUE;
	}
	else
		perr(HE_MSG,"number of colors must be 1 or 3");
	write_headeru2(&hd,&hdp,argc,argv,hips_convback);
	if (read3)
		dup_header(&hdp,&hdo);
	else {
		dup_headern(&hdp,&hdo);
		alloc_image(&hdo);
	}
	ff = 0;
	for (f=0;f<fr;f++) {
		fread_imagec(fp,&hd,&hdp,method,ff++,imagefile);
		h_applylut(&hdp,&hdo,count,pr);
		tlclip += hips_lclip; thclip += hips_hclip;
		write_imagec(&hd,&hdo,method,hips_convback,3*f);
		if (read3)
			fread_imagec(fp,&hd,&hdp,method,ff++,imagefile);
		h_applylut(&hdp,&hdo,count,pg);
		tlclip += hips_lclip; thclip += hips_hclip;
		write_imagec(&hd,&hdo,method,hips_convback,3*f+1);
		if (read3)
			fread_imagec(fp,&hd,&hdp,method,ff++,imagefile);
		h_applylut(&hdp,&hdo,count,pb);
		tlclip += hips_lclip; thclip += hips_hclip;
		write_imagec(&hd,&hdo,method,hips_convback,3*f+2);
	}
	if (tlclip || thclip) {
		fprintf(stderr,"Total of ");
		if (tlclip)
			fprintf(stderr,"%d underflows ",tlclip);
		if (tlclip && thclip)
			fprintf(stderr,"and ");
		if (thclip)
			fprintf(stderr,"%d overflows ",thclip);
		fprintf(stderr,"detected\n");
	}
	return(0);
}
