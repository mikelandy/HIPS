/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * gridwarp - warp an image using a control grid
 *
 * usage:	gridwarp [-c controlfile] [-s nr [nc]] [-w] <iseq >oseq
 *
 * gridwarp warps an input file spatially based on a grid of control points.
 * The algorithm (based on the discussion in Castleman, Digital Image
 * Processing, Prentice-Hall) is based on a pixel fill algorithm.  The control
 * grid points are a rectangular grid in the output image, and the user
 * specifies both this grid and the spatial locations of the points in the
 * input image which map to these grid locations in the output image.  For
 * each output image pixel, the program determines in which control grid
 * rectangle the pixel lies.  Pixels lying outside any control grid rectangle
 * utilize the mapping of the nearest control grid rectangle.  Then, it
 * determines the corresponding input image pixel location by bilinearly
 * interpolating between the four points corresponding to the four corners of
 * the rectangle.  Finally, using the four nearest neighbors to that input
 * pixel position, the output grey value is determined using bilinear
 * interpolation.
 *
 * The program assumes that pixels are points located on a rectangular grid.
 * For a coordinate system it assumes that the first pixel in an image is
 * located at position (0,0), and the last stored pixel in an image (or ROI) is
 * located at position (1,1).
 *
 * The control file (default file name is `controlfile') is formatted as
 * follows:
 *
 * firstx firsty	the first control grid point location in the
 *			output image
 * nx ny		the grid size
 * dx dy		the distance between successive grid points
 * xi1       yi1	the first input image control grid position
 *       .
 *       .
 *       .
 * xi(nx*ny) yi(nx*ny)	the last input image control grid position
 *
 * The control grid positions start with that corresponding to (firstx,firsty)
 * and proceed in column-first order (i.e. the 2nd one corresponds to
 * (firstx+dx,firsty).  The user may also specify the size of the output
 * images.  If -s is not specified, the output image size is the same as the
 * input image size.  If -s is specified but nc is not, nc defaults to be the
 * same as nr.
 *
 * By default the input image used is the region-of-interest of the input
 * image.  The -w flag specifies that the entire input image should be used.
 * The output image contains only the warped image or subimage, with ROI
 * set to be the entire image.
 *
 * pixel formats handled directly: BYTE
 *
 * to load:	cc -o gridwarp gridwarp.c -lhipsh -lhips -lm
 *
 * Mike Landy - 5/27/93
 */

#include <stdio.h>
#include <hipl_format.h>

static Flag_Format flagfmt[] = {
	{"c",{LASTFLAG},1,{{PTFILENAME,"controlfile","controlfile"},
		LASTPARAMETER}},
	{"s",{LASTFLAG},1,{{PTINT,"-1","rows"},{PTINT,"-1","cols"},
		LASTPARAMETER}},
	{"w",{LASTFLAG},0,{{PTBOOLEAN,"FALSE"},LASTPARAMETER}},
	LASTFLAG};
int types[] = {PFBYTE,LASTTYPE};

int main(argc,argv)

int argc;
char **argv;

{
	struct header hd,hdp,hdo,hdcb;
	int method,fr,f,nro,nco,i,j,nx,ny;
	float *igridx,*igridy,*ogridx,*ogridy,fx,fy,dx,dy;
	Filename filename,ctrlfile;
	FILE *fp,*fpc;
	h_boolean wflag;

	Progname = strsave(*argv);
	parseargs(argc,argv,flagfmt,&ctrlfile,&nro,&nco,&wflag,FFONE,&filename);
	if (!ctrlfile[0])
		perr(HE_MSG,"the controlfile must be specified with -c");
	fp = hfopenr(filename);
	fread_hdr_a(fp,&hd,filename);
	if (wflag)
		clearroi(&hd);
	method = fset_conversion(&hd,&hdp,types,filename);
	if (nro < 0) {
		nro = hdp.rows;
		nco = hdp.cols;
	}
	else if (nco < 0)
		nco = nro;
	dup_headern(&hdp,&hdo);
	setsize(&hdo,nro,nco);
	alloc_image(&hdo);
	setupconvback(&hd,&hdo,&hdcb);
	fpc = ffopen(ctrlfile,"r");
	if (fscanf(fpc,"%f %f %d %d %f %f",&fx,&fy,&nx,&ny,&dx,&dy) != 6)
		perr(HE_READFILE,ctrlfile);
	igridx = (float *) halloc(nx*ny,sizeof(float));
	igridy = (float *) halloc(nx*ny,sizeof(float));
	ogridx = (float *) halloc(nx,sizeof(float));
	ogridy = (float *) halloc(ny,sizeof(float));
	for (i=0;i<ny;i++)
		for (j=0;j<nx;j++)
			if (fscanf(fpc,"%f %f",igridx+i*nx+j,igridy+i*nx+j)
			    != 2)
				perr(HE_READFILE,ctrlfile);
	fclose(fpc);
	for (i=0;i<ny;i++)
		ogridy[i] = fy + dy*i;
	for (i=0;i<nx;i++)
		ogridx[i] = fx + dx*i;
	write_headeru2(&hdcb,&hdo,argc,argv,hips_convback);
	fr = hdp.num_frame;
	for (f=0;f<fr;f++) {
		fread_imagec(fp,&hd,&hdp,method,f,filename);
		h_gridwarp(&hdp,&hdo,nx,ny,ogridx,ogridy,igridx,igridy);
		write_imagec(&hdcb,&hdo,method,hips_convback,f);
	}
	return(0);
}
