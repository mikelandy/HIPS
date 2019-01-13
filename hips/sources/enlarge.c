/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * enlarge - enlarge a sequence by pixel replication or interpolation
 *
 * usage:	enlarge [-h hfactor] [-v vfactor] [-t tfactor] [-i]
 * 		enlarge [-s [spacefactor]] [-t tfactor] [-i]
 * 
 * Enlarge enlarges the input sequence vertically by vfactor, horizontally by 
 * hfactor and temporally by tfactor.  The spatial factors default to 1, and 
 * tfactor defaults to 1.  In the second calling sequence form, the user
 * specifies spacefactor, which is applied in both spatial dimensions, and
 * defaults to 2.  The -i switch causes the program to (tri-linearly)
 * interpolate the added pixels.  If no factors are specified, "-s 2" is the
 * default.
 *
 * to load:	cc -o enlarge enlarge.c -lhipsh -lhips -lm
 *
 * Rewritten by Charlie Chubb - 10/24/87
 * msl - folded in the -i switch - 11/5/87
 * HIPS 2 - msl - 1/12/91
 */

#include <stdio.h>
#include <hipl_format.h>
int types[] = {PFBYTE,PFINT,PFFLOAT,PFCOMPLEX,LASTTYPE};
static Flag_Format flagfmt[] = {
    {"h",{"s",LASTFLAG},1,{{PTINT,"1","hfactor"},LASTPARAMETER}},
    {"v",{"s",LASTFLAG},1,{{PTINT,"1","vfactor"},LASTPARAMETER}},
    {"t",{LASTFLAG},1,{{PTINT,"1","tfactor"},LASTPARAMETER}},
    {"s",{"h","v",LASTFLAG},0,{{PTBOOLEAN,"TRUE"},{PTINT,"2","spacefactor"},
	LASTPARAMETER}},
    {"i",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
    LASTFLAG};

int main(argc,argv)

int argc;
char **argv;

{
	h_boolean sflag,iflag,itflag=FALSE;
	int xfactor,yfactor,tfactor,sfactor;
	int f,fr,i,method;
	struct header hd,hdp1,hdp2,*thd,*chd1,*chd2,hdo,hdcb;
	struct hips_roi roi;
	Filename filename;
	FILE *fp;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&xfactor,&yfactor,&tfactor,&sflag,&sfactor,
		&iflag,FFONE,&filename);
	fp = hfopenr(filename);
	if (sflag)
		xfactor = yfactor = sfactor;
	if (iflag && tfactor>1)
		itflag = TRUE;
	fread_hdr_a(fp,&hd,filename);
	method = fset_conversion(&hd,&hdp1,types,filename);
	fr = hdp1.num_frame;
	if (hdp1.numcolor > 1 && tfactor > 1)
		perr(HE_MSG,"can't enlarge color images in the time dimension");
	dup_headern(&hdp1,&hdo);
	getroi(&hdo,&roi);
	setsize(&hdo,yfactor*hdp1.orows,xfactor*hdp1.ocols);
	hdo.num_frame *= tfactor;
	hd.num_frame *= tfactor;
	hdo.frow = roi.frow*yfactor;
	hdo.fcol = roi.fcol*xfactor;
	hdo.rows = roi.rows*yfactor;
	hdo.cols = roi.cols*xfactor;
	alloc_image(&hdo);
	if (hips_convback)
		setupconvback(&hd,&hdo,&hdcb);
	write_headeru2(&hdcb,&hdo,argc,argv,hips_convback);
	clearroi(&hdp1);
	clearroi(&hdo);
	if (itflag) {
		dup_headern(&hdp1,&hdp2);
		alloc_image(&hdp2);
		fread_imagec(fp,&hd,&hdp1,method,0,filename);
		chd1 = &hdp1;
		chd2 = &hdp2;
		for (f=0;f<fr;f++) {
			if (f == fr-1)
				chd2 = chd1;
			else
				fread_imagec(fp,&hd,chd2,method,f+1,filename);
			for (i=0;i<tfactor;i++) {
				h_ienlarge3(chd1,chd2,&hdo,xfactor,yfactor,
					tfactor,i);
				write_imagec(&hdcb,&hdo,method,hips_convback,f);
			}
			thd = chd1;
			chd1 = chd2;
			chd2 = thd;
		}
	}
	else if (iflag) {
		for (f=0;f<fr;f++) {
			fread_imagec(fp,&hd,&hdp1,method,f,filename);
			h_ienlarge(&hdp1,&hdo,xfactor,yfactor);
			write_imagec(&hdcb,&hdo,method,hips_convback,f);
		}
	}
	else {
		for (f=0;f<fr;f++) {
			fread_imagec(fp,&hd,&hdp1,method,f,filename);
			h_enlarge(&hdp1,&hdo,xfactor,yfactor);
			for (i=0;i<tfactor;i++)
				write_imagec(&hdcb,&hdo,method,hips_convback,f);
		}
	}
	return(0);
}
