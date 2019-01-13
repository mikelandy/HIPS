/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * coltransf.c - apply an affine color transformation
 *
 * usage:	coltransf [-f mappingfile]
 *
 * Coltransf applies an affine transformation to a color space.  The
 * mappingfile (the name defaults to color.map) takes the following form:
 *
 *	cin cout
 *	 A(1,1)    A(1,2)   ...  A(1,cin)    shift(1)
 *	 A(2,1)    A(2,2)   ...  A(2,cin)    shift(2)
 *	   .         .      ...      .          .
 *	   .         .      ...      .          .
 *	   .         .      ...      .          .
 *	A(cout,1) A(cout,2) ... A(cout,cin) shift(cout)
 *
 * Thus, A is a matrix of a linear transformation from an input color space
 * with cin colors, and an output color space with cout colors.  The shift
 * values are then added (allowing for a shift of origin).
 *	
 * to load:	cc -o coltransf coltransf.c -lhipsh -lhips -lm
 *
 * Mike Landy - 8/26/91
 */

#include <stdio.h>
#include <hipl_format.h>
#include <math.h>

static Flag_Format flagfmt[] = {
	{"f",{LASTFLAG},1,{{PTFILENAME,"color.map","mappingfile"},
		LASTPARAMETER}},
	LASTFLAG};
int types[] = {PFFLOAT,LASTTYPE};

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd,**hdp,hdo;
	int method,fr,f,cin,cout,i,j,ff,fff;
	Filename mapfile,imagefile;
	FILE *fp,*fp2;
	Pixelval val;
	float *A,*shift,*pA,*ps;

	Progname = strsave(*argv);
	val.v_float = 0;
	parseargs(argc,argv,flagfmt,&mapfile,FFONE,&imagefile);
	fp2 = ffopen(mapfile,"r");
	if (fscanf(fp2,"%d %d",&cin,&cout) != 2)
		perr(HE_READFILE,mapfile);
	A = (float *) memalloc(cin*cout,sizeof(float));
	shift = (float *) memalloc(cout,sizeof(float));
	pA = A;
	ps = shift;
	for (i=0;i<cout;i++) {
		for (j=0;j<cin;j++) {
			if (fscanf(fp2,"%f",pA++) != 1)
				perr(HE_READFILE,mapfile);
		}
		if (fscanf(fp2,"%f",ps++) != 1)
			perr(HE_READFILE,mapfile);
	}
	fclose(fp2);
	fp = hfopenr(imagefile);
	fread_hdr_a(fp,&hd,imagefile);
	hdp = (struct header **) memalloc(cin,sizeof(struct header *));
	for (i=0;i<cin;i++)
		hdp[i] = (struct header *) memalloc(1,sizeof(struct header));
	method = fset_conversion(&hd,hdp[0],types,imagefile);
	if (hdp[0]->numcolor != cin)
		perr(HE_MSG,"number of input colors doesn't match mapping file");
	fr = hdp[0]->num_frame/cin;
	hd.numcolor = hdp[0]->numcolor = cout;
	hd.num_frame = hdp[0]->num_frame = cout*fr;
	if (type_is_col3(&hd) && (cout != 3))
		hips_convback = FALSE;
	write_headeru2(&hd,hdp[0],argc,argv,hips_convback);
	clearroi(&hd); 
	clearroi(hdp[0]); 
	for (i=1;i<cin;i++) {
		dup_headern(hdp[0],hdp[i]);
		alloc_image(hdp[i]);
	}
	dup_headern(hdp[0],&hdo);
	alloc_image(&hdo);
	fff = ff = 0;
	for (f=0;f<fr;f++) {
		for (i=0;i<cin;i++)
			fread_imagec(fp,&hd,hdp[i],method,ff++,imagefile);
		pA = A;
		ps = shift;
		for (i=0;i<cout;i++) {
			h_setimage(&hdo,&val);
			for (j=0;j<cin;j++)
				h_scaleadd(hdp[j],&hdo,*pA++);
			h_linscale(&hdo,&hdo,1.,*ps++);
			write_imagec(&hd,&hdo,method,hips_convback,fff++);
		}
	}
	return(0);
}
