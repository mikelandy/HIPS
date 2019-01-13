/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * inv.fourtr.c - inverse Fourier transform
 *
 * usage: inv.fourtr [-d] [-C | -F | -I | -S | -B] [-w] < iseq > oseq
 *
 * This routine computes the inverse Fourier transform.  The default
 * format in which the calculations are carried out depends on the input
 * format.  By default, double and double complex images result in double
 * precision calculations, and all other input formats result in single
 * precision calculations.  If -d is specified, double precision calculations
 * are performed for all input formats.  By default, double precision
 * calculations produce double complex output images, and single precision
 * calculations produce complex output images.  However, the user can
 * specify the output format to be complex (-C), float (-F), integer (-I),
 * short (-S) or byte (-B).  As usual, the conversion from complex to real
 * formats is controlled by the standard switch -CR, which defaults to 
 * outputting the real part of the inverse transform.
 * By default, the inverse transform is computed on the ROI and only the ROI
 * is output.  If -w is specified, the output image is the entire image with
 * only the ROI replaced by its inverse transform.
 *
 * The program does not require square input pictures, but the linear
 * dimensions must both be powers of 2.
 *
 * To load: cc -o inv.fourtr inv.fourtr.c  -lhipsh -lhips -lm
 *
 * Yoav Cohen 5/17/82
 * modified for non-square pictures - Michael Landy - 1/13/84
 * modified for double precision - Michael Landy - 2/5/89
 * HIPS 2 - msl - 7/10/91
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {
	{"d",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"C",{"F","I","S","B",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"F",{"C","I","S","B",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"I",{"C","F","S","B",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"S",{"C","F","I","B",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"B",{"C","F","I","S",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"w",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	LASTFLAG};
int types[] = {PFCOMPLEX,PFDBLCOM,LASTTYPE};
int typesd[] = {PFDBLCOM,LASTTYPE};

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd,hdp,hdo,hdw,hdtmp;
	int method,fr,f;
	Filename filename;
	FILE *fp;
	h_boolean dflag,Cflag,Fflag,Iflag,Sflag,Bflag,wflag,oconv;

	Progname = strsave(*argv);
	hips_cplxtor = CPLX_REAL;
	parseargs(argc,argv,flagfmt,&dflag,&Cflag,&Fflag,&Iflag,&Sflag,&Bflag,
		&wflag,FFONE,&filename);
	fp = hfopenr(filename);
	fread_header(fp,&hd,filename);
	if (hd.pixel_format == PFFLOAT) {
		if (dflag) {
			method = METH_DBLCOM;
			dup_headern(&hd,&hdp);
			setformat(&hdp,PFDBLCOM);
			alloc_image(&hdp);
			/* input image stored in second half of imaginary part
					of transform */
			hd.image = hdp.image + 3*hd.sizepix*hd.orows*hd.ocols;
			hd.firstpix = hd.image +
				(hd.ocols*hd.frow+hd.fcol)*hd.sizepix;
		}
		else {
			method = METH_COMPLEX;
			dup_headern(&hd,&hdp);
			setformat(&hdp,PFCOMPLEX);
			alloc_image(&hdp);
			/* input image stored in imaginary part of transform */
			hd.image = hdp.image + hd.sizepix*hd.orows*hd.ocols;
			hd.firstpix = hd.image +
				(hd.ocols*hd.frow+hd.fcol)*hd.sizepix;
		}
	}
	else if (hd.pixel_format == PFDOUBLE) {
		method = METH_DBLCOM;
		dup_headern(&hd,&hdp);
		setformat(&hdp,PFDBLCOM);
		alloc_image(&hdp);
		/* input image stored in imaginary part of transform */
		hd.image = hdp.image + hd.sizepix*hd.orows*hd.ocols;
		hd.firstpix = hd.image +
			(hd.ocols*hd.frow+hd.fcol)*hd.sizepix;
	}
	else if (dflag) {
		alloc_image(&hd);
		method = fset_conversion(&hd,&hdp,typesd,filename);
	}
	else {
		alloc_image(&hd);
		method = fset_conversion(&hd,&hdp,types,filename);
	}
	oconv = Bflag || Sflag || Iflag || Fflag ||
		(Cflag && hdp.pixel_format != PFCOMPLEX);
	if (oconv) {
		dup_headern(&hdp,&hdo);
		if (Cflag)
			setformat(&hdo,PFCOMPLEX);
		else if (Fflag)
			setformat(&hdo,PFFLOAT);
		else if (Iflag)
			setformat(&hdo,PFINT);
		else if (Sflag)
			setformat(&hdo,PFSHORT);
		else
			setformat(&hdo,PFBYTE);
		/* store output in beginning of transform */
		hdo.image = hdp.image;
		hdo.firstpix = hdo.image +
			(hdo.ocols*hdo.frow+hdo.fcol)*hdo.sizepix;
		if (Sflag || Bflag) {
			dup_headern(&hdp,&hdtmp);
			setformat(&hdtmp,PFINT);
			hdtmp.image = hdp.image;
			hdtmp.firstpix = hdtmp.image +
			    (hdtmp.ocols*hdtmp.frow+hdtmp.fcol)*hdtmp.sizepix;
		}
		if (wflag)
			write_headeru(&hdo,argc,argv);
		else {
			dup_headern(&hdo,&hdw);
			setsize(&hdw,hdw.rows,hdw.cols);
			write_headeru(&hdw,argc,argv);
		}
	}
	else {
		if (wflag)
			write_headeru(&hdp,argc,argv);
		else {
			dup_headern(&hdp,&hdw);
			setsize(&hdw,hdw.rows,hdw.cols);
			write_headeru(&hdw,argc,argv);
		}
	}
	fr = hdp.num_frame;
	for (f=0;f<fr;f++) {
		fread_imagec(fp,&hd,&hdp,method,f,filename);
		h_invfourtr(&hdp);
		if (oconv) {
			if (Cflag)
				h_toc(&hdp,&hdo);
			else if (Fflag)
				h_tof(&hdp,&hdo);
			else if (Iflag)
				h_toi(&hdp,&hdo);
			else if (Sflag) {
				h_toi(&hdp,&hdtmp);
				h_tos(&hdtmp,&hdo);
			}
			else {
				h_toi(&hdp,&hdtmp);
				h_tob(&hdtmp,&hdo);
			}
			if (wflag)
				write_image(&hdo,f);
			else
				write_roi(&hdo,f);
		}
		else {
			if (wflag)
				write_image(&hdp,f);
			else
				write_roi(&hdp,f);
		}
	}
	return(0);
}
