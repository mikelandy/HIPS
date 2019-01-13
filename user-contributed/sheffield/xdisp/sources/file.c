/* file handling routines */

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <Xol/DrawArea.h>
#include <stdio.h>
#include <time.h>
#include <hipl_format.h>
#include <xdisp.h>

struct extpar *grepparam();
extern Filename filename;

/******************
 * load_polys()
 ******************/

void load_polys(fname)
  char	*fname;
{
    int i;
    FILE *fp;
    int n,r,c;
    char s[64];
    char name[64];
    polygon_t *p;
    vertex_t *v;

    strcpy(def_file_name,fname);

    if ((fp = fopen(fname,"r")) == NULL)
	xdisp_error("can't open file ",fname,1);
    else {
	p = polygon_list;

        while (fscanf(fp,"%s",name) != EOF) {
            fscanf(fp,"%s",s);
	    if (strcmp(s,"i") != 0) {
	        xdisp_error("suspect polygon file format","",0);
		return;
		}
	    else {
    	        fscanf(fp,"%d",&n);
	        if (n%2 != 0) {
	            xdisp_error("suspect polygon file format","",0);
		    return;
		    }
	        else {
	    	    n/=2;
    	    	    for (i = 0; i < n; i++) {
	                fscanf(fp,"%d%d",&r,&c);
		        add_poly_vertex(r,c);
	                }
		    if (name[0] != '.') {
		    	current_polygon->name = (char *)malloc(strlen(name));
		    	strtok(name,".");
		    	strcpy(current_polygon->name,name);
			}
	            add_final_vertex();
		    }
	        }
	    }
	touch_polys();
	fclose(fp);
	}
}


/******************
 * save_polys()
 ******************/

void save_polys(fname)
  char	*fname;
{
    int i;
    FILE *fp;
    polygon_t *p;
    vertex_t *v;

    strcpy(def_file_name,fname);

    if ((fp = fopen(fname,"w")) == NULL)
	xdisp_error("can't open file ",fname,1);
    else {
	p = polygon_list;

	while (p != NULL) {
	    if (p->name == NULL)
	    	fprintf(fp,".poi\n");
	    else
	    	fprintf(fp,"%s.poi\n",p->name);
	    fprintf(fp,"i\n");
    	    fprintf(fp,"%d\n",p->nvertex*2);
	    v = p->vertices;
	    while (v != NULL) {
		fprintf(fp,"%d %d\n",v->r,v->c);
		v = v->next;
		}
	    p = p->next;
	    }
	fclose(fp);
	}
}



/******************
 * load_header()
 ******************/

void load_header(fname)
  char	*fname;
{
    int i;
    FILE *fp;
    polygon_t *p;
    vertex_t *v;
    int *pars;
    int ct;
    struct header hd;
    struct extpar *xp;

    strcpy(def_file_name,fname);

    if ((fp = fopen(fname,"r")) == NULL) 
	xdisp_error("can't open file ",fname,1);
    else {
	fclose(fp);
    	fp = hfopenr(fname);
	fread_header(fp,&hd,fname);

	while((xp = grepparam(&hd,".poi")) != (struct extpar *)NULLPAR) {
	    getparam(&hd,xp->name,PFINT,&ct,&pars);
	    for (i = 0; i < ct; i+= 2)
		add_poly_vertex(pars[i],pars[i+1]);
	    if (xp->name[0] != '.') {
		current_polygon->name = (char *)malloc(strlen(xp->name));
		strtok(xp->name,".");
		strcpy(current_polygon->name,xp->name);
			}
	    add_final_vertex();
	    clearparam(&hd,xp->name);
	    }
	free_header(&hd);
	fclose(fp);
	}	
}



/******************
 * save_header()
 ******************/

void save_header(fname)
  char	*fname;
{
    int i;
    int n;
    polygon_t *p;
    vertex_t *v;
    int *pars;
    FILE *fp;
    int pct = 0;
    char s[64];
    struct header hd;

    strcpy(def_file_name,fname);

    if ((fp = fopen(fname,"w")) == NULL)
	xdisp_error("can't open file ",fname,1);
    else {
	dup_headern(&hdr,&hd);

    	p = polygon_list;

    	while (p != NULL) {
	    n = 2*p->nvertex;
	    v = p->vertices;
	    pars = (int *)malloc(n*sizeof(int));
	    for (i = 0; i < n; i+= 2) {
	    	pars[i] = v->r;
	    	pars[i+1] = v->c;
		v = v->next;
	    	}
	    if (p->name == NULL)
	    	sprintf(s,".poi");
	    else
	    	sprintf(s,"%s.poi",p->name);
	    setparamd(&hd,s,PFINT,n,pars);
	    p = p->next;
	    }

        fwrite_header(fp,&hd,fname);
	free_header(&hd);
	fclose(fp);
	}
}



/******************
 * print_image()
 ******************/

void print_image_old(fname)
  char	*fname;
{
    int i,j;
    FILE *fp;
    unsigned char *im = image_pix_data.im;
    int mag = image_pix_data.mag;
    int or = image_pix_data.origin.r;
    int oc = image_pix_data.origin.c;
    lut_t *lut = current_lut;
    XColor c;
    Dimension width,height;
    polygon_t *p = polygon_list;
    vertex_t *v;
    XtVaGetValues(draw,			/* get image dimensions */
		XtNwidth,&width,
		XtNheight,&height,
		NULL);

    width/= mag;
    height/= mag;

    im = im+(or*ncols+oc);

    strcpy(def_file_name,fname);

    if ((fp = fopen(fname,"w")) == NULL)
	xdisp_error("can't open file ",fname,1);
    else {

	/* PostScript Header */

	fprintf(fp,"%%!PS-Adobe-3.0 EPSF-3.0\n");
	fprintf(fp,"%%%%BoundingBox: 0 0 %d %d\n",width*mag,height*mag);

	/* Image */

	fprintf(fp,"gsave\n");

	fprintf(fp,"/picstr %d string def\n",3*width);
	fprintf(fp,"0 0 translate\n");
	fprintf(fp,"%d %d scale\n",width*mag,height*mag);
	fprintf(fp,"%d %d 8\n",width,height);
	fprintf(fp,"[%d 0 0 %d 0 %d]\n",width,-height,height);
	fprintf(fp,"{currentfile\n");
	fprintf(fp,"  picstr readhexstring pop}\n");
	fprintf(fp,"false 3\n");
	fprintf(fp,"colorimage\n");

	for (i = 0; i < height; i++)
	    for (j = 0; j < width; j++) {
		c = colors[ymap[im[i*ncols+j]]];
		fprintf(fp,"%.2x%.2x%.2x",
				c.red/256,
				c.green/256,
				c.blue/256);
		}

	fprintf(fp,"\ngrestore\n");

	/* Polygons */

	while (p != NULL) {
    	    v = p->vertices;
	    if (p->name != NULL) {
		fprintf(fp,"%d %d moveto\n",v->c-oc,v->r-or);
		fprintf(fp,"(%s) show\n",p->name);
		}
	    fprintf(fp,"%d %d moveto\n",v->c-oc,v->r-or);
	    v = v->next;
    	    while (v != NULL) {
		if ((v->c >= oc && v->c < oc+width) ||
		    (v->r >= or && v->r < or+height)) {
	    		fprintf(fp,"%d %d lineto\n",v->c-oc,v->r-or);
			}
		v = v->next;
		}
	    fprintf(fp,"closepath\n");
	    fprintf(fp,"stroke\n");
	    p = p->next;
	    }

	/* Trailer */

	fprintf(fp,"\nshowpage\n");
	fclose(fp);
	}
}


/******************
 * print_image()
 ******************/

void print_image(fname)
  char	*fname;
{
    int i,j;
    FILE *fp;
    XColor c;
    int width,height;
    XImage *xi;
    XWindowAttributes watts;
    Status status;
    time_t tp;

    status = XGetWindowAttributes(dpy,XtWindow(draw),&watts);

    if (status == 0) {
	fprintf(stderr,"failed to get window attributes\n");
	return;
	}

    width = watts.width;
    height = watts.height;

    strcpy(def_file_name,fname);

    if ((fp = fopen(fname,"w")) == NULL)
	xdisp_error("can't open file ",fname,1);
    else {

    	xi = XGetImage(dpy,XtWindow(draw),0,0,width,height,AllPlanes,XYPixmap);

	/* PostScript Header */

	fprintf(fp,"%%!PS-Adobe-3.0 EPSF-3.0\n");
	fprintf(fp,"%%%%BoundingBox: 0 0 %d %d\n",width,height);
	fprintf(fp,"%%%%Creator: xdisp\n");
	fprintf(fp,"%%%%Title: %s frame %d\n",filename,frame);
	time(&tp);
	fprintf(fp,"%%%%CreationDate: %s\n",ctime(&tp));

	/* Image */

	fprintf(fp,"/picstr %d string def\n",3*width);
	fprintf(fp,"0 0 translate\n");
	fprintf(fp,"%d %d scale\n",width,height);
	fprintf(fp,"%d %d 8\n",width,height);
	fprintf(fp,"[%d 0 0 %d 0 %d]\n",width,-height,height);
	fprintf(fp,"{currentfile\n");
	fprintf(fp,"  picstr readhexstring pop}\n");
	fprintf(fp,"false 3\n");
	fprintf(fp,"colorimage\n");

	for (i = 0; i < height; i++) {
	    fprintf(fp,"\n");
	    for (j = 0; j < width; j++) {
		c = colors[XGetPixel(xi,j,i)];
		fprintf(fp,"%.2x%.2x%.2x",
				c.red/256,
				c.green/256,
				c.blue/256);
		}
	    }

	/* Trailer */

	fprintf(fp,"\nshowpage\n");
	fclose(fp);

    	XDestroyImage(xi);
	}
}

