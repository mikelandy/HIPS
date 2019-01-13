/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * orseq.c - compute the logical OR of two sequences
 *
 * usage:	orseq file <iseq >oseq
 *
 * to load:	cc -o orseq orseq.c -lhipsh -lhips
 *
 * The region-of-interest of file is ored with the region-of-interest of iseq,
 * then iseq with that combination replacing its region-of-interest is output.
 * If either sequence is shorter than the other, the last frame is repeated
 * a sufficient number of times to match (and a warning message is printed).
 * The -NFH, -NFD and -FXP switches are honored.  The sequence on the standard
 * input (the second one specified) is treated as the primary sequence for
 * history, description and extended parameters.
 *
 * Types handled directly: MSBF, LSBF, BYTE
 *
 * Hips 2 - msl - 7/5/91
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {LASTFLAG};
int types[] = {PFMSBF,PFLSBF,PFBYTE,LASTTYPE};
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
	h_boolean use_hdo = FALSE;
	h_boolean imagecopy = FALSE;
	struct hips_roi roi;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,FFTWO,&filename1,&filename2);
	fp1 = hfopenr(filename1);
	fp2 = hfopenr(filename2);
	fread_hdr_a(fp2,&hd,filename2);
	fread_hdr_cca(fp1,&hdf,&hd,
		CM_ROWS|CM_COLS|CM_NUMCOLOR3|CM_FRAMESC,filename1);
	ofmt = maxformat(hd.pixel_format,hdf.pixel_format,types,
		filename2,filename1);
	method2 = pset_conversion(&hd,&hdp,ofmt,filename2);
	method1 = pset_conversion(&hdf,&hdfp,ofmt,filename1);
	if (hdp.num_frame != hdfp.num_frame)
		fprintf(stderr,mtcherr,Progname);
	getroi(&hdp,&roi);
	fr2 = hdp.num_frame;
	fr1 = hdfp.num_frame;
	hdp.num_frame = fr = (fr1 > fr2) ? fr1 : fr2;
	if (fr1 > fr2) {
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
	for (f=0;f<fr;f++) {
		if (f < fr1)
			fread_imagec(fp1,&hdf,&hdfp,method1,f,filename1);
		if (f < fr2) {
			fread_imagec(fp2,&hd,&hdp,method2,f,filename2);
			if (imagecopy) {
				clearroi(&hdp);
				clearroi(&hdo);
				h_copy(&hdp,&hdo);
				setroi2(&hdp,&roi);
				setroi2(&hdo,&roi);
			}
		}
		if (use_hdo) {
			h_or(&hdp,&hdfp,&hdo);
			write_image(&hdo,f);
		}
		else {
			h_or(&hdp,&hdfp,&hdp);
			write_image(&hdp,f);
		}
	}
	return(0);
}
