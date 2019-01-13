/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * adddesc.c - add information to a sequence header
 *
 * usage:	[-d "original sequence date" ]
 *		[-s "sequence name"	     ]
 *	adddesc [-o "originator's name"	     ]	... <frame
 *		[-a "additional description" ]
 *		[-r "description replacement"] 
 *
 * to load: cc -o adddesc adddesc.c -lhips
 *
 * Michael Landy - 2/8/82
 * HIPS 2 - msl - 1/14/91
 */

#include <hipl_format.h>
#include <stdio.h>

static Flag_Format flagfmt[] = {
    {"d",{LASTFLAG},1,{{PTBOOLEAN,"FALSE"},{PTSTRING,"","origdate"},
	LASTPARAMETER}},
    {"s",{LASTFLAG},1,{{PTBOOLEAN,"FALSE"},{PTSTRING,"","seqname"},
	LASTPARAMETER}},
    {"o",{LASTFLAG},1,{{PTBOOLEAN,"FALSE"},{PTSTRING,"","origname"},
	LASTPARAMETER}},
    {"a",{"r",LASTFLAG},1,{{PTBOOLEAN,"FALSE"},{PTSTRING,"","appenddesc"},
	LASTPARAMETER}},
    {"r",{"a",LASTFLAG},1,{{PTBOOLEAN,"FALSE"},{PTSTRING,"","replacedesc"},
	LASTPARAMETER}},
    LASTFLAG};

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd;
	int c,i,*fmts,fmtssize;
	hsize_t currsize;
	h_boolean dflag,sflag,oflag,aflag,rflag;
	Filename filename;
	char *odt,*snm,*onm,*adesc,*rdesc;
	FILE *fp;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&dflag,&odt,&sflag,&snm,&oflag,&onm,
		&aflag,&adesc,&rflag,&rdesc,FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	if (dflag)
		hd.orig_date = odt;
	if (sflag)
		hd.seq_name = snm;
	if (oflag)
		hd.orig_name = onm;
	if (aflag)
		desc_append(&hd,adesc);
	if (rflag)
		desc_set(&hd,rdesc);
	write_header(&hd);
	if (hd.sizeimage) {
		for (i=0;i<hd.num_frame;i++) {
			fread_image(fp,&hd,i,filename);
			write_image(&hd,i);
		}
	}
	else if (hd.pixel_format == PFMIXED) {
		fmtssize = hd.num_frame;
		getparam(&hd,"formats",PFINT,&fmtssize,&fmts);
		if (fmtssize != hd.num_frame)
			perr(HE_FMTSLEN,filename);
		setformat(&hd,fmts[0]);
		alloc_image(&hd);
		currsize = hd.sizeimage;
		for (i=0;i<hd.num_frame;i++) {
			setformat(&hd,fmts[i]);
			if (hd.sizeimage > currsize) {
				free(hd.image);
				alloc_image(&hd);
				currsize = hd.sizeimage;
			}
			fread_image(fp,&hd,i,filename);
			write_image(&hd,i);
		}
	}
	else {
		while ((c=getc(fp)) != EOF) putchar(c);
	}
	return(0);
}
