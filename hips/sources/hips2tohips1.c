/*
 * hips2tohips1.c - convert a HIPS-2 sequence back to HIPS-1 format
 *
 * Usage:	hips2tohips1 [-n] [-h]
 *
 * Load:	cc -o hips2tohips1 hips2tohips1.c -lhips
 *
 * By default the sequence history is deleted and the sequence description is
 * left in.  The history can be retained with -h and the description deleted
 * with -n.  Note that the program does not check whether the HIPS-2 sequence
 * history and/or description are appropriate for HIPS-1.  In particular, if
 * the description contains a line with a sole `.', that will confuse a HIPS-1
 * program.  If the history contains a line prior to the last line which does
 * not end with the sequence `| \', then the history will confuse a HIPS-1
 * program.  The default program operation is likely to work for most
 * sequences.
 *
 * Michael Landy - 1/17/92
 * based on padheader - msl - 2/11/93
 */

#include <hipl_format.h>
#include <stdio.h>

static Flag_Format flagfmt[] = {
	{"n",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"h",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	LASTFLAG};
h_boolean nflag,hflag;
void write_header1(),wnocrs(),wstr();

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd;
	int c,i,pfmt;
	Filename filename;
	FILE *fp;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&nflag,&hflag,FFONE,&filename);
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	pfmt = hd.pixel_format;
	if (pfmt != PFBYTE &&
	    pfmt != PFSHORT &&
	    pfmt != PFINT &&
	    pfmt != PFFLOAT &&
	    pfmt != PFCOMPLEX &&
	    pfmt != PFMSBF &&
	    pfmt != PFLSBF &&
	    pfmt != PFASCII &&
	    pfmt != PFDOUBLE &&
	    pfmt != PFDBLCOM &&
	    pfmt != PFQUAD &&
	    pfmt != PFQUAD1 &&
	    pfmt != PFSPAN &&
	    pfmt != PLOT3D &&
	    pfmt != PFPOLYLINE &&
	    pfmt != PFCOLVEC &&
	    pfmt != PFUKOOA &&
	    pfmt != PFTRAINING &&
	    pfmt != PFTOSPACE &&
	    pfmt != PFSTEREO &&
	    pfmt != PFRGPLINE &&
	    pfmt != PFRGISPLINE &&
	    pfmt != PFCHAIN &&
	    pfmt != PFLUT &&
	    pfmt != PFAHC &&
	    pfmt != PFOCT &&
	    pfmt != PFBT &&
	    pfmt != PFAHC3 &&
	    pfmt != PFBQ &&
	    pfmt != PFRLED &&
	    pfmt != PFRLEB &&
	    pfmt != PFRLEW &&
	    pfmt != PFPOLAR)
		perr(HE_MSG,"can't convert input pixel format to HIPS1");
	update_header(&hd,argc,argv);
	write_header1(&hd);
	if (hd.sizeimage) {
		for (i=0;i<hd.num_frame;i++) {
			fread_image(fp,&hd,i,filename);
			write_image(&hd,i);
		}
	}
	else {
		while ((c=getc(fp)) != EOF)
			putchar(c);
	}
	return(0);
}

void write_header1(hd)

struct header *hd;

{
	wnocrs(hd->orig_name);
	wnocrs(hd->seq_name);
	printf("%d\n",hd->num_frame);
	wnocrs(hd->orig_date);
	printf("%d\n",hd->orows);
	printf("%d\n",hd->ocols);
	if (hd->pixel_format == PFLSBF || hd->pixel_format == PFMSBF) {
		printf("%d\n",1);
		printf("%d\n",(hd->pixel_format == PFMSBF ? 1 : 2));
		printf("%d\n",PFBYTE);
	}
	else {
		printf("%lu\n",8*(hd->sizepix));
		printf("%d\n",0);
		printf("%d\n",hd->pixel_format);
	}
	if (hflag)
		wstr(hd->seq_history);
	else
		printf(" \n");
	if (!nflag)
		wstr(hd->seq_desc);
	printf(".\n");
}

void wstr(s)

char *s;

{
	int i;

	i = strlen(s);
	printf("%s",s);
	if (s[0] == '\0' || s[i-1] != '\n') {
		printf("\n");
		i++;
	}
}

void wnocrs(s)

char *s;

{
	char *t;
	int i;

	t = s;
	i = 0;
	while (*t != '\n' && *t != '\0') {
		t++;
		i++;
	}
	*t = '\0';
	printf("%s\n",s);
}
