/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * subsample - subsample a sequence spatially and/or temporally
 *
 * usage:	subsample [-h hrate [hoffset]] [-v vrate [voffset]]
 *			[-t trate [toffset]] [-d drate [doffset]]
 *			[-c crate [coffset]]
 * 
 * Subsample reduces the size of an image sequence by subsampling.  For each
 * image dimension (horizontal, vertical, time, depth plane, and color plane)
 * the user can specify the rate of sampling (an integer which defaults to 1,
 * or no subsampling at all) and the offset at which to start sampling (which
 * defaults to zero) and must be less than the corresponding rate.  The region
 * of interest becomes that part of the region of interest left in the image.
 * If the ROI is deleted entirely, then the output region of interest is reset
 * to be the entire image.
 *
 * to load:	cc -o subsample subsample.c -lhipsh -lhips -lm
 *
 * Michael Landy - 3/8/94
 */

#include <stdio.h>
#include <hipl_format.h>
int types[] = {PFBYTE,PFSHORT,PFINT,PFFLOAT,PFDOUBLE,PFCOMPLEX,PFDBLCOM,
		LASTTYPE};
static Flag_Format flagfmt[] = {
    {"h",{LASTFLAG},1,{{PTINT,"1","hrate"},{PTINT,"0","hoffset"},
	LASTPARAMETER}},
    {"v",{LASTFLAG},1,{{PTINT,"1","vrate"},{PTINT,"0","voffset"},
	LASTPARAMETER}},
    {"t",{LASTFLAG},1,{{PTINT,"1","trate"},{PTINT,"0","toffset"},
	LASTPARAMETER}},
    {"d",{LASTFLAG},1,{{PTINT,"1","drate"},{PTINT,"0","doffset"},
	LASTPARAMETER}},
    {"c",{LASTFLAG},1,{{PTINT,"1","crate"},{PTINT,"0","coffset"},
	LASTPARAMETER}},
    LASTFLAG};

int main(argc,argv)

int argc;
char **argv;

{
	int hrate,hoffset,vrate,voffset,trate,toffset,drate,doffset;
	int crate,coffset,f,nfr,c,ncol,d,ndepth,nr,nc,method;
	int roifrow,roifcol,roilrow,roilcol,nofr,nocol,nodepth,nor,noc;
	struct header hd,hdp,hdo,hdcb;
	struct hips_roi roi;
	Filename filename;
	FILE *fp;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&hrate,&hoffset,&vrate,&voffset,&trate,
		&toffset,&drate,&doffset,&crate,&coffset,FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	ncol = hd.numcolor;
	ndepth = hgetdepth(&hd);
	nfr = hd.num_frame / (ncol*ndepth);
	nr = hd.orows;
	nc = hd.ocols;
	if (hrate < 1 || vrate < 1 || trate < 1 || drate < 1 || crate < 1)
		perr(HE_MSG,"all rate parameters must be positive");
	if (hoffset < 0 || voffset < 0 || toffset < 0 || doffset < 0 ||
	    coffset < 0)
		perr(HE_MSG,"all offset parameters must be nonnegative");
	if (hrate > nc)
		perr(HE_MSG,"horizontal rate larger than the image width");
	if (vrate > nr)
		perr(HE_MSG,"horizontal rate larger than the image height");
	if (trate > nfr)
		perr(HE_MSG,"time rate larger than the sequence length");
	if (drate > ndepth)
		perr(HE_MSG,"depth rate larger than the image depth");
	if (crate > ncol)
		perr(HE_MSG,
		    "color plane rate larger than the number of color planes");
	if (hoffset >= hrate || voffset >= vrate || toffset >= trate ||
	    doffset >= drate || coffset >= crate)
		perr(HE_MSG,
		    "offsets must be smaller than their corresponding rates");
	method = fset_conversion(&hd,&hdp,types,filename);
	dup_headern(&hdp,&hdo);
	nofr = (nfr + trate - (1 + toffset)) / trate;
	nocol = (ncol + crate - (1 + coffset)) / crate;
	nodepth = (ndepth + drate - (1 + doffset)) / drate;
	nor = (nr + vrate - (1 + voffset)) / vrate;
	noc = (nc + hrate - (1 + hoffset)) / hrate;
	setsize(&hdo,nor,noc);
	hdo.num_frame = nofr*nodepth*nocol;
	hdo.numcolor = nocol;
	if (ndepth > 1)
		hsetdepth(&hdo,nodepth);
	getroi(&hd,&roi);
	roifrow = roi.frow - (voffset + 1);
	if (roifrow < 0)
		roifrow = 0;
	else
		roifrow = 1 + (roifrow/vrate);
	roilrow = (roi.frow + roi.rows - 1 - voffset) / vrate;
	roifcol = roi.fcol - (hoffset + 1);
	if (roifcol < 0)
		roifcol = 0;
	else
		roifcol = 1 + (roifcol/hrate);
	roilcol = (roi.fcol + roi.cols - 1 - hoffset) / hrate;
	if (roifrow > roilrow || roifcol > roilcol) {
		perr(HE_IMSG,
		    "region of interest disappeared, reset to entire image");
		clearroi(&hdo);
	}
	else
		setroi(&hdo,roifrow,roifcol,roilrow-roifrow+1,
			roilcol-roifcol+1);
	hdo.image = hdo.firstpix = hdp.image;
	if (hips_convback)
		setupconvback(&hd,&hdo,&hdcb);
	write_headeru2(&hdcb,&hdo,argc,argv,hips_convback);
	clearroi(&hd); clearroi(&hdp); clearroi(&hdo); clearroi(&hdcb);
	for (f=0;f<nfr;f++) {
		for (d=0;d<ndepth;d++) {
			for (c=0;c<ncol;c++) {
				if (((f - toffset) % trate == 0) &&
				    ((d - doffset) % drate == 0) &&
				    ((c - coffset) % crate == 0)) {
					fread_imagec(fp,&hd,&hdp,method,f,
						filename);
					h_sample(&hdp,&hdo,hrate,vrate,
						hoffset,voffset);
					write_imagec(&hdcb,&hdo,method,
						hips_convback,f);
				}
				else
					fread_image(fp,&hd,f,filename);
			}
		}
	}
	return(0);
}
