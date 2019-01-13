/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * scale.c - scale a sequence of images
 *
 * usage: scale [-l b [c]]         [-f | -i | -b | -s| -S] 
 * 	  scale [-q a [b [c]]]     [-f | -i | -b | -s| -S]
 * 	  scale [-m minout maxout] [-f | -i | -b | -s| -S] [-e]
 *
 * scale scales an input sequence.  With the -l switch, the scaling is linear
 * (opix = b*ipix + c, where c defaults to zero).  With the -q switch, the
 * scaling is quadratic (opix = a*ipix*ipix + b*ipix + c, where b and c
 * default to zero).  With the -m switch, the input pixel value range is
 * computed, and then pixels are scaled linearly so that the minimum pixel
 * value maps to minout, and the maximum pixel value maps to maxout.  If
 * no switches are specified at all, the default is `-m 0 255'.  With -m,
 * the default action is to compute scale factors based on pixel values in the
 * first frame, and use those same scale factors throughout.  If -e is
 * specified, each frame is independently scaled to map to the specified
 * output pixel value range.  All computations are done in floating point.
 * For -l and -q, the default output format is floating point, whereas for
 * -m the default output format is byte.  The output format can be specified
 * by the user as floating point (-f), integer (-i), byte (-b), short integer
 * (-s), or the same as the input (-S).  If -S is specified, the standard
 * switch -CB is effective.
 *
 * Michael Landy (Hips 2) - 6/19/91
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {
	{"l",{"q","m",LASTFLAG},1,{{PTBOOLEAN,"FALSE"},{PTDOUBLE,"0","b"},
		{PTDOUBLE,"0","c"},LASTPARAMETER}},
	{"q",{"l","m",LASTFLAG},1,{{PTBOOLEAN,"FALSE"},{PTDOUBLE,"0","a"},
		{PTDOUBLE,"0","b"},{PTDOUBLE,"0","c"},LASTPARAMETER}},
	{"m",{"l","q",LASTFLAG},1,{{PTBOOLEAN,"TRUE"},{PTDOUBLE,"0","minout"},
		{PTDOUBLE,"255","maxout"},LASTPARAMETER}},
	{"f",{"i","b","s","S",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"i",{"f","b","s","S",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"b",{"f","i","s","S",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"s",{"f","i","b","S",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"S",{"f","i","b","s",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"e",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	LASTFLAG};
int types[] = {PFBYTE,PFSHORT,PFINT,PFFLOAT,LASTTYPE};

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd,hdp,hdp2,hdo;
	int method,fr,f;
	double lb,lc,qa,qb,qc,minout,maxout,fmin,fmax;
	Filename filename;
	FILE *fp;
	h_boolean lflag,qflag,mflag,fflag,iflag,sflag,bflag,Sflag,eflag,imagecopy;
	Pixelval minval,maxval;
	char msg[200];
	int tlclip = 0, thclip = 0;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&lflag,&lb,&lc,&qflag,&qa,&qb,&qc,
		&mflag,&minout,&maxout,&fflag,&iflag,&bflag,&sflag,&Sflag,
		&eflag,FFONE,&filename);
	if (!(fflag || iflag || sflag || bflag || Sflag)) {
		if (mflag)
			bflag = TRUE;
		else
			fflag = TRUE;
	}
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	method = fset_conversion(&hd,&hdp,types,filename);
	imagecopy = FALSE;
	if (hdp.pixel_format != PFFLOAT) {
		if (hdp.rows != hdp.orows || hdp.cols != hdp.ocols)
			imagecopy = TRUE;
		dup_headern(&hdp,&hdp2);
		setformat(&hdp2,PFFLOAT);
		alloc_image(&hdp2);
	}
	else	/* PFFLOAT */
		dup_header(&hdp,&hdp2);
	if (bflag) {
		if (hdp.pixel_format == PFBYTE)
			dup_header(&hdp,&hdo);
		else {
			dup_headern(&hdp,&hdo);
			setformat(&hdo,PFBYTE);
			alloc_image(&hdo);
		}
		write_headeru(&hdo,argc,argv);
	}
	else if (sflag) {
		if (hdp.pixel_format == PFSHORT)
			dup_header(&hdp,&hdo);
		else {
			dup_headern(&hdp,&hdo);
			setformat(&hdo,PFSHORT);
			alloc_image(&hdo);
		}
		write_headeru(&hdo,argc,argv);
	}
	else if (iflag) {
		if (hdp.pixel_format == PFINT)
			dup_header(&hdp,&hdo);
		else {
			dup_headern(&hdp,&hdo);
			setformat(&hdo,PFINT);
			alloc_image(&hdo);
		}
		write_headeru(&hdo,argc,argv);
	}
	else if (fflag)
		write_headeru(&hdp2,argc,argv);
	else if (Sflag) {
		dup_header(&hdp,&hdo);
		write_headeru2(&hd,&hdo,argc,argv,hips_convback);
	}
	fr = hdp.num_frame;
	for (f=0;f<fr;f++) {
		fread_imagec(fp,&hd,&hdp,method,f,filename);
		if (imagecopy)
			h_tof(&hdp,&hdp2);
		if (lflag)
			h_linscale(&hdp,&hdp2,(float) lb,(float) lc);
		else if (qflag)
			h_quadscale(&hdp,&hdp2,(float) qa,(float) qb,
				(float) qc);
		else {
			if (f == 0 || eflag) {
				h_minmax(&hdp,&minval,&maxval,FALSE);
				switch(hdp.pixel_format) {
				case PFBYTE:	fmin = minval.v_byte;
						fmax = maxval.v_byte;
						break;
				case PFSHORT:	fmin = minval.v_short;
						fmax = maxval.v_short;
						break;
				case PFINT:	fmin = minval.v_int;
						fmax = maxval.v_int;
						break;
				case PFFLOAT:	fmin = minval.v_float;
						fmax = maxval.v_float;
						break;
				}
				if (fmax > fmin)
					lb = (maxout - minout) / (fmax - fmin);
				else
					lb = 0.1;
				lc = minout-fmin*lb;
				sprintf(msg,"frame %d max = %.2f, min = %.2f",
					f,fmax,fmin);
				perr(HE_IMSG,msg);
			}
			h_linscale(&hdp,&hdp2,(float) lb,(float) lc);
		}
		if (bflag) {
			h_tob(&hdp2,&hdo);
			write_image(&hdo,f);
		}
		else if (sflag) {
			h_tos(&hdp2,&hdo);
			write_image(&hdo,f);
		}
		else if (iflag) {
			h_toi(&hdp2,&hdo);
			write_image(&hdo,f);
		}
		else if (fflag)
			write_image(&hdp2,f);
		else if (Sflag) {
			switch(hdp.pixel_format) {
			case PFBYTE:	h_tob(&hdp2,&hdp); break;
			case PFSHORT:	h_tos(&hdp2,&hdp); break;
			case PFINT:	h_toi(&hdp2,&hdp); break;
			case PFFLOAT:	break;
			}
			write_imagec(&hd,&hdp,method,hips_convback,f);
		}
		tlclip += hips_lclip;
		thclip += hips_hclip;
	}
	if (tlclip > 0 || thclip > 0) {
		sprintf(msg,"%d underflows and %d overflows detected",
			tlclip,thclip);
		perr(HE_IMSG,msg);
	}
	return(0);
}
