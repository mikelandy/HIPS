/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * inv.fourtr3d.c - 3D inverse Fourier transform
 *
 * usage: inv.fourtr3d [-C | -F | -I | -S | -B] < iseq > oseq
 *
 * The calculations are carried out in single precision complex format,
 * however the user may specify the output format to be complex (-C, the
 * default), float (-F), integer (-I), short (-S) or byte (-B).  As usual, the
 * conversion from complex to real formats is controlled by the standard
 * switch -CR, which defaults to outputting the real part of the inverse
 * transform.  The program does not require cubic input sequences, but the
 * linear dimensions must all be powers of 2.
 *
 * To load: cc -o inv.fourtr3d inv.fourtr3d.c -lhipsh -lhips -lm
 *
 * Michael Landy - 11/17/92
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {
	{"C",{"F","I","S","B",LASTFLAG},0,{{PTBOOLEAN,"TRUE"},LASTPARAMETER}},
	{"F",{"C","I","S","B",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"I",{"C","F","S","B",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"S",{"C","F","I","B",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"B",{"C","F","I","S",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	LASTFLAG};
int types[] = {PFCOMPLEX,LASTTYPE};

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd,hdp,hdo,hdtmp;
	int method,nr,nrf;
	Filename filename;
	FILE *fp;
	h_boolean Cflag,Fflag,Iflag,Sflag,Bflag,oconv;

	Progname = strsave(*argv);
	hips_cplxtor = CPLX_REAL;
	parseargs(argc,argv,flagfmt,&Cflag,&Fflag,&Iflag,&Sflag,&Bflag,
		FFONE,&filename);
	oconv = Bflag || Sflag || Iflag || Fflag;
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
	if (oconv) {
		dup_headern(&hdp,&hdo);
		if (Fflag)
			setformat(&hdo,PFFLOAT);
		else if (Iflag)
			setformat(&hdo,PFINT);
		else if (Sflag)
			setformat(&hdo,PFSHORT);
		else
			setformat(&hdo,PFBYTE);
		/* store output in beginning of transform */
		hdo.firstpix = hdo.image = hdp.image;
		if (Sflag || Bflag) {
			dup_headern(&hdp,&hdtmp);
			setformat(&hdtmp,PFINT);
			hdtmp.firstpix = hdtmp.image = hdp.image;
		}
		hdo.rows = hdo.orows = nr;
		write_headeru(&hdo,argc,argv);
		hdo.rows = hdo.orows = nrf;
	}
	else {
		hdp.rows = hdp.orows = nr;
		write_headeru(&hdp,argc,argv);
		hdp.rows = hdp.orows = nrf;
	}
	/* read entire sequence */
	fread_imagec(fp,&hd,&hdp,method,0,filename);
	hdp.orows = hdp.rows = nr;
	h_invfourtr3d(&hdp);
	hdp.rows = hdp.orows = nrf;
	if (oconv) {
		if (Fflag)
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
		write_image(&hdo,0);
	}
	else
		write_image(&hdp,0);
	return(0);
}
