/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * mulseq.c - multiply images pixel by pixel
 *
 * usage:	mulseq file <iseq >oseq
 *
 * to load:	cc -o mulseq mulseq.c -lhipsh -lhips
 *
 * The region-of-interest of file is multiplied by the region-of-interest of
 * iseq, then iseq with that product replacing its region-of-interest is output.
 * If either sequence is shorter than the other, the last frame is repeated
 * a sufficient number of times to match (and a warning message is printed).
 * The -NFH, -NFD and -FXP switches are honored.  The sequence on the standard
 * input (the second one specified) is treated as the primary sequence for
 * history, description and extended parameters.
 *
 * Types handled directly: BYTE, SHORT, INT, FLOAT, FLOAT*COMPLEX, DOUBLE,
 *				DOUBLE*DBLCOM, COMPLEX, DBLCOM, INTPYR, FLOATPYR
 *
 * Hips 2 - msl - 7/3/91
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {LASTFLAG};
int types[] = {PFBYTE,PFSHORT,PFINT,PFFLOAT,PFDOUBLE,PFCOMPLEX,PFDBLCOM,
		PFINTPYR,PFFLOATPYR,LASTTYPE};
int typesc[] = {PFFLOAT,PFCOMPLEX,LASTTYPE};
int typesdc[] = {PFDOUBLE,PFDBLCOM,LASTTYPE};
char mtcherr[] = "%s: mismatch of number of frames, frames will be repeated\n";

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd,hdp,hdf,hdfp,hdo;
	int method1,method2,fr,fr1,fr2,f,ofmt,savesize,tmpfmt;
	char *savehist,*savedesc;
	Filename filename1,filename2;
	FILE *fp1,*fp2;
	h_boolean use_hdo = FALSE;
	h_boolean imagecopy = FALSE;
	struct hips_roi roi;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,FFTWO,&filename1,&filename2);
	fp1 = hfopenr(filename1);
	fp2 = hfopenr(filename2);
	fread_hdr_a(fp2,&hd,filename2);
	fread_hdr_cca(fp1,&hdf,&hd,
		CM_ROWS|CM_COLS|CM_NUMLEV|CM_NUMCOLOR3|CM_FRAMESC,filename1);
	ofmt = maxformat(hd.pixel_format,hdf.pixel_format,types,
		filename2,filename1);
	if (ofmt == PFCOMPLEX) {
		method2 = fset_conversion(&hd,&hdp,typesc,filename2);
		method1 = fset_conversion(&hdf,&hdfp,typesc,filename1);
	}
	else if (ofmt == PFDBLCOM) {
		method2 = fset_conversion(&hd,&hdp,typesdc,filename2);
		method1 = fset_conversion(&hdf,&hdfp,typesdc,filename1);
	}
	else {
		method2 = pset_conversion(&hd,&hdp,ofmt,filename2);
		method1 = pset_conversion(&hdf,&hdfp,ofmt,filename1);
	}
	if (hdp.num_frame != hdfp.num_frame)
		fprintf(stderr,mtcherr,Progname);
	getroi(&hdp,&roi);
	fr2 = hdp.num_frame;
	fr1 = hdfp.num_frame;
	hdp.num_frame = fr = (fr1 > fr2) ? fr1 : fr2;
	if (hdp.pixel_format != hdfp.pixel_format || fr1 > fr2) {
		use_hdo = TRUE;
		dup_headern(&hdp,&hdo);
		setformat(&hdo,ofmt);
		alloc_image(&hdo);
	}
	if (use_hdo) {
		if (hdp.rows != hdp.orows || hdp.cols != hdp.ocols)
			imagecopy = TRUE;
	}
	if (hips_fullxpar)
		mergeparam(&hdp,&hdfp);
	if (hips_fulldesc) {
		savedesc = hdp.seq_desc;
		savesize = hdp.sizedesc;
		if (hdfp.sizedesc > 1) {
			desc_set2(&hdp,HEP_SS,
				"****%s: stored sequence, file: %s****\n",
				Progname,filename1);
			desc_indentadd(&hdp,hdfp.seq_desc);
			if (savesize > 1) {
				desc_append2(&hdp,HEP_SS,
				    "****%s: input sequence, file: %s****\n",
				    Progname,filename2);
				desc_indentadd(&hdp,savedesc);
			}
		}
		else if (hdp.sizedesc > 1) {
			desc_set2(&hdp,HEP_SS,
				"****%s: input sequence, file: %s****\n",
				Progname,filename2);
			desc_indentadd(&hdp,savedesc);
		}
	}
	tmpfmt = hdp.pixel_format;
	hdp.pixel_format = ofmt;
	if (hips_fullhist) {
		savehist = hdp.seq_history;
		history_set(&hdp,HEP_SS,
			"****%s: stored sequence, file: %s****\n",Progname,
			filename1);
		history_indentadd(&hdp,hdfp.seq_history);
		history_append(&hdp,HEP_SS,
			"****%s: input sequence, file: %s****\n",Progname,
			filename2);
		history_indentadd(&hdp,savehist);
		write_headerun(&hdp,argc,argv);
	}
	else
		write_headeru(&hdp,argc,argv);
	hdp.pixel_format = tmpfmt;
	for (f=0;f<fr;f++) {
		if (f < fr1)
			fread_imagec(fp1,&hdf,&hdfp,method1,f,filename1);
		if (f < fr2) {
			fread_imagec(fp2,&hd,&hdp,method2,f,filename2);
			if (imagecopy) {
				if (hdp.pixel_format == PFFLOAT &&
				    hdo.pixel_format == PFCOMPLEX)
					h_toc(&hdp,&hdo);
				else if (hdp.pixel_format == PFDOUBLE &&
				    hdo.pixel_format == PFDBLCOM)
					h_todc(&hdp,&hdo);
				else {
					clearroi(&hdp);
					clearroi(&hdo);
					h_copy(&hdp,&hdo);
					setroi2(&hdp,&roi);
					setroi2(&hdo,&roi);
				}
			}
		}
		if (use_hdo) {
			h_mul(&hdp,&hdfp,&hdo);
			write_image(&hdo,f);
		}
		else {
			h_mul(&hdp,&hdfp,&hdp);
			write_image(&hdp,f);
		}
	}
	return(0);
}
