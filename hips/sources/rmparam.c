/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * rmparam.c - remove parameters from a HIPS image header
 *
 * usage:	rmparam -n name1 [name2 [name3 [name4 [name5 [name6]]]]]
 *
 * Rmparam is used to remove parameters from a HIPS image header.  Each
 * parameter named in the command line is removed from the input image
 * sequence parameters list.  If any parameter is not found, a warning is
 * issued.
 *
 * to load:	cc -o rmparam rmparam.c -lhips
 *
 * Mike Landy - 5/25/93
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {
	{"n",{LASTFLAG},1,{{PTBOOLEAN,"FALSE"},{PTSTRING,"","name1"},
		{PTSTRING,"","name2"},{PTSTRING,"","name3"},
		{PTSTRING,"","name4"},{PTSTRING,"","name5"},
		{PTSTRING,"","name6"},LASTPARAMETER}},
	LASTFLAG};

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd;
	Filename filename;
	FILE *fp;
	int i,c,fmtssize,currsize,*fmts;
	char *names[6],msg[100];
	h_boolean nflag;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&nflag,names,names+1,names+2,names+3,
		names+4,names+5,FFONE,&filename);
	if (!nflag)
		perr(HE_MSG,"-n flag is mandatory");
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	for (i=0;i<6;i++) {
		if (names[i][0]) {
			if (findparam(&hd,names[i]) != NULLPAR)
				clearparam(&hd,names[i]);
			else {
				sprintf(msg,
				    "name `%s' not found in parameters list",
				    names[i]);
				perr(HE_IMSG,msg);
			}
		}
	}
	write_headeru(&hd,argc,argv);
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
