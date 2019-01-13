/*	Copyright (c) 1991 Ulrik P.V. Skands

Disclaimer:  No guarantees of performance accompany this software,
nor is any responsibility assumed on the part of the authers.  All the
software has been tested extensively and every effort has been made to
insure its reliability.	  */

/*
 * distmin.c - calculates global distances in binary images
 *
 * Implementations of the distance algoritms by 
 * [1]  Gunilla Borgefors, 1984,
 *	"Distance Transformations in Arbitrary Dimensions", Computer
 *	Vision, Graphics, and Image Processing, Vol. 27,
 *	pp. 321-345, 1984.
 * [2]  Qin-Zhong Ye, 1988,
 *	"The Signed Euclidean Distance Transform and Its Applications",
 *	Int. Conf. on Pattern Recognition (9), Rome, Italy, 1988,
 *	pp. 495-499.
 *
 * usage - distmin [-h]
 *		   [-c <horizontal/vertical distance> <diagonal distance>]
 *		   [-e <vertical unit distance> <horizontal unit distance>]
 *
 * The input sequence must be a binary image with the feature 
 * value of zero.
 * The output can be either 
 *			int [-h], 
 *			float [-c [<d4> [<d8>]]] or 
 * 			complex [-e [<dh> [<dv>]]].
 *
 * Subroutines used:	h_hexagon.c
 *			h_chamfer.c
 *			h_sedt.c
 *
 * To load: 
 * cc -o distmin distmin.c h_hexagon.c h_chamfer.c h_sedt.c -lhipsh -lhips -lm
 *
 * Author:	Ulrik P.V. Skands, IMSOR.
 *		HIPS-2 Ulrik Skands, IT.
 *
 * Original  6/7-91  upvs
 * Updated  28/10-91 upvs
 *	     8/4-92  upvs
 * HIPS-2    5/10-92 upvs
 */

#include <stdio.h>
#include <hipl_format.h>
int types[] = {PFBYTE,LASTTYPE};
static Flag_Format flagfmt[] = {
    {"h",{"c","e",LASTFLAG},0,{{PTBOOLEAN,"TRUE"},LASTPARAMETER}},
    {"c",{"h","e",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},{PTDOUBLE,"1.","d4"},
    	{PTDOUBLE,"1.3507","d8"},LASTPARAMETER}},
    {"e",{"h","c",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},{PTDOUBLE,"-1.","dh"},
	{PTDOUBLE,"-1.","dv"},LASTPARAMETER}},
    LASTFLAG};

int h_hexagon(),h_chamfer(),h_sedt();

int main(argc,argv)

int argc;
char **argv;

{
	h_boolean		hflag,cflag,eflag;
	double 		cd1,cd2,ed1,ed2;
	int 		f,fr,method;
	struct header 	hd,hdp,hdo;
	Filename 	filename;
	FILE 		*fp;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&hflag,&cflag,&cd1,&cd2,&eflag,&ed1,&ed2,
		FFONE,&filename);
	if (!hflag && !cflag && !eflag)
		perr(HE_MSG,"one of -h, -c and -e must be specified");
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	method = fset_conversion(&hd,&hdp,types,filename);
	dup_headern(&hdp,&hdo);
	if(hflag) setformat(&hdo,PFINT);
	else {
	  if(cflag) setformat(&hdo,PFFLOAT);
	  else {
	    if(eflag) setformat(&hdo,PFCOMPLEX);
	    else {
	      perr(HE_MSG,"Execution error");
	    }
	  }
	}
	alloc_image(&hdo);
	write_headeru(&hdo,argc,argv);
	fr = hd.num_frame;
	for(f=0;f<fr;f++) {
		fread_imagec(fp,&hd,&hdp,method,f,filename);
		if(hflag) h_hexagon(&hdp,&hdo);
		else {
	  	  if(cflag) h_chamfer(&hdp,&hdo,cd1,cd2);
	  	  else {
	    	    if(eflag) h_sedt(&hdp,&hdo,ed1,ed2);
	    	    else {
	      	      perr(HE_MSG,"Execution error");
	    	    }
	 	  }
		}
		write_image(&hdo,f);
	}
	return(0);
}
