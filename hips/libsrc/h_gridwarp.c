/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 */

/*
 * h_gridwarp.c - subroutines to warp an image using a control grid
 *
 * h_gridwarp warps an input ROI spatially based on a grid of control points.
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
 * located at position (0,0), and the last stored pixel in an image is
 * located at position (1,1).
 *
 * Pixels outside of the input image are treated as if their values are
 * equal to hips_lchar.
 *
 * pixel formats: BYTE
 *
 * Michael Landy - 5/28/93
 */

#include <hipl_format.h>

int truncup(),truncdown();

int h_gridwarp(hdi,hdo,nx,ny,ogridx,ogridy,igridx,igridy)

struct header *hdi,*hdo;
int nx,ny;
float *ogridx,*ogridy,*igridx,*igridy;

{
	switch(hdi->pixel_format) {
	case PFBYTE:	return(h_gridwarp_b(hdi,hdo,nx,ny,ogridx,ogridy,igridx,
				igridy));
	default:	return(perr(HE_FMTSUBR,"h_gridwarp",
				hformatname(hdi->pixel_format)));
	}
}

int h_gridwarp_b(hdi,hdo,nx,ny,ogridx,ogridy,igridx,igridy)

struct header *hdi,*hdo;
int nx,ny;
float *ogridx,*ogridy,*igridx,*igridy;

{
	return(h_gridwarp_B(hdi->firstpix,hdo->firstpix,hdi->rows,hdi->cols,
		hdi->ocols,hdo->rows,hdo->cols,hdo->ocols,nx,ny,ogridx,ogridy,
		igridx,igridy));
}

#define interp(ul,ur,ll,lr,dx,dy) \
{ \
	topg = (ul) + dx*(((int) (ur))-((int) (ul))); \
	botg = (ll) + dx*(((int) (lr))-((int) (ll))); \
	*po++ = topg + dy*(botg-topg) + .5; \
}

int h_gridwarp_B(imagei,imageo,nri,nci,nlpi,nro,nco,nlpo,nx,ny,ogridx,ogridy,
	igridx,igridy)

byte *imagei,*imageo;
int nri,nci,nlpi,nro,nco,nlpo,nx,ny;
float *ogridx,*ogridy,*igridx,*igridy;

{
	int boxr,boxc,firstr,lastr,firstc,lastc,r,c,nro1,nco1,nx1,ny1,nexo;
	int ulb,urb,llb,lrb,ulr,ulc,nci1,nri1;
	float dx,dy,dxo,xol,yo,xil,yil,xil1,yil1,dxi,dyi;
	float xitopl,yitopl,xitopl1,yitopl1,xibotl,yibotl,xibotl1,yibotl1;
	float xor,xitopr,yitopr,xibotr,yibotr,yot,yob,xiul,yiul,xiur,yiur;
	float xill,yill,xilr,yilr,xir,yir,xi,yi,topg,botg;
	byte *pi,*po;

	nx1 = nx-1;
	ny1 = ny-1;
	nro1 = nro-1;
	nco1 = nco-1;
	nci1 = nci-1;
	nri1 = nri-1;
	dxo = 1./nco1;
	for (boxr=0;boxr<ny1;boxr++) {
	    if (boxr == 0)
		firstr = 0;
	    else
		firstr = truncup(nro1*ogridy[boxr]);
	    if (boxr == (ny1-1))
		lastr = nro1;
	    else
		lastr = truncdown(nro1*ogridy[boxr+1]);
	    for (boxc=0;boxc<nx1;boxc++) {
		if (boxc == 0)
		    firstc = 0;
		else
		    firstc = truncup(nco1*ogridx[boxc]);
		if (boxc == (nx1-1))
		    lastc = nco1;
		else
		    lastc = truncdown(nco1*ogridx[boxc+1]);
		po = imageo + firstr*nlpo + firstc;
		nexo = nlpo - (lastc+1-firstc);
		ulb = boxr*nx+boxc;
		urb = ulb + 1;
		llb = ulb + nx;
		lrb = llb + 1;

		xol = ((float) firstc)/nco1;
		xor = ((float) lastc)/nco1;
		yot = ((float) firstr)/nro1;
		yob = ((float) lastr)/nro1;

		xitopl = igridx[ulb] + (igridx[urb]-igridx[ulb]) *
			(xol - ogridx[boxc]) / (ogridx[boxc+1] - ogridx[boxc]);
		yitopl = igridy[ulb] + (igridy[urb]-igridy[ulb]) *
			(xol - ogridx[boxc]) / (ogridx[boxc+1] - ogridx[boxc]);
		xitopl1 = igridx[ulb] + (igridx[urb]-igridx[ulb]) *
			(xol + dxo - ogridx[boxc]) /
			(ogridx[boxc+1] - ogridx[boxc]);
		yitopl1 = igridy[ulb] + (igridy[urb]-igridy[ulb]) *
			(xol + dxo - ogridx[boxc]) /
			(ogridx[boxc+1] - ogridx[boxc]);
		xibotl = igridx[llb] + (igridx[lrb]-igridx[llb]) *
			(xol - ogridx[boxc]) / (ogridx[boxc+1] - ogridx[boxc]);
		yibotl = igridy[llb] + (igridy[lrb]-igridy[llb]) *
			(xol - ogridx[boxc]) / (ogridx[boxc+1] - ogridx[boxc]);
		xibotl1 = igridx[llb] + (igridx[lrb]-igridx[llb]) *
			(xol + dxo - ogridx[boxc]) /
			(ogridx[boxc+1] - ogridx[boxc]);
		yibotl1 = igridy[llb] + (igridy[lrb]-igridy[llb]) *
			(xol + dxo - ogridx[boxc]) /
			(ogridx[boxc+1] - ogridx[boxc]);
		xitopr = igridx[ulb] + (igridx[urb]-igridx[ulb]) *
			(xor - ogridx[boxc]) / (ogridx[boxc+1] - ogridx[boxc]);
		yitopr = igridy[ulb] + (igridy[urb]-igridy[ulb]) *
			(xor - ogridx[boxc]) / (ogridx[boxc+1] - ogridx[boxc]);
		xibotr = igridx[llb] + (igridx[lrb]-igridx[llb]) *
			(xor - ogridx[boxc]) / (ogridx[boxc+1] - ogridx[boxc]);
		yibotr = igridy[llb] + (igridy[lrb]-igridy[llb]) *
			(xor - ogridx[boxc]) / (ogridx[boxc+1] - ogridx[boxc]);

		xiul = xitopl + (xibotl-xitopl) *
			(yot - ogridy[boxr]) / (ogridy[boxr+1] - ogridy[boxr]);
		yiul = yitopl + (yibotl-yitopl) *
			(yot - ogridy[boxr]) / (ogridy[boxr+1] - ogridy[boxr]);
		xiur = xitopr + (xibotr-xitopr) *
			(yot - ogridy[boxr]) / (ogridy[boxr+1] - ogridy[boxr]);
		yiur = yitopr + (yibotr-yitopr) *
			(yot - ogridy[boxr]) / (ogridy[boxr+1] - ogridy[boxr]);
		xill = xitopl + (xibotl-xitopl) *
			(yob - ogridy[boxr]) / (ogridy[boxr+1] - ogridy[boxr]);
		yill = yitopl + (yibotl-yitopl) *
			(yob - ogridy[boxr]) / (ogridy[boxr+1] - ogridy[boxr]);
		xilr = xitopr + (xibotr-xitopr) *
			(yob - ogridy[boxr]) / (ogridy[boxr+1] - ogridy[boxr]);
		yilr = yitopr + (yibotr-yitopr) *
			(yob - ogridy[boxr]) / (ogridy[boxr+1] - ogridy[boxr]);

		if (xiul >= 0. && xiul <= 1. &&
		    yiul >= 0. && yiul <= 1. &&
		    xiur >= 0. && xiur <= 1. &&
		    yiur >= 0. && yiur <= 1. &&
		    xill >= 0. && xill <= 1. &&
		    yill >= 0. && yill <= 1. &&
		    xilr >= 0. && xilr <= 1. &&
		    yilr >= 0. && yilr <= 1.) {
		    for (r=firstr;r<=lastr;r++) {
			yo = ((float) r)/nro1;
			xil = xitopl + (xibotl-xitopl) * (yo - ogridy[boxr]) /
			    (ogridy[boxr+1] - ogridy[boxr]);
			yil = yitopl + (yibotl-yitopl) * (yo - ogridy[boxr]) /
			    (ogridy[boxr+1] - ogridy[boxr]);
			xil1 = xitopl1 + (xibotl1-xitopl1) *
			    (yo - ogridy[boxr]) /
			    (ogridy[boxr+1] - ogridy[boxr]);
			yil1 = yitopl1 + (yibotl1-yitopl1) *
			    (yo - ogridy[boxr]) /
			    (ogridy[boxr+1] - ogridy[boxr]);
			dxi = xil1 - xil;
			dyi = yil1 - yil;
			xi = xil*nci1;
			yi = yil*nri1;
			dxi *= nci1;
			dyi *= nri1;
			for(c=firstc;c<=lastc;c++) {
			    ulr = truncdown(yi);
			    ulc = truncdown(xi);
			    dy = yi-ulr;
			    dx = xi-ulc;
			    pi = imagei + ulr*nlpi + ulc;
			    interp(*pi,*(pi+1),*(pi+nlpi),*(pi+nlpi+1),dx,dy)
			    xi += dxi;
			    yi += dyi;
			}
			po += nexo;
		    }
		}
		else {
		  for (r=firstr;r<=lastr;r++) {
		    yo = ((float) r)/nro1;
		    xil = xitopl + (xibotl-xitopl) *
			(yo - ogridy[boxr]) / (ogridy[boxr+1] - ogridy[boxr]);
		    yil = yitopl + (yibotl-yitopl) *
			(yo - ogridy[boxr]) / (ogridy[boxr+1] - ogridy[boxr]);
		    xil1 = xitopl1 + (xibotl1-xitopl1) *
			(yo - ogridy[boxr]) / (ogridy[boxr+1] - ogridy[boxr]);
		    yil1 = yitopl1 + (yibotl1-yitopl1) *
			(yo - ogridy[boxr]) / (ogridy[boxr+1] - ogridy[boxr]);
		    xir = xitopr + (xibotr-xitopr) *
			(yo - ogridy[boxr]) / (ogridy[boxr+1] - ogridy[boxr]);
		    yir = yitopr + (yibotr-yitopr) *
			(yo - ogridy[boxr]) / (ogridy[boxr+1] - ogridy[boxr]);
		    dxi = xil1 - xil;
		    dyi = yil1 - yil;
		    xi = xil*nci1;
		    yi = yil*nri1;
		    dxi *= nci1;
		    dyi *= nri1;
		    if (xil >= 0. && xil <= 1. &&
			yil >= 0. && yil <= 1. &&
			xir >= 0. && xir <= 1. &&
			yir >= 0. && yir <= 1.) {
			for(c=firstc;c<=lastc;c++) {
			    ulr = truncdown(yi);
			    ulc = truncdown(xi);
			    dy = yi-ulr;
			    dx = xi-ulc;
			    pi = imagei + ulr*nlpi + ulc;
			    interp(*pi,*(pi+1),*(pi+nlpi),*(pi+nlpi+1),dx,dy)
			    xi += dxi;
			    yi += dyi;
			}
		    }
		    else {
			for(c=firstc;c<=lastc;c++) {
			    ulr = truncdown(yi);
			    ulc = truncdown(xi);
			    dy = yi-ulr;
			    dx = xi-ulc;
			    pi = imagei + ulr*nlpi + ulc;
			    if (ulr < 0) {
				if (ulr < -1)
				    *po++ = hips_lchar;
				else {
				    if (ulc < 0) {
					if (ulc < -1)
					    *po++ = hips_lchar;
					else
					    interp(hips_lchar,hips_lchar,
						hips_lchar,*(pi+nlpi+1),dx,dy)
				    }
				    else if (ulc >= nci1) {
					if (ulc == nci1)
					    interp(hips_lchar,hips_lchar,
						*(pi+nlpi),hips_lchar,dx,dy)
					else
					    *po++ = hips_lchar;
				    }
				    else
					interp(hips_lchar,hips_lchar,
					    *(pi+nlpi),*(pi+nlpi+1),dx,dy)
				}
			    }
			    else if (ulr >= nri1) {
				if (ulr == nri1) {
				    if (ulc < 0) {
					if (ulc < -1)
					    *po++ = hips_lchar;
					else
					    interp(hips_lchar,*(pi+1),
						hips_lchar,hips_lchar,dx,dy)
				    }
				    else if (ulc >= nci1) {
					if (ulc == nci1)
					    interp(*pi,hips_lchar,hips_lchar,
						hips_lchar,dx,dy)
					else
					    *po++ = hips_lchar;
				    }
				    else
					interp(*pi,*(pi+1),hips_lchar,
					    hips_lchar,dx,dy)
				}
				else
				    *po++ = hips_lchar;
			    }
			    else {
				if (ulc < 0) {
				    if (ulc < -1)
					*po++ = hips_lchar;
				    else
					interp(hips_lchar,*(pi+1),hips_lchar,
					    *(pi+nlpi+1),dx,dy)
				}
				else if (ulc >= nci1) {
				    if (ulc == nci1)
					interp(*pi,hips_lchar,*(pi+nlpi),
					    hips_lchar,dx,dy)
				    else
					*po++ = hips_lchar;
				}
				else
				    interp(*pi,*(pi+1),*(pi+nlpi),*(pi+nlpi+1),
					dx,dy)
			    }
			    xi += dxi;
			    yi += dyi;
			}
		    }
		    po += nexo;
		  }
		}
	    }
	}
	return(HIPS_OK);
}

int truncup(x)

float x;

{
	int i;

	i = x;
	if (i == x)
		return(i);
	else if (x < 0) {
		i = -x;
		return(-i);
	}
	else
		return(i+1);
}

int truncdown(x)

float x;

{
	int i;

	if (x < 0) {
		i = -x;
		return(-i-1);
	}
	else {
		i = x;
		return(i);
	}
}
