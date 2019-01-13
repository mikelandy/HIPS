/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * comicstrip - display batches of frames in a single matrix of pictures
 *
 * usage:	comicstrip [-s mr [mc]] [-m margin] [-e] <iseq >oseq
 *
 * Comicstrip takes batches of successive frames in an image sequence and
 * displays them in a matrix of rows and columns in a single image.  By
 * default, the program calculates the squarest matrix it can to fit the
 * entire sequence (with more columns than rows, if necessary).  If only
 * mr is specified, this is the number of rows of images, and the number of
 * columns is set sufficiently large to contain the entire sequence.  If mc
 * is specified, then each output frame contains the frames from a batch of
 * mr*mc input frames (although the final output frame may only be partially
 * filled.  For bit-packed output frames, the column width will be rounded up
 * to an even multiple of 8.  A border is interpolated between each row and
 * column.  The border size may be specified by -m, and defaults to no border
 * at all.  The borders are filled with the background value which is set by
 * -UL which defaults, as usual, to zero).  By
 * default, the program only displays the region of interest.  If -e is
 * specified, the ROI is cleared and entire images are displayed.  The output
 * region of interest is cleared.
 *
 * to load:	cc -o comicstrip comicstrip.c -lhipsh -lhips -lm
 *
 * Hips 2 - msl - 8/16/91
 */

#include <hipl_format.h>
#include <math.h>

static Flag_Format flagfmt[] = {
	{"s",{LASTFLAG},1,{{PTINT,"-1","matrixrows"},
		{PTINT,"-1","matrixcols"},LASTPARAMETER}},
	{"m",{LASTFLAG},1,{{PTINT,"0","margin"},LASTPARAMETER}},
	{"e",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	LASTFLAG};
int types[] = {PFMSBF,PFLSBF,PFBYTE,PFSBYTE,PFSHORT,PFUSHORT,PFINT,PFUINT,
	PFFLOAT,PFDOUBLE,LASTTYPE};

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd,hdp,hdo;
	int method,fr,f,ofr,of,mr,mc,margin,pfmt,nr,nc,r,c;
	Filename filename;
	FILE *fp;
	h_boolean eflag;
	Pixelval val;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&mr,&mc,&margin,&eflag,FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	if (eflag)
		clearroi(&hd);
	method = fset_conversion(&hd,&hdp,types,filename);
	if (hdp.numcolor != 1) {
		perr(HE_IMSG,"number of color planes reset to 1");
		hdp.numcolor = 1;
	}
	if (hgetdepth(&hdp) != 1) {
		perr(HE_IMSG,"number of depth planes reset to 1");
		hsetdepth(&hdp,1);
	}
	pfmt = hdp.pixel_format;
	nr = hdp.rows;
	nc = hdp.cols;
	if ((pfmt == PFMSBF || pfmt == PFLSBF) && ((nc+margin)%8 != 0))
		margin += 8 - ((nc+margin)%8);
	fr = hdp.num_frame;
	if (mr < 0) {
		mr = sqrt((double) fr + .05);
	}
	if (mc < 0)
		mc = (mr >= fr) ? 1 : ((fr + mr - 1)/mr);
	dup_headern(&hdp,&hdo);
	setsize(&hdo,(mr*hdp.rows)+((mr-1)*margin),
		(mc*hdp.cols)+((mc-1)*margin));
	hdo.num_frame = ofr = (fr + mr*mc - 1)/(mr*mc);
	alloc_image(&hdo);
	write_headeru(&hdo,argc,argv);
	switch (pfmt) {
	case PFMSBF:
	case PFLSBF:	val.v_byte = hips_lchar ? 255 : 0; break;
	case PFBYTE:	val.v_byte = hips_lchar; break;
	case PFSBYTE:	val.v_sbyte = hips_lchar; break;
	case PFSHORT:	val.v_short = hips_lchar; break;
	case PFUSHORT:	val.v_ushort = hips_lchar; break;
	case PFINT:	val.v_int = hips_lchar; break;
	case PFUINT:	val.v_uint = hips_lchar; break;
	case PFFLOAT:	val.v_float = hips_lchar; break;
	case PFDOUBLE:	val.v_double = hips_lchar; break;
	}
	if (margin != 0)
		h_setimage(&hdo,&val);
	f = 0;
	for (of=0;of<ofr;of++) {
		for (r=0;r<mr;r++) {
			for (c=0;c<mc;c++) {
				setroi(&hdo,
#ifdef ULORIG
					r*(nr + margin),
#else
					(mr - r - 1)*(nr + margin),
#endif
					c*(nc + margin),nr,nc);
				if (f++ < fr) {
					fread_imagec(fp,&hd,&hdp,method,f-1,
						filename);
					h_copy(&hdp,&hdo);
				}
				else if (of != 0)
					h_setimage(&hdo,&val);
			}
		}
		write_image(&hdo,f);
	}
	return(0);
}
