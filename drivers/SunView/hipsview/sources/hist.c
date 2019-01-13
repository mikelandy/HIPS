#include <hipl_format.h>
#include "hipsview.h"
#include <sunwindow/window_hs.h>

static int L_wd, L_ht;

gethist(hist, image, x0, y0, wd, ht, nx, ny)
int  hist[];
unsigned char image[];
register int  wd, ny;
{			/* form intensity histograms (8 bit pixels) */
	register int		i;
	register int		*hP,  *hPmax;
	register unsigned char	*imP, *imPmax, *imP0;

	L_wd = wd; L_ht = ht;
	hPmax = &hist[255];
	for(hP= &hist[0]; hP<=hPmax; )	 /* clear hist */
		*hP++ = 0;

	imP0   = &image[x0+y0*nx];	/* compute histogram */
	imPmax = imP0 + ht*nx;
	while(--ht>=0){
		if(imP0>imPmax)
			break;
		for(imP=imP0, i=wd; --i>=0; )
			hist[*imP++]++;
		imP0 += nx;
	}
}

static float	ifs;		/* ifs = 254. / (float)ihist[255]; */
	
/* generate ihist for equalization with optimal intensity range utilization */
/* see O'Gorman comput. Graphics Imasge Process.  41,229-232(1988)	*/
/* reserve 0 & 255 for fore/background color control			*/
getihist(ihist, hist)
int  ihist[], hist[];
{
	register int   sum, it, *hP, *ihP, *ihPmax;

	ihPmax = &ihist[255];
					/* integrate hist */
	sum = -hist[0]/2;
	for(ihP= &ihist[0], hP= &hist[0];  ihP<=ihPmax;  ihP++, hP++){
		 sum += *hP;
		*ihP  = sum;
	}
	ihist[255] -= hist[255]/2;
					/* normalize ihist */
	ifs = 254. / (float)ihist[255];
	for(ihP= &ihist[0];  ihP<=ihPmax;  ihP++){
		it = *ihP * ifs;
		if(it<=0) it = 1;
		*ihP = it;
	}

	if( (sum+hist[0]/2) != (L_wd*L_ht) )
		printf("err in histogram sum = %d\n", sum); /* debug */
}
	
hist_eq(ihist, eq_image, raw_image, size)
int  ihist[];
unsigned char eq_image[], raw_image[];
{			 /* histogram equalize  (8 bit per pixel) image */
	register unsigned char *eiP, *riP, *riPmax;

	riPmax = &raw_image[size];
	for(eiP= &eq_image[0], riP= &raw_image[0];  riP<riPmax; )
		*eiP++ = ihist[*riP++];
}

makehistplot(mode, pic_type, mprP)
struct pixrect *mprP;
{
	register int 	ix, iy, peak, *hP, *hPmax;
	register float	fpeak;
	float	frange, foffset, val0, val1;
	int	 ic0, ic1, cnt0, cnt1, ipeak, percentofpeak;
	int	*l_hist;
	extern ht_lothresh, ht_hithresh, ht_markx;
 
	if(mode=='e')	   l_hist = ehist;
	else if(mode=='i') l_hist = ihist;
	else		   l_hist =  hist;
	setmpr(mprP, 0);
	peak = 0;
	hPmax = &l_hist[255];
	for(hP= &l_hist[0]; hP<= hPmax; hP++){
		if(*hP>peak) peak = *hP;
	}

	ix = 0;
	fpeak = (float)peak;
	for(hP= &l_hist[0]; hP<= hPmax; hP++){
		iy = 100. * *hP / fpeak + 0.5;
		if(histplot_mode=='i')
			pr_vector(mprP, ix, ZBASE-iy-1, ix, ZBASE-iy, PIX_SET, 1);
		else
			pr_vector(mprP, ix, ZBASE-iy-1, ix, ZBASE, PIX_SET, 1);
		ix++;
	}

	if(mode!='i')
		sprintf(plottext1, "%chist peak=%d win=(%d %d)\n",
					mode, peak, L_wd, L_ht);
	else{
		ipeak = peak / ifs;
		sprintf(plottext1, "ihist total cnt=%d win=(%d %d)\n",
					ipeak, L_wd, L_ht);
	}

	if(pic_type=='h')
		ic0 = ht_markx;			/* form hist plot cursor */
	else
		ic0 = ht_lothresh;		/* form hist plot cursor */
	if(ic0<0) ic0 = 0;
	if(ic0>=256) ic0 = 255;
	iy = 100. * l_hist[ic0] / fpeak + 0.5;

	if(histplot_mode=='i')
		pr_vector(mprP, ic0, ZBASE-iy, ic0, ZBASE+4, PIX_SET, 1);
	else{
		pr_vector(mprP, ic0, ZBASE-iy+3, ic0, ZBASE+4, PIX_SRC^PIX_DST, 1);
		pr_vector(mprP, ic0, ZBASE, ic0, ZBASE+4, PIX_SET, 1);
	}

	frange = rawtopix8scale();
	foffset = rawtopix8offset();
	if(pic_type=='t'){
		ic1 = ht_hithresh;
		if(ic1<0) ic1 = 0;
		if(ic1>=256) ic1 = 255;
		iy = 100. * l_hist[ic1] / fpeak + 0.5;
		pr_vector(mprP, ic1, ZBASE-iy, ic1, ZBASE+4, PIX_SET, 1);
		cnt0 = l_hist[ic0];   cnt1 = l_hist[ic1];
		if(mode=='i'){
			cnt0 /= ifs;   cnt1 /= ifs;
		}
		if(hdtype==PFBYTE){
			sprintf(plottext2, " bin:cnt %d:%d to %d:%d",
				ic0, cnt0, ic1, cnt1);
		}else{
			val0    = ic0*frange + foffset;
			val1    = ic1*frange + foffset;
			sprintf(plottext2, " bin:cnt %d<%.2f>:%d to %d<%.2f>:%d"
			     ,ic0, val0, cnt0, ic1, val1, cnt1);
		}
	}else{
		if(mode=='i'){
			if(hdtype==PFBYTE){
				sprintf(plottext2, " bin=%d ihpix=%d",
					ic0, ihist[ic0]);
			}else{
				val0    = ic0*frange + foffset;
				sprintf(plottext2," bin=%d<%.2f> ihpix=%d",
					ic0, val0, ihist[ic0]);
			}
		}else{
			cnt0 = l_hist[ic0];
			if(hdtype==PFBYTE){
				sprintf(plottext2, " bin=%d cnt=%d", ic0, cnt0);
			}else{
				val0    = ic0*frange + foffset;
				sprintf(plottext2," bin=%d<%.2f> cnt=%d",
					ic0, val0, cnt0);
			}
		}
	}
 
	plot_cursor_wx = ic0;
	plot_cursor_wy = win_ht - PLOT_NY + ZBASE - iy;

	pr_vector(mprP, 0, ZBASE, ix, ZBASE, PIX_SET, 1);/* zero base line */
}
