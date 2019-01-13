static char *SccsId = "%W%      %G%";

/*      Copyright (c) 1991 The Turing Institute

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authors.  All the
software has been tested extensively and every effort has been made to
insure its reliability.   */

/* hiptovit - convert Hips header into Vitok header (byte format only)
 *
 * usage:	hiptovit 
 *
 * to load:	cc -o hiptovit hiptovit.c -lhips
 *
 * Peter Mowforth & Jin Zhengping -8/5/85 
 * Rewritten by Jin Zhengping - 31 August 1991
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *
 *  VITOK HEADER DESCRIPTION:
 *
 *     The header is always a number of short-types which contain some 
 *  information about the file. As the basic header serves the GEC standard
 *  (also used by the image capturing system) which has 11 items:
 *   
 *   head[0]   headrlength (11 in the GEC standard)
 *   head[1]   ydim = number of pixels in y - dimension
 *   head[2]   xdim = number of pixels in x - dimension
 *   head[3] ... head[10] are not used (?)
 *
 *     This basic header may be extended by appending further items. The total
 *  number of items is stored in head[0]. Particularly, of these appended
 *  items
 *
 *   head[head[0] - 2]  (i.e. one before last) contains
 *		       either 'in' for "integer" image files
 *                     or     'do' for "double" image files.
 *   head[head[0] - 1]  (i.e. the last item) contains information about the
 *                     operator which produced the file.
 *
 */


#include <stdio.h>
#include <hipl_format.h>
int types[] = {PFBYTE,LASTTYPE};
static Flag_Format flagfmt[] =
{
	LASTFLAG
};

int main(argc,argv)
int  argc;
char *argv[];

{
	short		head[11];
	struct		header hd;
	Filename        filename;
	FILE            *fp;
	byte		*bp;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,FFONE,&filename);
	fp=hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	if(hd.pixel_format!=PFBYTE)
		perr(HE_FMT, hd.pixel_format);
        head[0] = (short)11;	
	head[1] = hd.orows ;
	head[2] = hd.ocols ;
	bp = (byte *) &(head[head[0]-2]);
	*bp++ = 'i';
	*bp++ = 'n';
	if(fwrite((char *)head, sizeof(short), *head, stdout) != *head)
		perr(HE_HDRWRT);
	fread_image(fp,&hd,0,filename);
	write_image(&hd,0);
	return(0);
}
