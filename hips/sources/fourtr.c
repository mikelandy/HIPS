/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * fourtr.c - Fourier transform and spectrum
 *
 * usage: fourtr [-d] [-s] [-z] [-w] < iseq > oseq
 *
 * The output is a Fourier transform by default, or a Fourier spectrum if -s
 * is specified.  If -z is specified, the DC spectrum value is printed, and
 * then set to zero (since if makes the rest of the spectrum hard to see).  The
 * default output format depends on the input format and whether -s was
 * specified.  By default, double and double complex images produce double
 * complex transforms and double spectra, and all other formats produce
 * complex transforms and float spectra.  If -d is specified, the double
 * complex transforms and double spectra will be produced for all input
 * formats.  For spectra, the coefficients are shifted, so that the (0,0)
 * coefficient is near the center of the output picture.  By default, the
 * transform is computed on the ROI and only the ROI is output.  If -w is
 * specified, the output image is the entire image with only the ROI replaced
 * by its transform or spectrum.
 *
 * The program does not require square input pictures, but the linear
 * dimensions must both be powers of 2.
 *
 * To load: cc -o fourtr fourtr.c  -lhipsh -lhips -lm
 *
 * Yoav Cohen 5/14/82
 * modified for non-square images - Michael Landy - 1/13/84
 * added double precision - Michael Landy - 2/5/89
 * removed small image limits - Michael Landy - 10/29/89
 * HIPS 2 - msl - 7/10/91
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {
	{"d",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"s",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"z",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"w",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	LASTFLAG};
int types[] = {PFCOMPLEX,PFDBLCOM,LASTTYPE};
int typesd[] = {PFDBLCOM,LASTTYPE};

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd,hdp,hdo,hdw;
	int method,fr,f;
	Filename filename;
	FILE *fp;
	h_boolean dflag,sflag,zflag,wflag;
	float zval;
	Pixelval p;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&dflag,&sflag,&zflag,&wflag,
		FFONE,&filename);
	hips_cplxtor = CPLX_MAG;
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
	if (sflag) {
		dup_headern(&hdp,&hdo);
		setformat(&hdo,(hdp.pixel_format == PFCOMPLEX) ?
			PFFLOAT : PFDOUBLE);
		/* store spectrum in beginning of transform */
		hdo.image = hdp.image;
		hdo.firstpix = hdo.image +
			(hdo.ocols*hdo.frow+hdo.fcol)*hdo.sizepix;
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
	if (hdp.pixel_format == PFCOMPLEX)
		p.v_float = hdp.rows*hdp.cols;
	else
		p.v_double = hdp.rows*hdp.cols;
	fr = hdp.num_frame;
	for (f=0;f<fr;f++) {
		fread_imagec(fp,&hd,&hdp,method,f,filename);
		h_fourtr(&hdp);
		h_divscale(&hdp,&hdp,&p);
		if (sflag) {
			if (hdo.pixel_format == PFFLOAT)
				h_tof(&hdp,&hdo);
			else
				h_tod(&hdp,&hdo);
			if (zflag) {
				if (hdo.pixel_format == PFFLOAT) {
					zval = *((float *) hdo.firstpix);
					*((float *) hdo.firstpix) = 0;
				}
				else {
					zval = *((double *) hdo.firstpix);
					*((double *) hdo.firstpix) = 0;
				}
				fprintf(stderr,
				    "DC spectrum value was %f; set to zero\n",
				    zval);
			}
			h_flipquad(&hdo,&hdo);
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
