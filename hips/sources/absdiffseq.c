/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * absdiffseq.c - difference images pixel by pixel
 *
 * usage:	absdiffseq seq1 <seq2 >oseq
 *
 * to load:	cc -o absdiffseq absdiffseq.c -lhipsh -lhips
 *
 *
 * The region-of-interest of seq1 is subtracted from the region-of-interest of
 * seq2, then seq2 with the absolute value of the difference replacing its
 * ROI is output.
 * If either sequence is shorter than the other, the last frame is repeated
 * a sufficient number of times to match (and a warning message is printed).
 * The -NFH, -NFD and -FXP switches are honored.  The sequence on the standard
 * input (the second one specified) is treated as the primary sequence for
 * history, description and extended parameters.
 *
 * Types handled directly: BYTE, SHORT, INT, FLOAT, DOUBLE
 *
 * Hips 2 - msl - 7/3/91
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {LASTFLAG};
int types[] = {PFBYTE,PFSHORT,PFINT,PFFLOAT,PFDOUBLE,LASTTYPE};
char mtcherr[] = "%s: mismatch of number of frames, frames will be repeated\n";

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd1,hd1p,hd2,hd2p,hdo;
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
	fread_hdr_a(fp2,&hd2,filename2);
	fread_hdr_cca(fp1,&hd1,&hd2,
		CM_ROWS|CM_COLS|CM_NUMCOLOR3|CM_FRAMESC,filename1);
	ofmt = maxformat(hd1.pixel_format,hd2.pixel_format,types,
		filename1,filename2);
	method2 = pset_conversion(&hd2,&hd2p,ofmt,filename2);
	method1 = pset_conversion(&hd1,&hd1p,ofmt,filename1);
	if (hd2p.num_frame != hd1p.num_frame)
		fprintf(stderr,mtcherr,Progname);
	getroi(&hd2p,&roi);
	fr2 = hd2p.num_frame;
	fr1 = hd1p.num_frame;
	hd2p.num_frame = fr = (fr1 > fr2) ? fr1 : fr2;
	if (fr1 > fr2) {
		use_hdo = TRUE;
		dup_headern(&hd2p,&hdo);
		setformat(&hdo,ofmt);
		alloc_image(&hdo);
	}
	if (use_hdo) {
		if (hd2p.rows != hd2p.orows || hd2p.cols != hd2p.ocols)
			imagecopy = TRUE;
	}
	if (hips_fullxpar)
		mergeparam(&hd2p,&hd1p);
	if (hips_fulldesc) {
		savedesc = hd2p.seq_desc;
		savesize = hd2p.sizedesc;
		if (hd1p.sizedesc > 1) {
			desc_set2(&hd2p,HEP_SS,
			    "****%s: first sequence, file: %s****\n",
			    Progname,filename1);
			desc_indentadd(&hd2p,hd1p.seq_desc);
			if (savesize > 1) {
			    desc_append2(&hd2p,HEP_SS,
			      "****%s: second sequence, file: %s****\n",
			      Progname,filename2);
			    desc_indentadd(&hd2p,savedesc);
			}
		}
		else if (hd2p.sizedesc > 1) {
			desc_set2(&hd2p,HEP_SS,
			    "****%s: second sequence, file: %s****\n",
			    Progname,filename2);
			desc_indentadd(&hd2p,savedesc);
		}
	}
	if (hips_fullhist) {
		savehist = hd2p.seq_history;
		history_set(&hd2p,HEP_SS,
		    "****%s: first sequence, file: %s****\n",Progname,
		    filename1);
		history_indentadd(&hd2p,hd1p.seq_history);
		history_append(&hd2p,HEP_SS,
		    "****%s: second sequence, file: %s****\n",Progname,
		    filename2);
		history_indentadd(&hd2p,savehist);
		write_headerun(&hd2p,argc,argv);
	}
	else
		write_headeru(&hd2p,argc,argv);
	for (f=0;f<fr;f++) {
		if (f < fr1)
			fread_imagec(fp1,&hd1,&hd1p,method1,f,filename1);
		if (f < fr2) {
			fread_imagec(fp2,&hd2,&hd2p,method2,f,filename2);
			if (imagecopy) {
				clearroi(&hd2p);
				clearroi(&hdo);
				h_copy(&hd2p,&hdo);
				setroi2(&hd2p,&roi);
				setroi2(&hdo,&roi);
			}
		}
		if (use_hdo) {
			h_absdiff(&hd2p,&hd1p,&hdo);
			write_image(&hdo,f);
		}
		else {
			h_absdiff(&hd2p,&hd1p,&hd2p);
			write_image(&hd2p,f);
		}
	}
	return(0);
}
