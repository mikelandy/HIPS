#include <hipl_format.h>
#include "hipsview.h"
#include <stdio.h>
#include <sunwindow/window_hs.h>

makesliceplot(slice, size, markx, marky, mprP)
unsigned char slice[];
struct pixrect *mprP;
{
	register int 	ix, iy;
	register unsigned char *sp;
	register int	ic, peak, pix;
	int dx, dy, absdx, absdy, ndx, ndy;
	float	fpeak8, frawpix, f_ndx, f_ndy, f_iy;
	int	pixtype;
	int	xScaleIFm(), yScaleIFm();

	peak = 0;
	for(sp= &slice[0]; sp<= &slice[size]; sp++)
		if(*sp>peak) peak = *sp;

	setmpr(mprP, 0);
	ix = 0;
	fpeak8 = (float)peak;
	for(sp= &slice[0]; sp< &slice[size]; sp++){
		iy = 100. * *sp / fpeak8 + 0.5;
		pr_vector(mprP, ix, ZBASE-iy-1, ix, ZBASE-iy, PIX_SET, 1);
		ix++;
	}

	switch(showsw){			/* slice plot cursor */
	case 'x':
		ic = markx;
		break;
	case 'y':
		if(markx>=nv_lim) markx = nv_lim-1;
		ic = (marky<nv_lim) ? marky : markx;
		break;
	case 'v':
		dx = slx2 - slx1;
		dy = sly2 - sly1;
		if(dx==0) dx=1;
		if(dy==0) dy=1;
		absdx = (dx>=0) ? dx : -dx;
		absdy = (dy>=0) ? dy : -dy;
		if(absdx>=absdy){
			if(marky<nv_lim){
				ic = markx - slx1;
				if(dx<0) ic = -ic;
				vmark_wx = markx;
			}else{
				ic = (markx>=absdx) ? absdx : markx;
				vmark_wx  = slx1;
				vmark_wx += (dx>=0) ? ic : -ic;
			}
			if(ic<0) ic = 0;
			if(ic>=size) ic = size-1;

			f_ndy = (dy*(float)ic)/(float)dx;
			f_ndy += (f_ndy>=0) ? 0.5 : -0.5;
			ndy = f_ndy;
			vmark_wy  = sly1;
			vmark_wy += (dx>=0)?ndy:-ndy;
		}else{
			if(marky<nv_lim){
				ic = marky - sly1;
				if(dy<0) ic = -ic;
				vmark_wy = marky;
			}else{
				ic = (markx>=absdy) ? absdy : markx;
				vmark_wy  = sly1;
				vmark_wy += (dy>=0)?ic:-ic;
			}
			if(ic<0) ic = 0;
			if(ic>=size) ic = size-1;

			f_ndx = (dx*(float)ic)/(float)dy;
			f_ndx += (f_ndx>=0) ? 0.5 : -0.5;
			ndx = f_ndx;
			vmark_wx  = slx1;
			vmark_wx += (dy>=0)?ndx:-ndx;
		}
/*
printf("ic=%d dxy(%d,%d) ndxy(%d,%d) vmark_wxy(%d,%d) markxy(%d,%d)\n",
		ic, dx, dy, ndx, ndy, vmark_wx, vmark_wy, markx, marky);
fflush(stdout);
*//*debug*/
		break;
	default:
		return;
	}

		/* make plot cursor */
	pix = slice[ic];
	f_iy = 100. * pix / fpeak8;
	f_iy += (f_iy>=0) ? 0.5 : -0.5;
	iy = f_iy;

	pr_vector(mprP, ic, ZBASE-iy, ic, ZBASE+4, PIX_SET, 1);

	plot_cursor_wx = ic;
	plot_cursor_wy = win_ht - PLOT_NY + ZBASE - iy;
				/* zero base line */
	pr_vector(mprP, 0, ZBASE, ix, ZBASE, PIX_SET, 1);

	plot_cursor_wx = ic;
	switch(showsw){
	case 'x':
		ix = ic, iy = sly1;
		sprintf(plottext2, "X_slice");
		goto L_plot2;
	case 'y':
		ix = slx1, iy = ic;
		sprintf(plottext2, "Y_slice");
		goto L_plot2;
	case 'v':
		ix = vmark_wx; iy = vmark_wy;
		{ int sx1 = xScaleIFm(slx1);
		  int sy1 = yScaleIFm(sly1);
		  sprintf(plottext2, "Vslice xy0[%d,%d]to duv[%d,%d]",
			sx1, sy1, xScaleIFm(slx2)-sx1, yScaleIFm(sly2)-sy1 );
		}
           L_plot2:
		pixtype = getrawfpix(ix, iy, &frawpix);
		if(hdtype==PFBYTE && pic_base_type!='e'){
			sprintf(plottext1, "pix[%3d,%3d]= %3d",
				xScaleIFm(ix), yScaleIFm(iy), pix );
		}else{
			sprintf(plottext1, "pix[%3d,%3d]= %3d%c<%.2f>",
				xScaleIFm(ix), yScaleIFm(iy), pix,
				pixtype, frawpix );
		}
		break;
	}
}

int
onvec_wx(wx)
register int wx;
{
	if(slx1<slx2){
		if(wx<slx1) wx = slx1;
		else if(wx>slx2) wx = slx2;
	}else{
		if(wx<slx2) wx = slx2;
		else if(wx>slx1) wx = slx1;
	}
	return(wx);	
}

int
onvec_wy(wy)
register int wy;
{
	if(sly1<sly2){
		if(wy<sly1) wy = sly1;
		else if(wy>sly2) wy = sly2;
	}else{
		if(wy<sly2) wy = sly2;
		else if(wy>sly1) wy = sly1;
	}
	return(wy);	
}
