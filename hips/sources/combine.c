/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * combine.c - combine two real images into one complex image
 *
 * usage:	combine [-pm | -ri] image1 <image2 >oseq
 *
 * to load:	cc -o combine combine.c -lhipsh -lhips
 *
 * The two images are either real and imaginary parts (with -ri) or phase and
 * magnitude (with -pm, the default).
 * If either sequence is shorter than the other, the last frame is repeated
 * a sufficient number of times to match (and a warning message is printed).
 * The -NFH, -NFD and -FXP switches are honored.  The sequence on the standard
 * input (the second one specified) is treated as the primary sequence for
 * history, description and extended parameters.
 *
 * Types handled directly: FLOAT->COMPLEX, DOUBLE->DBLCOM
 *
 * Hips 2 - msl - 8/9/91
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {
	{"pm",{"ri",LASTFLAG},0,{{PTBOOLEAN,"TRUE"},LASTPARAMETER}},
	{"ri",{"pm",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	LASTFLAG};
int types[] = {PFFLOAT,PFDOUBLE,LASTTYPE};
char mtcherr[] = "%s: mismatch of number of frames, frames will be repeated\n";

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd,hdp,hdf,hdfp,hdo;
	int method1,method2,fr,fr1,fr2,f,ofmt,savesize;
	char *savehist,*savedesc;
	Filename filename1,filename2;
	FILE *fp1,*fp2;
	h_boolean pmflag,riflag;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&pmflag,&riflag,
		FFTWO,&filename1,&filename2);
	fp1 = hfopenr(filename1);
	fp2 = hfopenr(filename2);
	fread_hdr_a(fp2,&hd,filename2);
	fread_hdr_cca(fp1,&hdf,&hd,
		CM_OROWS|CM_OCOLS|CM_NUMCOLOR3|CM_FRAMESC,filename1);
	ofmt = maxformat(hd.pixel_format,hdf.pixel_format,types,
		filename2,filename1);
	method2 = pset_conversion(&hd,&hdp,ofmt,filename2);
	method1 = pset_conversion(&hdf,&hdfp,ofmt,filename1);
	if (hdp.num_frame != hdfp.num_frame)
		fprintf(stderr,mtcherr,Progname);
	fr2 = hdp.num_frame;
	fr1 = hdfp.num_frame;
	hdp.num_frame = fr = (fr1 > fr2) ? fr1 : fr2;
	dup_headern(&hdp,&hdo);
	setformat(&hdo,ofmt==PFFLOAT ? PFCOMPLEX : PFDBLCOM);
	alloc_image(&hdo);
	if (hips_fullxpar)
		mergeparam(&hdo,&hdfp);
	if (hips_fulldesc) {
		savedesc = hdo.seq_desc;
		savesize = hdo.sizedesc;
		if (hdfp.sizedesc > 1) {
			desc_set2(&hdo,HEP_SS,
				"****%s: first sequence, file: %s****\n",
				Progname,filename1);
			desc_indentadd(&hdo,hdfp.seq_desc);
			if (savesize > 1) {
				desc_append2(&hdo,HEP_SS,
				    "****%s: second sequence, file: %s****\n",
				    Progname,filename2);
				desc_indentadd(&hdo,savedesc);
			}
		}
		else if (hdo.sizedesc > 1) {
			desc_set2(&hdo,HEP_SS,
				"****%s: second sequence, file: %s****\n",
				Progname,filename2);
			desc_indentadd(&hdo,savedesc);
		}
	}
	if (hips_fullhist) {
		savehist = hdo.seq_history;
		history_set(&hdo,HEP_SS,
			"****%s: first sequence, file: %s****\n",Progname,
			filename1);
		history_indentadd(&hdo,hdfp.seq_history);
		history_append(&hdo,HEP_SS,
			"****%s: second sequence, file: %s****\n",Progname,
			filename2);
		history_indentadd(&hdo,savehist);
		write_headerun(&hdo,argc,argv);
	}
	else
		write_headeru(&hdo,argc,argv);
	clearroi(&hdp); clearroi(&hdfp); clearroi(&hdo);
	for (f=0;f<fr;f++) {
		if (f < fr1)
			fread_imagec(fp1,&hdf,&hdfp,method1,f,filename1);
		if (f < fr2) {
			fread_imagec(fp2,&hd,&hdp,method2,f,filename2);
		}
		h_combine(&hdfp,&hdp,&hdo,pmflag);
		write_image(&hdo,f);
	}
	return(0);
}
