/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * fourtr3d.c - 3D Fourier transform and spectrum
 *
 * usage: fourtr3d [-s] < iseq > oseq
 *
 * The output is a complex Fourier transform by default, or a float Fourier
 * spectrum if -s * is specified.  For spectra, the coefficients are shifted,
 * so that the (0,0,0) coefficient is near the center of the output sequence.
 *
 * The program does not require cubic input sequences, but the linear
 * dimensions must all be powers of 2.
 *
 * To load: cc -o fourtr3d fourtr3d.c  -lhipsh -lhips -lm
 *
 * Yoav Cohen 5/17/82
 * HIPS 2 - msl - 7/10/91
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {
	{"s",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	LASTFLAG};
int types[] = {PFCOMPLEX,LASTTYPE};

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd,hdp,hdo;
	int method,f,nr,nrf;
	Filename filename;
	FILE *fp;
	h_boolean sflag;
	Pixelval p;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&sflag,FFONE,&filename);
	hips_cplxtor = CPLX_MAG;
	fp = hfopenr(filename);
	fread_header(fp,&hd,filename);
	clearroi(&hd);
	nr = hd.rows;
	nrf = nr*hd.num_frame;
	setsize(&hd,nrf,hd.ocols);	/* fake the image allocation */
	if (hd.pixel_format == PFFLOAT) {
		method = METH_COMPLEX;
		dup_headern(&hd,&hdp);
		setformat(&hdp,PFCOMPLEX);
		alloc_image(&hdp);
		/* input image stored in imaginary part of transform */
		hd.image = hdp.image + hd.sizepix*hd.orows*hd.ocols;
		hd.firstpix = hd.image;
	}
	else {
		alloc_image(&hd);
		method = fset_conversion(&hd,&hdp,types,filename);
		if (hdp.numcolor > 1)
			perr(HE_MSG,"can't handle color images");
	}
	if (sflag) {
		dup_headern(&hdp,&hdo);
		setformat(&hdo,PFFLOAT);
		/* store spectrum in beginning of transform */
		hdo.firstpix = hdo.image = hdp.image;
		hdo.orows = hdo.rows = nr;
		write_headeru(&hdo,argc,argv);
		hdo.orows = hdo.rows = nrf;
	}
	else {
		hdp.orows = hdp.rows = nr;
		write_headeru(&hdp,argc,argv);
		hdp.orows = hdp.rows = nrf;
	}
	p.v_float = nrf*hdp.cols;
	/* read entire sequence */
	fread_imagec(fp,&hd,&hdp,method,0,filename);
	hdp.orows = hdp.rows = nr;
	h_fourtr3d(&hdp);
	hdp.rows = hdp.orows = nrf;
	h_divscale(&hdp,&hdp,&p);
	if (sflag) {
		h_tof(&hdp,&hdo);
		dup_header(&hdo,&hd);
		hd.rows = hd.orows = nr;
		for (f=0;f<hd.num_frame;f++) {
			h_flipquad(&hd,&hd);
			hd.firstpix += nr*hd.cols*hd.sizepix;
		}
		setsize(&hdo,nrf/2,hdo.ocols);
		hdo.image += nrf*hdo.ocols*hdo.sizepix/2;
		write_image(&hdo,0);
		hdo.image = hdo.firstpix;
		write_image(&hdo,0);
	}
	else
		write_image(&hdp,0);
	return(0);
}
