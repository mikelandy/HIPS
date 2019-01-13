/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * pad - pad a sequence, inserting it into a uniform background
 *
 * usage:	pad [-g gray-levelr [gray-levelg [gray-levelb]]]
 *			[-s rows [ cols [from-row [from-col ]]]]
 *			[-f frames [from-frame]] [-r] < in > out
 * 
 * defaults:	gray-levelr: 0,
 *		gray-levelg: gray-levelr
 *		gray-levelb: gray-levelg
 *		rows,cols: dimensions of input frame + 2,
 *		frames: number of input frames,
 *		from-frames,from-row,from-col: so that input is centered
 *			in the background.
 *
 * If the inserted image is not contained in the background, an error message
 * is given and no output is produced.
 *
 * to load:	cc -o pad pad.c -lhipsh -lhips -lm
 *
 * HIPS 2 - msl - 6/23/91
 * RGB/etc. - msl - 5/24/93
 */

#include <stdio.h>
#include <hipl_format.h>
int types[] = {PFBYTE,PFSHORT,PFINT,PFFLOAT,LASTTYPE};
static Flag_Format flagfmt[] = {
    {"g",{LASTFLAG},1,{{PTDOUBLE,"0","greylevelr"},
	    {PTDOUBLE,"-999999.","greylevelg"},
	    {PTDOUBLE,"-999999.","greylevelb"},LASTPARAMETER}},
    {"s",{LASTFLAG},1,{{PTINT,"-1","rows"},{PTINT,"-1","cols"},
	{PTINT,"-1","from-row"},{PTINT,"-1","from-col"},LASTPARAMETER}},
    {"f",{LASTFLAG},1,{{PTINT,"-1","frames"},{PTINT,"-1","from-frame"},
	LASTPARAMETER}},
    {"r",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
    LASTFLAG};

int main(argc,argv)

int argc;
char **argv;

{
	int method,i,j,orows,ocols,ofrow,ofcol,oframes,offrame,ifr,irows,icols;
	int ncolor;
	double gl,glr,glg,glb,currgl;
	Pixelval val;
	h_boolean noconversion,rflag;
	struct header hd,hdp,hdo,hdcb;
	struct hips_roi roi;
	Filename filename;
	FILE *fp;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&glr,&glg,&glb,&orows,&ocols,&ofrow,&ofcol,
		&oframes,&offrame,&rflag,FFONE,&filename);
	if (glg < -999998.)
		glg = glr;
	if (glb < -999998.)
		glb = glg;
	currgl = (glr > glg) ? glr : glg;	/* set currgl to be unique */
	currgl = (currgl > glb) ? (currgl+1) : (glb+1);
	fp = hfopenr(filename);
	fread_header(fp,&hd,filename);
	ifr = hd.num_frame/hd.numcolor;
	irows = hd.orows;
	icols = hd.ocols;
	if (orows == -1)
		orows = irows + 2;
	if (ocols == -1)
		ocols = icols + 2;
	if (oframes == -1)
		oframes = ifr;
	if (ofrow == -1)
		ofrow = (orows - irows)/2;
	if (ofcol == -1)
		ofcol = (ocols - icols)/2;
	if (offrame == -1)
		offrame = (oframes - ifr)/2;
	if (ofrow < 0 || (ofrow + irows) > orows ||
	    ofcol < 0 || (ofcol + icols) > ocols ||
	    offrame < 0 || (offrame + ifr) > oframes)
		perr(HE_MSG,
		    "input image placed outside of output image boundaries");
	getroi(&hd,&roi);
	if (roi.frow == 0 && roi.fcol == 0 && roi.rows == hd.orows &&
		roi.cols == hd.ocols && !rflag) {
			roi.rows = orows;
			roi.cols = ocols;
	}
	else {
		roi.frow += ofrow;
		roi.fcol += ofcol;
	}
	noconversion = in_typeslist(hd.pixel_format,types);
	if (noconversion) {
		setsize(&hd,orows,ocols);
		setroi2(&hd,&roi);
	}
	else
		clearroi(&hd);
	alloc_image(&hd);
	hd.num_frame = oframes*hd.numcolor;
	method = fset_conversion(&hd,&hdp,types,filename);
	ncolor = hdp.numcolor;
	if (noconversion)
		write_headeru(&hdp,argc,argv);
	else {
		dup_headern(&hdp,&hdo);
		setsize(&hdo,orows,ocols);
		setroi2(&hdo,&roi);
		alloc_image(&hdo);
		if (hips_convback)
			setupconvback(&hd,&hdo,&hdcb);
		write_headeru2(&hdcb,&hdo,argc,argv,hips_convback);
		clearroi(&hdp);
	}
	roi.frow = ofrow;
	roi.fcol = ofcol;
	roi.rows = irows;
	roi.cols = icols;
	if (noconversion)
		setroi2(&hdp,&roi);
	else
		setroi2(&hdo,&roi);
	for (i=0;i<offrame;i++) {
		for (j=0;j<ncolor;j++) {
			gl = (j==0) ? glr : ((j==1) ? glg : glb);
			switch (hdp.pixel_format) {
			case PFBYTE:	val.v_byte = gl; break;
			case PFSHORT:	val.v_short = gl; break;
			case PFINT:	val.v_int = gl; break;
			case PFFLOAT:	val.v_float = gl; break;
			}
			if (noconversion) {
				clearroi(&hdp);
				if (gl != currgl)
					h_setimage(&hdp,&val);
				setroi2(&hdp,&roi);
				write_image(&hdp,i*ncolor+j);
			}
			else {
				clearroi(&hdo);
				if (gl != currgl)
					h_setimage(&hdo,&val);
				setroi2(&hdo,&roi);
				write_imagec(&hdcb,&hdo,method,hips_convback,
					i*ncolor+j);
			}
			currgl = gl;
		}
	}
	for (i=offrame;i<offrame+ifr;i++) {
		for (j=0;j<ncolor;j++) {
			gl = (j==0) ? glr : ((j==1) ? glg : glb);
			switch (hdp.pixel_format) {
			case PFBYTE:	val.v_byte = gl; break;
			case PFSHORT:	val.v_short = gl; break;
			case PFINT:	val.v_int = gl; break;
			case PFFLOAT:	val.v_float = gl; break;
			}
			if (noconversion) {
				clearroi(&hdp);
				if ((orows > irows || ocols > icols) &&
				    gl != currgl)
					h_setimage(&hdp,&val);
				setroi2(&hdp,&roi);
				fread_roi(fp,&hdp,i*ncolor+j,filename);
				write_image(&hdp,i*ncolor+j);
			}
			else {
				clearroi(&hdo);
				if (gl != currgl)
					h_setimage(&hdo,&val);
				setroi2(&hdo,&roi);
				fread_imagec(fp,&hd,&hdp,method,i*ncolor+j,
					filename);
				h_copy(&hdp,&hdo);
				write_imagec(&hdcb,&hdo,method,hips_convback,
					i*ncolor+j);
			}
			currgl = gl;
		}
	}
	currgl = (glr > glg) ? glr : glg;	/* set currgl to be unique */
	currgl = (currgl > glb) ? (currgl+1) : (glb+1);
	if (offrame+ifr < oframes) {
		for (i=offrame+ifr;i<oframes;i++) {
			for (j=0;j<ncolor;j++) {
				gl = (j==0) ? glr : ((j==1) ? glg : glb);
				switch (hdp.pixel_format) {
				case PFBYTE:	val.v_byte = gl; break;
				case PFSHORT:	val.v_short = gl; break;
				case PFINT:	val.v_int = gl; break;
				case PFFLOAT:	val.v_float = gl; break;
				}
				if (noconversion) {
					clearroi(&hdp);
					if (gl != currgl)
						h_setimage(&hdp,&val);
					setroi2(&hdp,&roi);
					write_image(&hdp,i*ncolor+j);
				}
				else {
					clearroi(&hdo);
					if (gl != currgl)
						h_setimage(&hdo,&val);
					setroi2(&hdo,&roi);
					write_imagec(&hdcb,&hdo,method,
						hips_convback,i*ncolor+j);
				}
				currgl = gl;
			}
		}
	}
	return(0);
}
