/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * genheader - generate an image header
 *
 * usage:	genheader [-MP -LP -B -SB -S -US -I -UI -F -D -C -DC -RGB -RGBZ
 *				-ZRGB -BGR -BGRZ -ZBGR]
 *			[-s rows [cols]] [-f frames] [-nd numdepth]
 *			[-nc numcolor] [-p] <idata >oseq
 *
 * Creates an image header.  The output pixel format may be either MSBF (-MP),
 * LSBF (-LP), byte (-B, the default), signed byte (-SB), short (-S), unsigned
 * short (-US), integer (-I), unsigned integer (-UI), float (-F), double (-D),
 * complex (-C), double complex (-DC), RGB, RGBZ, ZRGB, BGR, BGRZ, ZBGR.
 * Switches -s, -f, -nd and -nc specify the size, number of color frames,
 * number of depth planes and number of color planes.  The actual number of
 * frames of data is frames*numdepth*numcolor.  The number of rows defaults to
 * 512, the number of columns defaults to the number of rows, and the number
 * of color frames, depths and color planes default to 1.  Switch -p specifies
 * that there is a header-less input sequence which should be read and
 * appended to the new HIPS header.
 *
 * to load:	cc -o genheader genheader.c -hipsh -lhips
 *
 * HIPS 2 - Michael Landy - 7/5/91
 * added ZRGB/BGR/BGRZ/ZBGR - msl - 5/24/93
 * added depths - msl - 3/8/94
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {
    {"MP",{"LP","B","SB","S","US","I","UI","F","D","C","DC","RGB","RGBZ",
	"ZRGB","BGR","BGRZ","ZBGR",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},
	LASTPARAMETER}},
    {"LP",{"MP","B","SB","S","US","I","UI","F","D","C","DC","RGB","RGBZ",
	"ZRGB","BGR","BGRZ","ZBGR",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},
	LASTPARAMETER}},
    {"B",{"MP","LP","SB","S","US","I","UI","F","D","C","DC","RGB","RGBZ",
	"ZRGB","BGR","BGRZ","ZBGR",LASTFLAG},0,{{PTBOOLEAN,"TRUE"},
	LASTPARAMETER}},
    {"SB",{"LP","MP","B","S","US","I","UI","F","D","C","DC","RGB","RGBZ",
	"ZRGB","BGR","BGRZ","ZBGR",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},
	LASTPARAMETER}},
    {"S",{"MP","LP","B","SB","US","I","UI","F","D","C","DC","RGB","RGBZ",
	"ZRGB","BGR","BGRZ","ZBGR",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},
	LASTPARAMETER}},
    {"US",{"MP","LP","B","SB","S","I","UI","F","D","C","DC","RGB","RGBZ",
	"ZRGB","BGR","BGRZ","ZBGR",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},
	LASTPARAMETER}},
    {"I",{"MP","LP","B","SB","S","US","UI","F","D","C","DC","RGB","RGBZ",
	"ZRGB","BGR","BGRZ","ZBGR",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},
	LASTPARAMETER}},
    {"UI",{"MP","LP","B","SB","S","US","I","F","D","C","DC","RGB","RGBZ",
	"ZRGB","BGR","BGRZ","ZBGR",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},
	LASTPARAMETER}},
    {"F",{"MP","LP","B","SB","S","US","I","UI","D","C","DC","RGB","RGBZ",
	"ZRGB","BGR","BGRZ","ZBGR",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},
	LASTPARAMETER}},
    {"D",{"MP","LP","B","SB","S","US","I","UI","F","C","DC","RGB","RGBZ",
	"ZRGB","BGR","BGRZ","ZBGR",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},
	LASTPARAMETER}},
    {"C",{"MP","LP","B","SB","S","US","I","UI","F","D","DC","RGB","RGBZ",
	"ZRGB","BGR","BGRZ","ZBGR",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},
	LASTPARAMETER}},
    {"DC",{"MP","LP","B","SB","S","US","I","UI","F","D","C","RGB","RGBZ",
	"ZRGB","BGR","BGRZ","ZBGR",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},
	LASTPARAMETER}},
    {"RGB",{"MP","LP","B","SB","S","US","I","UI","F","D","C","DC","RGBZ",
	"ZRGB","BGR","BGRZ","ZBGR",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},
	LASTPARAMETER}},
    {"RGBZ",{"MP","LP","B","SB","S","US","I","UI","F","D","C","DC","RGB",
	"ZRGB","BGR","BGRZ","ZBGR",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},
	LASTPARAMETER}},
    {"ZRGB",{"MP","LP","B","SB","S","US","I","UI","F","D","C","DC","RGB",
	"RGBZ","BGR","BGRZ","ZBGR",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},
	LASTPARAMETER}},
    {"BGR",{"MP","LP","B","SB","S","US","I","UI","F","D","C","DC","RGB",
	"RGBZ","ZRGB","BGRZ","ZBGR",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},
	LASTPARAMETER}},
    {"BGRZ",{"MP","LP","B","SB","S","US","I","UI","F","D","C","DC","RGB",
	"RGBZ","ZRGB","BGR","ZBGR",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},
	LASTPARAMETER}},
    {"ZBGR",{"MP","LP","B","SB","S","US","I","UI","F","D","C","DC","RGB",
	"RGBZ","ZRGB","BGR","BGRZ",LASTFLAG},0,{{PTBOOLEAN,"FALSE"},
	LASTPARAMETER}},
    {"s",{LASTFLAG},1,{{PTINT,"512","rows"},{PTINT,"-1","cols"},LASTPARAMETER}},
    {"f",{LASTFLAG},1,{{PTINT,"1","frames"},LASTPARAMETER}},
    {"nd",{LASTFLAG},1,{{PTINT,"1","numdepth"},LASTPARAMETER}},
    {"nc",{LASTFLAG},1,{{PTINT,"1","numcolor"},LASTPARAMETER}},
    {"p",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
    LASTFLAG};

int main(argc,argv)

int argc;
char **argv;

{
	int oform;
	h_boolean MPflag,LPflag,Bflag,SBflag,Sflag,USflag,Iflag,UIflag;
	h_boolean Fflag,Dflag,Cflag,DCflag,RGBflag,RGBZflag,ZRGBflag;
	h_boolean BGRflag,BGRZflag,ZBGRflag,pflag;
	int rows,cols,frames,f,numcolor,numdepth;
	struct header hd;
	Filename filename;
	FILE *fp;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&MPflag,&LPflag,&Bflag,&SBflag,&Sflag,
		&USflag,&Iflag,&UIflag,&Fflag,&Dflag,&Cflag,&DCflag,
		&RGBflag,&RGBZflag,&ZRGBflag,&BGRflag,&BGRZflag,&ZBGRflag,
		&rows,&cols,&frames,&numdepth,&numcolor,&pflag,FFONE,&filename);
	if (cols == -1)
		cols = rows;
	if (MPflag)
		oform = PFMSBF;
	else if (LPflag)
		oform = PFLSBF;
	else if (Bflag)
		oform = PFBYTE;
	else if (SBflag)
		oform = PFSBYTE;
	else if (Sflag)
		oform = PFSHORT;
	else if (USflag)
		oform = PFUSHORT;
	else if (Iflag)
		oform = PFINT;
	else if (UIflag)
		oform = PFUINT;
	else if (Fflag)
		oform = PFFLOAT;
	else if (Dflag)
		oform = PFDOUBLE;
	else if (Cflag)
		oform = PFCOMPLEX;
	else if (DCflag)
		oform = PFDBLCOM;
	else if (RGBflag)
		oform = PFRGB;
	else if (RGBZflag)
		oform = PFRGBZ;
	else if (ZRGBflag)
		oform = PFZRGB;
	else if (BGRflag)
		oform = PFBGR;
	else if (BGRZflag)
		oform = PFBGRZ;
	else
		oform = PFZBGR;
	init_header(&hd,"","",frames*numdepth*numcolor,"",rows,cols,oform,
		numcolor,"");
	if (numdepth > 1)
		hsetdepth(&hd,numdepth);
	write_headeru(&hd,argc,argv);
	if (pflag) {
		fp = hfopenr(filename);
		alloc_image(&hd);
		for (f=0;f<frames*numdepth*numcolor;f++) {
			fread_image(fp,&hd,f,filename);
			write_image(&hd);
		}
	}
	return(0);
}
