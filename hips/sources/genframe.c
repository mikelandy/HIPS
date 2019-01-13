/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * genframe - generate a uniform frame
 *
 * usage:	genframe [-B -S -I -F -C] [-s rows [cols]] [-f frames]
 *			[-nd numdepth] [-nc numcolor]
 *			[-g greylevel [imaginarypart] | -G r g b] >oseq
 *
 * Creates a uniform sequence.  The output format may be either byte (-B,
 * the default), short (-S), integer (-I), float (-F) or complex (-C).
 * Switches -s, -f, -nd and -nc specify the size, number of color frames,
 * number of depths and number of color planes (the number of output frame is
 * the product of frames and numcolor.  The number of rows defaults to 512,
 * the number of columns defaults to the number of rows, and the numbers of
 * color frames, depths and planes default to 1.  The greylevel is specified
 * with -g, and defaults to zero.  The -G switch may be used when numcolor is
 * set to 3, setting each of the three color planes to the value of r (for
 * red), g (for green) and b (for blue).
 *
 * to load:	cc -o genframe genframe.c -hipsh -lhips
 *
 * HIPS 2 - Michael Landy - 7/5/91
 * added depths - msl - 3/8/94
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {
    {"B",{"S","I","F","C",LASTFLAG},0,{{PTBOOLEAN,"TRUE"},LASTPARAMETER}},
    {"S",{"B","I","F","C",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
    {"I",{"B","S","F","C",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
    {"F",{"B","S","I","C",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
    {"C",{"B","S","I","F",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
    {"s",{LASTFLAG},1,{{PTINT,"512","rows"},{PTINT,"-1","cols"},LASTPARAMETER}},
    {"f",{LASTFLAG},1,{{PTINT,"1","frames"},LASTPARAMETER}},
    {"nd",{LASTFLAG},1,{{PTINT,"1","numdepth"},LASTPARAMETER}},
    {"nc",{LASTFLAG},1,{{PTINT,"1","numcolor"},LASTPARAMETER}},
    {"g",{"G",LASTFLAG},1,{{PTBOOLEAN,"TRUE"},{PTDOUBLE,"0","greylevel"},
	{PTDOUBLE,"0","imaginarypart"},LASTPARAMETER}},
    {"G",{"g",LASTFLAG},3,{{PTDOUBLE,"0","r"},{PTDOUBLE,"0","g"},
	{PTDOUBLE,"0","b"},LASTPARAMETER}},
    LASTFLAG};

int main(argc,argv)

int argc;
char **argv;

{
	int oform;
	h_boolean Bflag,Sflag,Iflag,Fflag,Cflag,gflag;
	int rows,cols,frames,nc,nd,f,d,c;
	double gl,gli,r,g,b;
	struct header hd;
	Pixelval val,valr,valg,valb;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&Bflag,&Sflag,&Iflag,&Fflag,&Cflag,
		&rows,&cols,&frames,&nd,&nc,&gflag,&gl,&gli,&r,&g,&b,FFNONE);
	if (!gflag && nc != 3)
		perr(HE_MSG,"-G may only be used with -nc 3");
	if (cols == -1)
		cols = rows;
	if (Bflag) {
		oform = PFBYTE;
		val.v_byte = gl;
		valr.v_byte = r;
		valg.v_byte = g;
		valb.v_byte = b;
	}
	else if (Sflag) {
		oform = PFSHORT;
		val.v_short = gl;
		valr.v_short = r;
		valg.v_short = g;
		valb.v_short = b;
	}
	else if (Iflag) {
		oform = PFINT;
		val.v_int = gl;
		valr.v_int = r;
		valg.v_int = g;
		valb.v_int = b;
	}
	else if (Fflag) {
		oform = PFFLOAT;
		val.v_float = gl;
		valr.v_float = r;
		valg.v_float = g;
		valb.v_float = b;
	}
	else {
		oform = PFCOMPLEX;
		val.v_complex[0] = gl;
		val.v_complex[1] = gli;
		valr.v_complex[0] = r;
		valr.v_complex[1] = 0;
		valg.v_complex[0] = g;
		valg.v_complex[1] = 0;
		valb.v_complex[0] = b;
		valb.v_complex[1] = 0;
	}
	init_header(&hd,"","",frames*nc*nd,"",rows,cols,oform,nc,"");
	if (nd > 1)
		hsetdepth(&hd,nd);
	write_headeru(&hd,argc,argv);
	alloc_image(&hd);
	if (gflag)
		h_setimage(&hd,&val);
	for (f=0;f<frames;f++) {
	    for (d=0;d<nd;d++) {
		for (c=0;c<nc;c++) {
			if (!gflag)
				h_setimage(&hd,(c==0) ? &valr :
					((c==1) ? &valg : &valb));
			write_image(&hd);
		}
	    }
	}
	return(0);
}
