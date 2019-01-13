/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * pixto3d.c - convert a sequence in pixel-format to PLOT3D format
 *
 * usage: pixto3D [-v] [-f] [-z] [-s n] [-a n] [-g]  < in > out
 *
 * The output frame is positioned so that its (cols/2,rows/2) pixel is on the
 * (0,0,0) coordinate. Rows and columns in the header are set to 512x512. By
 * default, a pixel in position (i,j) with brightness b is represented by a
 * point at coordinate (i,j,b).  The whole thing is flattened (i.e. z 
 * coordinates are all zero) with the -f option.  The -v option uses vectors
 * from (i,j,0) to (i,j,b).  The -z option plots zero pixels, otherwise they
 * are discarded.  The -sn option spaces out the pixels by a distance "n",
 * which defaults to 1.  The -an option divides the z coordinate amplitude of
 * plotted pixels by "n".  The -g option plots points or vectors with the
 * pixel brightness of the pixel being represented.  Otherwise all
 * brightnesses are set to hips_hchar.
 *
 * to load: cc -o pixto3d pixto3d.c -lhips
 *
 * pixel formats handled directly: BYTE, FLOAT
 *
 * Yoav Cohen 2/16/82
 * modified by msl (suggestions by Tom Riedl) - 7/20/85
 * HIPS 2 - msl - 8/1/91
 */

#include <hipl_format.h>
#include <stdio.h>

static Flag_Format flagfmt[] = {
	{"v",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"f",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"z",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"g",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	{"s",{LASTFLAG},1,{{PTDOUBLE,"1","space"},LASTPARAMETER}},
	{"a",{LASTFLAG},1,{{PTDOUBLE,"1","amplitudescale"},LASTPARAMETER}},
	LASTFLAG};
int types[] = {PFBYTE,PFFLOAT,LASTTYPE};

int main(argc,argv)

int argc;
char **argv;

{
	int nbytes,ifr,nf,i,j,rows,cols,method,nex;
	double rot_m[3][3],sh_v[3];
	float *pffr,val,brt,z=0;
	double space,ampl;
	char buf[FBUFLIMIT];
	byte *pfr;
	struct header hd,hdp,hdo;
	Filename filename;
	FILE *fp;
	h_boolean vflag,fflag,zflag,gflag;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&vflag,&fflag,&zflag,&gflag,&space,&ampl,
		FFONE,&filename);
	brt = hips_hchar;
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	method = fset_conversion(&hd,&hdp,types,filename);
	dup_headern(&hdp,&hdo);
	setformat(&hdo,PLOT3D);
	setsize(&hdo,512,512);
	write_headeru(&hdo,argc,argv);
	nf=hdp.num_frame;
	rows=hd.rows; cols=hd.cols;

	sh_v[0] = -cols*space/2;
	sh_v[1] = -rows*space/2;
	sh_v[2] = 0.0;
	for(i=0;i<3;i++)
		for(j=0;j<3;j++)
			rot_m[i][j] = (i==j) ? 1.0 : 0.0;

	nex = hd.ocols - hd.cols;
	for(ifr=0;ifr<nf;ifr++) {
		fread_imagec(fp,&hd,&hdp,method,ifr,filename);
		nbytes=0;
		switch(hdp.pixel_format) {
		case PFBYTE:	
			pfr = hdp.firstpix;
			for(i=0;i<rows;i++) {
			    for(j=0;j<cols;j++) {
				val = *pfr++;
				if (val==0 && !zflag)
					continue;
				if (!fflag)
					z = val/ampl;
				if (gflag)
					brt = val;
				if (vflag)
				    nbytes = addvec(buf,nbytes,FBUFLIMIT,
					(double)brt,(double)(j*space),
					(double)(i*space),0.0,
					(double)(j*space),(double)(i*space),z);
				else
				    nbytes = addpoint(buf,nbytes,FBUFLIMIT,
					(double)brt,(double)(j*space),
					(double)(i*space),z);
			    }
			    pfr += nex;
			}
			break;
		case PFFLOAT:	
			pffr = (float *) hdp.firstpix;
			for(i=0;i<rows;i++) {
			    for(j=0;j<cols;j++) {
				val = *pffr++;
				if (val==0 && !zflag)
					continue;
				if (!fflag)
					z = val/ampl;
				if (gflag)
					brt = val;
				if (vflag)
				    nbytes = addvec(buf,nbytes,FBUFLIMIT,
					(double)brt,(double)(j*space),
					(double)(i*space),0.0,
					(double)(j*space),(double)(i*space),z);
				else
				    nbytes = addpoint(buf,nbytes,FBUFLIMIT,
					(double)brt,(double)(j*space),
					(double)(i*space),z);
			    }
			    pffr += nex;
			}
			break;
		default:
			perr(HE_MSG,"unrecognized format");
		}
		write_frame(stdout,buf,nbytes,sh_v,rot_m,ifr);
	}
	return(0);
}
