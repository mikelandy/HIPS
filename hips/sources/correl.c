/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * correl.c - cross-correlate two images
 *
 * usage:	correl [-s rows [cols]] file <iseq >oseq
 *
 * to load:	cc -o correl correl.c -lhipsh -lhips
 *
 * The region-of-interest of file is cross-correlated with the
 * region-of-interest of iseq, and only that cross-correlation image is
 * output.  The -s switch specifies the size of the output images (the amount
 * of offsets attempted).  The center of the output image corresponds to
 * centering both input images.  Rightward and downward offsets in the output
 * images correspond to sliding the file image rightward and downward.  If -s
 * is not specified, the default size is the largest possible (the sum of the
 * input sizes minus 1).  If -s is specified by cols is not, it defaults to
 * the value of rows.
 *
 * If either sequence is shorter than the other, the last frame is repeated
 * a sufficient number of times to match (and a warning message is printed).
 * The -NFH, -NFD and -FXP switches are honored.  The sequence on the standard
 * input (the second one specified) is treated as the primary sequence for
 * history, description and extended parameters.
 *
 * Types handled directly: FLOAT
 *
 * Michael Landy - 4/8/89
 * Hips 2 - msl - 8/10/91
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {
	{"s",{LASTFLAG},1,{{PTINT,"-1","rows"},{PTINT,"-1","cols"},
		LASTPARAMETER}},
	LASTFLAG};
int types[] = {PFFLOAT,LASTTYPE};
char mtcherr[] = "%s: mismatch of number of frames, frames will be repeated\n";

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd,hdp,hdf,hdfp,hdo;
	int method1,method2,fr,fr1,fr2,f,ofmt,savesize,nro,nco,dr0,dc0;
	char *savehist,*savedesc;
	Filename filename1,filename2;
	FILE *fp1,*fp2;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&nro,&nco,FFTWO,&filename1,&filename2);
	fp1 = hfopenr(filename1);
	fp2 = hfopenr(filename2);
	fread_hdr_a(fp2,&hd,filename2);
	fread_hdr_cca(fp1,&hdf,&hd,CM_NUMCOLOR3|CM_FRAMESC,filename1);
	if (nro == -1) {
		nro = hd.rows + hdf.rows - 1;
		nco = hd.cols + hdf.cols - 1;
	}
	else if (nco == -1)
		nco = nro;
	dr0 = (1-hdf.rows) - ((int) ((nro - (hd.rows+hdf.rows-1))/2));
	dc0 = (1-hdf.cols) - ((int) ((nco - (hd.cols+hdf.cols-1))/2));
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
	setformat(&hdo,ofmt);
	setsize(&hdo,nro,nco);
	alloc_image(&hdo);
	if (hips_fullxpar)
		mergeparam(&hdo,&hdfp);
	if (hips_fulldesc) {
		savedesc = hdp.seq_desc;
		savesize = hdp.sizedesc;
		if (hdfp.sizedesc > 1) {
			desc_set2(&hdo,HEP_SS,
				"****%s: stored sequence, file: %s****\n",
				Progname,filename1);
			desc_indentadd(&hdo,hdfp.seq_desc);
			if (savesize > 1) {
				desc_append2(&hdo,HEP_SS,
				    "****%s: input sequence, file: %s****\n",
				    Progname,filename2);
				desc_indentadd(&hdo,savedesc);
			}
		}
		else if (hdp.sizedesc > 1) {
			desc_set2(&hdo,HEP_SS,
				"****%s: input sequence, file: %s****\n",
				Progname,filename2);
			desc_indentadd(&hdo,savedesc);
		}
	}
	if (hips_fullhist) {
		savehist = hdp.seq_history;
		history_set(&hdo,HEP_SS,
			"****%s: stored sequence, file: %s****\n",Progname,
			filename1);
		history_indentadd(&hdo,hdfp.seq_history);
		history_append(&hdo,HEP_SS,
			"****%s: input sequence, file: %s****\n",Progname,
			filename2);
		history_indentadd(&hdo,savehist);
		write_headerun(&hdo,argc,argv);
	}
	else
		write_headeru(&hdo,argc,argv);
	for (f=0;f<fr;f++) {
		if (f < fr1)
			fread_imagec(fp1,&hdf,&hdfp,method1,f,filename1);
		if (f < fr2)
			fread_imagec(fp2,&hd,&hdp,method2,f,filename2);
		h_correl(&hdp,&hdfp,&hdo,dr0,dc0);
		write_image(&hdo,f);
	}
	return(0);
}
