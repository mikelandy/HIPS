/*********************************
 * statistical routines
 *********************************/

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <Xol/DrawArea.h>
#include <hipl_format.h>
#include <xdisp.h>
#include <math.h>

float f();
float f_i();
void calculate();

void do_stats()
{
  char s[512];

    if (polygon_list == NULL) {
	fprintf(stderr,"No polygons defined\n");
	return;
	}
    if (frame >= nframes) {
	fprintf(stderr,"No stats available for overlay frame\n");
	return;
	}

    calculate(s);
    fprintf(stderr,s);
}

void calculate(s)
char *s;
{
  int i,j;
  double m1,m2,xr,xi,sxr,sxi,x,meanr,meani;
  XRectangle rect;
  Region reg;
  polygon_t *p = polygon_list;
  vertex_t *v;
  XPoint *xp,*xpp;
  int np = 0;
  int area = 0;

    sxr = sxi = 0.0;
    m1 = m2 = 0.0;

    while (p != NULL) {
	np++;
	xp = xpp = (XPoint *)malloc(p->nvertex*sizeof(XPoint));
	v = p->vertices;
	while (v != NULL) {xp->x = v->c; xp->y = v->r; xp++; v = v->next;}
	reg = XPolygonRegion(xpp,p->nvertex,EvenOddRule);
	XClipBox(reg,&rect);
        for (i = 0; i <= rect.width; i++)
	    for (j = 0; j <= rect.height; j++)
	        if (XPointInRegion(reg,rect.x+i,rect.y+j)) {
		    area++;
		    xr = (double)f(frame,rect.y+j,rect.x+i);
		    xi = (double)f_i(frame,rect.y+j,rect.x+i);
		    x = (pixel_format == PFCOMPLEX) ? sqrt(xr*xr+xi*xi) : xr;
		    sxr+= xr;
		    sxi+= xi;
		    m1+= x;
		    m2+= x*x;
		    }
	free(xpp);
	p = p->next;
	}

    s+= strlen(sprintf(s,"No of polys = %d\n",np));
    s+= strlen(sprintf(s,"Area = %d/%d\n",area,nrows*ncols));

    meanr = sxr/(double)area;
    meani = sxi/(double)area;

    if (pixel_format == PFCOMPLEX) {
	s+= strlen(sprintf(s,"Mean = "));
	convert_complex((float)meanr,(float)meani,s);
	s+= strlen(s);
	s+= strlen(sprintf(s,"\n"));
	}
    else
	s+= strlen(sprintf(s,"Mean = %.*f\n",precision,(float)meanr));

    s+= strlen(sprintf(s,"2nd moment = %.*f\n",
			precision,(float)(m2/(double)area)));
    s+= strlen(sprintf(s,"2nd moment about mean = %.*f\n",precision,
		(float)((m2-2.0*sxr*meanr-2.0*sxi*meani+
			(meanr*meanr+meani*meani)*(double)area)/(double)area)));
}
