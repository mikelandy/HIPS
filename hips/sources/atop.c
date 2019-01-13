/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * atop - converts images to various pixel formats from ASCII
 *
 * usage:	atop [-B -S -I -F -C -RGB -RGBZ -ZRGB -BGR -BGRZ -ZBGR]
 *			[-c rows [cols]] [-f frames]
 *			[-d numdepth] [-nc numcolor] <iseq >oseq
 *
 * Creates a pixel formatted image from an Ascii image.  The -c switch is
 * used to create images from ascii data.  With this switch, no header is
 * expected in the input, and one is created for the output.  Cols defaults
 * to rows.  If -c is specified, -f may be used to specify the number of
 * frames, which defaults to 1, -d may be used to specify the number of depths,
 * which defaults to 1, and -nc may be used to specify the number of
 * colorplanes, which also defaults to 1.  The actual number of frames of
 * input is frames*numdepth*numcolor.  By default, the output format is
 * bytes (also set with -B).  Other formats available are shorts (-S),
 * ints (-I), floats (-F), complex (-C), and the various 3-color formats.
 * Note that for complex pixels, the number of columns in the input header is
 * halved, and for the 3-color formats the number of columns is divided by
 * three (to be consistent with ptoa.c).
 *
 * to load:	cc -o atop atop.c -lhips
 *
 * Mike Landy - 10/12/83
 * HIPS 2 - msl - 1/8/91
 * RGB/RGBZ/etc. - msl - 5/24/93
 * added -d - msl - 3/2/94
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {
    {"B",{"S","I","F","C","RGB","RGBZ","ZRGB","BGR","BGRZ","ZBGR",LASTFLAG},0,
	{{PTBOOLEAN,"TRUE"},LASTPARAMETER}},
    {"S",{"B","I","F","C","RGB","RGBZ","ZRGB","BGR","BGRZ","ZBGR",LASTFLAG},0,
	{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
    {"I",{"B","S","F","C","RGB","RGBZ","ZRGB","BGR","BGRZ","ZBGR",LASTFLAG},0,
	{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
    {"F",{"B","S","I","C","RGB","RGBZ","ZRGB","BGR","BGRZ","ZBGR",LASTFLAG},0,
	{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
    {"C",{"B","S","I","F","RGB","RGBZ","ZRGB","BGR","BGRZ","ZBGR",LASTFLAG},0,
	{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
    {"RGB",{"B","S","I","F","C","RGBZ","ZRGB","BGR","BGRZ","ZBGR",LASTFLAG},0,
	{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
    {"RGBZ",{"B","S","I","F","C","RGB","ZRGB","BGR","BGRZ","ZBGR",LASTFLAG},0,
	{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
    {"ZRGB",{"B","S","I","F","C","RGB","RGBZ","BGR","BGRZ","ZBGR",LASTFLAG},0,
	{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
    {"BGR",{"B","S","I","F","C","RGB","RGBZ","ZRGB","BGRZ","ZBGR",LASTFLAG},0,
	{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
    {"BGRZ",{"B","S","I","F","C","RGB","RGBZ","ZRGB","BGR","ZBGR",LASTFLAG},0,
	{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
    {"ZBGR",{"B","S","I","F","C","RGB","RGBZ","ZRGB","BGR","BGRZ",LASTFLAG},0,
	{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
    {"c",{LASTFLAG},1,{{PTBOOLEAN,"FALSE"},{PTINT,"-1","rows"},
	{PTINT,"-1","cols"},LASTPARAMETER}},
    {"f",{LASTFLAG},1,{{PTINT,"1","frames"},LASTPARAMETER}},
    {"d",{LASTFLAG},1,{{PTINT,"1","numdepth"},LASTPARAMETER}},
    {"nc",{LASTFLAG},1,{{PTINT,"1","numcolor"},LASTPARAMETER}},
    LASTFLAG};
void atob(),atos(),atoii(),atoff(),atoc(),atorgb(),atorgbz(),atozrgb();
void atobgr(),atobgrz(),atozbgr();

int main(argc,argv)

int argc;
char **argv;

{
	int oform;
	h_boolean Bflag,Sflag,Iflag,Fflag,Cflag,RGBflag,RGBZflag,ZRGBflag;
	h_boolean BGRflag,BGRZflag,ZBGRflag,cflag;
	int rows,cols,frames,numdepth,numcol;
	struct header hd;
	struct hips_roi roi;
	Filename filename;
	FILE *fp;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&Bflag,&Sflag,&Iflag,&Fflag,&Cflag,&RGBflag,
		&RGBZflag,&ZRGBflag,&BGRflag,&BGRZflag,&ZBGRflag,&cflag,
		&rows,&cols,&frames,&numdepth,&numcol,FFONE,&filename);
	fp = hfopenr(filename);
	if (Bflag)
		oform = PFBYTE;
	else if (Sflag)
		oform = PFSHORT;
	else if (Iflag)
		oform = PFINT;
	else if (Fflag)
		oform = PFFLOAT;
	else if (Cflag)
		oform = PFCOMPLEX;
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
	if (cflag && cols == -1)
		cols = rows;
	if (cflag) {
		init_header(&hd,"","",frames*numdepth*numcol,"",rows,cols,oform,
			numcol,"");
		if (numdepth != 1)
			hsetdepth(&hd,numdepth);
	}
	else {
		fread_header(fp,&hd,filename);
		if (hd.pixel_format != PFASCII)
			perr(HE_MSG,"pixel format must be Ascii");
		if (oform == PFCOMPLEX) {
			getroi(&hd,&roi);
			setsize(&hd,hd.rows,hd.cols/2);
			roi.cols /= 2;
			roi.fcol /= 2;
			setroi2(&hd,&roi);
		}
		else if (ptype_is_col3(oform)) {
			getroi(&hd,&roi);
			setsize(&hd,hd.rows,hd.cols/3);
			roi.cols /= 3;
			roi.fcol /= 3;
			setroi2(&hd,&roi);
		}
		setformat(&hd,oform);
	}
	write_headeru(&hd,argc,argv);
	alloc_image(&hd);
	switch(oform) {
	case PFBYTE:	atob(fp,&hd,filename); break;
	case PFSHORT:	atos(fp,&hd,filename); break;
	case PFINT:	atoii(fp,&hd,filename); break;
	case PFFLOAT:	atoff(fp,&hd,filename); break;
	case PFCOMPLEX:	atoc(fp,&hd,filename); break;
	case PFRGB:	atorgb(fp,&hd,filename); break;
	case PFRGBZ:	atorgbz(fp,&hd,filename); break;
	case PFZRGB:	atozrgb(fp,&hd,filename); break;
	case PFBGR:	atobgr(fp,&hd,filename); break;
	case PFBGRZ:	atobgrz(fp,&hd,filename); break;
	case PFZBGR:	atozbgr(fp,&hd,filename); break;
	}
	return(0);
}

void atob(fp,hd,filename)

FILE *fp;
struct header *hd;
Filename filename;

{
	int f,i,k;
	byte *op;

	for (f=0;f<hd->num_frame;f++) {
		op = hd->image;
		for (i=0;i<hd->numpix;i++) {
			if (fscanf(fp,"%d",&k) == EOF)
				perr(HE_READFR,f);
			*op++ = (k <= 0) ? 0 : ((k >= 255) ? 255 : k);
		}
		write_image(hd);
	}
}

void atos(fp,hd,filename)

FILE *fp;
struct header *hd;
Filename filename;

{
	int f,i,k;
	short *op;

	for (f=0;f<hd->num_frame;f++) {
		op = (short *) hd->image;
		for (i=0;i<hd->numpix;i++) {
			if (fscanf(fp,"%d",&k) == EOF)
				perr(HE_READFR,f);
			*op++ = k;
		}
		write_image(hd);
	}
}

void atoii(fp,hd,filename)

FILE *fp;
struct header *hd;
Filename filename;

{
	int f,i,k;
	int *op;

	for (f=0;f<hd->num_frame;f++) {
		op = (int *) hd->image;
		for (i=0;i<hd->numpix;i++) {
			if (fscanf(fp,"%d",&k) == EOF)
				perr(HE_READFR,f);
			*op++ = k;
		}
		write_image(hd);
	}
}

void atoff(fp,hd,filename)

FILE *fp;
struct header *hd;
Filename filename;

{
	int f,i;
	float *op,k;

	for (f=0;f<hd->num_frame;f++) {
		op = (float *) hd->image;
		for (i=0;i<hd->numpix;i++) {
			if (fscanf(fp,"%f",&k) == EOF)
				perr(HE_READFR,f);
			*op++ = k;
		}
		write_image(hd);
	}
}

void atoc(fp,hd,filename)

FILE *fp;
struct header *hd;
Filename filename;

{
	int f,i;
	float *op,k;

	for (f=0;f<hd->num_frame;f++) {
		op = (float *) hd->image;
		for (i=0;i<hd->numpix;i++) {
			if (fscanf(fp,"%f",&k) == EOF)
				perr(HE_READFR,f);
			*op++ = k;
			if (fscanf(fp,"%f",&k) == EOF)
				perr(HE_READFR,f);
			*op++ = k;
		}
		write_image(hd);
	}
}

void atorgb(fp,hd,filename)

FILE *fp;
struct header *hd;
Filename filename;

{
	int f,i,k1,k2,k3;
	byte *op;

	for (f=0;f<hd->num_frame;f++) {
		op = hd->image;
		for (i=0;i<hd->numpix;i++) {
			if (fscanf(fp,"%d %d %d",&k1,&k2,&k3) == EOF)
				perr(HE_READFR,f);
			*op++ = (k1 <= 0) ? 0 : ((k1 >= 255) ? 255 : k1);
			*op++ = (k2 <= 0) ? 0 : ((k2 >= 255) ? 255 : k2);
			*op++ = (k3 <= 0) ? 0 : ((k3 >= 255) ? 255 : k3);
		}
		write_image(hd);
	}
}

void atorgbz(fp,hd,filename)

FILE *fp;
struct header *hd;
Filename filename;

{
	int f,i,k1,k2,k3;
	byte *op;

	for (f=0;f<hd->num_frame;f++) {
		op = hd->image;
		for (i=0;i<hd->numpix;i++) {
			if (fscanf(fp,"%d %d %d",&k1,&k2,&k3) == EOF)
				perr(HE_READFR,f);
			*op++ = (k1 <= 0) ? 0 : ((k1 >= 255) ? 255 : k1);
			*op++ = (k2 <= 0) ? 0 : ((k2 >= 255) ? 255 : k2);
			*op++ = (k3 <= 0) ? 0 : ((k3 >= 255) ? 255 : k3);
			*op++ = 0;
		}
		write_image(hd);
	}
}

void atozrgb(fp,hd,filename)

FILE *fp;
struct header *hd;
Filename filename;

{
	int f,i,k1,k2,k3;
	byte *op;

	for (f=0;f<hd->num_frame;f++) {
		op = hd->image;
		for (i=0;i<hd->numpix;i++) {
			if (fscanf(fp,"%d %d %d",&k1,&k2,&k3) == EOF)
				perr(HE_READFR,f);
			*op++ = 0;
			*op++ = (k1 <= 0) ? 0 : ((k1 >= 255) ? 255 : k1);
			*op++ = (k2 <= 0) ? 0 : ((k2 >= 255) ? 255 : k2);
			*op++ = (k3 <= 0) ? 0 : ((k3 >= 255) ? 255 : k3);
		}
		write_image(hd);
	}
}

void atobgr(fp,hd,filename)

FILE *fp;
struct header *hd;
Filename filename;

{
	int f,i,k1,k2,k3;
	byte *op;

	for (f=0;f<hd->num_frame;f++) {
		op = hd->image;
		for (i=0;i<hd->numpix;i++) {
			if (fscanf(fp,"%d %d %d",&k1,&k2,&k3) == EOF)
				perr(HE_READFR,f);
			*op++ = (k3 <= 0) ? 0 : ((k3 >= 255) ? 255 : k3);
			*op++ = (k2 <= 0) ? 0 : ((k2 >= 255) ? 255 : k2);
			*op++ = (k1 <= 0) ? 0 : ((k1 >= 255) ? 255 : k1);
		}
		write_image(hd);
	}
}

void atobgrz(fp,hd,filename)

FILE *fp;
struct header *hd;
Filename filename;

{
	int f,i,k1,k2,k3;
	byte *op;

	for (f=0;f<hd->num_frame;f++) {
		op = hd->image;
		for (i=0;i<hd->numpix;i++) {
			if (fscanf(fp,"%d %d %d",&k1,&k2,&k3) == EOF)
				perr(HE_READFR,f);
			*op++ = (k3 <= 0) ? 0 : ((k3 >= 255) ? 255 : k3);
			*op++ = (k2 <= 0) ? 0 : ((k2 >= 255) ? 255 : k2);
			*op++ = (k1 <= 0) ? 0 : ((k1 >= 255) ? 255 : k1);
			*op++ = 0;
		}
		write_image(hd);
	}
}

void atozbgr(fp,hd,filename)

FILE *fp;
struct header *hd;
Filename filename;

{
	int f,i,k1,k2,k3;
	byte *op;

	for (f=0;f<hd->num_frame;f++) {
		op = hd->image;
		for (i=0;i<hd->numpix;i++) {
			if (fscanf(fp,"%d %d %d",&k1,&k2,&k3) == EOF)
				perr(HE_READFR,f);
			*op++ = 0;
			*op++ = (k3 <= 0) ? 0 : ((k3 >= 255) ? 255 : k3);
			*op++ = (k2 <= 0) ? 0 : ((k2 >= 255) ? 255 : k2);
			*op++ = (k1 <= 0) ? 0 : ((k1 >= 255) ? 255 : k1);
		}
		write_image(hd);
	}
}
