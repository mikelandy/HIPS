/* histogram handling routines */

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <xdisp.h>


/*****************************
 * create_hists()
 *****************************/

void create_hists()
{
  int i,j;
  int fr;
  int max;
  unsigned char *im;

    for (fr = 0; fr < nframes; fr++) {

        /*
         * Form the histogram...
         */

	im = images[fr];

        for (i = 0; i < nrows; i++)
	    for (j = 0; j < ncols; j++) {
	        hists[fr][*im]++;
	        cdfs[fr][*im++]++;
		}

	max = 0;

        for (i = 0; i < HIST_WIDTH; i++)
	    max = hists[fr][i] > max ? hists[fr][i] : max;

        /*
         * ...and plot it in the drawing area
         */

    	hist_pixmaps[fr] = XCreatePixmap(dpy,rws,HIST_WIDTH,HIST_HEIGHT,depth);

        for (i = 0; i < 4; i++) {
    	    hist_pix_data[i].width = hist_pix_data[i].max_width = HIST_WIDTH;
	    hist_pix_data[i].height = hist_pix_data[i].max_height = HIST_HEIGHT;
    	    hist_pix_data[i].pix = hist_pixmaps[0];
    	    hist_pix_data[i].gc = gc;
    	    hist_pix_data[i].pix_cache = NULL;
	    }

	XSetForeground(dpy,gc,black_pixel);
	XFillRectangle(dpy,hist_pixmaps[fr],gc,0,0,HIST_WIDTH,HIST_HEIGHT);

	XSetForeground(dpy,gc,white_pixel);

        for (i = 0; i < HIST_WIDTH; i++) 
	    if (j = (hists[fr][i]*HIST_HEIGHT)/max)
		XDrawLine(dpy,hist_pixmaps[fr],
				gc,i,HIST_HEIGHT,i,HIST_HEIGHT-j);

        /*
         * Form cdf...
         */

    	for (i = 1; i < HIST_WIDTH; i++)
	    cdfs[fr][i]+= cdfs[fr][i-1];

    	max = cdfs[fr][HIST_WIDTH-1];

        /*
         * ...and plot it in the drawing area
         */

    	cdf_pixmaps[fr] = XCreatePixmap(dpy,rws,HIST_WIDTH,HIST_HEIGHT,depth);

	XSetForeground(dpy,gc,black_pixel);
	XFillRectangle(dpy,cdf_pixmaps[fr],gc,0,0,HIST_WIDTH,HIST_HEIGHT);

	XSetForeground(dpy,gc,white_pixel);

        for (i = 0; i < HIST_WIDTH; i++) 
	    if (j = (cdfs[fr][i]*HIST_HEIGHT)/max)
		XDrawLine(dpy,cdf_pixmaps[fr],
				gc,i,HIST_HEIGHT,i,HIST_HEIGHT-j);

        }

}


/*****************************
 * update_limit_lines()
 *****************************/

void update_limit_lines(n)
int n;
{
  int resln = gamma_limits[n].ulx-gamma_limits[n].llx+1;
  int ll = gamma_limits[n].llx+(gamma_limits[n].ll*resln)/256;
  int ul = gamma_limits[n].llx+(gamma_limits[n].ul*resln)/256;

    if (XtIsRealized(hist[n])) {
    	XDrawLine(dpy,XtWindow(hist[n]),llgc,ll,0,ll,HIST_HEIGHT-1);
    	XDrawLine(dpy,XtWindow(hist[n]),llgc,ul,0,ul,HIST_HEIGHT-1);
	}
}


/*****************************
 * update_limit_text()
 *****************************/

void update_limit_text(n)
int n;
{
  int resln = gamma_limits[n].ulx-gamma_limits[n].llx+1;
  int ll = gamma_limits[n].llx+(gamma_limits[n].ll*resln)/256;
  int ul = gamma_limits[n].llx+(gamma_limits[n].ul*resln)/256;
  char s[16];

    sprintf(s,"%d<>%d",ll,ul);
    XtVaSetValues(s_text[n],
		XtNstring,s,
		NULL);
}

