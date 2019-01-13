/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * morpherode  - apply the morphological operator of erosion
 *
 * Usage:      morpherode element < inseq > oseq
 *
 * Morpherode applies the erosion operation to an image.  Although it
 * applies to byte-formatted images, it effectively treats the image as a
 * binary image, where dark pixels (with grey levels below 128) are treated as
 * `foreground' or `object' pixels, and others are treated as background.  The
 * erosion of the input image is controlled by another HIPS image, specified
 * in the command line, the structuring element.  For each foreground element
 * in the image, the structuring element is centered on that pixel.  All
 * other pixels lying at the same position as foreground pixels of the
 * structuring element will receive a `vote'.  After applying this procedure
 * to all foreground image elements, the program computes the maximum number
 * of `votes' received by any image pixel (the best foreground fit with the
 * structuring element).  Then, pixels with that many votes are set to the
 * darkest value that overlaps that pixel position (among the structural
 * element pixels that overlapped, and its former value in the input image).
 * Foreground pixels which received fewer votes are set to 255. Background
 * input image pixels which received fewer votes are left unchanged in the
 * output image.
 *
 * The program determines the smallest rectangle which fits around the
 * foreground pixels in the structuring element, and only uses these pixels
 * in the loops, for efficiency.  However, the center of the structure element
 * is centered over the pixel, so the user may determine the shift caused by
 * the erosion operation.  For example, if the element image is the following
 * 5x5 image:
 *
 *	  1 255 255 255 255
 *	255 255 255 255 255
 *	255 255   1 255 255
 *	255 255 255 255 255
 *	255 255 255 255 255,
 *
 * then the output image will have foreground pixels where the input image had
 * a foreground pixel in that position and in the position which is two pixel
 * positions down and to the right.  The element image is effectively
 * reflected about its vertical and horizontal axes before it is used.
 * The ROI of the input image is dilated by the structuring
 * element's ROI, and the result replaces the input image's ROI.
 * If either sequence is shorter than the other, the last frame is repeated
 * a sufficient number of times to match (and a warning message is printed).
 * The -NFH, -NFD and -FXP switches are honored.  The sequence on the standard
 * input (the second one specified) is treated as the primary sequence for
 * history, description and extended parameters.
 *
 * Types handled directly: BYTE
 *
 * To load:     cc -o morpherode morpherode.c -lhipsh -lhips -lm
 *
 * Ahmed. Abbood 19/10/1988
 * rewritten by Michael Landy 10/29/89
 * HIPS 2 - msl - 8/3/91
 */

#include <stdio.h>
#include <hipl_format.h>

#define gray 127

int types[] = {PFBYTE,LASTTYPE};
static Flag_Format flagfmt[] = {LASTFLAG};
char mtcherr[] = "%s: mismatch of number of frames, frames will be repeated\n";

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd,hdp,hdf,hdfp,hdo,hdt;
	int method1,method2,fr,fr1,fr2,f,ofmt,savesize,centr,centc;
	int centrr,centcc;
	char *savehist,*savedesc,msg[100];
	Filename filename1,filename2;
	FILE *fp1,*fp2;
	h_boolean imagecopy = FALSE;
	struct hips_roi roi,roie;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,FFTWO,&filename1,&filename2);
	fp1 = hfopenr(filename1);
	fp2 = hfopenr(filename2);
	fread_hdr_a(fp2,&hd,filename2);
	fread_hdr_a(fp1,&hdf,filename1);
	ofmt = maxformat(hd.pixel_format,hdf.pixel_format,types,
		filename2,filename1);
	method2 = pset_conversion(&hd,&hdp,ofmt,filename2);
	method1 = pset_conversion(&hdf,&hdfp,ofmt,filename1);
	if (hdp.num_frame != hdfp.num_frame)
		fprintf(stderr,mtcherr,Progname);
	getroi(&hdp,&roi);
	getroi(&hdfp,&roie);
	centr = hdf.rows/2;
	centc = hdf.cols/2;
	fr2 = hdp.num_frame;
	fr1 = hdfp.num_frame;
	hdp.num_frame = fr = (fr1 > fr2) ? fr1 : fr2;
	dup_headern(&hdp,&hdo);
	alloc_image(&hdo);
	dup_headern(&hdp,&hdt);
	setsize(&hdt,hdt.rows,hdt.cols);
	setformat(&hdt,PFINT);
	alloc_image(&hdt);
	if (hdp.rows != hdp.orows || hdp.cols != hdp.ocols)
		imagecopy = TRUE;
	if (hips_fullxpar)
		mergeparam(&hdp,&hdfp);
	if (hips_fulldesc) {
		savedesc = hdp.seq_desc;
		savesize = hdp.sizedesc;
		if (hdfp.sizedesc > 1) {
			desc_set2(&hdp,HEP_SS,
				"****%s: structuring element, file: %s****\n",
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
			"****%s: structuring element, file: %s****\n",Progname,
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
		if (f < fr1) {
			fread_imagec(fp1,&hdf,&hdfp,method1,f,filename1);
			setroi2(&hdfp,&roie);
			h_minroi(&hdfp,gray);
			if (hdfp.rows == 0) {
			 sprintf(msg,
			  "structuring element has no nonzero pixels, frame %d",
			  f);
			  perr(HE_MSG,msg);
			}
			centrr = centr + roie.frow - hdfp.frow;
			centcc = centc + roie.fcol - hdfp.fcol;
		}
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
		h_morphero(&hdp,&hdfp,&hdt,&hdo,centrr,centcc,gray);
		write_image(&hdo,f);
	}
	return(0);
}
